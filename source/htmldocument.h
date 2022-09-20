#ifndef HTMLDOCUMENT_H
#define HTMLDOCUMENT_H

#include "document.h"

namespace htmlbook {

class HTMLElement : public Element {
public:
    HTMLElement(Document* document, const GlobalString& tagName);

    Box* createBox(const RefPtr<BoxStyle>& style) override;
    void buildPseudoBox(Counters& counters, Box* parent, PseudoType pseudoType);
    void buildBox(Counters& counters, Box* parent) override;
};

class HTMLImageElement final : public HTMLElement {
public:
    HTMLImageElement(Document* document);

    void collectAttributeStyle(const GlobalString& name, const std::string& value, std::string& output) const final;
    const std::string& src() const;
    const std::string& altText() const;
    RefPtr<Image> image() const;

    Box* createBox(const RefPtr<BoxStyle>& style) final;
};

class HTMLLIElement final : public HTMLElement {
public:
    HTMLLIElement(Document* document);

    void collectAttributeStyle(const GlobalString& name, const std::string& value, std::string& output) const final;
};

class HTMLOLElement final : public HTMLElement {
public:
    HTMLOLElement(Document* document);

    void collectAttributeStyle(const GlobalString& name, const std::string& value, std::string& output) const final;
};

class HTMLDocument final : public Document {
public:
    HTMLDocument(const PageSize& pageSize);

    bool load(const std::string_view& content) final;

    Box* createBox(const RefPtr<BoxStyle>& style) final;
};

} // namespace htmlbook

#endif // HTMLDOCUMENT_H
