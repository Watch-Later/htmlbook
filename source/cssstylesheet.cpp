#include "cssstylesheet.h"
#include "cssparser.h"
#include "document.h"

#include <algorithm>

namespace htmlbook {

std::shared_ptr<CSSInitialValue> CSSInitialValue::create()
{
    static std::shared_ptr<CSSInitialValue> value(new CSSInitialValue);
    return value;
}

std::shared_ptr<CSSInheritValue> CSSInheritValue::create()
{
    static std::shared_ptr<CSSInheritValue> value(new CSSInheritValue);
    return value;
}

std::shared_ptr<CSSIdentValue> CSSIdentValue::create(CSSValueID value)
{
    static std::map<CSSValueID, std::shared_ptr<CSSIdentValue>> table;
    auto it = table.find(value);
    if(it == table.end()) {
        std::shared_ptr<CSSIdentValue> item(new CSSIdentValue(value));
        return std::get<1>(*table.emplace_hint(it, value, item));
    }

    return it->second;
}

std::shared_ptr<CSSCustomIdentValue> CSSCustomIdentValue::create(const GlobalString& value)
{
    return std::shared_ptr<CSSCustomIdentValue>(new CSSCustomIdentValue(value));
}

std::shared_ptr<CSSIntegerValue> CSSIntegerValue::create(int value)
{
    return std::shared_ptr<CSSIntegerValue>(new CSSIntegerValue(value));
}

std::shared_ptr<CSSNumberValue> CSSNumberValue::create(double value)
{
    return std::shared_ptr<CSSNumberValue>(new CSSNumberValue(value));
}

std::shared_ptr<CSSPercentValue> CSSPercentValue::create(double value)
{
    return std::shared_ptr<CSSPercentValue>(new CSSPercentValue(value));
}

std::shared_ptr<CSSAngleValue> CSSAngleValue::create(double value, Unit unit)
{
    return std::shared_ptr<CSSAngleValue>(new CSSAngleValue(value, unit));
}

std::shared_ptr<CSSLengthValue> CSSLengthValue::create(double value, Unit unit)
{
    return std::shared_ptr<CSSLengthValue>(new CSSLengthValue(value, unit));
}

std::shared_ptr<CSSStringValue> CSSStringValue::create(std::string&& value)
{
    return std::shared_ptr<CSSStringValue>(new CSSStringValue(std::move(value)));
}

std::shared_ptr<CSSUrlValue> CSSUrlValue::create(std::string &&value)
{
    return std::shared_ptr<CSSUrlValue>(new CSSUrlValue(std::move(value)));
}

std::shared_ptr<CSSColorValue> CSSColorValue::create(uint32_t value)
{
    return std::shared_ptr<CSSColorValue>(new CSSColorValue(value));
}

std::shared_ptr<CSSColorValue> CSSColorValue::create(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    return create(a << 24 | r << 16 | g << 8 | b);
}

std::shared_ptr<CSSCounterValue> CSSCounterValue::create(CSSValueID listStyle, const GlobalString& identifier, std::string&& seperator)
{
    return std::shared_ptr<CSSCounterValue>(new CSSCounterValue(listStyle, identifier, std::move(seperator)));
}

std::shared_ptr<CSSPairValue> CSSPairValue::create(std::shared_ptr<CSSValue> first, std::shared_ptr<CSSValue> second)
{
    return std::shared_ptr<CSSPairValue>(new CSSPairValue(std::move(first), std::move(second)));
}

std::shared_ptr<CSSRectValue> CSSRectValue::create(std::shared_ptr<CSSValue> top, std::shared_ptr<CSSValue> right, std::shared_ptr<CSSValue> bottom, std::shared_ptr<CSSValue> left)
{
    return std::shared_ptr<CSSRectValue>(new CSSRectValue(std::move(top), std::move(right), std::move(bottom), std::move(left)));
}

std::shared_ptr<CSSListValue> CSSListValue::create(CSSValueList&& values)
{
    return std::shared_ptr<CSSListValue>(new CSSListValue(std::move(values)));
}

std::shared_ptr<CSSFunctionValue> CSSFunctionValue::create(CSSValueID id, CSSValueList&& values)
{
    return std::shared_ptr<CSSFunctionValue>(new CSSFunctionValue(id, std::move(values)));
}

CSSShorthand CSSShorthand::longhand(CSSPropertyID id)
{
    switch(id) {
    case CSSPropertyID::BorderColor: {
        static const CSSPropertyID data[4] = {
            CSSPropertyID::BorderTopColor,
            CSSPropertyID::BorderRightColor,
            CSSPropertyID::BorderBottomColor,
            CSSPropertyID::BorderLeftColor
        };

        return CSSShorthand(data, 4);
    }

    case CSSPropertyID::BorderStyle: {
        static const CSSPropertyID data[4] = {
            CSSPropertyID::BorderTopStyle,
            CSSPropertyID::BorderRightStyle,
            CSSPropertyID::BorderBottomStyle,
            CSSPropertyID::BorderLeftStyle
        };

        return CSSShorthand(data, 4);
    }

    case CSSPropertyID::BorderWidth: {
        static const CSSPropertyID data[4] = {
            CSSPropertyID::BorderTopWidth,
            CSSPropertyID::BorderRightWidth,
            CSSPropertyID::BorderBottomWidth,
            CSSPropertyID::BorderLeftWidth
        };

        return CSSShorthand(data, 4);
    }

    case CSSPropertyID::BorderTop: {
        static const CSSPropertyID data[3] = {
            CSSPropertyID::BorderTopColor,
            CSSPropertyID::BorderTopStyle,
            CSSPropertyID::BorderTopWidth
        };

        return CSSShorthand(data, 3);
    }

    case CSSPropertyID::BorderRight: {
        static const CSSPropertyID data[3] = {
            CSSPropertyID::BorderRightColor,
            CSSPropertyID::BorderRightStyle,
            CSSPropertyID::BorderRightWidth
        };

        return CSSShorthand(data, 3);
    }

    case CSSPropertyID::BorderBottom: {
        static const CSSPropertyID data[3] = {
            CSSPropertyID::BorderBottomColor,
            CSSPropertyID::BorderBottomStyle,
            CSSPropertyID::BorderBottomWidth
        };

        return CSSShorthand(data, 3);
    }

    case CSSPropertyID::BorderLeft: {
        static const CSSPropertyID data[3] = {
            CSSPropertyID::BorderLeftColor,
            CSSPropertyID::BorderLeftStyle,
            CSSPropertyID::BorderLeftWidth
        };

        return CSSShorthand(data, 3);
    }

    case CSSPropertyID::BorderRadius: {
        static const CSSPropertyID data[4] = {
            CSSPropertyID::BorderTopRightRadius,
            CSSPropertyID::BorderTopLeftRadius,
            CSSPropertyID::BorderBottomLeftRadius,
            CSSPropertyID::BorderBottomRightRadius
        };

        return CSSShorthand(data, 3);
    }

    case CSSPropertyID::BorderSpacing: {
        static const CSSPropertyID data[2] = {
            CSSPropertyID::BorderHorizontalSpacing,
            CSSPropertyID::BorderVerticalSpacing
        };

        return CSSShorthand(data, 2);
    }

    case CSSPropertyID::Padding: {
        static const CSSPropertyID data[4] = {
            CSSPropertyID::PaddingTop,
            CSSPropertyID::PaddingRight,
            CSSPropertyID::PaddingBottom,
            CSSPropertyID::PaddingLeft
        };

        return CSSShorthand(data, 4);
    }

    case CSSPropertyID::Margin: {
        static const CSSPropertyID data[4] = {
            CSSPropertyID::MarginTop,
            CSSPropertyID::MarginRight,
            CSSPropertyID::MarginBottom,
            CSSPropertyID::MarginLeft
        };

        return CSSShorthand(data, 4);
    }

    case CSSPropertyID::Outline: {
        static const CSSPropertyID data[4] = {
            CSSPropertyID::OutlineColor,
            CSSPropertyID::OutlineOffset,
            CSSPropertyID::OutlineStyle,
            CSSPropertyID::OutlineWidth
        };

        return CSSShorthand(data, 4);
    }

    case CSSPropertyID::ListStyle: {
        static const CSSPropertyID data[3] = {
            CSSPropertyID::ListStyleImage,
            CSSPropertyID::ListStylePosition,
            CSSPropertyID::ListStyleType
        };

        return CSSShorthand(data, 3);
    }

    case CSSPropertyID::ColumnRule: {
        static const CSSPropertyID data[3] = {
            CSSPropertyID::ColumnRuleColor,
            CSSPropertyID::ColumnRuleStyle,
            CSSPropertyID::ColumnRuleWidth
        };

        return CSSShorthand(data, 3);
    }

    default:
        return CSSShorthand(nullptr, 0);
    }
}

CSSPropertyID csspropertyid(const std::string_view& name)
{
    static const struct {
        std::string_view name;
        CSSPropertyID id;
    } table[] = {
        {"align-content", CSSPropertyID::AlignContent},
        {"align-items", CSSPropertyID::AlignItems},
        {"align-self", CSSPropertyID::AlignSelf},
        {"all", CSSPropertyID::All},
        {"background", CSSPropertyID::Background},
        {"background-attachment", CSSPropertyID::BackgroundAttachment},
        {"background-clip", CSSPropertyID::BackgroundClip},
        {"background-color", CSSPropertyID::BackgroundColor},
        {"background-image", CSSPropertyID::BackgroundImage},
        {"background-origin", CSSPropertyID::BackgroundOrigin},
        {"background-position", CSSPropertyID::BackgroundPosition},
        {"background-repeat", CSSPropertyID::BackgroundRepeat},
        {"background-size", CSSPropertyID::BackgroundSize},
        {"border", CSSPropertyID::Border},
        {"border-bottom", CSSPropertyID::BorderBottom},
        {"border-bottom-color", CSSPropertyID::BorderBottomColor},
        {"border-bottom-left-radius", CSSPropertyID::BorderBottomLeftRadius},
        {"border-bottom-right-radius", CSSPropertyID::BorderBottomRightRadius},
        {"border-bottom-style", CSSPropertyID::BorderBottomStyle},
        {"border-bottom-width", CSSPropertyID::BorderBottomWidth},
        {"border-collapse", CSSPropertyID::BorderCollapse},
        {"border-color", CSSPropertyID::BorderColor},
        {"border-left", CSSPropertyID::BorderLeft},
        {"border-left-color", CSSPropertyID::BorderLeftColor},
        {"border-left-style", CSSPropertyID::BorderLeftStyle},
        {"border-left-width", CSSPropertyID::BorderLeftWidth},
        {"border-radius", CSSPropertyID::BorderRadius},
        {"border-right", CSSPropertyID::BorderRight},
        {"border-right-color", CSSPropertyID::BorderRightColor},
        {"border-right-style", CSSPropertyID::BorderRightStyle},
        {"border-right-width", CSSPropertyID::BorderRightWidth},
        {"border-spacing", CSSPropertyID::BorderSpacing},
        {"border-style", CSSPropertyID::BorderStyle},
        {"border-top", CSSPropertyID::BorderTop},
        {"border-top-color", CSSPropertyID::BorderTopColor},
        {"border-top-left-radius", CSSPropertyID::BorderTopLeftRadius},
        {"border-top-right-radius", CSSPropertyID::BorderTopRightRadius},
        {"border-top-style", CSSPropertyID::BorderTopStyle},
        {"border-top-width", CSSPropertyID::BorderTopWidth},
        {"border-width", CSSPropertyID::BorderWidth},
        {"bottom", CSSPropertyID::Bottom},
        {"box-sizing", CSSPropertyID::BoxSizing},
        {"caption-side", CSSPropertyID::CaptionSide},
        {"clear", CSSPropertyID::Clear},
        {"clip", CSSPropertyID::Clip},
        {"clip-path", CSSPropertyID::ClipPath},
        {"clip-rule", CSSPropertyID::ClipRule},
        {"color", CSSPropertyID::Color},
        {"column-break-after", CSSPropertyID::ColumnBreakAfter},
        {"column-break-before", CSSPropertyID::ColumnBreakBefore},
        {"column-break-inside", CSSPropertyID::ColumnBreakInside},
        {"column-count", CSSPropertyID::ColumnCount},
        {"column-gap", CSSPropertyID::ColumnGap},
        {"column-rule", CSSPropertyID::ColumnRule},
        {"column-rule-color", CSSPropertyID::ColumnRuleColor},
        {"column-rule-style", CSSPropertyID::ColumnRuleStyle},
        {"column-rule-width", CSSPropertyID::ColumnRuleWidth},
        {"column-span", CSSPropertyID::ColumnSpan},
        {"column-width", CSSPropertyID::ColumnWidth},
        {"columns", CSSPropertyID::Columns},
        {"content", CSSPropertyID::Content},
        {"counter-increment", CSSPropertyID::CounterIncrement},
        {"counter-reset", CSSPropertyID::CounterReset},
        {"counter-set", CSSPropertyID::CounterSet},
        {"cx", CSSPropertyID::Cx},
        {"cy", CSSPropertyID::Cy},
        {"direction", CSSPropertyID::Direction},
        {"display", CSSPropertyID::Display},
        {"empty-cells", CSSPropertyID::EmptyCells},
        {"fill", CSSPropertyID::Fill},
        {"fill-opacity", CSSPropertyID::FillOpacity},
        {"fill-rule", CSSPropertyID::FillRule},
        {"flex", CSSPropertyID::Flex},
        {"flex-basis", CSSPropertyID::FlexBasis},
        {"flex-direction", CSSPropertyID::FlexDirection},
        {"flex-flow", CSSPropertyID::FlexFlow},
        {"flex-grow", CSSPropertyID::FlexGrow},
        {"flex-shrink", CSSPropertyID::FlexShrink},
        {"flex-wrap", CSSPropertyID::FlexWrap},
        {"float", CSSPropertyID::Float},
        {"font", CSSPropertyID::Font},
        {"font-family", CSSPropertyID::FontFamily},
        {"font-size", CSSPropertyID::FontSize},
        {"font-style", CSSPropertyID::FontStyle},
        {"font-variant", CSSPropertyID::FontVariant},
        {"font-weight", CSSPropertyID::FontWeight},
        {"height", CSSPropertyID::Height},
        {"hyphens", CSSPropertyID::Hyphens},
        {"justify-content", CSSPropertyID::JustifyContent},
        {"left", CSSPropertyID::Left},
        {"letter-spacing", CSSPropertyID::LetterSpacing},
        {"line-break", CSSPropertyID::LineBreak},
        {"line-height", CSSPropertyID::LineHeight},
        {"list-style", CSSPropertyID::ListStyle},
        {"list-style-image", CSSPropertyID::ListStyleImage},
        {"list-style-position", CSSPropertyID::ListStylePosition},
        {"list-style-type", CSSPropertyID::ListStyleType},
        {"margin", CSSPropertyID::Margin},
        {"margin-bottom", CSSPropertyID::MarginBottom},
        {"margin-left", CSSPropertyID::MarginLeft},
        {"margin-right", CSSPropertyID::MarginRight},
        {"margin-top", CSSPropertyID::MarginTop},
        {"marker-end", CSSPropertyID::MarkerEnd},
        {"marker-mid", CSSPropertyID::MarkerMid},
        {"marker-start", CSSPropertyID::MarkerStart},
        {"mask", CSSPropertyID::Mask},
        {"max-height", CSSPropertyID::MaxHeight},
        {"max-width", CSSPropertyID::MaxWidth},
        {"min-height", CSSPropertyID::MinHeight},
        {"min-width", CSSPropertyID::MinWidth},
        {"mix-blend-mode", CSSPropertyID::MixBlendMode},
        {"opacity", CSSPropertyID::Opacity},
        {"order", CSSPropertyID::Order},
        {"orphans", CSSPropertyID::Orphans},
        {"outline", CSSPropertyID::Outline},
        {"outline-color", CSSPropertyID::OutlineColor},
        {"outline-offset", CSSPropertyID::OutlineOffset},
        {"outline-style", CSSPropertyID::OutlineStyle},
        {"outline-width", CSSPropertyID::OutlineWidth},
        {"overflow", CSSPropertyID::Overflow},
        {"overflow-wrap", CSSPropertyID::OverflowWrap},
        {"overflow-x", CSSPropertyID::OverflowX},
        {"overflow-y", CSSPropertyID::OverflowY},
        {"padding", CSSPropertyID::Padding},
        {"padding-bottom", CSSPropertyID::PaddingBottom},
        {"padding-left", CSSPropertyID::PaddingLeft},
        {"padding-right", CSSPropertyID::PaddingRight},
        {"padding-top", CSSPropertyID::PaddingTop},
        {"page", CSSPropertyID::Page},
        {"page-break-after", CSSPropertyID::PageBreakAfter},
        {"page-break-before", CSSPropertyID::PageBreakBefore},
        {"page-break-inside", CSSPropertyID::PageBreakInside},
        {"page-orientation", CSSPropertyID::PageOrientation},
        {"paint-order", CSSPropertyID::PaintOrder},
        {"position", CSSPropertyID::Position},
        {"quotes", CSSPropertyID::Quotes},
        {"r", CSSPropertyID::R},
        {"right", CSSPropertyID::Right},
        {"rx", CSSPropertyID::Rx},
        {"ry", CSSPropertyID::Ry},
        {"size", CSSPropertyID::Size},
        {"src", CSSPropertyID::Src},
        {"stop-color", CSSPropertyID::StopColor},
        {"stop-opacity", CSSPropertyID::StopOpacity},
        {"stroke", CSSPropertyID::Stroke},
        {"stroke-dasharray", CSSPropertyID::StrokeDasharray},
        {"stroke-dashoffset", CSSPropertyID::StrokeDashoffset},
        {"stroke-linecap", CSSPropertyID::StrokeLinecap},
        {"stroke-linejoin", CSSPropertyID::StrokeLinejoin},
        {"stroke-miterlimit", CSSPropertyID::StrokeMiterlimit},
        {"stroke-opacity", CSSPropertyID::StrokeOpacity},
        {"stroke-width", CSSPropertyID::StrokeWidth},
        {"tab-size", CSSPropertyID::TabSize},
        {"table-layout", CSSPropertyID::TableLayout},
        {"text-align", CSSPropertyID::TextAlign},
        {"text-anchor", CSSPropertyID::TextAnchor},
        {"text-decoration", CSSPropertyID::TextDecoration},
        {"text-decoration-color", CSSPropertyID::TextDecorationColor},
        {"text-decoration-line", CSSPropertyID::TextDecorationLine},
        {"text-decoration-style", CSSPropertyID::TextDecorationStyle},
        {"text-indent", CSSPropertyID::TextIndent},
        {"text-overflow", CSSPropertyID::TextOverflow},
        {"text-transform", CSSPropertyID::TextTransform},
        {"top", CSSPropertyID::Top},
        {"transform", CSSPropertyID::Transform},
        {"transform-origin", CSSPropertyID::TransformOrigin},
        {"unicode-bidi", CSSPropertyID::UnicodeBidi},
        {"vector-effect", CSSPropertyID::VectorEffect},
        {"vertical-align", CSSPropertyID::VerticalAlign},
        {"visibility", CSSPropertyID::Visibility},
        {"white-space", CSSPropertyID::WhiteSpace},
        {"widows", CSSPropertyID::Widows},
        {"width", CSSPropertyID::Width},
        {"word-break", CSSPropertyID::WordBreak},
        {"word-spacing", CSSPropertyID::WordSpacing},
        {"word-wrap", CSSPropertyID::WordWrap},
        {"x", CSSPropertyID::X},
        {"y", CSSPropertyID::Y},
        {"z-index", CSSPropertyID::ZIndex}
    };

    auto it = std::lower_bound(table, std::end(table), name, [](auto& item, auto& name) { return item.name < name; });
    if(it != std::end(table) && it->name == name)
        return it->id;
    return CSSPropertyID::Unknown;
}

bool CSSSimpleSelector::matchnth(int count) const
{
    auto a = std::get<0>(m_matchPattern);
    auto b = std::get<1>(m_matchPattern);
    if(a == 0)
        return count == b;
    if(a > 0) {
        if(count < b)
            return false;
        return (count - b) % a == 0;
    }

    if(count > b)
        return false;
    return (b - count) % -a == 0;
}

std::unique_ptr<CSSStyleRule> CSSStyleRule::create(CSSSelectorList&& selectors, CSSPropertyList&& properties)
{
    return std::unique_ptr<CSSStyleRule>(new CSSStyleRule(std::move(selectors), std::move(properties)));
}

std::unique_ptr<CSSImportRule> CSSImportRule::create(std::string&& href)
{
    return std::unique_ptr<CSSImportRule>(new CSSImportRule(std::move(href)));
}

std::unique_ptr<CSSFontFaceRule> CSSFontFaceRule::create(CSSPropertyList&& properties)
{
    return std::unique_ptr<CSSFontFaceRule>(new CSSFontFaceRule(std::move(properties)));
}

std::unique_ptr<CSSPageMarginRule> CSSPageMarginRule::create(MarginType marginType, CSSPropertyList&& properties)
{
    return std::unique_ptr<CSSPageMarginRule>(new CSSPageMarginRule(marginType, std::move(properties)));
}

std::unique_ptr<CSSPageRule> CSSPageRule::create(CSSPageSelectorList&& selectors, CSSPageMarginRuleList&& margins, CSSPropertyList&& properties)
{
    return std::unique_ptr<CSSPageRule>(new CSSPageRule(std::move(selectors), std::move(margins), std::move(properties)));
}

bool CSSRuleData::match(const Element* element, PseudoType pseudoType) const
{
    auto it = m_selector->rbegin();
    auto end = m_selector->rend();
    if(it == end)
        return false;

    if(!matchCompoundSelector(element, pseudoType, it->compoundSelector()))
        return false;

    auto combinator = it->combinator();
    ++it;

    while(it != end) {
        switch(combinator) {
        case CSSComplexSelector::Combinator::Descendant:
        case CSSComplexSelector::Combinator::Child:
            element = element->parentElement();
            break;
        case CSSComplexSelector::Combinator::DirectAdjacent:
        case CSSComplexSelector::Combinator::InDirectAdjacent:
            element = element->previousElement();
            break;
        case CSSComplexSelector::Combinator::None:
            assert(false);
        }

        if(element == nullptr)
            return false;

        if(matchCompoundSelector(element, pseudoType, it->compoundSelector())) {
            combinator = it->combinator();
            ++it;
        } else if(combinator != CSSComplexSelector::Combinator::Descendant
            && combinator != CSSComplexSelector::Combinator::InDirectAdjacent) {
            return false;
        }
    }

    return true;
}

bool CSSRuleData::matchCompoundSelector(const Element* element, PseudoType pseudoType, const CSSCompoundSelector& selector)
{
    for(auto& sel : selector) {
        if(!matchSimpleSelector(element, pseudoType, sel))
            return false;
    }

    return true;
}

bool CSSRuleData::matchCompoundSelectorList(const Element* element, PseudoType pseudoType, const CSSCompoundSelectorList& selectors)
{
    for(auto& selector : selectors) {
        if(!matchCompoundSelector(element, pseudoType, selector))
            return false;
    }

    return true;
}

bool CSSRuleData::matchSimpleSelector(const Element* element, PseudoType pseudoType, const CSSSimpleSelector& selector)
{
    switch(selector.matchType()) {
    case CSSSimpleSelector::MatchType::Tag:
        return matchTagSelector(element, selector);
    case CSSSimpleSelector::MatchType::Id:
        return matchIdSelector(element, selector);
    case CSSSimpleSelector::MatchType::Class:
        return matchClassSelector(element, selector);
    case CSSSimpleSelector::MatchType::AttributeHas:
        return matchAttributeHasSelector(element, selector);
    case CSSSimpleSelector::MatchType::AttributeEquals:
        return matchAttributeEqualsSelector(element, selector);
    case CSSSimpleSelector::MatchType::AttributeIncludes:
        return matchAttributeIncludesSelector(element, selector);
    case CSSSimpleSelector::MatchType::AttributeContains:
        return matchAttributeContainsSelector(element, selector);
    case CSSSimpleSelector::MatchType::AttributeDashEquals:
        return matchAttributeDashEqualsSelector(element, selector);
    case CSSSimpleSelector::MatchType::AttributeStartsWith:
        return matchAttributeStartsWithSelector(element, selector);
    case CSSSimpleSelector::MatchType::AttributeEndsWith:
        return matchAttributeEndsWithSelector(element, selector);
    case CSSSimpleSelector::MatchType::PseudoClassIs:
        return matchPseudoClassIsSelector(element, pseudoType, selector);
    case CSSSimpleSelector::MatchType::PseudoClassNot:
        return matchPseudoClassNotSelector(element, pseudoType, selector);
    case CSSSimpleSelector::MatchType::PseudoClassLink:
        return matchPseudoClassLinkSelector(element, selector);
    case CSSSimpleSelector::MatchType::PseudoClassEnabled:
        return matchPseudoClassEnabledSelector(element, selector);
    case CSSSimpleSelector::MatchType::PseudoClassDisabled:
        return matchPseudoClassDisabledSelector(element, selector);
    case CSSSimpleSelector::MatchType::PseudoClassChecked:
        return matchPseudoClassCheckedSelector(element, selector);
    case CSSSimpleSelector::MatchType::PseudoClassLang:
        return matchPseudoClassLangSelector(element, selector);
    case CSSSimpleSelector::MatchType::PseudoClassRoot:
        return matchPseudoClassRootSelector(element, selector);
    case CSSSimpleSelector::MatchType::PseudoClassEmpty:
        return matchPseudoClassEmptySelector(element, selector);
    case CSSSimpleSelector::MatchType::PseudoClassFirstChild:
        return matchPseudoClassFirstChildSelector(element, selector);
    case CSSSimpleSelector::MatchType::PseudoClassLastChild:
        return matchPseudoClassLastChildSelector(element, selector);
    case CSSSimpleSelector::MatchType::PseudoClassOnlyChild:
        return matchPseudoClassOnlyChildSelector(element, selector);
    case CSSSimpleSelector::MatchType::PseudoClassFirstOfType:
        return matchPseudoClassFirstOfTypeSelector(element, selector);
    case CSSSimpleSelector::MatchType::PseudoClassLastOfType:
        return matchPseudoClassLastOfTypeSelector(element, selector);
    case CSSSimpleSelector::MatchType::PseudoClassOnlyOfType:
        return matchPseudoClassOnlyOfTypeSelector(element, selector);
    case CSSSimpleSelector::MatchType::PseudoClassNthChild:
        return matchPseudoClassNthChildSelector(element, selector);
    case CSSSimpleSelector::MatchType::PseudoClassNthLastChild:
        return matchPseudoClassNthLastChildSelector(element, selector);
    case CSSSimpleSelector::MatchType::PseudoClassNthOfType:
        return matchPseudoClassNthOfTypeSelector(element, selector);
    case CSSSimpleSelector::MatchType::PseudoClassNthLastOfType:
        return matchPseudoClassNthLastOfTypeSelector(element, selector);
    case CSSSimpleSelector::MatchType::PseudoElementBefore:
        return pseudoType == PseudoType::Before;
    case CSSSimpleSelector::MatchType::PseudoElementAfter:
        return pseudoType == PseudoType::After;
    case CSSSimpleSelector::MatchType::PseudoElementMarker:
        return pseudoType == PseudoType::Marker;
    case CSSSimpleSelector::MatchType::PseudoElementFirstLetter:
        return pseudoType == PseudoType::FirstLetter;
    case CSSSimpleSelector::MatchType::PseudoElementFirstLine:
        return pseudoType == PseudoType::FirstLine;
    default:
        return false;
    }
}

bool CSSRuleData::matchTagSelector(const Element* element, const CSSSimpleSelector& selector)
{
    return element->tagName() == selector.name();
}

bool CSSRuleData::matchIdSelector(const Element* element, const CSSSimpleSelector& selector)
{
    return element->id() == selector.name();
}

bool CSSRuleData::matchClassSelector(const Element* element, const CSSSimpleSelector& selector)
{
    for(auto& name : element->classNames()) {
        if(name == selector.name())
            return true;
    }

    return false;
}

bool CSSRuleData::matchAttributeHasSelector(const Element* element, const CSSSimpleSelector& selector)
{
    return element->hasAttribute(selector.name());
}

bool CSSRuleData::matchAttributeEqualsSelector(const Element* element, const CSSSimpleSelector& selector)
{
    auto attribute = element->findAttribute(selector.name());
    if(attribute == nullptr)
        return false;
    return equals(attribute->value(), selector.value(), selector.isCaseSensitive());
}

bool CSSRuleData::matchAttributeIncludesSelector(const Element* element, const CSSSimpleSelector& selector)
{
    auto attribute = element->findAttribute(selector.name());
    if(attribute == nullptr)
        return false;
    return includes(attribute->value(), selector.value(), selector.isCaseSensitive());
}

bool CSSRuleData::matchAttributeContainsSelector(const Element* element, const CSSSimpleSelector& selector)
{
    auto attribute = element->findAttribute(selector.name());
    if(attribute == nullptr)
        return false;
    return contains(attribute->value(), selector.value(), selector.isCaseSensitive());
}

bool CSSRuleData::matchAttributeDashEqualsSelector(const Element* element, const CSSSimpleSelector& selector)
{
    auto attribute = element->findAttribute(selector.name());
    if(attribute == nullptr)
        return false;
    return dashequals(attribute->value(), selector.value(), selector.isCaseSensitive());
}

bool CSSRuleData::matchAttributeStartsWithSelector(const Element* element, const CSSSimpleSelector& selector)
{
    auto attribute = element->findAttribute(selector.name());
    if(attribute == nullptr)
        return false;
    return startswith(attribute->value(), selector.value(), selector.isCaseSensitive());
}

bool CSSRuleData::matchAttributeEndsWithSelector(const Element* element, const CSSSimpleSelector& selector)
{
    auto attribute = element->findAttribute(selector.name());
    if(attribute == nullptr)
        return false;
    return endswith(attribute->value(), selector.value(), selector.isCaseSensitive());
}

bool CSSRuleData::matchPseudoClassIsSelector(const Element* element, PseudoType pseudoType, const CSSSimpleSelector& selector)
{
    return matchCompoundSelectorList(element, pseudoType, selector.subSelectors());
}

bool CSSRuleData::matchPseudoClassNotSelector(const Element* element, PseudoType pseudoType, const CSSSimpleSelector& selector)
{
    return !matchCompoundSelectorList(element, pseudoType, selector.subSelectors());
}

bool CSSRuleData::matchPseudoClassLinkSelector(const Element* element, const CSSSimpleSelector& selector)
{
    return element->tagName() == htmlnames::aTag && element->hasAttribute(htmlnames::langAttr);
}

bool CSSRuleData::matchPseudoClassEnabledSelector(const Element* element, const CSSSimpleSelector& selector)
{
    return element->tagName() == htmlnames::inputTag && element->hasAttribute(htmlnames::enabledAttr);
}

bool CSSRuleData::matchPseudoClassDisabledSelector(const Element* element, const CSSSimpleSelector& selector)
{
    return element->tagName() == htmlnames::inputTag && element->hasAttribute(htmlnames::disabledAttr);
}

bool CSSRuleData::matchPseudoClassCheckedSelector(const Element* element, const CSSSimpleSelector& selector)
{
    return element->tagName() == htmlnames::inputTag && element->hasAttribute(htmlnames::checkedAttr);
}

bool CSSRuleData::matchPseudoClassLangSelector(const Element* element, const CSSSimpleSelector& selector)
{
    return dashequals(element->lang(), selector.name(), false);
}

bool CSSRuleData::matchPseudoClassRootSelector(const Element* element, const CSSSimpleSelector& selector)
{
    return element->tagName() == htmlnames::htmlTag;
}

bool CSSRuleData::matchPseudoClassEmptySelector(const Element* element, const CSSSimpleSelector& selector)
{
    return element->firstChild() == nullptr;
}

bool CSSRuleData::matchPseudoClassFirstChildSelector(const Element* element, const CSSSimpleSelector& selector)
{
    return element->previousSibling() == nullptr;
}

bool CSSRuleData::matchPseudoClassLastChildSelector(const Element* element, const CSSSimpleSelector& selector)
{
    return element->nextSibling() == nullptr;
}

bool CSSRuleData::matchPseudoClassOnlyChildSelector(const Element* element, const CSSSimpleSelector& selector)
{
    return !matchPseudoClassFirstChildSelector(element, selector) && !matchPseudoClassLastChildSelector(element, selector);
}

bool CSSRuleData::matchPseudoClassFirstOfTypeSelector(const Element* element, const CSSSimpleSelector& selector)
{
    auto child = element->previousElement();
    while(child) {
        if(child->tagName() == element->tagName())
            return false;
        child = element->previousElement();
    }

    return true;
}

bool CSSRuleData::matchPseudoClassLastOfTypeSelector(const Element* element, const CSSSimpleSelector& selector)
{
    auto child = element->nextElement();
    while(child) {
        if(child->tagName() == element->tagName())
            return false;
        child = element->nextElement();
    }

    return true;
}

bool CSSRuleData::matchPseudoClassOnlyOfTypeSelector(const Element* element, const CSSSimpleSelector& selector)
{
    return !matchPseudoClassFirstOfTypeSelector(element, selector) && !matchPseudoClassLastOfTypeSelector(element, selector);
}

bool CSSRuleData::matchPseudoClassNthChildSelector(const Element* element, const CSSSimpleSelector& selector)
{
    int count = 1;
    auto child = element->previousElement();
    while(child) {
        count += 1;
        child = element->previousElement();
    }

    return selector.matchnth(count);
}

bool CSSRuleData::matchPseudoClassNthLastChildSelector(const Element* element, const CSSSimpleSelector& selector)
{
    int count = 1;
    auto child = element->nextElement();
    while(child) {
        count += 1;
        child = element->nextElement();
    }

    return selector.matchnth(count);
}

bool CSSRuleData::matchPseudoClassNthOfTypeSelector(const Element* element, const CSSSimpleSelector& selector)
{
    int count = 1;
    auto child = element->previousElement();
    while(child && child->tagName() == element->tagName()) {
        count += 1;
        child = element->previousElement();
    }

    return selector.matchnth(count);
}

bool CSSRuleData::matchPseudoClassNthLastOfTypeSelector(const Element* element, const CSSSimpleSelector& selector)
{
    int count = 1;
    auto child = element->nextElement();
    while(child && child->tagName() == element->tagName()) {
        count += 1;
        child = element->nextElement();
    }

    return selector.matchnth(count);
}

void CSSRuleDataListMap::add(const CSSRuleData& data, const GlobalString& name)
{
    auto it = m_ruleDataListMap.find(name);
    if(it != m_ruleDataListMap.end()) {
        auto& rules = std::get<1>(*it);
        rules.push_back(data);
        return;
    }

    CSSRuleDataList rules;
    rules.push_back(data);
    m_ruleDataListMap.emplace(name, std::move(rules));
}

const CSSRuleDataList* CSSRuleDataListMap::get(const GlobalString& name) const
{
    auto it = m_ruleDataListMap.find(name);
    if(it == m_ruleDataListMap.end())
        return nullptr;
    return &it->second;
}

std::unique_ptr<CSSStyleSheet> CSSStyleSheet::create(Document* document)
{
    return std::unique_ptr<CSSStyleSheet>(new CSSStyleSheet(document));
}

void CSSStyleSheet::parse(const std::string_view& content)
{
    CSSParser::parseSheet(this, content);
}

void CSSStyleSheet::addRule(std::unique_ptr<CSSRule>&& rule)
{
    if(rule->isStyleRule()) {
        addStyleRule(rule->toStyleRule());
    } else if(rule->isImportRule()) {
        addImportRule(rule->toImportRule());
    } else if(rule->isPageRule()) {
        addPageRule(rule->toPageRule());
    } else if(rule->isFontFaceRule()) {
        addFontFaceRule(rule->toFontFaceRule());
    } else {
        assert(false);
    }

    m_rules.push_back(std::move(rule));
}

void CSSStyleSheet::addStyleRule(const CSSStyleRule* rule)
{
    for(auto& selector : rule->selectors()) {
        uint32_t specificity = 0;
        const CSSSimpleSelector* lastSimpleSelector = nullptr;
        for(auto& complexSelector : selector) {
            for(auto& simpleSelector : complexSelector.compoundSelector()) {
                lastSimpleSelector = &simpleSelector;
                switch(simpleSelector.matchType()) {
                case CSSSimpleSelector::MatchType::Id:
                    specificity += 0x10000;
                    break;
                case CSSSimpleSelector::MatchType::Class:
                    specificity += 0x100;
                    break;
                case CSSSimpleSelector::MatchType::Tag:
                    specificity += 0x1;
                    break;
                default:
                    break;
                }
            }
        }

        CSSRuleData ruleData(selector, rule->properties(), specificity, m_rules.size());
        switch(lastSimpleSelector->matchType()) {
        case CSSSimpleSelector::MatchType::Id:
            m_idRules.add(ruleData, lastSimpleSelector->name());
            break;
        case CSSSimpleSelector::MatchType::Class:
            m_classRules.add(ruleData, lastSimpleSelector->name());
            break;
        case CSSSimpleSelector::MatchType::Tag:
            m_tagRules.add(ruleData, lastSimpleSelector->name());
            break;
        case CSSSimpleSelector::MatchType::PseudoElementBefore:
            m_beforeElementRules.push_back(ruleData);
            break;
        case CSSSimpleSelector::MatchType::PseudoElementAfter:
            m_afterElementRules.push_back(ruleData);
            break;
        case CSSSimpleSelector::MatchType::PseudoElementMarker:
            m_markerElementRules.push_back(ruleData);
            break;
        case CSSSimpleSelector::MatchType::PseudoElementFirstLetter:
            m_firstLetterRules.push_back(ruleData);
            break;
        case CSSSimpleSelector::MatchType::PseudoElementFirstLine:
            m_firstLineRules.push_back(ruleData);
            break;
        default:
            m_universeRules.push_back(ruleData);
            break;
        }
    }
}

void CSSStyleSheet::addPageRule(const CSSPageRule* rule)
{
    for(auto& selector : rule->selectors()) {
        uint32_t specificity = 0;
        for(auto& sel : selector) {
            switch(sel.matchType()) {
            case CSSSimpleSelector::MatchType::Tag:
                specificity += 0x10000;
                break;
            case CSSSimpleSelector::MatchType::PseudoPageFirst:
                specificity += 0x100;
                break;
            case CSSSimpleSelector::MatchType::PseudoPageLeft:
            case CSSSimpleSelector::MatchType::PseudoPageRight:
            case CSSSimpleSelector::MatchType::PseudoPageBlank:
                specificity += 0x1;
                break;
            default:
                assert(false);
            }
        }

        CSSPageRuleData ruleData(rule, selector, specificity, m_rules.size());
        m_pageRules.push_back(ruleData);
    }
}

void CSSStyleSheet::addImportRule(const CSSImportRule* rule)
{
}

void CSSStyleSheet::addFontFaceRule(const CSSFontFaceRule* rule)
{
}

} // namespace htmlbook
