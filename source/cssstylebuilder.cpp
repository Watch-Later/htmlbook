#include "cssstylebuilder.h"
#include "document.h"

#include <algorithm>

namespace htmlbook {

static const CSSStyleSheet* userAgentStyleSheet() {
    static std::unique_ptr<CSSStyleSheet> userAgentStyleSheet;
    return userAgentStyleSheet.get();
}

std::shared_ptr<BoxStyle> CSSStyleBuilder::styleForElement(const Element* element, const BoxStyle& parentStyle)
{
    m_rules.clear();
    m_properties.clear();

    addStyleSheet(element, userAgentStyleSheet());
    addStyleSheet(element, m_document->authorStyleSheet());
    addStyleSheet(element, m_document->userStyleSheet());
    addPropertyList(element->inlineStyle());
    addPropertyList(element->presentationAttributeStyle());
    return convertPropertyList(m_properties, element, parentStyle);
}

void CSSStyleBuilder::addStyleSheet(const Element* element, const CSSStyleSheet* styleSheet)
{
    assert(m_rules.empty());
    if(styleSheet == nullptr)
        return;

    addRuleDataList(element, styleSheet->idRules(element->id()));
    for(auto& className : element->classNames())
        addRuleDataList(element, styleSheet->classRules(className));

    addRuleDataList(element, styleSheet->tagRules(element->tagName()));
    addRuleDataList(element, styleSheet->universeRules());

    std::sort(m_rules.begin(), m_rules.end());
    for(auto& rule : m_rules)
        addPropertyList(rule.properties());
    m_rules.clear();
}

void CSSStyleBuilder::addRuleDataList(const Element* element, const CSSRuleDataList* rules)
{
    if(rules == nullptr)
        return;
    for(auto& rule : *rules) {
        if(!rule.match(element, PseudoType::None))
            continue;
        m_rules.push_back(rule);
    }
}

void CSSStyleBuilder::addPropertyList(const CSSPropertyList& properties)
{
    for(auto& property : properties) {
        auto it = std::find_if(m_properties.begin(), m_properties.end(), [&](auto& item) { return property.id() == item.id(); });
        if(it == m_properties.end()) {
            m_properties.push_back(property);
            continue;
        }

        if(it->important() && !property.important())
            continue;
        *it = property;
    }
}

std::shared_ptr<BoxStyle> CSSStyleBuilder::convertPropertyList(const CSSPropertyList& properties, const Element* element, const BoxStyle& parentStyle)
{
    auto newStyle = BoxStyle::create(element);
    newStyle->inheritFrom(parentStyle);
    for(auto& property : properties) {
        auto id = property.id();
        auto value = property.value();
        if(value->isInitialValue() || (value->isInheritValue() && !(value = parentStyle.get(id))))
            continue;
        newStyle->set(id, std::move(value));
    }

    return newStyle;
}

} // namespace htmlbook
