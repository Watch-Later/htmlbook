#ifndef CSSSTYLESHEET_H
#define CSSSTYLESHEET_H

#include "globalstring.h"

#include <memory>
#include <list>
#include <map>

namespace htmlbook {

class CSSInitialValue;
class CSSInheritValue;
class CSSIdentValue;
class CSSCustomIdentValue;
class CSSIntegerValue;
class CSSNumberValue;
class CSSPercentValue;
class CSSAngleValue;
class CSSLengthValue;
class CSSStringValue;
class CSSUrlValue;
class CSSColorValue;
class CSSCounterValue;
class CSSPairValue;
class CSSRectValue;
class CSSListValue;
class CSSFunctionValue;

class CSSValue {
public:
    virtual ~CSSValue() = default;
    virtual const CSSInitialValue* toInitialValue() const { return nullptr; }
    virtual const CSSInheritValue* toInheritValue() const { return nullptr; }
    virtual const CSSIdentValue* toIdentValue() const { return nullptr; }
    virtual const CSSCustomIdentValue* toCustomIdentValue() const { return nullptr; }
    virtual const CSSIntegerValue* toIntegerValue() const { return nullptr; }
    virtual const CSSNumberValue* toNumberValue() const { return nullptr; }
    virtual const CSSPercentValue* toPercentValue() const { return nullptr; }
    virtual const CSSAngleValue* toAngleValue() const { return nullptr; }
    virtual const CSSLengthValue* toLengthValue() const { return nullptr; }
    virtual const CSSStringValue* toStringValue() const { return nullptr; }
    virtual const CSSUrlValue* toUrlValue() const { return nullptr; }
    virtual const CSSColorValue* toColorValue() const { return nullptr; }
    virtual const CSSCounterValue* toCounterValue() const { return nullptr; }
    virtual const CSSPairValue* toPairValue() const { return nullptr; }
    virtual const CSSRectValue* toRectValue() const { return nullptr; }
    virtual const CSSListValue* toListValue() const { return nullptr; }
    virtual const CSSFunctionValue* toFunctionValue() const { return nullptr; }

    bool isInitialValue() const { return toInitialValue(); }
    bool isInheritValue() const { return toInheritValue(); }
    bool isIdentValue() const { return toIdentValue(); }
    bool isCustomIdentValue() const { return toCustomIdentValue(); }
    bool isIntegerValue() const { return toIntegerValue(); }
    bool isNumberValue() const { return toNumberValue(); }
    bool isPercentValue() const { return toPercentValue(); }
    bool isAngleValue() const { return toAngleValue(); }
    bool isLengthValue() const { return toLengthValue(); }
    bool isStringValue() const { return toStringValue(); }
    bool isUrlValue() const { return toUrlValue(); }
    bool isColorValue() const { return toColorValue(); }
    bool isCounterValue() const { return toCounterValue(); }
    bool isPairValue() const { return toPairValue(); }
    bool isRectValue() const { return toRectValue(); }
    bool isListValue() const { return toListValue(); }
    bool isFunctionValue() const { return toFunctionValue(); }

protected:
    CSSValue() = default;
};

using CSSValueList = std::vector<std::shared_ptr<CSSValue>>;

class CSSInitialValue final : public CSSValue {
public:
    static std::shared_ptr<CSSInitialValue> create();

    const CSSInitialValue* toInitialValue() const final { return this; }

private:
    CSSInitialValue() = default;
};

class CSSInheritValue final : public CSSValue {
public:
    static std::shared_ptr<CSSInheritValue> create();

    const CSSInheritValue* toInheritValue() const final { return this; }

private:
    CSSInheritValue() = default;
};

enum class CSSValueID {
    Format,
    Disc,
    Circle,
    Decimal,
    DecimalLeadingZero,
    LowerAlpha,
    LowerLatin,
    LowerRoman,
    UpperAlpha,
    UpperLatin,
    UpperRoman,
    CurrentColor,
    Balance,
    Unknown,
    A3,
    A4,
    A5,
    Absolute,
    All,
    Always,
    Anywhere,
    Attr,
    Auto,
    Avoid,
    B4,
    B5,
    Baseline,
    Bevel,
    BidiOverride,
    Block,
    Bold,
    Bolder,
    BorderBox,
    Both,
    Bottom,
    BreakAll,
    BreakSpaces,
    BreakWord,
    Butt,
    Capitalize,
    Center,
    Clip,
    CloseQuote,
    Collapse,
    Color,
    ColorBurn,
    ColorDodge,
    Column,
    ColumnReverse,
    Contain,
    ContentBox,
    Cover,
    Cursive,
    Darken,
    Dashed,
    Difference,
    Dotted,
    Double,
    Ellipsis,
    Embed,
    End,
    Evenodd,
    Exclusion,
    Fantasy,
    Fill,
    Fixed,
    Flex,
    FlexEnd,
    FlexStart,
    Groove,
    HardLight,
    Hidden,
    Hide,
    Hue,
    Inherit,
    Initial,
    Inline,
    InlineBlock,
    InlineFlex,
    InlineTable,
    Inset,
    Inside,
    Isolate,
    IsolateOverride,
    Italic,
    Justify,
    KeepAll,
    Landscape,
    Large,
    Larger,
    Ledger,
    Left,
    Legal,
    Letter,
    Lighten,
    Lighter,
    LineThrough,
    ListItem,
    Local,
    Loose,
    Lowercase,
    Ltr,
    Luminosity,
    Manual,
    Markers,
    Matrix,
    Medium,
    Middle,
    Miter,
    Monospace,
    Multiply,
    NoCloseQuote,
    NoOpenQuote,
    NoRepeat,
    NonScalingStroke,
    None,
    Nonzero,
    Normal,
    Nowrap,
    Oblique,
    OpenQuote,
    Outset,
    Outside,
    Overlay,
    Overline,
    PaddingBox,
    Plaintext,
    Portrait,
    Pre,
    PreLine,
    PreWrap,
    Relative,
    Repeat,
    RepeatX,
    RepeatY,
    Ridge,
    Right,
    Rotate,
    RotateLeft,
    RotateRight,
    RotateX,
    RotateY,
    Round,
    Row,
    RowReverse,
    Rtl,
    SansSerif,
    Saturation,
    Scale,
    ScaleX,
    ScaleY,
    Screen,
    Scroll,
    Separate,
    Serif,
    Show,
    Skew,
    SkewX,
    SkewY,
    Small,
    SmallCaps,
    Smaller,
    SoftLight,
    Solid,
    SpaceAround,
    SpaceBetween,
    Square,
    Start,
    Static,
    Stretch,
    Strict,
    Stroke,
    Sub,
    Super,
    Table,
    TableCaption,
    TableCell,
    TableColumn,
    TableColumnGroup,
    TableFooterGroup,
    TableHeaderGroup,
    TableRow,
    TableRowGroup,
    TextBottom,
    TextTop,
    Thick,
    Thin,
    Top,
    Translate,
    TranslateX,
    TranslateY,
    Underline,
    Uppercase,
    Upright,
    Visible,
    Wavy,
    Wrap,
    WrapReverse,
    XLarge,
    XSmall,
    XxLarge,
    XxSmall,
    XxxLarge
};

class CSSIdentValue final : public CSSValue {
public:
    static std::shared_ptr<CSSIdentValue> create(CSSValueID value);

    CSSValueID value() const { return m_value; }
    const CSSIdentValue* toIdentValue() const final { return this; }

private:
    CSSIdentValue(CSSValueID value) : m_value(value) {}
    CSSValueID m_value;
};

class CSSCustomIdentValue final : public CSSValue {
public:
    static std::shared_ptr<CSSCustomIdentValue> create(const GlobalString& value);

    const GlobalString& value() const { return m_value; }
    const CSSCustomIdentValue* toCustomIdentValue() const final { return this; }

private:
    CSSCustomIdentValue(const GlobalString& value) : m_value(value) {}
    GlobalString m_value;
};

class CSSIntegerValue final : public CSSValue {
public:
    static std::shared_ptr<CSSIntegerValue> create(int value);

    int value() const { return m_value; }
    const CSSIntegerValue* toIntegerValue() const final { return this; }

private:
    CSSIntegerValue(int value) : m_value(value) {}
    int m_value;
};

class CSSNumberValue final : public CSSValue {
public:
    static std::shared_ptr<CSSNumberValue> create(double value);

    double value() const { return m_value; }
    const CSSNumberValue* toNumberValue() const final { return this; }

private:
    CSSNumberValue(double value) : m_value(value) {}
    double m_value;
};

class CSSPercentValue final : public CSSValue {
public:
    static std::shared_ptr<CSSPercentValue> create(double value);

    double value() const { return m_value; }
    const CSSPercentValue* toPercentValue() const final { return nullptr; }

private:
    CSSPercentValue(double value) : m_value(value) {}
    double m_value;
};

class CSSAngleValue final : public CSSValue {
public:
    enum class Unit {
        Degrees,
        Radians,
        Gradians,
        Turns,
    };

    static std::shared_ptr<CSSAngleValue> create(double value, Unit unit);

    double value() const { return m_value; }
    Unit unit() const { return m_unit; }
    const CSSAngleValue* toAngleValue() const final { return this; }

private:
    CSSAngleValue(double value, Unit unit)
        : m_value(value), m_unit(unit)
    {}

    double m_value;
    Unit m_unit;
};

class CSSLengthValue final : public CSSValue {
public:
    enum class Unit {
        None,
        Ems,
        Exs,
        Pixels,
        Centimeters,
        Millimeters,
        Inches,
        Points,
        Picas,
        ViewportWidth,
        ViewportHeight,
        ViewportMin,
        ViewportMax,
        Rems,
        Chs
    };

    static std::shared_ptr<CSSLengthValue> create(double value, Unit unit);

    double value() const { return m_value; }
    Unit unit() const { return m_unit; }
    const CSSLengthValue* toLengthValue() const final { return this; }

private:
    CSSLengthValue(double value, Unit unit)
        : m_value(value), m_unit(unit)
    {}

    double m_value;
    Unit m_unit;
};

class CSSStringValue final : public CSSValue {
public:
    static std::shared_ptr<CSSStringValue> create(std::string&& value);

    const std::string& value() const { return m_value; }
    const CSSStringValue* toStringValue() const final { return this; }

private:
    CSSStringValue(std::string&& value) : m_value(std::move(value)) {}
    std::string m_value;
};

class CSSUrlValue final : public CSSValue {
public:
    static std::shared_ptr<CSSUrlValue> create(std::string&& value);

    const std::string& value() const { return m_value; }
    const CSSUrlValue* toUrlValue() const final { return this; }

private:
    CSSUrlValue(std::string&& value) : m_value(std::move(value)) {}
    std::string m_value;
};

class CSSColorValue final : public CSSValue {
public:
    static std::shared_ptr<CSSColorValue> create(uint32_t value);
    static std::shared_ptr<CSSColorValue> create(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

    uint32_t value() const { return m_value; }
    const CSSColorValue* toColorValue() const final { return this; }

private:
    CSSColorValue(uint32_t value) : m_value(value) {}
    uint32_t m_value;
};

class CSSCounterValue final : public CSSValue {
public:
    static std::shared_ptr<CSSCounterValue> create(CSSValueID listStyle, const GlobalString& identifier, std::string&& seperator);

    CSSValueID listStyle() const { return m_listStyle; }
    const GlobalString& identifier() const { return m_identifier; }
    const std::string& seperator() const { return m_seperator; }
    const CSSCounterValue* toCounterValue() const final { return this; }

private:
    CSSCounterValue( CSSValueID listStyle, const GlobalString& identifier, std::string&& seperator)
        : m_listStyle(listStyle), m_identifier(identifier), m_seperator(std::move(seperator))
    {}

    CSSValueID m_listStyle;
    GlobalString m_identifier;
    std::string m_seperator;
};

class CSSPairValue final : public CSSValue {
public:
    static std::shared_ptr<CSSPairValue> create(std::shared_ptr<CSSValue> first, std::shared_ptr<CSSValue> second);

    std::shared_ptr<CSSValue> first() const { return m_first; }
    std::shared_ptr<CSSValue> second() const { return m_second; }
    const CSSPairValue* toPairValue() const final { return this; }

private:
    CSSPairValue(std::shared_ptr<CSSValue> first, std::shared_ptr<CSSValue> second)
        : m_first(first), m_second(second)
    {}

    std::shared_ptr<CSSValue> m_first;
    std::shared_ptr<CSSValue> m_second;
};

class CSSRectValue final : public CSSValue {
public:
    static std::shared_ptr<CSSRectValue> create(std::shared_ptr<CSSValue> top, std::shared_ptr<CSSValue> right, std::shared_ptr<CSSValue> bottom, std::shared_ptr<CSSValue> left);

    std::shared_ptr<CSSValue> top() const { return m_top; }
    std::shared_ptr<CSSValue> right() const { return m_right; }
    std::shared_ptr<CSSValue> bottom() const { return m_bottom; }
    std::shared_ptr<CSSValue> left() const { return m_left; }
    const CSSRectValue* toRectValue() const final { return this; }

private:
    CSSRectValue(std::shared_ptr<CSSValue> top, std::shared_ptr<CSSValue> right, std::shared_ptr<CSSValue> bottom, std::shared_ptr<CSSValue> left)
        : m_top(top), m_right(right), m_bottom(bottom), m_left(left)
    {}

    std::shared_ptr<CSSValue> m_top;
    std::shared_ptr<CSSValue> m_right;
    std::shared_ptr<CSSValue> m_bottom;
    std::shared_ptr<CSSValue> m_left;
};

class CSSListValue final : public CSSValue {
public:
    static std::shared_ptr<CSSListValue> create(CSSValueList&& values);

    size_t length() const { return m_values.size(); }
    const std::shared_ptr<CSSValue>& at(size_t index) const { return m_values.at(index); }
    const CSSValueList& values() const { return m_values; }
    const CSSListValue* toListValue() const final { return this; }

private:
    CSSListValue(CSSValueList&& values) : m_values(std::move(values)) {}
    CSSValueList m_values;
};

class CSSFunctionValue final : public CSSValue {
public:
    static std::shared_ptr<CSSFunctionValue> create(CSSValueID id, CSSValueList&& values);

    CSSValueID id() const { return m_id; }
    const CSSValueList& values() const { return m_values; }
    const CSSFunctionValue* toFunctionValue() const final { return this; }

private:
    CSSFunctionValue(CSSValueID id, CSSValueList&& values)
        : m_id(id), m_values(std::move(values))
    {}

    CSSValueID m_id;
    CSSValueList m_values;
};

enum class CSSPropertyID {
    BorderSpacing,
    BorderHorizontalSpacing,
    BorderVerticalSpacing,
    ColumnFill,
    Unknown,
    AlignContent,
    AlignItems,
    AlignSelf,
    All,
    Background,
    BackgroundAttachment,
    BackgroundClip,
    BackgroundColor,
    BackgroundImage,
    BackgroundOrigin,
    BackgroundPosition,
    BackgroundRepeat,
    BackgroundSize,
    Border,
    BorderBottom,
    BorderBottomColor,
    BorderBottomLeftRadius,
    BorderBottomRightRadius,
    BorderBottomStyle,
    BorderBottomWidth,
    BorderCollapse,
    BorderColor,
    BorderLeft,
    BorderLeftColor,
    BorderLeftStyle,
    BorderLeftWidth,
    BorderRadius,
    BorderRight,
    BorderRightColor,
    BorderRightStyle,
    BorderRightWidth,
    BorderStyle,
    BorderTop,
    BorderTopColor,
    BorderTopLeftRadius,
    BorderTopRightRadius,
    BorderTopStyle,
    BorderTopWidth,
    BorderWidth,
    Bottom,
    BoxSizing,
    CaptionSide,
    Clear,
    Clip,
    ClipPath,
    ClipRule,
    Color,
    ColumnBreakAfter,
    ColumnBreakBefore,
    ColumnBreakInside,
    ColumnCount,
    ColumnGap,
    ColumnRule,
    ColumnRuleColor,
    ColumnRuleStyle,
    ColumnRuleWidth,
    ColumnSpan,
    ColumnWidth,
    Columns,
    Content,
    CounterIncrement,
    CounterReset,
    CounterSet,
    Cx,
    Cy,
    Direction,
    Display,
    EmptyCells,
    Fill,
    FillOpacity,
    FillRule,
    Flex,
    FlexBasis,
    FlexDirection,
    FlexFlow,
    FlexGrow,
    FlexShrink,
    FlexWrap,
    Float,
    Font,
    FontFamily,
    FontSize,
    FontStyle,
    FontVariant,
    FontWeight,
    Height,
    Hyphens,
    JustifyContent,
    Left,
    LetterSpacing,
    LineBreak,
    LineHeight,
    ListStyle,
    ListStyleImage,
    ListStylePosition,
    ListStyleType,
    Margin,
    MarginBottom,
    MarginLeft,
    MarginRight,
    MarginTop,
    MarkerEnd,
    MarkerMid,
    MarkerStart,
    Mask,
    MaxHeight,
    MaxWidth,
    MinHeight,
    MinWidth,
    MixBlendMode,
    Opacity,
    Order,
    Orphans,
    Outline,
    OutlineColor,
    OutlineOffset,
    OutlineStyle,
    OutlineWidth,
    Overflow,
    OverflowWrap,
    OverflowX,
    OverflowY,
    Padding,
    PaddingBottom,
    PaddingLeft,
    PaddingRight,
    PaddingTop,
    Page,
    PageBreakAfter,
    PageBreakBefore,
    PageBreakInside,
    PageOrientation,
    PaintOrder,
    Position,
    Quotes,
    R,
    Right,
    Rx,
    Ry,
    Size,
    Src,
    StopColor,
    StopOpacity,
    Stroke,
    StrokeDasharray,
    StrokeDashoffset,
    StrokeLinecap,
    StrokeLinejoin,
    StrokeMiterlimit,
    StrokeOpacity,
    StrokeWidth,
    TabSize,
    TableLayout,
    TextAlign,
    TextAnchor,
    TextDecoration,
    TextDecorationColor,
    TextDecorationLine,
    TextDecorationStyle,
    TextIndent,
    TextOverflow,
    TextTransform,
    Top,
    Transform,
    TransformOrigin,
    UnicodeBidi,
    VectorEffect,
    VerticalAlign,
    Visibility,
    WhiteSpace,
    Widows,
    Width,
    WordBreak,
    WordSpacing,
    WordWrap,
    X,
    Y,
    ZIndex
};

CSSPropertyID csspropertyid(const std::string_view& name);

class CSSProperty {
public:
    CSSProperty(CSSPropertyID id, bool important, std::shared_ptr<CSSValue> value)
        : m_id(id), m_important(important), m_value(value)
    {}

    CSSPropertyID id() const { return m_id; }
    bool important() const { return m_important; }
    std::shared_ptr<CSSValue> value() const { return m_value; }

private:
    CSSPropertyID m_id;
    bool m_important;
    std::shared_ptr<CSSValue> m_value;
};

using CSSPropertyList = std::vector<CSSProperty>;

class CSSShorthand {
public:
    static CSSShorthand longhand(CSSPropertyID id);

    const CSSPropertyID* data() const { return m_data; }
    CSSPropertyID at(size_t index) const { return m_data[index]; }
    size_t length() const { return m_length; }
    bool empty() const { return m_length == 0; }

private:
    CSSShorthand(const CSSPropertyID* data, size_t length)
        : m_data(data), m_length(length)
    {}

    const CSSPropertyID* m_data;
    size_t m_length;
};

class CSSSimpleSelector;
class CSSComplexSelector;

using CSSCompoundSelector = std::list<CSSSimpleSelector>;
using CSSSelector = std::list<CSSComplexSelector>;

using CSSCompoundSelectorList = std::list<CSSCompoundSelector>;
using CSSSelectorList = std::list<CSSSelector>;

using CSSPageSelector = CSSCompoundSelector;
using CSSPageSelectorList = CSSCompoundSelectorList;

class CSSSimpleSelector {
public:
    enum class MatchType {
        Universal,
        Tag,
        Id,
        Class,
        AttributeHas,
        AttributeEquals,
        AttributeIncludes,
        AttributeContains,
        AttributeDashEquals,
        AttributeStartsWith,
        AttributeEndsWith,
        PseudoClassIs,
        PseudoClassNot,
        PseudoClassLink,
        PseudoClassEnabled,
        PseudoClassDisabled,
        PseudoClassChecked,
        PseudoClassLang,
        PseudoClassRoot,
        PseudoClassEmpty,
        PseudoClassFirstChild,
        PseudoClassLastChild,
        PseudoClassOnlyChild,
        PseudoClassFirstOfType,
        PseudoClassLastOfType,
        PseudoClassOnlyOfType,
        PseudoClassNthChild,
        PseudoClassNthLastChild,
        PseudoClassNthOfType,
        PseudoClassNthLastOfType,
        PseudoElementBefore,
        PseudoElementAfter,
        PseudoElementMarker,
        PseudoElementFirstLetter,
        PseudoElementFirstLine,
        PseudoPageFirst,
        PseudoPageLeft,
        PseudoPageRight,
        PseudoPageBlank
    };

    enum class AttributeCaseType {
        Sensitive,
        InSensitive
    };

    using MatchPattern = std::pair<int, int>;

    CSSSimpleSelector(MatchType matchType) : m_matchType(matchType) {}
    CSSSimpleSelector(MatchType matchType, const GlobalString& name) : m_matchType(matchType), m_name(name) {}
    CSSSimpleSelector(MatchType matchType, const MatchPattern& matchPattern) : m_matchType(matchType), m_matchPattern(matchPattern) {}

    CSSSimpleSelector(MatchType matchType, std::unique_ptr<CSSCompoundSelectorList>&& subSelectors)
        : m_matchType(matchType), m_subSelectors(std::move(subSelectors))
    {}

    CSSSimpleSelector(MatchType matchType, AttributeCaseType attributeCaseType, const GlobalString& name, std::unique_ptr<std::string>&& value)
        : m_matchType(matchType), m_attributeCaseType(attributeCaseType), m_name(name), m_value(std::move(value))
    {}

    MatchType matchType() const { return m_matchType; }
    AttributeCaseType attributeCaseType() const { return m_attributeCaseType; }
    const MatchPattern& matchPattern() const { return m_matchPattern; }
    const GlobalString& name() const { return m_name; }
    const std::string& value() const { return *m_value; }
    const CSSCompoundSelectorList& subSelectors() const { return *m_subSelectors; }
    bool isCaseSensitive() const { return m_attributeCaseType == AttributeCaseType::Sensitive; }

    bool matchnth(int count) const;

private:
    MatchType m_matchType;
    AttributeCaseType m_attributeCaseType;
    MatchPattern m_matchPattern;
    GlobalString m_name;
    std::unique_ptr<std::string> m_value;
    std::unique_ptr<CSSCompoundSelectorList> m_subSelectors;
};

class CSSComplexSelector {
public:
    enum class Combinator {
        None,
        Descendant,
        Child,
        DirectAdjacent,
        InDirectAdjacent
    };

    CSSComplexSelector(Combinator combinator, CSSCompoundSelector&& compoundSelector)
        : m_combinator(combinator), m_compoundSelector(std::move(compoundSelector))
    {}

    Combinator combinator() const { return m_combinator; }
    const CSSCompoundSelector& compoundSelector() const { return m_compoundSelector; }

private:
    Combinator m_combinator;
    CSSCompoundSelector m_compoundSelector;
};

class Element;

class CSSSelectorMatch {
public:
    static bool match(const Element* element, const CSSSelector& selector);

private:
    static bool matchCompoundSelectorList(const Element* element, const CSSCompoundSelectorList& selectors);
    static bool matchCompoundSelector(const Element* element, const CSSCompoundSelector& selector);
    static bool matchSimpleSelector(const Element* element, const CSSSimpleSelector& selector);

    static bool matchTagSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchIdSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchClassSelector(const Element* element, const CSSSimpleSelector& selector);

    static bool matchAttributeHasSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchAttributeEqualsSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchAttributeIncludesSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchAttributeContainsSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchAttributeDashEqualsSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchAttributeStartsWithSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchAttributeEndsWithSelector(const Element* element, const CSSSimpleSelector& selector);

    static bool matchPseudoClassIsSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchPseudoClassNotSelector(const Element* element, const CSSSimpleSelector& selector);

    static bool matchPseudoClassLinkSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchPseudoClassEnabledSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchPseudoClassDisabledSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchPseudoClassCheckedSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchPseudoClassLangSelector(const Element* element, const CSSSimpleSelector& selector);

    static bool matchPseudoClassRootSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchPseudoClassEmptySelector(const Element* element, const CSSSimpleSelector& selector);

    static bool matchPseudoClassFirstChildSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchPseudoClassLastChildSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchPseudoClassOnlyChildSelector(const Element* element, const CSSSimpleSelector& selector);

    static bool matchPseudoClassFirstOfTypeSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchPseudoClassLastOfTypeSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchPseudoClassOnlyOfTypeSelector(const Element* element, const CSSSimpleSelector& selector);

    static bool matchPseudoClassNthChildSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchPseudoClassNthLastChildSelector(const Element* element, const CSSSimpleSelector& selector);

    static bool matchPseudoClassNthOfTypeSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchPseudoClassNthLastOfTypeSelector(const Element* element, const CSSSimpleSelector& selector);
};

class CSSStyleRule;
class CSSImportRule;
class CSSFontFaceRule;
class CSSPageMarginRule;
class CSSPageRule;

class CSSRule {
public:
    virtual ~CSSRule() = default;
    virtual const CSSStyleRule* toStyleRule() const { return nullptr; }
    virtual const CSSImportRule* toImportRule() const { return nullptr; }
    virtual const CSSFontFaceRule* toFontFaceRule() const { return nullptr; }
    virtual const CSSPageMarginRule* toPageMarginRule() const { return nullptr; }
    virtual const CSSPageRule* toPageRule() const { return nullptr; }

    bool isStyleRule() const { return toStyleRule(); }
    bool isImportRule() const { return toImportRule(); }
    bool isFontFaceRule() const { return toFontFaceRule(); }
    bool isPageMarginRule() const { return toPageMarginRule(); }
    bool isPageRule() const { return toPageRule(); }

protected:
    CSSRule() = default;
};

using CSSRuleList = std::vector<std::unique_ptr<CSSRule>>;

class CSSStyleRule final : public CSSRule {
public:
    static std::unique_ptr<CSSStyleRule> create(CSSSelectorList&& selectors, CSSPropertyList&& properties);

    const CSSSelectorList& selectors() const { return m_selectors; }
    const CSSPropertyList& properties() const { return m_properties; }
    const CSSStyleRule* toStyleRule() const final { return this; }

private:
    CSSStyleRule(CSSSelectorList&& selectors, CSSPropertyList&& properties)
        : m_selectors(std::move(selectors)), m_properties(std::move(properties))
    {}

    CSSSelectorList m_selectors;
    CSSPropertyList m_properties;
};

class CSSImportRule final : public CSSRule {
public:
    static std::unique_ptr<CSSImportRule> create(std::string&& href);

    const std::string& href() const { return m_href; }
    const CSSImportRule* toImportRule() const final { return this; }

private:
    CSSImportRule(std::string&& href) : m_href(std::move(href)) {}
    std::string m_href;
};

class CSSFontFaceRule : public CSSRule {
public:
    static std::unique_ptr<CSSFontFaceRule> create(CSSPropertyList&& properties);

    const CSSPropertyList& properties() const { return m_properties; }
    const CSSFontFaceRule* toFontFaceRule() const final { return this; }

private:
    CSSFontFaceRule(CSSPropertyList&& properties)
        : m_properties(std::move(properties))
    {}

    CSSPropertyList m_properties;
};

class CSSPageMarginRule final : public CSSRule {
public:
    enum class MarginType {
        TopLeftCorner,
        TopLeft,
        TopCenter,
        TopRight,
        TopRightCorner,
        BottomLeftCorner,
        BottomLeft,
        BottomCenter,
        BottomRight,
        BottomRightCorner,
        LeftTop,
        LeftMiddle,
        LeftBottom,
        RightTop,
        RightMiddle,
        RightBottom
    };

    static std::unique_ptr<CSSPageMarginRule> create(MarginType marginType, CSSPropertyList&& properties);

    MarginType marginType() const { return m_marginType; }
    const CSSPropertyList& properties() const { return m_properties; }
    const CSSPageMarginRule* toPageMarginRule() const final { return this; }

private:
    CSSPageMarginRule(MarginType marginType, CSSPropertyList&& properties)
        : m_marginType(marginType), m_properties(std::move(properties))
    {}

    MarginType m_marginType;
    CSSPropertyList m_properties;
};

using CSSPageMarginRuleList = std::vector<std::unique_ptr<CSSPageMarginRule>>;

class CSSPageRule : public CSSRule {
public:
    static std::unique_ptr<CSSPageRule> create(CSSPageSelectorList&& selectors, CSSPageMarginRuleList&& margins, CSSPropertyList&& properties);

    const CSSPageSelectorList& selectors() const { return m_selectors; }
    const CSSPageMarginRuleList& margins() const { return m_margins; }
    const CSSPropertyList& properties() const { return m_properties; }
    const CSSPageRule* toPageRule() const final { return this; }

private:
    CSSPageRule(CSSPageSelectorList&& selectors, CSSPageMarginRuleList&& margins, CSSPropertyList&& properties)
        : m_selectors(std::move(selectors)), m_margins(std::move(margins)), m_properties(std::move(properties))
    {}

    CSSPageSelectorList m_selectors;
    CSSPageMarginRuleList m_margins;
    CSSPropertyList m_properties;
};

class CSSRuleData {
public:
    CSSRuleData(const CSSSelector& selector, const CSSPropertyList& properties, uint32_t specificity, uint32_t position)
        : m_selector(&selector), m_properties(&properties), m_specificity(specificity), m_position(position)
    {}

    const CSSSelector& selector() const { return *m_selector; }
    const CSSPropertyList& properties() const { return *m_properties; }
    uint32_t specificity() const { return m_specificity; }
    uint32_t position() const { return m_position; }
    bool match(const Element* element) const { return CSSSelectorMatch::match(element, *m_selector); }

private:
    const CSSSelector* m_selector;
    const CSSPropertyList* m_properties;
    uint32_t m_specificity;
    uint32_t m_position;
};

inline bool operator<(const CSSRuleData& a, const CSSRuleData& b) { return a.specificity() == b.specificity() ? a.position() < b.position() : a.specificity() < b.specificity(); }

using CSSRuleDataList = std::vector<CSSRuleData>;

class CSSRuleDataListMap {
public:
    CSSRuleDataListMap() = default;

    void add(const CSSRuleData& data, const GlobalString& name);
    const CSSRuleDataList* get(const GlobalString& name) const;

private:
    using RuleDataListMap = std::map<GlobalString, CSSRuleDataList>;
    RuleDataListMap m_ruleDataListMap;
};

class CSSPageRuleData {
public:
    CSSPageRuleData(const CSSPageRule* rule, const CSSPageSelector& selector, uint32_t specificity, uint32_t position)
        : m_rule(rule), m_selector(&selector), m_specificity(specificity), m_position(position)
    {}

    const CSSPageRule* rule() const { return m_rule; }
    const CSSPageSelector* selector() const { return m_selector; }
    uint32_t specificity() const { return m_specificity; }
    uint32_t position() const { return m_position; }

private:
    const CSSPageRule* m_rule;
    const CSSPageSelector* m_selector;
    uint32_t m_specificity;
    uint32_t m_position;
};

using CSSPageRuleDataList = std::vector<CSSPageRuleData>;

class Document;

class CSSStyleSheet {
public:
    static std::unique_ptr<CSSStyleSheet> create(Document* document);

    void parse(const std::string_view& content);

    const CSSRuleDataList* idRules(const GlobalString& name) const { return m_idRules.get(name); }
    const CSSRuleDataList* tagRules(const GlobalString& name) const { return m_tagRules.get(name); }
    const CSSRuleDataList* classRules(const GlobalString& name) const { return m_classRules.get(name); }

    const CSSRuleDataList* universeRules() const { return &m_universeRules; }
    const CSSRuleDataList* beforeElementRules() const { return &m_beforeElementRules; }
    const CSSRuleDataList* afterElementRules() const { return &m_afterElementRules; }
    const CSSRuleDataList* markerElementRules() const { return &m_markerElementRules; }
    const CSSRuleDataList* firstLetterElementRules() const { return &m_firstLetterRules; }
    const CSSRuleDataList* firstLineElementRules() const { return &m_firstLineRules; }
    const CSSPageRuleDataList* pageRules() const { return &m_pageRules; }

    void addRule(std::unique_ptr<CSSRule>&& rule);

private:
    void addStyleRule(const CSSStyleRule* rule);
    void addPageRule(const CSSPageRule* rule);
    void addImportRule(const CSSImportRule* rule);
    void addFontFaceRule(const CSSFontFaceRule* rule);

    CSSStyleSheet(Document* document) : m_document(document) {}
    Document* m_document;
    CSSRuleList m_rules;

    CSSRuleDataListMap m_idRules;
    CSSRuleDataListMap m_classRules;
    CSSRuleDataListMap m_tagRules;

    CSSRuleDataList m_universeRules;
    CSSRuleDataList m_beforeElementRules;
    CSSRuleDataList m_afterElementRules;
    CSSRuleDataList m_markerElementRules;
    CSSRuleDataList m_firstLetterRules;
    CSSRuleDataList m_firstLineRules;
    CSSPageRuleDataList m_pageRules;
};

} // namespace htmlbook

#endif // CSSSTYLESHEET_H
