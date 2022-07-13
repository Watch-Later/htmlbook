#include "htmlelement.h"

namespace htmlbook {

HTMLElement::HTMLElement(Document* document, const GlobalString& tagName)
    : Element(document, tagName, namespaceuri::xhtml)
{
}

HTMLImageElement::HTMLImageElement(Document* document)
    : HTMLElement(document, htmlnames::imgTag)
{
}

void HTMLImageElement::parseAttribute(const GlobalString& name, const std::string_view& value)
{
}

} // namespace htmlbook
