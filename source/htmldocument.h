#ifndef HTMLDOCUMENT_H
#define HTMLDOCUMENT_H

#include "document.h"

#include <optional>

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

    void collectAttributeStyle(std::stringstream& output, const GlobalString& name, const HeapString& value) const final;
};

class HTMLImageElement final : public HTMLElement {
public:
    HTMLImageElement(Document* document);

    void collectAttributeStyle(std::stringstream& output, const GlobalString& name, const HeapString& value) const final;
    const HeapString& src() const;
    const HeapString& altText() const;
    RefPtr<Image> image() const;

    Box* createBox(const RefPtr<BoxStyle>& style) final;
};

class HTMLFontElement final : public HTMLElement {
public:
    HTMLFontElement(Document* document);

    void collectAttributeStyle(std::stringstream& output, const GlobalString& name, const HeapString& value) const final;
};

class HTMLHRElement final : public HTMLElement {
public:
    HTMLHRElement(Document* document);

    void collectAttributeStyle(std::stringstream& output, const GlobalString& name, const HeapString& value) const final;
};

class HTMLLIElement final : public HTMLElement {
public:
    HTMLLIElement(Document* document);

    std::optional<int> value() const;
};

class HTMLOLElement final : public HTMLElement {
public:
    HTMLOLElement(Document* document);

    int start() const;
};

class HTMLTableElement final : public HTMLElement {
public:
    HTMLTableElement(Document* document);

    void collectAttributeStyle(std::stringstream& output, const GlobalString& name, const HeapString& value) const final;
};

class HTMLTableSectionElement final : public HTMLElement {
public:
    HTMLTableSectionElement(Document* document, const GlobalString& tagName);

    void collectAttributeStyle(std::stringstream& output, const GlobalString& name, const HeapString& value) const final;
};

class HTMLTableCaptionElement final : public HTMLElement {
public:
    HTMLTableCaptionElement(Document* document);

    void collectAttributeStyle(std::stringstream& output, const GlobalString& name, const HeapString& value) const final;
};

class HTMLTableRowElement final : public HTMLElement {
public:
    HTMLTableRowElement(Document* document);

    void collectAttributeStyle(std::stringstream& output, const GlobalString& name, const HeapString& value) const final;
};

class HTMLTableColElement final : public HTMLElement {
public:
    HTMLTableColElement(Document* document, const GlobalString& tagName);

    void collectAttributeStyle(std::stringstream& output, const GlobalString& name, const HeapString& value) const final;
    int span() const;

    Box* createBox(const RefPtr<BoxStyle>& style) final;
};

class HTMLTableCellElement final : public HTMLElement {
public:
    HTMLTableCellElement(Document* document, const GlobalString& tagName);

    void collectAttributeStyle(std::stringstream& output, const GlobalString& name, const HeapString& value) const final;
    int colSpan() const;
    int rowSpan() const;

    Box* createBox(const RefPtr<BoxStyle>& style) final;
};

class HTMLStyleElement final : public HTMLElement {
public:
    HTMLStyleElement(Document* document);

    const HeapString& type() const;

    void finishParsingChildren() final;
};

class HTMLLinkElement final : public HTMLElement {
public:
    HTMLLinkElement(Document* document);

    const HeapString& href() const;
    const HeapString& type() const;
    const HeapString& rel() const;

    void finishParsingChildren() final;
};

class HTMLDocument final : public Document {
public:
    HTMLDocument(const PageSize& size, PageOrientation orientation, const PageMargins& margins);

    bool load(const std::string_view& content) final;

    Box* createBox(const RefPtr<BoxStyle>& style) final;

private:
    PageSize m_pageSize;
    PageOrientation m_pageOrientation;
    PageMargins m_pageMargins;
    std::string m_title;
    std::string m_subject;
    std::string m_author;
    std::string m_creator;
    std::string m_creationDate;
    std::string m_modificationDate;
};

} // namespace htmlbook

#endif // HTMLDOCUMENT_H
