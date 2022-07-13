#ifndef CSSSTYLEBUILDER_H
#define CSSSTYLEBUILDER_H

#include "cssstylesheet.h"
#include "boxstyle.h"

namespace htmlbook {

class Document;
class Element;

class CSSStyleBuilder {
public:
    CSSStyleBuilder(Document* document) : m_document(document) {}

    std::shared_ptr<BoxStyle> styleForElement(const Element* element, const BoxStyle& parentStyle);
    std::shared_ptr<BoxStyle> pseudoStyleForElement(const Element* element, const BoxStyle& parentStyle, PseudoType pseudoType);
    std::shared_ptr<BoxStyle> styleForPage(const GlobalString& pageName, size_t pageIndex);

private:
    void addStyleSheet(const Element* element, const CSSStyleSheet* styleSheet);
    void addRuleDataList(const Element* element, const CSSRuleDataList* rules);
    void addPropertyList(const CSSPropertyList& properties);

    Document* m_document;
    CSSRuleDataList m_rules;
    CSSPropertyList m_properties;
};

} // namespace htmlbook

#endif // CSSSTYLEBUILDER_H
