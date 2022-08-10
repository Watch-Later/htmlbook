#include "document.h"
#include "htmlparser.h"
#include "htmlelement.h"
#include "cssstylesheet.h"
#include "cssparser.h"
#include "resource.h"

namespace htmlbook {

Node::Node(Document* document)
    : m_document(document)
{
}

Node::~Node()
{
    if(m_parentNode)
        m_parentNode->removeChild(this);
    assert(m_box == nullptr);
}

void Node::reparent(ContainerNode* newParent)
{
    if(m_parentNode)
        m_parentNode->removeChild(this);
    newParent->appendChild(this);
}

TextNode::TextNode(Document* document, std::string data)
    : Node(document), m_data(std::move(data))
{
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

void ContainerNode::insertBefore(Node* child, Node* nextChild)
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
    for(auto& attribute : m_attributes) {
        if(!isPresentationAttribute(attribute.name()))
            continue;
        value += attribute.name();
        value += ':';
        value += attribute.value();
        value += ';';
    }

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
        return;
    }

    o << '>';
    ContainerNode::serialize(o);
    o << '<';
    o << '/';
    o << m_tagName;
    o << '>';
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
    if(m_authorStyleSheet == nullptr)
        m_authorStyleSheet = CSSStyleSheet::create(this);
    m_authorStyleSheet->parse(content);
}

void Document::setUserStyleSheet(const std::string_view& content)
{
    m_userStyleSheet = CSSStyleSheet::create(this);
    m_userStyleSheet->parse(content);
}

void Document::clearUserStyleSheet()
{
    m_userStyleSheet.reset();
}

RefPtr<FontFace> FontFaceCache::get(const std::string& family, bool italic, bool smallCaps, int weight) const
{
    return nullptr;
}

void FontFaceCache::add(const std::string& family, bool italic, bool smallCaps, int weight, RefPtr<FontFace> face)
{
    auto& faces = m_fontFaceDataMap[family];
    faces.emplace_back(italic, smallCaps, weight, std::move(face));
}

void FontFaceCache::clear()
{
    m_fontFaceDataMap.clear();
}

RefPtr<FontFace> Document::fetchFont(const std::string& family, bool italic, bool smallCaps, int weight) const
{
    if(auto face = fontCache()->getFace(family, italic, smallCaps, weight))
        return face;
    if(m_client == nullptr)
        return nullptr;
    std::vector<char> data;
    if(!m_client->loadFont(family, italic, smallCaps, weight, data))
        return nullptr;
    auto face = FontFace::create(std::move(data));
    if(face == nullptr)
        return nullptr;
    fontCache()->addFace(family, italic, smallCaps, weight, face);
    return face;
}

RefPtr<FontFace> Document::getFontFace(const std::string& family, bool italic, bool smallCaps, int weight) const
{
    if(auto face = m_fontFaceCache.get(family, italic, smallCaps, weight))
        return face;
    return fetchFont(family, italic, smallCaps, weight);
}

void Document::addFontFace(const std::string& family, bool italic, bool smallCaps, int weight, RefPtr<FontFace> face)
{
    m_fontFaceCache.add(family, italic, smallCaps, weight, std::move(face));
}

bool Document::fetchUrl(const Url& url, std::string& mimeType, std::string& textEncoding, std::vector<char>& data) const
{
    if(url.protocolIs("data"))
        return url.decodeData(mimeType, textEncoding, data);
    if(m_client == nullptr)
        return false;
    return m_client->loadUrl(url.value(), mimeType, textEncoding, data);
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
    if(!fetchUrl(completeUrl, mimeType, textEncoding, data))
        return nullptr;
    auto resource = ResourceType::create(mimeType, textEncoding, std::move(data));
    m_resourceCache.emplace(completeUrl.value(), resource);
    return resource;
}

} // namespace htmlbook
