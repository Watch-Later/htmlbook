#include "cssrule.h"
#include "cssparser.h"
#include "document.h"
#include "resource.h"
#include "boxstyle.h"

namespace htmlbook {

RefPtr<CSSInitialValue> CSSInitialValue::create(Heap* heap)
{
    return adoptPtr(new (heap) CSSInitialValue);
}

RefPtr<CSSInheritValue> CSSInheritValue::create(Heap* heap)
{
    return adoptPtr(new (heap) CSSInheritValue);
}

RefPtr<CSSIdentValue> CSSIdentValue::create(Heap* heap, CSSValueID value)
{
    return adoptPtr(new (heap) CSSIdentValue(value));
}

RefPtr<CSSCustomIdentValue> CSSCustomIdentValue::create(Heap* heap, const HeapString& value)
{
    return adoptPtr(new (heap) CSSCustomIdentValue(value));
}

RefPtr<CSSIntegerValue> CSSIntegerValue::create(Heap* heap, int value)
{
    return adoptPtr(new (heap) CSSIntegerValue(value));
}

RefPtr<CSSNumberValue> CSSNumberValue::create(Heap* heap, double value)
{
    return adoptPtr(new (heap) CSSNumberValue(value));
}

RefPtr<CSSPercentValue> CSSPercentValue::create(Heap* heap, double value)
{
    return adoptPtr(new (heap) CSSPercentValue(value));
}

RefPtr<CSSAngleValue> CSSAngleValue::create(Heap* heap, double value, Unit unit)
{
    return adoptPtr(new (heap) CSSAngleValue(value, unit));
}

RefPtr<CSSLengthValue> CSSLengthValue::create(Heap* heap, double value, Unit unit)
{
    return adoptPtr(new (heap) CSSLengthValue(value, unit));
}

RefPtr<CSSStringValue> CSSStringValue::create(Heap* heap, const HeapString& value)
{
    return adoptPtr(new (heap) CSSStringValue(value));
}

RefPtr<CSSUrlValue> CSSUrlValue::create(Heap* heap, const HeapString& value)
{
    return adoptPtr(new (heap) CSSUrlValue(value));
}

RefPtr<CSSImageValue> CSSImageValue::create(Heap* heap, const HeapString& value)
{
    return adoptPtr(new (heap) CSSImageValue(value));
}

RefPtr<Image> CSSImageValue::fetch(Document* document) const
{
    if(m_image == nullptr) {
        auto imageResource = document->fetchImageResource(m_value);
        if(imageResource == nullptr)
            return nullptr;
        m_image = imageResource->image();
    }

    return m_image;
}

CSSImageValue::CSSImageValue(const HeapString& value)
    : m_value(value)
{
}

RefPtr<CSSColorValue> CSSColorValue::create(Heap* heap, uint32_t value)
{
    return adoptPtr(new (heap) CSSColorValue(value));
}

RefPtr<CSSColorValue> CSSColorValue::create(Heap* heap, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    return adoptPtr(new (heap) CSSColorValue(a << 24 | r << 16 | g << 8 | b));
}

RefPtr<CSSCounterValue> CSSCounterValue::create(Heap* heap, const GlobalString& identifier, ListStyleType listStyle, const HeapString& separator)
{
    return adoptPtr(new (heap) CSSCounterValue(identifier, listStyle, separator));
}

RefPtr<CSSPairValue> CSSPairValue::create(Heap* heap, RefPtr<CSSValue> first, RefPtr<CSSValue> second)
{
    return adoptPtr(new (heap) CSSPairValue(std::move(first), std::move(second)));
}

RefPtr<CSSRectValue> CSSRectValue::create(Heap* heap, RefPtr<CSSValue> top, RefPtr<CSSValue> right, RefPtr<CSSValue> bottom, RefPtr<CSSValue> left)
{
    return adoptPtr(new (heap) CSSRectValue(std::move(top), std::move(right), std::move(bottom), std::move(left)));
}

RefPtr<CSSListValue> CSSListValue::create(Heap* heap, CSSValueList values)
{
    return adoptPtr(new (heap) CSSListValue(std::move(values)));
}

RefPtr<CSSFunctionValue> CSSFunctionValue::create(Heap* heap, CSSValueID id, CSSValueList values)
{
    return adoptPtr(new (heap) CSSFunctionValue(id, std::move(values)));
}

RefPtr<CSSFunctionValue> CSSFunctionValue::create(Heap* heap, CSSValueID id, RefPtr<CSSValue> value)
{
    CSSValueList values(heap);
    values.push_back(std::move(value));
    return adoptPtr(new (heap) CSSFunctionValue(id, std::move(values)));;
}

CSSShorthand CSSShorthand::longhand(CSSPropertyID id)
{
    switch(id) {
    case CSSPropertyID::BorderColor: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::BorderTopColor,
            CSSPropertyID::BorderRightColor,
            CSSPropertyID::BorderBottomColor,
            CSSPropertyID::BorderLeftColor
        };

        return CSSShorthand(data, std::size(data));
    }

    case CSSPropertyID::BorderStyle: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::BorderTopStyle,
            CSSPropertyID::BorderRightStyle,
            CSSPropertyID::BorderBottomStyle,
            CSSPropertyID::BorderLeftStyle
        };

        return CSSShorthand(data, std::size(data));
    }

    case CSSPropertyID::BorderWidth: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::BorderTopWidth,
            CSSPropertyID::BorderRightWidth,
            CSSPropertyID::BorderBottomWidth,
            CSSPropertyID::BorderLeftWidth
        };

        return CSSShorthand(data, std::size(data));
    }

    case CSSPropertyID::BorderTop: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::BorderTopColor,
            CSSPropertyID::BorderTopStyle,
            CSSPropertyID::BorderTopWidth
        };

        return CSSShorthand(data, std::size(data));
    }

    case CSSPropertyID::BorderRight: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::BorderRightColor,
            CSSPropertyID::BorderRightStyle,
            CSSPropertyID::BorderRightWidth
        };

        return CSSShorthand(data, std::size(data));
    }

    case CSSPropertyID::BorderBottom: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::BorderBottomColor,
            CSSPropertyID::BorderBottomStyle,
            CSSPropertyID::BorderBottomWidth
        };

        return CSSShorthand(data, std::size(data));
    }

    case CSSPropertyID::BorderLeft: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::BorderLeftColor,
            CSSPropertyID::BorderLeftStyle,
            CSSPropertyID::BorderLeftWidth
        };

        return CSSShorthand(data, std::size(data));
    }

    case CSSPropertyID::BorderRadius: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::BorderTopRightRadius,
            CSSPropertyID::BorderTopLeftRadius,
            CSSPropertyID::BorderBottomLeftRadius,
            CSSPropertyID::BorderBottomRightRadius
        };

        return CSSShorthand(data, std::size(data));
    }

    case CSSPropertyID::BorderSpacing: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::BorderHorizontalSpacing,
            CSSPropertyID::BorderVerticalSpacing
        };

        return CSSShorthand(data, std::size(data));
    }

    case CSSPropertyID::Padding: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::PaddingTop,
            CSSPropertyID::PaddingRight,
            CSSPropertyID::PaddingBottom,
            CSSPropertyID::PaddingLeft
        };

        return CSSShorthand(data, std::size(data));
    }

    case CSSPropertyID::Margin: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::MarginTop,
            CSSPropertyID::MarginRight,
            CSSPropertyID::MarginBottom,
            CSSPropertyID::MarginLeft
        };

        return CSSShorthand(data, std::size(data));
    }

    case CSSPropertyID::Outline: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::OutlineColor,
            CSSPropertyID::OutlineOffset,
            CSSPropertyID::OutlineStyle,
            CSSPropertyID::OutlineWidth
        };

        return CSSShorthand(data, std::size(data));
    }

    case CSSPropertyID::ListStyle: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::ListStyleImage,
            CSSPropertyID::ListStylePosition,
            CSSPropertyID::ListStyleType
        };

        return CSSShorthand(data, std::size(data));
    }

    case CSSPropertyID::ColumnRule: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::ColumnRuleColor,
            CSSPropertyID::ColumnRuleStyle,
            CSSPropertyID::ColumnRuleWidth
        };

        return CSSShorthand(data, std::size(data));
    }

    case CSSPropertyID::FlexFlow: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::FlexDirection,
            CSSPropertyID::FlexWrap
        };

        return CSSShorthand(data, std::size(data));
    }

    case CSSPropertyID::Flex: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::FlexGrow,
            CSSPropertyID::FlexShrink,
            CSSPropertyID::FlexBasis
        };

        return CSSShorthand(data, std::size(data));
    }

    case CSSPropertyID::Background: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::BackgroundColor,
            CSSPropertyID::BackgroundImage,
            CSSPropertyID::BackgroundRepeat,
            CSSPropertyID::BackgroundAttachment,
            CSSPropertyID::BackgroundOrigin,
            CSSPropertyID::BackgroundClip,
            CSSPropertyID::BackgroundPosition,
            CSSPropertyID::BackgroundSize
        };

        return CSSShorthand(data, std::size(data));
    }

    case CSSPropertyID::Columns: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::ColumnWidth,
            CSSPropertyID::ColumnCount
        };

        return CSSShorthand(data, std::size(data));
    }

    case CSSPropertyID::Font: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::FontStyle,
            CSSPropertyID::FontVariant,
            CSSPropertyID::FontWeight,
            CSSPropertyID::FontSize,
            CSSPropertyID::LineHeight,
            CSSPropertyID::FontFamily
        };

        return CSSShorthand(data, std::size(data));
    }

    case CSSPropertyID::Border: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::BorderTopWidth,
            CSSPropertyID::BorderRightWidth,
            CSSPropertyID::BorderBottomWidth,
            CSSPropertyID::BorderLeftWidth,
            CSSPropertyID::BorderTopStyle,
            CSSPropertyID::BorderRightStyle,
            CSSPropertyID::BorderBottomStyle,
            CSSPropertyID::BorderLeftStyle,
            CSSPropertyID::BorderTopColor,
            CSSPropertyID::BorderRightColor,
            CSSPropertyID::BorderBottomColor,
            CSSPropertyID::BorderLeftColor
        };

        return CSSShorthand(data, std::size(data));
    }

    case CSSPropertyID::Overflow: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::OverflowX,
            CSSPropertyID::OverflowY
        };

        return CSSShorthand(data, std::size(data));
    }

    case CSSPropertyID::TextDecoration: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::TextDecorationLine,
            CSSPropertyID::TextDecorationStyle,
            CSSPropertyID::TextDecorationColor
        };

        return CSSShorthand(data, std::size(data));
    }

    default:
        return CSSShorthand(nullptr, 0);
    }
}

CSSPropertyID csspropertyid(const std::string_view& name)
{
    static const struct {
        std::string_view name;
        CSSPropertyID value;
    } table[] = {
        {"align-content", CSSPropertyID::AlignContent},
        {"align-items", CSSPropertyID::AlignItems},
        {"align-self", CSSPropertyID::AlignSelf},
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
        {"border-horizontal-spacing", CSSPropertyID::BorderHorizontalSpacing},
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
        {"border-vertical-spacing", CSSPropertyID::BorderVerticalSpacing},
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
        {"column-fill", CSSPropertyID::ColumnFill},
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
        return it->value;
    return CSSPropertyID::Unknown;
}

bool CSSSimpleSelector::matchnth(int count) const
{
    auto [a, b] = m_matchPattern;
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

RefPtr<CSSStyleRule> CSSStyleRule::create(Heap* heap, CSSSelectorList selectors, CSSPropertyList properties)
{
    return adoptPtr(new (heap) CSSStyleRule(std::move(selectors), std::move(properties)));
}

RefPtr<CSSImportRule> CSSImportRule::create(Heap* heap, const HeapString& href)
{
    return adoptPtr(new (heap) CSSImportRule(heap, href));
}

const CSSRuleList& CSSImportRule::fetch(Document* document) const
{
    if(!m_rules.empty())
        return m_rules;
    if(auto textResource = document->fetchTextResource(m_href)) {
        CSSParser parser(m_heap);
        parser.parseSheet(m_rules, textResource->text());
    }

    return m_rules;
}

RefPtr<CSSFontFaceRule> CSSFontFaceRule::create(Heap* heap, CSSPropertyList properties)
{
    return adoptPtr(new (heap) CSSFontFaceRule(std::move(properties)));
}

RefPtr<CSSPageMarginRule> CSSPageMarginRule::create(Heap* heap, MarginType marginType, CSSPropertyList properties)
{
    return adoptPtr(new (heap) CSSPageMarginRule(marginType, std::move(properties)));
}

RefPtr<CSSPageRule> CSSPageRule::create(Heap* heap, CSSPageSelectorList selectors, CSSPageMarginRuleList margins, CSSPropertyList properties)
{
    return adoptPtr(new (heap) CSSPageRule(std::move(selectors), std::move(margins), std::move(properties)));
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
    return element->tagName() == aTag && element->hasAttribute(hrefAttr);
}

bool CSSRuleData::matchPseudoClassEnabledSelector(const Element* element, const CSSSimpleSelector& selector)
{
    return element->tagName() == inputTag && element->hasAttribute(enabledAttr);
}

bool CSSRuleData::matchPseudoClassDisabledSelector(const Element* element, const CSSSimpleSelector& selector)
{
    return element->tagName() == inputTag && element->hasAttribute(disabledAttr);
}

bool CSSRuleData::matchPseudoClassCheckedSelector(const Element* element, const CSSSimpleSelector& selector)
{
    return element->tagName() == inputTag && element->hasAttribute(checkedAttr);
}

bool CSSRuleData::matchPseudoClassLangSelector(const Element* element, const CSSSimpleSelector& selector)
{
    return dashequals(element->lang(), selector.name(), false);
}

bool CSSRuleData::matchPseudoClassRootSelector(const Element* element, const CSSSimpleSelector& selector)
{
    return !element->parentElement();
}

bool CSSRuleData::matchPseudoClassEmptySelector(const Element* element, const CSSSimpleSelector& selector)
{
    return !element->firstChild();
}

bool CSSRuleData::matchPseudoClassFirstChildSelector(const Element* element, const CSSSimpleSelector& selector)
{
    return !element->previousElement();
}

bool CSSRuleData::matchPseudoClassLastChildSelector(const Element* element, const CSSSimpleSelector& selector)
{
    return !element->nextElement();
}

bool CSSRuleData::matchPseudoClassOnlyChildSelector(const Element* element, const CSSSimpleSelector& selector)
{
    return matchPseudoClassFirstChildSelector(element, selector) && matchPseudoClassLastChildSelector(element, selector);
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
    return matchPseudoClassFirstOfTypeSelector(element, selector) && matchPseudoClassLastOfTypeSelector(element, selector);
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

RefPtr<FontFace> CSSFontFaceCache::get(const std::string_view& family, bool italic, bool smallCaps, int weight) const
{
    return nullptr;
}

void CSSFontFaceCache::add(const HeapString& family, bool italic, bool smallCaps, int weight, RefPtr<FontFace> face)
{
    m_fontFaceDataMap[family].emplace_back(italic, smallCaps, weight, std::move(face));
}

static const CSSRuleList& userAgentRules() {
    static CSSRuleList userAgentRules;
    return userAgentRules;
}

CSSStyleSheet::CSSStyleSheet(Document* document)
    : m_document(document)
{
    addRules(userAgentRules());
}

RefPtr<BoxStyle> CSSStyleSheet::styleForElement(Element* element, const RefPtr<BoxStyle>& parentStyle) const
{
    CSSStyleBuilder builder(element, parentStyle, PseudoType::None);
    builder.add(m_idRules.get(element->id()));
    for(auto& className : element->classNames())
        builder.add(m_classRules.get(className));
    builder.add(m_tagRules.get(element->tagName()));
    builder.add(&m_universeRules);
    return builder.build();
}

RefPtr<BoxStyle> CSSStyleSheet::pseudoStyleForElement(Element* element, const RefPtr<BoxStyle>& parentStyle, PseudoType pseudoType) const
{
    CSSStyleBuilder builder(element, parentStyle, pseudoType);
    builder.add(m_pseudoRules.get(pseudoType));
    return builder.build();
}

RefPtr<FontFace> CSSStyleSheet::getFontFace(const std::string_view& family, bool italic, bool smallCaps, int weight) const
{
    if(auto face = m_fontFaceCache.get(family, italic, smallCaps, weight))
        return face;
    return resourceLoader()->loadFont(family, italic, smallCaps, weight);
}

void CSSStyleSheet::parseStyle(const std::string_view& content)
{
    CSSRuleList rules(m_document->heap());
    CSSParser parser(m_document->heap());
    parser.parseSheet(rules, content);
    addRules(rules);
}

void CSSStyleSheet::addRules(const CSSRuleList& rules)
{
    for(const auto& rule : rules) {
        if(auto styleRule = to<CSSStyleRule>(*rule)) {
            addStyleRule(styleRule);
        } else if(auto pageRule = to<CSSPageRule>(*rule)) {
            addPageRule(pageRule);
        } else if(auto fontFaceRule = to<CSSFontFaceRule>(*rule)) {
            addFontFaceRule(fontFaceRule);
        } else if(auto importRule = to<CSSImportRule>(*rule)) {
            addRules(importRule->fetch(m_document));
        } else {
            assert(false);
        }

        m_position += 1;
    }
}

void CSSStyleSheet::addStyleRule(const RefPtr<CSSStyleRule>& rule)
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

        CSSRuleData ruleData(rule, &selector, specificity, m_position);
        assert(lastSimpleSelector != nullptr);
        switch(lastSimpleSelector->matchType()) {
        case CSSSimpleSelector::MatchType::Id:
            m_idRules.add(lastSimpleSelector->value(), ruleData);
            break;
        case CSSSimpleSelector::MatchType::Class:
            m_classRules.add(lastSimpleSelector->value(), ruleData);
            break;
        case CSSSimpleSelector::MatchType::Tag:
            m_tagRules.add(lastSimpleSelector->name(), ruleData);
            break;
        case CSSSimpleSelector::MatchType::PseudoElementBefore:
            m_pseudoRules.add(PseudoType::Before, ruleData);
            break;
        case CSSSimpleSelector::MatchType::PseudoElementAfter:
            m_pseudoRules.add(PseudoType::After, ruleData);
            break;
        case CSSSimpleSelector::MatchType::PseudoElementMarker:
            m_pseudoRules.add(PseudoType::Marker, ruleData);
            break;
        case CSSSimpleSelector::MatchType::PseudoElementFirstLetter:
            m_pseudoRules.add(PseudoType::FirstLetter, ruleData);
            break;
        case CSSSimpleSelector::MatchType::PseudoElementFirstLine:
            m_pseudoRules.add(PseudoType::FirstLine, ruleData);
            break;
        default:
            m_universeRules.push_back(ruleData);
            break;
        }
    }
}

void CSSStyleSheet::addPageRule(const RefPtr<CSSPageRule>& rule)
{
    for(auto& selector : rule->selectors()) {
        uint32_t specificity = 0;
        for(auto& sel : selector) {
            switch(sel.matchType()) {
            case CSSSimpleSelector::MatchType::Id:
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

        CSSPageRuleData ruleData(rule, &selector, specificity, m_position);
        m_pageRules.insert(ruleData);
    }
}

void CSSStyleSheet::addFontFaceRule(const RefPtr<CSSFontFaceRule>& rule)
{
    RefPtr<CSSValue> fontFamily;
    RefPtr<CSSValue> fontStyle;
    RefPtr<CSSValue> fontVariant;
    RefPtr<CSSValue> fontWeight;
    RefPtr<CSSValue> src;
    for(auto& property : rule->properties()) {
        switch(property.id()) {
        case CSSPropertyID::FontFamily:
            fontFamily = property.value();
            break;
        case CSSPropertyID::FontStyle:
            fontStyle = property.value();
            break;
        case CSSPropertyID::FontVariant:
            fontVariant = property.value();
            break;
        case CSSPropertyID::FontWeight:
            fontWeight = property.value();
            break;
        case CSSPropertyID::Src:
            src = property.value();
            break;
        default:
            break;
        }
    }

    if(fontFamily == nullptr || !fontFamily->isListValue())
        return;
    if(src == nullptr || !src->isListValue())
        return;

    bool italic = false;
    if(fontStyle && fontStyle->isIdentValue()) {
        auto ident = to<CSSIdentValue>(*fontStyle);
        switch(ident->value()) {
        case CSSValueID::Normal:
            italic = false;
            break;
        case CSSValueID::Italic:
        case CSSValueID::Oblique:
            italic = true;
            break;
        default:
            assert(false);
        }
    }

    bool smallCaps = false;
    if(fontVariant && fontVariant->isIdentValue()) {
        auto ident = to<CSSIdentValue>(*fontVariant);
        switch(ident->value()) {
        case CSSValueID::Normal:
            smallCaps = false;
            break;
        case CSSValueID::SmallCaps:
            smallCaps = true;
            break;
        default:
            assert(false);
        }
    }

    int weight = 400;
    if(fontWeight) {
        if(fontWeight->isIdentValue()) {
            auto ident = to<CSSIdentValue>(*fontWeight);
            switch(ident->value()) {
            case CSSValueID::Normal:
            case CSSValueID::Lighter:
                weight = 400;
                break;
            case CSSValueID::Bold:
            case CSSValueID::Bolder:
                weight = 700;
                break;
            default:
                assert(false);
            }
        } else if(fontWeight->isIntegerValue()) {
            auto integer = to<CSSIntegerValue>(*fontWeight);
            weight = integer->value();
        }
    }

    auto fetch = [&](auto source) -> RefPtr<FontFace> {
        if(auto function = to<CSSFunctionValue>(*source->front())) {
            auto family = to<CSSStringValue>(*function->front());
            return resourceLoader()->loadFont(family->value(), italic, smallCaps, weight);
        }

        auto url = to<CSSUrlValue>(*source->front());
        if(source->size() == 2) {
            auto function = to<CSSFunctionValue>(*source->back());
            auto format = to<CSSStringValue>(*function->front());
            if(!equals(format->value(), "truetype", false) && !equals(format->value(), "opentype", false)) {
                return nullptr;
            }
        }

        auto fontResource = m_document->fetchFontResource(url->value());
        if(fontResource == nullptr)
            return nullptr;
        return fontResource->face();
    };

    for(auto& value : to<CSSListValue>(*src)->values()) {
        auto face = fetch(to<CSSListValue>(*value));
        if(face == nullptr)
            continue;
        for(auto& value : to<CSSListValue>(*fontFamily)->values()) {
            auto family = to<CSSStringValue>(*value);
            m_fontFaceCache.add(family->value(), italic, smallCaps, weight, face);
        }
    }
}

CSSStyleBuilder::CSSStyleBuilder(Element* element, const RefPtr<BoxStyle>& parentStyle, PseudoType pseudoType)
    : m_pseudoType(pseudoType), m_element(element), m_parentStyle(parentStyle)
{
}

void CSSStyleBuilder::add(const CSSRuleDataList* rules)
{
    if(rules == nullptr)
        return;
    for(auto& rule : *rules) {
        if(!rule.match(m_element, m_pseudoType))
            continue;
        m_rules.push_back(rule);
    }
}

void CSSStyleBuilder::add(const CSSPropertyList& properties)
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

RefPtr<BoxStyle> CSSStyleBuilder::build()
{
    std::sort(m_rules.begin(), m_rules.end());
    for(auto& rule : m_rules)
        add(rule.properties());
    if(m_pseudoType == PseudoType::None) {
        add(m_element->inlineStyle());
        add(m_element->presentationAttributeStyle());
    }

    if(m_properties.empty()) {
        if(m_pseudoType == PseudoType::None)
            return BoxStyle::create(m_parentStyle, Display::Inline);
        return nullptr;
    }

    auto newStyle = BoxStyle::create(m_element, m_pseudoType, Display::Inline);
    newStyle->inheritFrom(*m_parentStyle);
    for(auto& property : m_properties) {
        auto id = property.id();
        auto value = property.value();
        if(value->isInitialValue()) {
            newStyle->remove(id);
            continue;
        }

        if(value->isInheritValue() && !(value = m_parentStyle->get(id)))
            continue;
        newStyle->set(id, std::move(value));
    }

    return newStyle;
}

} // namespace htmlbook
