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

    static void addProperty(CSSPropertyList& properties, CSSPropertyID id, bool important, std::shared_ptr<CSSValue> value);
    static void addExpandedProperty(CSSPropertyList& properties, CSSPropertyID id, bool important, std::shared_ptr<CSSValue> value);

    static std::shared_ptr<CSSValue> consumeNone(CSSTokenStream& input);
    static std::shared_ptr<CSSValue> consumeAuto(CSSTokenStream& input);
    static std::shared_ptr<CSSValue> consumeNormal(CSSTokenStream& input);
    static std::shared_ptr<CSSValue> consumeNoneOrAuto(CSSTokenStream& input);
    static std::shared_ptr<CSSValue> consumeNoneOrNormal(CSSTokenStream& input);

    static std::shared_ptr<CSSValue> consumeInteger(CSSTokenStream& input, bool negative);
    static std::shared_ptr<CSSValue> consumePositiveInteger(CSSTokenStream& input);
    static std::shared_ptr<CSSValue> consumePercent(CSSTokenStream& input, bool negative);
    static std::shared_ptr<CSSValue> consumeNumber(CSSTokenStream& input, bool negative);
    static std::shared_ptr<CSSValue> consumeLength(CSSTokenStream& input, bool negative, bool unitless);
    static std::shared_ptr<CSSValue> consumeLengthOrAuto(CSSTokenStream& input, bool negative, bool unitless);
    static std::shared_ptr<CSSValue> consumeLengthOrNormal(CSSTokenStream& input, bool negative, bool unitless);
    static std::shared_ptr<CSSValue> consumeLengthOrPercent(CSSTokenStream& input, bool negative, bool unitless);
    static std::shared_ptr<CSSValue> consumeNumberOrPercent(CSSTokenStream& input, bool negative);
    static std::shared_ptr<CSSValue> consumeIntegerOrAuto(CSSTokenStream& input, bool negative);
    static std::shared_ptr<CSSValue> consumePositiveIntegerOrAuto(CSSTokenStream& input);
    static std::shared_ptr<CSSValue> consumeLengthOrPercentOrAuto(CSSTokenStream& input, bool negative, bool unitless);
    static std::shared_ptr<CSSValue> consumeLengthOrPercentOrNone(CSSTokenStream& input, bool negative, bool unitless);
    static std::shared_ptr<CSSValue> consumeLengthOrPercentOrNormal(CSSTokenStream& input, bool negative, bool unitless);

    static std::shared_ptr<CSSValue> consumeString(CSSTokenStream& input);
    static std::shared_ptr<CSSValue> consumeCustomIdent(CSSTokenStream& input);
    static std::shared_ptr<CSSValue> consumeUrl(CSSTokenStream& input);
    static std::shared_ptr<CSSValue> consumeUrlOrNone(CSSTokenStream& input);
    static std::shared_ptr<CSSValue> consumeColor(CSSTokenStream& input);
    static std::shared_ptr<CSSValue> consumeRgb(CSSTokenStream& input);

    static std::shared_ptr<CSSValue> consumeFillOrStroke(CSSTokenStream& input);
    static std::shared_ptr<CSSValue> consumeQuotes(CSSTokenStream& input);
    static std::shared_ptr<CSSValue> consumeContent(CSSTokenStream& input);
    static std::shared_ptr<CSSValue> consumeContentAttr(CSSTokenStream& input);
    static std::shared_ptr<CSSValue> consumeContentCounter(CSSTokenStream& input, bool counters);
    static std::shared_ptr<CSSValue> consumeCounter(CSSTokenStream& input, bool increment);
    static std::shared_ptr<CSSValue> consumePage(CSSTokenStream& input);
    static std::shared_ptr<CSSValue> consumeSize(CSSTokenStream& input);
    static std::shared_ptr<CSSValue> consumeFontWeight(CSSTokenStream& input);
    static std::shared_ptr<CSSValue> consumeFontSize(CSSTokenStream& input, bool unitless);
    static std::shared_ptr<CSSValue> consumeFontFamilyName(CSSTokenStream& input);
    static std::shared_ptr<CSSValue> consumeFontFamilyValue(CSSTokenStream& input);
    static std::shared_ptr<CSSValue> consumeFontFamily(CSSTokenStream& input);
    static std::shared_ptr<CSSValue> consumeFontFaceSourceValue(CSSTokenStream& input);
    static std::shared_ptr<CSSValue> consumeFontFaceSource(CSSTokenStream& input);
    static std::shared_ptr<CSSValue> consumeLineWidth(CSSTokenStream& input, bool unitless);
    static std::shared_ptr<CSSValue> consumeBorderRadiusValue(CSSTokenStream& input);
    static std::shared_ptr<CSSValue> consumeClip(CSSTokenStream& input);
    static std::shared_ptr<CSSValue> consumeDashList(CSSTokenStream& input);
    static std::shared_ptr<CSSValue> consumeVerticalAlign(CSSTokenStream& input);
    static std::shared_ptr<CSSValue> consumeTextDecorationLine(CSSTokenStream& input);
    static std::shared_ptr<CSSValue> consumeBackgroundPosition(CSSTokenStream& input);
    static std::shared_ptr<CSSValue> consumeBackgroundSize(CSSTokenStream& input);
    static std::shared_ptr<CSSValue> consumeAngle(CSSTokenStream& input);
    static std::shared_ptr<CSSValue> consumeTransformValue(CSSTokenStream& input);
    static std::shared_ptr<CSSValue> consumeTransformList(CSSTokenStream& input);
    static std::shared_ptr<CSSValue> consumePaintOrder(CSSTokenStream& input);
    static std::shared_ptr<CSSValue> consumeLonghand(CSSTokenStream& input, CSSPropertyID id);

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
