#include "document.h"
#include "htmldocument.h"
#include "cssparser.h"
#include "resource.h"
#include "box.h"

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

Heap* Node::heap() const
{
    return m_document->heap();
}

TextNode::TextNode(Document* document, const HeapString& data)
    : Node(document), m_data(data)
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
    : ContainerNode(document)
    , m_tagName(tagName)
    , m_namespaceUri(namespaceUri)
    , m_classNames(document->heap())
    , m_attributes(document->heap())
{
}

const HeapString& Element::lang() const
{
    return getAttribute(langAttr);
}

const Attribute* Element::findAttribute(const GlobalString& name) const
{
    for(auto& attribute : m_attributes) {
        if(name == attribute.name()) {
            return &attribute;
        }
    }

    return nullptr;
}

bool Element::hasAttribute(const GlobalString& name) const
{
    for(auto& attribute : m_attributes) {
        if(name == attribute.name()) {
            return true;
        }
    }

    return false;
}

const HeapString& Element::getAttribute(const GlobalString& name) const
{
    for(auto& attribute : m_attributes) {
        if(name == attribute.name()) {
            return attribute.value();
        }
    }

    return emptyGlo;
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

void Element::setAttribute(const GlobalString& name, const HeapString& value)
{
    parseAttribute(name, value);
    for(auto& attribute : m_attributes) {
        if(name == attribute.name()) {
            attribute.setValue(value);
            return;
        }
    }

    m_attributes.emplace_back(name, value);
}

void Element::removeAttribute(const GlobalString& name)
{
    parseAttribute(name, emptyGlo);
    auto it = m_attributes.begin();
    auto end = m_attributes.end();
    for(; it != end; ++it) {
        if(name == it->name()) {
            m_attributes.erase(it);
            return;
        }
    }
}

void Element::parseAttribute(const GlobalString& name, const HeapString& value)
{
    if(name == idAttr) {
        m_id = value;
    } else if(name == classAttr) {
        m_classNames.clear();
        if(value.empty())
            return;
        size_t begin = 0;
        while(true) {
            while(begin < value.length() && isspace(value[begin]))
                ++begin;
            if(begin >= value.length())
                break;
            size_t end = begin + 1;
            while(end < value.length() && !isspace(value[end]))
                ++end;
            m_classNames.push_back(value.substring(begin, end - begin));
            begin = end + 1;
        }
    }
}

CSSPropertyList Element::inlineStyle() const
{
    auto value = getAttribute(styleAttr);
    if(value.empty())
        return CSSPropertyList{};

    CSSPropertyList properties(heap());
    CSSParser parser(heap());
    parser.parseStyle(properties, value);
    return properties;
}

CSSPropertyList Element::presentationAttributeStyle() const
{
    std::stringstream output;
    for(auto& attribute : attributes()) {
        collectAttributeStyle(output, attribute.name(), attribute.value());
        output << ';';
    }

    CSSPropertyList properties(heap());
    CSSParser parser(heap());
    parser.parseStyle(properties, output.str());
    return properties;
}

Element* Element::parentElement() const
{
    return to<Element>(parentNode());
}

Element* Element::previousElement() const
{
    auto node = previousSibling();
    while(node) {
        if(auto element = to<Element>(node))
            return element;
        node = node->previousSibling();
    }

    return nullptr;
}

Element* Element::nextElement() const
{
    auto node = nextSibling();
    while(node) {
        if(auto element = to<Element>(node))
            return element;
        node = node->nextSibling();
    }

    return nullptr;
}

void Element::serialize(std::ostream& o) const
{
    o << '<';
    o << m_tagName;
    for(auto& attribute : attributes()) {
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

Document::Document()
    : ContainerNode(nullptr)
    , m_idCache(heap())
    , m_resourceCache(heap())
{
}

TextNode* Document::createText(const std::string_view& value)
{
    return new TextNode(this, m_heap.createString(value));
}

Element* Document::createElement(const GlobalString& tagName, const GlobalString& namespaceUri)
{
    if(namespaceUri == namespaceuri::xhtml) {
        if(tagName == bodyTag)
            return new HTMLBodyElement(this);
        if(tagName == imageTag)
            return new HTMLImageElement(this);
        if(tagName == fontTag)
            return new HTMLFontElement(this);
        if(tagName == hrTag)
            return new HTMLHRElement(this);
        if(tagName == liTag)
            return new HTMLLIElement(this);
        if(tagName == olTag)
            return new HTMLOLElement(this);
        if(tagName == tableTag)
            return new HTMLTableElement(this);
        if(tagName == theadTag || tagName == tbodyTag || tagName == tfootTag)
            return new HTMLTableSectionElement(this, tagName);
        if(tagName == captionTag)
            return new HTMLTableCaptionElement(this);
        if(tagName == trTag)
            return new HTMLTableRowElement(this);
        if(tagName == colTag || tagName == colgroupTag)
            return new HTMLTableColElement(this, tagName);
        if(tagName == tdTag || tagName == thTag)
            return new HTMLTableCellElement(this, tagName);
        if(tagName == styleTag)
            return new HTMLStyleElement(this);
        if(tagName == linkTag)
            return new HTMLLinkElement(this);
        return new HTMLElement(this, tagName);
    }

    return new Element(this, tagName, namespaceUri);
}

void Document::addAuthorStyleSheet(const std::string_view& content)
{
    assert(m_ruleCache == nullptr);
    CSSParser parser(heap());
    parser.parseSheet(m_authorRules, content);
}

void Document::addUserStyleSheet(const std::string_view& content)
{
    assert(m_ruleCache == nullptr);
    CSSParser parser(heap());
    parser.parseSheet(m_userRules, content);
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

RefPtr<FontFace> Document::getFontFace(const std::string_view& family, bool italic, bool smallCaps, int weight)
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

void Document::buildBox(Counters& counters, Box* parent)
{
    assert(parent == nullptr);
    auto style = BoxStyle::create(this, PseudoType::None);
    style->set(CSSPropertyID::Display, CSSIdentValue::create(CSSValueID::Block));
    style->set(CSSPropertyID::Position, CSSIdentValue::create(CSSValueID::Absolute));
    style->set(CSSPropertyID::ZIndex, CSSIntegerValue::create(0));

    auto box = createBox(style);
    ContainerNode::buildBox(counters, box);
    box->buildBox(nullptr);
}

template<typename ResourceType>
RefPtr<ResourceType> Document::fetchResource(const std::string_view& url)
{
    auto completeUrl = m_baseUrl.complete(url);
    if(completeUrl.empty())
        return nullptr;
    auto it = m_resourceCache.find(completeUrl);
    if(it != m_resourceCache.end())
        return to<ResourceType>(it->second.get());
    std::string mimeType;
    std::string textEncoding;
    std::vector<char> data;
    if(!resourceLoader()->loadUrl(completeUrl, mimeType, textEncoding, data))
        return nullptr;
    auto resource = ResourceType::create(mimeType, textEncoding, std::move(data));
    m_resourceCache.emplace(std::move(completeUrl), resource);
    return resource;
}

} // namespace htmlbook
