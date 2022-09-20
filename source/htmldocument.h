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

    void parseAttribute(const GlobalString& name, const std::string_view& value) final;
};

class HTMLDocument final : public Document {
public:
    HTMLDocument(const PageSize& pageSize);

    bool load(const std::string_view& content) final;

    Box* createBox(const RefPtr<BoxStyle>& style) final;
};

} // namespace htmlbook

#endif // HTMLDOCUMENT_H
