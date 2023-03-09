#ifndef CSSPARSER_H
#define CSSPARSER_H

#include "csstokenizer.h"
#include "cssrule.h"

namespace htmlbook {

class CSSParser {
public:
    explicit CSSParser(Heap* heap)
        : m_heap(heap)
    {}

    void parseSheet(CSSRuleList& rules, const std::string_view& content);
    void parseStyle(CSSPropertyList& properties, const std::string_view& content);

private:
    RefPtr<CSSRule> consumeRule(CSSTokenStream& input);
    RefPtr<CSSRule> consumeStyleRule(CSSTokenStream& input);
    RefPtr<CSSRule> consumeAtRule(CSSTokenStream& input);
    RefPtr<CSSRule> consumeImportRule(CSSTokenStream& input);
    RefPtr<CSSRule> consumeFontFaceRule(CSSTokenStream& prelude, CSSTokenStream& block);
    RefPtr<CSSRule> consumePageRule(CSSTokenStream& prelude, CSSTokenStream& block);
    RefPtr<CSSPageMarginRule> consumePageMarginRule(CSSTokenStream& input);

    bool consumePageSelector(CSSTokenStream& input, CSSPageSelector& selector);
    bool consumePageSelectorList(CSSTokenStream& input, CSSPageSelectorList& selectors);

    bool consumeSelectorList(CSSTokenStream& input, CSSSelectorList& selectors);
    bool consumeCompoundSelectorList(CSSTokenStream& input, CSSCompoundSelectorList& selectors);

    bool consumeSelector(CSSTokenStream& input, CSSSelector& selector);
    bool consumeCompoundSelector(CSSTokenStream& input, CSSCompoundSelector& selector, bool& failed);
    bool consumeSimpleSelector(CSSTokenStream& input, CSSCompoundSelector& selector, bool& failed);

    bool consumeTagSelector(CSSTokenStream& input, CSSCompoundSelector& selector);
    bool consumeIdSelector(CSSTokenStream& input, CSSCompoundSelector& selector);
    bool consumeClassSelector(CSSTokenStream& input, CSSCompoundSelector& selector);
    bool consumeAttributeSelector(CSSTokenStream& input, CSSCompoundSelector& selector);
    bool consumePseudoSelector(CSSTokenStream& input, CSSCompoundSelector& selector);

    bool consumeCombinator(CSSTokenStream& input, CSSComplexSelector::Combinator& combinator);
    bool consumeMatchPattern(CSSTokenStream& input, CSSSimpleSelector::MatchPattern& pattern);

    void consumeDeclaractionList(CSSTokenStream& input, CSSPropertyList& properties);
    bool consumeDeclaraction(CSSTokenStream& input, CSSPropertyList& properties);
    bool consumeDeclaractionValue(CSSTokenStream& input, CSSPropertyList& properties, CSSPropertyID id, bool important);

    void addProperty(CSSPropertyList& properties, CSSPropertyID id, bool important, RefPtr<CSSValue> value);
    void addExpandedProperty(CSSPropertyList& properties, CSSPropertyID id, bool important, RefPtr<CSSValue> value);

    RefPtr<CSSValue> consumeNone(CSSTokenStream& input);
    RefPtr<CSSValue> consumeAuto(CSSTokenStream& input);
    RefPtr<CSSValue> consumeNormal(CSSTokenStream& input);
    RefPtr<CSSValue> consumeNoneOrAuto(CSSTokenStream& input);
    RefPtr<CSSValue> consumeNoneOrNormal(CSSTokenStream& input);

    RefPtr<CSSValue> consumeInteger(CSSTokenStream& input, bool negative);
    RefPtr<CSSValue> consumePositiveInteger(CSSTokenStream& input);
    RefPtr<CSSValue> consumePercent(CSSTokenStream& input, bool negative);
    RefPtr<CSSValue> consumeNumber(CSSTokenStream& input, bool negative);
    RefPtr<CSSValue> consumeLength(CSSTokenStream& input, bool negative, bool unitless);
    RefPtr<CSSValue> consumeLengthOrAuto(CSSTokenStream& input, bool negative, bool unitless);
    RefPtr<CSSValue> consumeLengthOrNormal(CSSTokenStream& input, bool negative, bool unitless);
    RefPtr<CSSValue> consumeLengthOrPercent(CSSTokenStream& input, bool negative, bool unitless);
    RefPtr<CSSValue> consumeNumberOrPercent(CSSTokenStream& input, bool negative);
    RefPtr<CSSValue> consumeIntegerOrAuto(CSSTokenStream& input, bool negative);
    RefPtr<CSSValue> consumePositiveIntegerOrAuto(CSSTokenStream& input);
    RefPtr<CSSValue> consumeLengthOrPercentOrAuto(CSSTokenStream& input, bool negative, bool unitless);
    RefPtr<CSSValue> consumeLengthOrPercentOrNone(CSSTokenStream& input, bool negative, bool unitless);
    RefPtr<CSSValue> consumeLengthOrPercentOrNormal(CSSTokenStream& input, bool negative, bool unitless);

    RefPtr<CSSValue> consumeString(CSSTokenStream& input);
    RefPtr<CSSValue> consumeCustomIdent(CSSTokenStream& input);
    RefPtr<CSSValue> consumeUrl(CSSTokenStream& input, bool image);
    RefPtr<CSSValue> consumeUrlOrNone(CSSTokenStream& input, bool image);
    RefPtr<CSSValue> consumeColor(CSSTokenStream& input);
    RefPtr<CSSValue> consumeRgb(CSSTokenStream& input);

    RefPtr<CSSValue> consumeFillOrStroke(CSSTokenStream& input);
    RefPtr<CSSValue> consumeQuotes(CSSTokenStream& input);
    RefPtr<CSSValue> consumeContent(CSSTokenStream& input);
    RefPtr<CSSValue> consumeContentAttr(CSSTokenStream& input);
    RefPtr<CSSValue> consumeContentCounter(CSSTokenStream& input, bool counters);
    RefPtr<CSSValue> consumeCounter(CSSTokenStream& input, bool increment);
    RefPtr<CSSValue> consumePage(CSSTokenStream& input);
    RefPtr<CSSValue> consumeSize(CSSTokenStream& input);
    RefPtr<CSSValue> consumeFontWeight(CSSTokenStream& input);
    RefPtr<CSSValue> consumeFontSize(CSSTokenStream& input, bool unitless);
    RefPtr<CSSValue> consumeFontFamilyValue(CSSTokenStream& input);
    RefPtr<CSSValue> consumeFontFamily(CSSTokenStream& input);
    RefPtr<CSSValue> consumeFontFaceSourceValue(CSSTokenStream& input);
    RefPtr<CSSValue> consumeFontFaceSource(CSSTokenStream& input);
    RefPtr<CSSValue> consumeLineWidth(CSSTokenStream& input, bool unitless);
    RefPtr<CSSValue> consumeBorderRadiusValue(CSSTokenStream& input);
    RefPtr<CSSValue> consumeClip(CSSTokenStream& input);
    RefPtr<CSSValue> consumeDashList(CSSTokenStream& input);
    RefPtr<CSSValue> consumeVerticalAlign(CSSTokenStream& input);
    RefPtr<CSSValue> consumeTextDecorationLine(CSSTokenStream& input);
    RefPtr<CSSValue> consumeBackgroundPosition(CSSTokenStream& input);
    RefPtr<CSSValue> consumeBackgroundSize(CSSTokenStream& input);
    RefPtr<CSSValue> consumeAngle(CSSTokenStream& input);
    RefPtr<CSSValue> consumeTransformValue(CSSTokenStream& input);
    RefPtr<CSSValue> consumeTransform(CSSTokenStream& input);
    RefPtr<CSSValue> consumePaintOrder(CSSTokenStream& input);
    RefPtr<CSSValue> consumeLonghand(CSSTokenStream& input, CSSPropertyID id);

    bool consumeFlex(CSSTokenStream& input, CSSPropertyList& properties, bool important);
    bool consumeBackground(CSSTokenStream& input, CSSPropertyList& properties, bool important);
    bool consumeColumns(CSSTokenStream& input, CSSPropertyList& properties, bool important);
    bool consumeFont(CSSTokenStream& input, CSSPropertyList& properties, bool important);
    bool consumeBorder(CSSTokenStream& input, CSSPropertyList& properties, bool important);
    bool consumeBorderRadius(CSSTokenStream& input, CSSPropertyList& properties, bool important);
    bool consume2Shorthand(CSSTokenStream& input, CSSPropertyList& properties, CSSPropertyID id, bool important);
    bool consume4Shorthand(CSSTokenStream& input, CSSPropertyList& properties, CSSPropertyID id, bool important);
    bool consumeShorthand(CSSTokenStream& input, CSSPropertyList& properties, CSSPropertyID id, bool important);

private:
    Heap* m_heap;
};

} // namespace htmlbook

#endif // CSSPARSER_H
