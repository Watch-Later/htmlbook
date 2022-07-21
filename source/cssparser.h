#ifndef CSSPARSER_H
#define CSSPARSER_H

#include "csstokenizer.h"
#include "cssstylesheet.h"

namespace htmlbook {

class CSSParser {
public:
    static void parseSheet(CSSStyleSheet* sheet, const std::string_view& content);
    static void parseStyle(CSSPropertyList& properties, const std::string_view& content);

private:
    static std::unique_ptr<CSSRule> consumeRule(CSSTokenStream& input);
    static std::unique_ptr<CSSRule> consumeStyleRule(CSSTokenStream& input);
    static std::unique_ptr<CSSRule> consumeAtRule(CSSTokenStream& input);
    static std::unique_ptr<CSSRule> consumeImportRule(CSSTokenStream& input);
    static std::unique_ptr<CSSRule> consumeFontFaceRule(CSSTokenStream& prelude, CSSTokenStream& block);
    static std::unique_ptr<CSSRule> consumePageRule(CSSTokenStream& prelude, CSSTokenStream& block);
    static std::unique_ptr<CSSPageMarginRule> consumePageMarginRule(CSSTokenStream& input);

    static bool consumePageSelectorList(CSSTokenStream& input, CSSPageSelectorList& selectors);
    static bool consumePageSelector(CSSTokenStream& input, CSSPageSelector& selector);
    static bool consumeSelectorList(CSSTokenStream& input, CSSSelectorList& selectors);
    static bool consumeSelector(CSSTokenStream& input, CSSSelector& selector);
    static bool consumeCompoundSelectorList(CSSTokenStream& input, CSSCompoundSelectorList& selectors);
    static bool consumeCompoundSelector(CSSTokenStream& input, CSSCompoundSelector& selector);
    static bool consumeCombinator(CSSTokenStream& input, CSSComplexSelector::Combinator& combinator);
    static bool consumeTagSelector(CSSTokenStream& input, CSSCompoundSelector& selector);
    static bool consumeIdSelector(CSSTokenStream& input, CSSCompoundSelector& selector);
    static bool consumeClassSelector(CSSTokenStream& input, CSSCompoundSelector& selector);
    static bool consumeAttributeSelector(CSSTokenStream& input, CSSCompoundSelector& selector);
    static bool consumeMatchPattern(CSSTokenStream& input, CSSSimpleSelector::MatchPattern& pattern);
    static bool consumePseudoSelector(CSSTokenStream& input, CSSCompoundSelector& selector);
    static bool consumeSimpleSelector(CSSTokenStream& input, CSSCompoundSelector& selector);

    static void consumeDeclaractionList(CSSTokenStream& input, CSSPropertyList& properties);
    static bool consumeDeclaraction(CSSTokenStream& input, CSSPropertyList& properties);
    static bool consumeDeclaractionValue(CSSTokenStream& input, CSSPropertyList& properties, CSSPropertyID id, bool important);

    static void addProperty(CSSPropertyList& properties, CSSPropertyID id, bool important, RefPtr<CSSValue> value);
    static void addExpandedProperty(CSSPropertyList& properties, CSSPropertyID id, bool important, RefPtr<CSSValue> value);

    static RefPtr<CSSValue> consumeNone(CSSTokenStream& input);
    static RefPtr<CSSValue> consumeAuto(CSSTokenStream& input);
    static RefPtr<CSSValue> consumeNormal(CSSTokenStream& input);
    static RefPtr<CSSValue> consumeNoneOrAuto(CSSTokenStream& input);
    static RefPtr<CSSValue> consumeNoneOrNormal(CSSTokenStream& input);

    static RefPtr<CSSValue> consumeInteger(CSSTokenStream& input, bool negative);
    static RefPtr<CSSValue> consumePositiveInteger(CSSTokenStream& input);
    static RefPtr<CSSValue> consumePercent(CSSTokenStream& input, bool negative);
    static RefPtr<CSSValue> consumeNumber(CSSTokenStream& input, bool negative);
    static RefPtr<CSSValue> consumeLength(CSSTokenStream& input, bool negative, bool unitless);
    static RefPtr<CSSValue> consumeLengthOrAuto(CSSTokenStream& input, bool negative, bool unitless);
    static RefPtr<CSSValue> consumeLengthOrNormal(CSSTokenStream& input, bool negative, bool unitless);
    static RefPtr<CSSValue> consumeLengthOrPercent(CSSTokenStream& input, bool negative, bool unitless);
    static RefPtr<CSSValue> consumeNumberOrPercent(CSSTokenStream& input, bool negative);
    static RefPtr<CSSValue> consumeIntegerOrAuto(CSSTokenStream& input, bool negative);
    static RefPtr<CSSValue> consumePositiveIntegerOrAuto(CSSTokenStream& input);
    static RefPtr<CSSValue> consumeLengthOrPercentOrAuto(CSSTokenStream& input, bool negative, bool unitless);
    static RefPtr<CSSValue> consumeLengthOrPercentOrNone(CSSTokenStream& input, bool negative, bool unitless);
    static RefPtr<CSSValue> consumeLengthOrPercentOrNormal(CSSTokenStream& input, bool negative, bool unitless);

    static RefPtr<CSSValue> consumeString(CSSTokenStream& input);
    static RefPtr<CSSValue> consumeCustomIdent(CSSTokenStream& input);
    static RefPtr<CSSValue> consumeUrl(CSSTokenStream& input, bool image);
    static RefPtr<CSSValue> consumeUrlOrNone(CSSTokenStream& input, bool image);
    static RefPtr<CSSValue> consumeColor(CSSTokenStream& input);
    static RefPtr<CSSValue> consumeRgb(CSSTokenStream& input);

    static RefPtr<CSSValue> consumeFillOrStroke(CSSTokenStream& input);
    static RefPtr<CSSValue> consumeQuotes(CSSTokenStream& input);
    static RefPtr<CSSValue> consumeContent(CSSTokenStream& input);
    static RefPtr<CSSValue> consumeContentAttr(CSSTokenStream& input);
    static RefPtr<CSSValue> consumeContentCounter(CSSTokenStream& input, bool counters);
    static RefPtr<CSSValue> consumeCounter(CSSTokenStream& input, bool increment);
    static RefPtr<CSSValue> consumePage(CSSTokenStream& input);
    static RefPtr<CSSValue> consumeSize(CSSTokenStream& input);
    static RefPtr<CSSValue> consumeFontWeight(CSSTokenStream& input);
    static RefPtr<CSSValue> consumeFontSize(CSSTokenStream& input, bool unitless);
    static RefPtr<CSSValue> consumeFontFamilyName(CSSTokenStream& input);
    static RefPtr<CSSValue> consumeFontFamilyValue(CSSTokenStream& input);
    static RefPtr<CSSValue> consumeFontFamily(CSSTokenStream& input);
    static RefPtr<CSSValue> consumeFontFaceSourceValue(CSSTokenStream& input);
    static RefPtr<CSSValue> consumeFontFaceSource(CSSTokenStream& input);
    static RefPtr<CSSValue> consumeLineWidth(CSSTokenStream& input, bool unitless);
    static RefPtr<CSSValue> consumeBorderRadiusValue(CSSTokenStream& input);
    static RefPtr<CSSValue> consumeClip(CSSTokenStream& input);
    static RefPtr<CSSValue> consumeDashList(CSSTokenStream& input);
    static RefPtr<CSSValue> consumeVerticalAlign(CSSTokenStream& input);
    static RefPtr<CSSValue> consumeTextDecorationLine(CSSTokenStream& input);
    static RefPtr<CSSValue> consumeBackgroundPosition(CSSTokenStream& input);
    static RefPtr<CSSValue> consumeBackgroundSize(CSSTokenStream& input);
    static RefPtr<CSSValue> consumeAngle(CSSTokenStream& input);
    static RefPtr<CSSValue> consumeTransformValue(CSSTokenStream& input);
    static RefPtr<CSSValue> consumeTransformList(CSSTokenStream& input);
    static RefPtr<CSSValue> consumePaintOrder(CSSTokenStream& input);
    static RefPtr<CSSValue> consumeLonghand(CSSTokenStream& input, CSSPropertyID id);

    static bool consumeFlex(CSSTokenStream& input, CSSPropertyList& properties, bool important);
    static bool consumeBackground(CSSTokenStream& input, CSSPropertyList& properties, bool important);
    static bool consumeColumns(CSSTokenStream& input, CSSPropertyList& properties, bool important);
    static bool consumeFont(CSSTokenStream& input, CSSPropertyList& properties, bool important);
    static bool consumeBorder(CSSTokenStream& input, CSSPropertyList& properties, bool important);
    static bool consumeBorderRadius(CSSTokenStream& input, CSSPropertyList& properties, bool important);
    static bool consume2Shorthand(CSSTokenStream& input, CSSPropertyList& properties, CSSPropertyID id, bool important);
    static bool consume4Shorthand(CSSTokenStream& input, CSSPropertyList& properties, CSSPropertyID id, bool important);
    static bool consumeShorthand(CSSTokenStream& input, CSSPropertyList& properties, CSSPropertyID id, bool important);
};

} // namespace htmlbook

#endif // CSSPARSER_H
