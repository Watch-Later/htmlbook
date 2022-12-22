#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "cssrule.h"
#include "url.h"

#include <cassert>
#include <sstream>

namespace htmlbook {

class ContainerNode;
class Document;
class Box;
class BoxStyle;
class Counters;

class Node : public HeapMember {
public:
    Node(Document* document);

    virtual ~Node();
    virtual bool isTextNode() const { return false; }
    virtual bool isContainerNode() const { return false; }
    virtual bool isElementNode() const { return false; }
    virtual bool isDocumentNode() const { return false; }

    const GlobalString& tagName() const;
    const GlobalString& namespaceUri() const;

    void reparent(ContainerNode* newParent);

    Heap* heap() const;
    Document* document() const { return m_document; }
    ContainerNode* parentNode() const { return m_parentNode; }
    Node* nextSibling() const { return m_nextSibling; }
    Node* previousSibling() const { return m_previousSibling; }

    void setParentNode(ContainerNode* parentNode) { m_parentNode = parentNode; }
    void setNextSibling(Node* nextSibling) { m_nextSibling = nextSibling; }
    void setPreviousSibling(Node* previousSibling) { m_previousSibling = previousSibling; }

    void setBox(Box* box) { m_box = box; }
    Box* box() const { return m_box; }
    RefPtr<BoxStyle> style() const;

    virtual Box* createBox(const RefPtr<BoxStyle>& style) { return nullptr; }
    virtual void buildBox(Counters& counters, Box* parent) {}
    virtual void serialize(std::ostream& o) const = 0;

private:
    Document* m_document;
    ContainerNode* m_parentNode{nullptr};
    Node* m_nextSibling{nullptr};
    Node* m_previousSibling{nullptr};
    Box* m_box{nullptr};
};

class TextNode final : public Node {
public:
    TextNode(Document* document, const HeapString& data);

    bool isTextNode() const final { return true; }

    const HeapString& data() const { return m_data; }
    void setData(const HeapString& data) { m_data = data; }

    Box* createBox(const RefPtr<BoxStyle>& style) final;
    void buildBox(Counters& counters, Box* parent) final;
    void serialize(std::ostream& o) const final;

private:
    HeapString m_data;
};

template<>
struct is<TextNode> {
    static bool check(const Node& value) { return value.isTextNode(); }
};

class ContainerNode : public Node {
public:
    ContainerNode(Document* document);
    ~ContainerNode() override;

    virtual void beginParsingChildren() {}
    virtual void finishParsingChildren() {}

    bool isContainerNode() const final { return true; }

    Node* firstChild() const { return m_firstChild; }
    Node* lastChild() const { return m_lastChild; }

    void setFirstChild(Node* child) { m_firstChild = child; }
    void setLastChild(Node* child) { m_lastChild = child; }

    void appendChild(Node* child);
    void insertChild(Node* child, Node* nextChild);
    void removeChild(Node* child);
    void reparentChildren(ContainerNode* newParent);

    void buildBox(Counters& counters, Box* parent) override;
    void serialize(std::ostream& o) const override;

private:
    Node* m_firstChild{nullptr};
    Node* m_lastChild{nullptr};
};

template<>
struct is<ContainerNode> {
    static bool check(const Node& value) { return value.isContainerNode(); }
};

class Attribute {
public:
    Attribute() = default;
    Attribute(const GlobalString& name, const HeapString& value)
        : m_name(name), m_value(value)
    {}

    const GlobalString& name() const { return m_name; }
    void setName(const GlobalString& name) { m_name = name; }

    const HeapString& value() const { return m_value; }
    void setValue(const HeapString& value) { m_value = value; }

    bool empty() const { return m_value.empty(); }

private:
    GlobalString m_name;
    HeapString m_value;
};

inline bool operator==(const Attribute& a, const Attribute& b) { return a.name() == b.name() && a.value() == b.value(); }
inline bool operator!=(const Attribute& a, const Attribute& b) { return a.name() != b.name() || a.value() != b.value(); }

using AttributeList = std::pmr::list<Attribute>;
using ClassNameList = std::pmr::list<HeapString>;

class Element : public ContainerNode {
public:
    Element(Document* document, const GlobalString& tagName, const GlobalString& namespaceUri);

    bool isElementNode() const final { return true; }

    const GlobalString& tagName() const { return m_tagName; }
    const GlobalString& namespaceUri() const { return m_namespaceUri; }
    const AttributeList& attributes() const { return m_attributes; }

    const HeapString& lang() const;
    const HeapString& id() const { return m_id; }
    const ClassNameList& classNames() const { return m_classNames; }

    const Attribute* findAttribute(const GlobalString& name) const;
    bool hasAttribute(const GlobalString& name) const;
    const HeapString& getAttribute(const GlobalString& name) const;
    void setAttributeList(const AttributeList& attributes);
    void setAttribute(const Attribute& attribute);
    void setAttribute(const GlobalString& name, const HeapString& value);
    void removeAttribute(const GlobalString& name);
    virtual void parseAttribute(const GlobalString& name, const HeapString& value);
    virtual void collectAttributeStyle(std::stringstream& output, const GlobalString& name, const HeapString& value) const {}

    CSSPropertyList inlineStyle() const;
    CSSPropertyList presentationAttributeStyle() const;

    Element* parentElement() const;
    Element* previousElement() const;
    Element* nextElement() const;

    void serialize(std::ostream& o) const override;

private:
    GlobalString m_tagName;
    GlobalString m_namespaceUri;
    HeapString m_id;
    ClassNameList m_classNames;
    AttributeList m_attributes;
};

template<>
struct is<Element> {
    static bool check(const Node& value) { return value.isElementNode(); }
};

inline const GlobalString& Node::tagName() const
{
    if(auto element = to<Element>(this))
        return element->tagName();
    return emptyGlo;
}

inline const GlobalString& Node::namespaceUri() const
{
    if(auto element = to<Element>(this))
        return element->namespaceUri();
    return emptyGlo;
}

class Resource;
class TextResource;
class ImageResource;
class FontResource;
class FontFace;

class Document : public ContainerNode {
public:
    Document(Heap* heap);

    bool isDocumentNode() const final { return true; }

    Heap* heap() const { return m_heap; }
    TextNode* createTextNode(const std::string_view& value);
    Element* createElement(const GlobalString& tagName, const GlobalString& namespaceUri);

    const Url& baseUrl() const { return m_baseUrl; }
    void setBaseUrl(Url value) { m_baseUrl = std::move(value); }

    virtual bool load(const std::string_view& content) = 0;

    void addStyleSheet(const std::string_view& content);

    RefPtr<BoxStyle> styleForElement(Element* element, const RefPtr<BoxStyle>& parentStyle);
    RefPtr<BoxStyle> pseudoStyleForElement(Element* element, const RefPtr<BoxStyle>& parentStyle, PseudoType pseudoType);
    RefPtr<FontFace> getFontFace(const std::string_view& family, bool italic, bool smallCaps, int weight);

    RefPtr<TextResource> fetchTextResource(const std::string_view& url);
    RefPtr<ImageResource> fetchImageResource(const std::string_view& url);
    RefPtr<FontResource> fetchFontResource(const std::string_view& url);

    Element* rootElement() const;
    RefPtr<BoxStyle> rootStyle() const;
    const CSSStyleSheet& styleSheet() const { return m_styleSheet; }

    float viewportWidth() const;
    float viewportHeight() const;

    void buildBox(Counters& counters, Box* parent) override;
    void build();

private:
    template<typename ResourceType>
    RefPtr<ResourceType> fetchResource(const std::string_view& url);
    Heap* m_heap;
    Url m_baseUrl;
    std::pmr::map<HeapString, Element*> m_idCache;
    std::pmr::map<Url, RefPtr<Resource>> m_resourceCache;
    CSSStyleSheet m_styleSheet;
};

template<>
struct is<Document> {
    static bool check(const Node& value) { return value.isDocumentNode(); }
};

} // namespace htmlbook

#endif // DOCUMENT_H
