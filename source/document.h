#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "cssstylesheet.h"
#include "url.h"

#include <cassert>

namespace htmlbook {

class ContainerNode;
class Document;
class Box;

class Node {
public:
    Node(Document* document);

    virtual ~Node();
    virtual bool isTextNode() const { return false; }
    virtual bool isContainerNode() const { return false; }
    virtual bool isElementNode() const { return false; }

    const GlobalString& tagName() const;
    const GlobalString& namespaceUri() const;

    void reparent(ContainerNode* newParent);

    Document* document() const { return m_document; }
    ContainerNode* parentNode() const { return m_parentNode; }
    Node* nextSibling() const { return m_nextSibling; }
    Node* previousSibling() const { return m_previousSibling; }
    Box* box() const { return m_box; }

    void setParentNode(ContainerNode* parentNode) { m_parentNode = parentNode; }
    void setNextSibling(Node* nextSibling) { m_nextSibling = nextSibling; }
    void setPreviousSibling(Node* previousSibling) { m_previousSibling = previousSibling; }
    void setBox(Box* box) { m_box = box; }

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
    TextNode(Document* document, std::string data);

    bool isTextNode() const final { return true; }

    const std::string& data() const { return m_data; }
    void setData(std::string data) { m_data = std::move(data); }
    void appendData(const std::string& data) { m_data += data; }
    void clearData() { m_data.clear(); }

    void serialize(std::ostream& o) const final;

private:
    std::string m_data;
};

template<>
struct is<TextNode> {
    static bool check(const Node& value) { return value.isTextNode(); }
};

class ContainerNode : public Node {
public:
    ContainerNode(Document* document);

    virtual ~ContainerNode();
    virtual void beginParsingChildern() {}
    virtual void finishParsingChildern() {}

    bool isContainerNode() const final { return true; }

    Node* firstChild() const { return m_firstChild; }
    Node* lastChild() const { return m_lastChild; }

    void setFirstChild(Node* child) { m_firstChild = child; }
    void setLastChild(Node* child) { m_lastChild = child; }

    void appendChild(Node* child);
    void insertBefore(Node* child, Node* nextChild);
    void removeChild(Node* child);
    void reparentChildren(ContainerNode* newParent);

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
    Attribute(const GlobalString& name, std::string value)
        : m_name(name), m_value(std::move(value))
    {}

    const GlobalString& name() const { return m_name; }
    void setName(const GlobalString& name) { m_name = name; }

    const std::string& value() const { return m_value; }
    void setValue(std::string value) { m_value = std::move(value); }

    bool empty() const { return m_value.empty(); }

private:
    GlobalString m_name;
    std::string m_value;
};

inline bool operator==(const Attribute& a, const Attribute& b) { return a.name() == b.name() && a.value() == b.value(); }
inline bool operator!=(const Attribute& a, const Attribute& b) { return a.name() != b.name() || a.value() != b.value(); }

using AttributeList = std::vector<Attribute>;

class Element : public ContainerNode {
public:
    Element(Document* document, const GlobalString& tagName, const GlobalString& namespaceUri);

    bool isElementNode() const final { return true; }

    const GlobalString& tagName() const { return m_tagName; }
    const GlobalString& namespaceUri() const { return m_namespaceUri; }
    const AttributeList& attributes() const { return m_attributes; }

    std::string_view lang() const;
    const GlobalString& id() const { return m_id; }
    const GlobalStringList& classNames() const { return m_classNames; }

    void setId(const std::string_view& value);
    void setClass(const std::string_view& value);

    Attribute* findAttribute(const GlobalString& name) const;
    bool hasAttribute(const GlobalString& name) const;
    std::string_view getAttribute(const GlobalString& name) const;
    void setAttributeList(const AttributeList& attributes);
    void setAttribute(const Attribute& attribute);
    void setAttribute(const GlobalString& name, std::string value);
    void removeAttribute(const GlobalString& name);
    virtual void parseAttribute(const GlobalString& name, const std::string_view& value);
    virtual bool isPresentationAttribute(const GlobalString& name) const { return false; }

    CSSPropertyList inlineStyle() const;
    CSSPropertyList presentationAttributeStyle() const;

    Element* parentElement() const;
    Element* previousElement() const;
    Element* nextElement() const;

    void serialize(std::ostream& o) const override;

private:
    GlobalString m_tagName;
    GlobalString m_namespaceUri;
    mutable AttributeList m_attributes;

    GlobalString m_id;
    GlobalStringList m_classNames;
};

template<>
struct is<Element> {
    static bool check(const Node& value) { return value.isElementNode(); }
};

inline const GlobalString& Node::tagName() const
{
    if(isElementNode())
        return to<Element>(this)->tagName();
    return emptyString;
}

inline const GlobalString& Node::namespaceUri() const
{
    if(isElementNode())
        return to<Element>(this)->tagName();
    return emptyString;
}

class ResourceData;
class Resource;
class TextResource;
class ImageResource;
class FontResource;
class FontFace;

class Document : public ContainerNode {
public:
    Document();

    virtual ~Document();
    virtual std::shared_ptr<ResourceData> fetchUrl(const std::string_view& url) = 0;
    virtual std::shared_ptr<ResourceData> fetchFont(const std::string_view& family, bool italic, bool smallCaps, int weight) = 0;

    Element* createElement(const GlobalString& tagName, const GlobalString& namespaceUri);

    const Url& baseUrl() const { return m_baseUrl; }
    void setBaseUrl(Url baseUrl) { m_baseUrl = std::move(baseUrl); }

    void load(const std::string_view& content);

    void updateIdCache(const GlobalString& oldValue, const GlobalString& newValue, Element* element);
    void addFontFace(const std::string_view& family, bool italic, bool smallCaps, int weight, RefPtr<FontFace> face);
    RefPtr<FontFace> getFontFace(const std::string_view& family, bool italic, bool smallCaps, int weight);

    void addAuthorStyleSheet(const std::string_view& content);
    void setUserStyleSheet(const std::string_view& content);
    void clearUserStyleSheet();

    const CSSStyleSheet* authorStyleSheet() const { return m_authorStyleSheet.get(); }
    const CSSStyleSheet* userStyleSheet() const { return m_userStyleSheet.get(); }

    RefPtr<TextResource> fetchTextResource(const std::string_view& url);
    RefPtr<ImageResource> fetchImageResource(const std::string_view& url);
    RefPtr<FontResource> fetchFontResource(const std::string_view& url);

private:
    Url m_baseUrl;
    std::map<Url, RefPtr<Resource>> m_resourceCache;
    std::multimap<GlobalString, Element*> m_idCache;
    std::unique_ptr<CSSStyleSheet> m_authorStyleSheet;
    std::unique_ptr<CSSStyleSheet> m_userStyleSheet;
};

} // namespace htmlbook

#endif // DOCUMENT_H
