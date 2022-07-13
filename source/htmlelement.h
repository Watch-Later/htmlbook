#ifndef HTMLELEMENT_H
#define HTMLELEMENT_H

#include "document.h"

namespace htmlbook {

class HTMLElement : public Element {
public:
    HTMLElement(Document* document, const GlobalString& tagName);
};

class HTMLImageElement final : public HTMLElement {
public:
    HTMLImageElement(Document* document);

    void parseAttribute(const GlobalString& name, const std::string_view& value) final;
};

} // namespace htmlbook

#endif // HTMLELEMENT_H
