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

class HTMLBodyElement final : public HTMLElement {
public:
    HTMLBodyElement(Document* document);

    void collectAttributeStyle(const GlobalString& name, const std::string& value, std::string& output) const final;
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

class HTMLFontElement final : public HTMLElement {
public:
    HTMLFontElement(Document* document);

    void collectAttributeStyle(const GlobalString& name, const std::string& value, std::string& output) const final;
};

class HTMLHRElement final : public HTMLElement {
public:
    HTMLHRElement(Document* document);

    void collectAttributeStyle(const GlobalString& name, const std::string& value, std::string& output) const final;
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

class HTMLTableElement final : public HTMLElement {
public:
    HTMLTableElement(Document* document);

    void collectAttributeStyle(const GlobalString& name, const std::string& value, std::string& output) const final;
};

class HTMLTableSectionElement final : public HTMLElement {
public:
    HTMLTableSectionElement(Document* document, const GlobalString& tagName);

    void collectAttributeStyle(const GlobalString& name, const std::string& value, std::string& output) const final;
};

class HTMLTableCaptionElement final : public HTMLElement {
public:
    HTMLTableCaptionElement(Document* document);

    void collectAttributeStyle(const GlobalString& name, const std::string& value, std::string& output) const final;
};

class HTMLTableRowElement final : public HTMLElement {
public:
    HTMLTableRowElement(Document* document);

    void collectAttributeStyle(const GlobalString& name, const std::string& value, std::string& output) const final;
};

class HTMLTableColElement final : public HTMLElement {
public:
    HTMLTableColElement(Document* document, const GlobalString& tagName);

    void collectAttributeStyle(const GlobalString& name, const std::string& value, std::string& output) const final;
    int span() const;

    Box* createBox(const RefPtr<BoxStyle>& style) final;
};

class HTMLTableCellElement final : public HTMLElement {
public:
    HTMLTableCellElement(Document* document, const GlobalString& tagName);

    void collectAttributeStyle(const GlobalString& name, const std::string& value, std::string& output) const final;
    int colSpan() const;
    int rowSpan() const;

    Box* createBox(const RefPtr<BoxStyle>& style) final;
};

class HTMLStyleElement final : public HTMLElement {
public:
    HTMLStyleElement(Document* document);

    void finishParsingChildren() final;
};

class HTMLLinkElement final : public HTMLElement {
public:
    HTMLLinkElement(Document* document);

    const std::string& href() const;
    const std::string& type() const;
    const std::string& rel() const;

    void finishParsingChildren() final;
};

class HTMLDocument final : public Document {
public:
    HTMLDocument(const PageSize& pageSize);

    bool load(const std::string_view& content) final;

    Box* createBox(const RefPtr<BoxStyle>& style) final;
};

} // namespace htmlbook

#endif // HTMLDOCUMENT_H
