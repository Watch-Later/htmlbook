#include "document.h"
#include "htmlparser.h"
#include "htmlelement.h"
#include "cssparser.h"
#include "resource.h"
#include "boxstyle.h"
#include "box.h"
#include "counter.h"

namespace htmlbook {

Node::Node(Document* document)
    : m_document(document)
{
}

Node::~Node()
{
    if(m_parentNode)
        m_parentNode->removeChild(this);
    delete m_box;
}

void Node::reparent(ContainerNode* newParent)
{
    if(m_parentNode)
        m_parentNode->removeChild(this);
    newParent->appendChild(this);
}

RefPtr<BoxStyle> Node::style() const
{
    return m_box ? m_box->style() : nullptr;
}

TextNode::TextNode(Document* document, std::string data)
    : Node(document), m_data(std::move(data))
{
}

Box* TextNode::createBox(const RefPtr<BoxStyle>& style)
{
    if(m_data.empty())
        return nullptr;
    auto box = new TextBox(this, style);
    box->setText(m_data);
    return box;
}

void TextNode::buildBox(Counters& counters, Box* parent)
{
    auto box = createBox(parent->style());
    if(box == nullptr)
        return;
    parent->addBox(box);
}

void TextNode::serialize(std::ostream& o) const
{
    o << m_data;
}

ContainerNode::ContainerNode(Document* document)
    : Node(document)
{
}

ContainerNode::~ContainerNode()
{
    auto child = m_firstChild;
    while(child) {
        Node* next = child->nextSibling();
        child->setParentNode(nullptr);
        child->setPreviousSibling(nullptr);
        child->setNextSibling(nullptr);
        delete child;
        child = next;
    }
}

void ContainerNode::appendChild(Node* child)
{
    assert(child->parentNode() == nullptr);
    assert(child->previousSibling() == nullptr);
    assert(child->nextSibling() == nullptr);
    child->setParentNode(this);
    if(m_firstChild == nullptr) {
        m_firstChild = m_lastChild = child;
        return;
    }

    child->setPreviousSibling(m_lastChild);
    m_lastChild->setNextSibling(child);
    m_lastChild = child;
}

void ContainerNode::insertChild(Node* child, Node* nextChild)
{
    if(nextChild == nullptr) {
        appendChild(child);
        return;
    }

    assert(nextChild->parentNode() == this);
    assert(child->parentNode() == nullptr);
    assert(child->previousSibling() == nullptr);
    assert(child->nextSibling() == nullptr);

    auto previousChild = nextChild->previousSibling();
    nextChild->setPreviousSibling(child);
    assert(m_lastChild != previousChild);
    if(previousChild == nullptr) {
        assert(m_firstChild == nextChild);
        m_firstChild = child;
    } else {
        assert(m_firstChild != nextChild);
        previousChild->setNextSibling(child);
    }

    child->setParentNode(this);
    child->setPreviousSibling(previousChild);
    child->setNextSibling(nextChild);
}

void ContainerNode::removeChild(Node* child)
{
    assert(child->parentNode() == this);
    auto nextChild = child->nextSibling();
    auto previousChild = child->previousSibling();
    if(nextChild)
        nextChild->setPreviousSibling(previousChild);
    if(previousChild)
        previousChild->setNextSibling(nextChild);

    if(m_firstChild == child)
        m_firstChild = nextChild;
    if(m_lastChild == child)
        m_lastChild = previousChild;

    child->setParentNode(nullptr);
    child->setPreviousSibling(nullptr);
    child->setNextSibling(nullptr);
}

void ContainerNode::reparentChildren(ContainerNode* newParent)
{
    while(auto child = firstChild()) {
        child->reparent(newParent);
    }
}

void ContainerNode::buildBox(Counters& counters, Box* parent)
{
    if(parent->children() == nullptr)
        return;
    auto child = m_firstChild;
    while(child) {
        child->buildBox(counters, parent);
        child = child->nextSibling();
    }
}

void ContainerNode::serialize(std::ostream& o) const
{
    auto child = m_firstChild;
    while(child) {
        child->serialize(o);
        child = child->nextSibling();
    }
}

Element::Element(Document* document, const GlobalString& tagName, const GlobalString& namespaceUri)
    : ContainerNode(document), m_tagName(tagName), m_namespaceUri(namespaceUri)
{
}

void Element::setId(const std::string_view& value)
{
    m_id = value;
    document()->updateIdCache(m_id, this);
}

void Element::setClass(const std::string_view& value)
{
    m_classNames.clear();
    if(value.empty())
        return;

    auto it = value.begin();
    auto end = value.end();
    while(true) {
        while(it != end && isspace(*it))
            ++it;
        if(it == end)
            break;

        size_t count = 0;
        auto begin = it;
        while(it != end && !isspace(*it)) {
            ++count;
            ++it;
        }

        std::string_view name(begin, count);
        assert(!name.empty());
        m_classNames.emplace_back(name);
    }
}

Attribute* Element::findAttribute(const GlobalString& name) const
{
    for(auto& attribute : m_attributes) {
        if(name == attribute.name())
            return &attribute;
    }

    return nullptr;
}

bool Element::hasAttribute(const GlobalString& name) const
{
    return findAttribute(name);
}

std::string_view Element::getAttribute(const GlobalString& name) const
{
    auto attribute = findAttribute(name);
    if(attribute == nullptr)
        return nullString;
    return attribute->value();
}

void Element::setAttributeList(const AttributeList& attributes)
{
    for(auto& attribute : attributes)
        setAttribute(attribute);
}

void Element::setAttribute(const Attribute& attribute)
{
    setAttribute(attribute.name(), attribute.value());
}

void Element::setAttribute(const GlobalString& name, std::string value)
{
    parseAttribute(name, value);
    if(auto attribute = findAttribute(name)) {
        attribute->setValue(std::move(value));
    } else {
        m_attributes.emplace_back(name, std::move(value));
    }
}

void Element::removeAttribute(const GlobalString& name)
{
    auto it = m_attributes.begin();
    auto end = m_attributes.end();
    for(; it != end; ++it) {
        if(name == it->name()) {
            parseAttribute(name, nullString);
            m_attributes.erase(it);
            break;
        }
    }
}

void Element::parseAttribute(const GlobalString& name, const std::string_view& value)
{
    if(name == htmlnames::idAttr) {
        setId(value);
    } else if(name == htmlnames::classAttr) {
        setClass(value);
    }
}

CSSPropertyList Element::inlineStyle() const
{
    auto value = getAttribute(htmlnames::styleAttr);
    if(value.empty())
        return CSSPropertyList{};

    CSSPropertyList properties;
    CSSParser::parseStyle(properties, value);
    return properties;
}

CSSPropertyList Element::presentationAttributeStyle() const
{
    std::string value;
    collectPresentationAttributeStyle(value);
    if(value.empty())
        return CSSPropertyList{};

    CSSPropertyList properties;
    CSSParser::parseStyle(properties, value);
    return properties;
}

std::string_view Element::lang() const
{
    return getAttribute(htmlnames::langAttr);
}

Element* Element::parentElement() const
{
    auto parent = parentNode();
    if(parent->isElementNode())
        return to<Element>(parent);
    return nullptr;
}

Element* Element::previousElement() const
{
    auto previousChild = previousSibling();
    while(previousChild) {
        if(previousChild->isElementNode())
            return to<Element>(previousChild);
        previousChild = previousChild->previousSibling();
    }

    return nullptr;
}

Element* Element::nextElement() const
{
    auto nextChild = nextSibling();
    while(nextChild) {
        if(nextChild->isElementNode())
            return to<Element>(nextChild);
        nextChild = nextChild->nextSibling();
    }

    return nullptr;
}

Box* Element::createBox(const RefPtr<BoxStyle>& style)
{
    return Box::create(this, style);
}

void Element::buildPseudoBox(Counters& counters, Box* parent, PseudoType pseudoType)
{
    if(pseudoType == PseudoType::Marker && !parent->isListItemBox())
        return;
    auto style = document()->pseudoStyleForElement(this, *parent->style(), pseudoType);
    if(style == nullptr || style->display() == Display::None)
        return;

    auto box = Box::create(nullptr, style);
    parent->addBox(box);
    if(pseudoType == PseudoType::Before || pseudoType == PseudoType::After) {
        counters.update(*style);
        buildPseudoBox(counters, box, PseudoType::Marker);
    }

    auto addText = [&](const auto& text) {
        if(text.empty())
            return;
        auto lastBox = box->lastBox();
        if(lastBox && lastBox->isTextBox()) {
            auto textBox = to<TextBox>(lastBox);
            textBox->appendText(text);
            return;
        }

        auto textBox = new TextBox(nullptr, style);
        textBox->setText(text);
        box->addBox(textBox);
    };

    auto addImage = [&](const auto& image) {
        if(image == nullptr)
            return;
        auto imageBox = new ImageBox(nullptr, style);
        imageBox->setImage(image);
        box->addBox(imageBox);
    };

    auto content = style->get(CSSPropertyID::Content);
    if(content == nullptr || !content->isListValue()) {
        if(pseudoType == PseudoType::Marker)
            return;
        if(auto image = style->listStyleImage()) {
            addImage(image);
            return;
        }

        static const GlobalString listItem("list-item");
        addText(counters.format(listItem, style->listStyleType(), emptyString));
        return;
    }

    for(auto& value : to<CSSListValue>(*content)->values()) {
        if(auto string = to<CSSStringValue>(*value)) {
            addText(string->value());
        } else if(auto image = to<CSSImageValue>(*value)) {
            addImage(image->fetch(document()));
        } else if(auto counter = to<CSSCounterValue>(*value)) {
            addText(counters.format(counter->identifier(), counter->listStyle(), counter->seperator()));
        } else if(auto ident = to<CSSIdentValue>(*value)) {
            auto usequote = (ident->value() == CSSValueID::OpenQuote || ident->value() == CSSValueID::CloseQuote);
            auto openquote = (ident->value() == CSSValueID::OpenQuote || ident->value() == CSSValueID::NoOpenQuote);
            if(counters.quoteDepth() > 0 && !openquote)
                counters.decreaseQuoteDepth();
            if(usequote)
                addText(style->getQuote(openquote, counters.quoteDepth()));
            if(openquote)
                counters.increaseQuoteDepth();
        } else {
            auto function = to<CSSFunctionValue>(*value);
            auto name = to<CSSCustomIdentValue>(*function->front());
            auto attribute = findAttribute(name->value());
            if(attribute == nullptr)
                continue;
            addText(attribute->value());
        }
    }
}

void Element::buildBox(Counters& counters, Box* parent)
{
    auto style = document()->styleForElement(this, *parent->style());
    if(style == nullptr || style->display() == Display::None)
        return;
    auto box = createBox(style);
    if(box == nullptr)
        return;
    parent->addBox(box);
    counters.push();
    counters.update(*style);
    buildPseudoBox(counters, box, PseudoType::Marker);
    buildPseudoBox(counters, box, PseudoType::Before);
    ContainerNode::buildBox(counters, box);
    buildPseudoBox(counters, box, PseudoType::After);
    counters.pop();
}

void Element::serialize(std::ostream& o) const
{
    o << '<';
    o << m_tagName;
    for(auto& attribute : m_attributes) {
        o << ' ';
        o << attribute.name();
        o << '=';
        o << '"';
        o << attribute.value();
        o << '"';
    }

    if(firstChild() == nullptr) {
        o << '/';
        o << '>';
    } else {
        o << '>';
        ContainerNode::serialize(o);
        o << '<';
        o << '/';
        o << m_tagName;
        o << '>';
    }
}

Document::Document(const PageSize& pageSize)
    : ContainerNode(nullptr), m_pageSize(pageSize)
{
}

Element* Document::createElement(const GlobalString& tagName, const GlobalString& namespaceUri)
{
    if(namespaceUri == namespaceuri::xhtml)
        return new HTMLElement(this, tagName);
    return new Element(this, tagName, namespaceUri);
}

void Document::load(const std::string_view& content)
{
    HTMLParser parser(this, content);
    parser.parse();
}

void Document::updateIdCache(const GlobalString& name, Element* element)
{
    m_idCache.emplace(name, element);
}

void Document::addAuthorStyleSheet(const std::string_view& content)
{
    CSSParser::parseSheet(m_authorRules, content);
    m_ruleCache.reset();
}

void Document::addUserStyleSheet(const std::string_view& content)
{
    CSSParser::parseSheet(m_userRules, content);
    m_ruleCache.reset();
}

void Document::clearUserStyleSheet()
{
    m_userRules.clear();
    m_ruleCache.reset();
}

const CSSRuleCache* Document::ruleCache()
{
    if(m_ruleCache == nullptr)
        m_ruleCache = CSSRuleCache::create(this);
    return m_ruleCache.get();
}

RefPtr<BoxStyle> Document::styleForElement(Element* element, const BoxStyle& parentStyle)
{
    return ruleCache()->styleForElement(element, parentStyle);
}

RefPtr<BoxStyle> Document::pseudoStyleForElement(Element* element, const BoxStyle& parentStyle, PseudoType pseudoType)
{
    return ruleCache()->pseudoStyleForElement(element, parentStyle, pseudoType);
}

RefPtr<FontFace> Document::getFontFace(const std::string& family, bool italic, bool smallCaps, int weight)
{
    return ruleCache()->getFontFace(family, italic, smallCaps, weight);
}

RefPtr<TextResource> Document::fetchTextResource(const std::string_view& url)
{
    return fetchResource<TextResource>(url);
}

RefPtr<ImageResource> Document::fetchImageResource(const std::string_view& url)
{
    return fetchResource<ImageResource>(url);
}

RefPtr<FontResource> Document::fetchFontResource(const std::string_view& url)
{
    return fetchResource<FontResource>(url);
}

Element* Document::rootElement() const
{
    return nullptr;
}

RefPtr<BoxStyle> Document::rootStyle() const
{
    if(auto element = rootElement())
        return element->style();
    return style();
}

float Document::viewportWidth() const
{
    return 0.0;
}

float Document::viewportHeight() const
{
    return 0.0;
}

Box* Document::createBox(const RefPtr<BoxStyle>& style)
{
    return new BlockBox(this, style);
}

void Document::buildBox(Counters& counters, Box* parent)
{
    assert(parent == nullptr);
    auto style = BoxStyle::create(this, PseudoType::None);
    style->set(CSSPropertyID::Display, CSSIdentValue::create(CSSValueID::Block));
    style->set(CSSPropertyID::Position, CSSIdentValue::create(CSSValueID::Absolute));
    style->set(CSSPropertyID::ZIndex, CSSIntegerValue::create(0));

    auto box = createBox(style);
    ContainerNode::buildBox(counters, box);
    box->build(nullptr);
}

template<typename ResourceType>
RefPtr<ResourceType> Document::fetchResource(const std::string_view& url)
{
    auto completeUrl = m_baseUrl.complete(url);
    if(completeUrl.empty())
        return nullptr;
    auto it = m_resourceCache.find(completeUrl.value());
    if(it != m_resourceCache.end())
        return to<ResourceType>(it->second.get());
    std::string mimeType;
    std::string textEncoding;
    std::vector<char> data;
    if(!resourceLoader()->loadUrl(completeUrl, mimeType, textEncoding, data))
        return nullptr;
    auto resource = ResourceType::create(mimeType, textEncoding, std::move(data));
    m_resourceCache.emplace(completeUrl.value(), resource);
    return resource;
}

} // namespace htmlbook
