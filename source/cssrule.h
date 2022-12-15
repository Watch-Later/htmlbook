#ifndef CSSRULE_H
#define CSSRULE_H

#include "globalstring.h"
#include "pointer.h"

#include <memory>
#include <list>
#include <map>
#include <set>

namespace htmlbook {

class Document;

class CSSValue : public RefCounted<CSSValue> {
public:
    virtual ~CSSValue() = default;
    virtual bool isInitialValue() const { return false; }
    virtual bool isInheritValue() const { return false; }
    virtual bool isIdentValue() const { return false; }
    virtual bool isCustomIdentValue() const { return false; }
    virtual bool isIntegerValue() const { return false; }
    virtual bool isNumberValue() const { return false; }
    virtual bool isPercentValue() const { return false; }
    virtual bool isAngleValue() const { return false; }
    virtual bool isLengthValue() const { return false; }
    virtual bool isStringValue() const { return false; }
    virtual bool isUrlValue() const { return false; }
    virtual bool isImageValue() const { return false; }
    virtual bool isColorValue() const { return false; }
    virtual bool isCounterValue() const { return false; }
    virtual bool isPairValue() const { return false; }
    virtual bool isRectValue() const { return false; }
    virtual bool isListValue() const { return false; }
    virtual bool isFunctionValue() const { return false; }

protected:
    CSSValue() = default;
};

using CSSValueList = std::vector<RefPtr<CSSValue>>;

class CSSInitialValue final : public CSSValue {
public:
    static RefPtr<CSSInitialValue> create();

    bool isInitialValue() const final { return true; }

private:
    CSSInitialValue() = default;
};

template<>
struct is<CSSInitialValue> {
    static bool check(const CSSValue& value) { return value.isInitialValue(); }
};

class CSSInheritValue final : public CSSValue {
public:
    static RefPtr<CSSInheritValue> create();

    bool isInheritValue() const final { return true; }

private:
    CSSInheritValue() = default;
};

template<>
struct is<CSSInheritValue> {
    static bool check(const CSSValue& value) { return value.isInheritValue(); }
};

enum class CSSValueID {
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
    Balance,
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
    Circle,
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
    CurrentColor,
    Darken,
    Dashed,
    Decimal,
    DecimalLeadingZero,
    Difference,
    Disc,
    Dotted,
    Double,
    Ellipsis,
    Embed,
    End,
    Evenodd,
    Exclusion,
    Fill,
    Fixed,
    Flex,
    FlexEnd,
    FlexStart,
    Format,
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
    LowerAlpha,
    LowerLatin,
    LowerRoman,
    Lowercase,
    Ltr,
    Luminosity,
    Manual,
    Markers,
    Matrix,
    Medium,
    Middle,
    Miter,
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
    Saturation,
    Scale,
    ScaleX,
    ScaleY,
    Screen,
    Scroll,
    Separate,
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
    SpaceEvenly,
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
    UpperAlpha,
    UpperLatin,
    UpperRoman,
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
    static RefPtr<CSSIdentValue> create(CSSValueID value);

    CSSValueID value() const { return m_value; }
    bool isIdentValue() const final { return true; }

private:
    CSSIdentValue(CSSValueID value) : m_value(value) {}
    CSSValueID m_value;
};

template<>
struct is<CSSIdentValue> {
    static bool check(const CSSValue& value) { return value.isIdentValue(); }
};

class CSSCustomIdentValue final : public CSSValue {
public:
    static RefPtr<CSSCustomIdentValue> create(const HeapString& value);

    const HeapString& value() const { return m_value; }
    bool isCustomIdentValue() const final { return true; }

private:
    CSSCustomIdentValue(const HeapString& value) : m_value(value) {}
    HeapString m_value;
};

template<>
struct is<CSSCustomIdentValue> {
    static bool check(const CSSValue& value) { return value.isCustomIdentValue(); }
};

class CSSIntegerValue final : public CSSValue {
public:
    static RefPtr<CSSIntegerValue> create(int value);

    int value() const { return m_value; }
    bool isIntegerValue() const final { return true; }

private:
    CSSIntegerValue(int value) : m_value(value) {}
    int m_value;
};

template<>
struct is<CSSIntegerValue> {
    static bool check(const CSSValue& value) { return value.isIntegerValue(); }
};

class CSSNumberValue final : public CSSValue {
public:
    static RefPtr<CSSNumberValue> create(double value);

    double value() const { return m_value; }
    bool isNumberValue() const final { return true; }

private:
    CSSNumberValue(double value) : m_value(value) {}
    double m_value;
};

template<>
struct is<CSSNumberValue> {
    static bool check(const CSSValue& value) { return value.isNumberValue(); }
};

class CSSPercentValue final : public CSSValue {
public:
    static RefPtr<CSSPercentValue> create(double value);

    double value() const { return m_value; }
    bool isPercentValue() const final { return true; }

private:
    CSSPercentValue(double value) : m_value(value) {}
    double m_value;
};

template<>
struct is<CSSPercentValue> {
    static bool check(const CSSValue& value) { return value.isPercentValue(); }
};

class CSSAngleValue final : public CSSValue {
public:
    enum class Unit {
        Degrees,
        Radians,
        Gradians,
        Turns,
    };

    static RefPtr<CSSAngleValue> create(double value, Unit unit);

    double value() const { return m_value; }
    Unit unit() const { return m_unit; }
    bool isAngleValue() const final { return true; }

private:
    CSSAngleValue(double value, Unit unit)
        : m_value(value), m_unit(unit)
    {}

    double m_value;
    Unit m_unit;
};

template<>
struct is<CSSAngleValue> {
    static bool check(const CSSValue& value) { return value.isAngleValue(); }
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

    static RefPtr<CSSLengthValue> create(double value, Unit unit);

    double value() const { return m_value; }
    Unit unit() const { return m_unit; }
    bool isLengthValue() const final { return true; }

private:
    CSSLengthValue(double value, Unit unit)
        : m_value(value), m_unit(unit)
    {}

    double m_value;
    Unit m_unit;
};

template<>
struct is<CSSLengthValue> {
    static bool check(const CSSValue& value) { return value.isLengthValue(); }
};

class CSSStringValue final : public CSSValue {
public:
    static RefPtr<CSSStringValue> create(const HeapString& value);

    const HeapString& value() const { return m_value; }
    bool isStringValue() const final { return true; }

private:
    CSSStringValue(const HeapString& value) : m_value(value) {}
    HeapString m_value;
};

template<>
struct is<CSSStringValue> {
    static bool check(const CSSValue& value) { return value.isStringValue(); }
};

class CSSUrlValue final : public CSSValue {
public:
    static RefPtr<CSSUrlValue> create(const HeapString& value);

    const HeapString& value() const { return m_value; }
    bool isUrlValue() const final { return true; }

private:
    CSSUrlValue(const HeapString& value) : m_value(value) {}
    HeapString m_value;
};

template<>
struct is<CSSUrlValue> {
    static bool check(const CSSValue& value) { return value.isUrlValue(); }
};

class Image;

class CSSImageValue final : public CSSValue {
public:
    static RefPtr<CSSImageValue> create(const HeapString& value);

    const HeapString& value() const { return m_value; }
    const RefPtr<Image>& image() const { return m_image; }
    RefPtr<Image> fetch(Document* document) const;
    bool isImageValue() const final { return true; }

private:
    CSSImageValue(const HeapString& value);
    HeapString m_value;
    mutable RefPtr<Image> m_image;
};

template<>
struct is<CSSImageValue> {
    static bool check(const CSSValue& value) { return value.isImageValue(); }
};

class CSSColorValue final : public CSSValue {
public:
    static RefPtr<CSSColorValue> create(uint32_t value);
    static RefPtr<CSSColorValue> create(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

    uint32_t value() const { return m_value; }
    bool isColorValue() const final { return true; }

private:
    CSSColorValue(uint32_t value) : m_value(value) {}
    uint32_t m_value;
};

template<>
struct is<CSSColorValue> {
    static bool check(const CSSValue& value) { return value.isColorValue(); }
};

enum class ListStyleType : uint8_t {
    None,
    Disc,
    Circle,
    Square,
    Decimal,
    DecimalLeadingZero,
    LowerAlpha,
    LowerLatin,
    LowerRoman,
    UpperAlpha,
    UpperLatin,
    UpperRoman
};

class CSSCounterValue final : public CSSValue {
public:
    static RefPtr<CSSCounterValue> create(const GlobalString& identifier, ListStyleType listStyle, const HeapString& separator);

    const GlobalString& identifier() const { return m_identifier; }
    ListStyleType listStyle() const { return m_listStyle; }
    const HeapString& separator() const { return m_separator; }
    bool isCounterValue() const final { return true; }

private:
    CSSCounterValue(const GlobalString& identifier, ListStyleType listStyle, const HeapString& separator)
        : m_identifier(identifier), m_listStyle(listStyle), m_separator(separator)
    {}

    GlobalString m_identifier;
    ListStyleType m_listStyle;
    HeapString m_separator;
};

template<>
struct is<CSSCounterValue> {
    static bool check(const CSSValue& value) { return value.isCounterValue(); }
};

class CSSPairValue final : public CSSValue {
public:
    static RefPtr<CSSPairValue> create(RefPtr<CSSValue> first, RefPtr<CSSValue> second);

    const RefPtr<CSSValue>& first() const { return m_first; }
    const RefPtr<CSSValue>& second() const { return m_second; }
    bool isPairValue() const final { return true; }

private:
    CSSPairValue(RefPtr<CSSValue> first, RefPtr<CSSValue> second)
        : m_first(first), m_second(second)
    {}

    RefPtr<CSSValue> m_first;
    RefPtr<CSSValue> m_second;
};

template<>
struct is<CSSPairValue> {
    static bool check(const CSSValue& value) { return value.isPairValue(); }
};

class CSSRectValue final : public CSSValue {
public:
    static RefPtr<CSSRectValue> create(RefPtr<CSSValue> top, RefPtr<CSSValue> right, RefPtr<CSSValue> bottom, RefPtr<CSSValue> left);

    const RefPtr<CSSValue>& top() const { return m_top; }
    const RefPtr<CSSValue>& right() const { return m_right; }
    const RefPtr<CSSValue>& bottom() const { return m_bottom; }
    const RefPtr<CSSValue>& left() const { return m_left; }
    bool isRectValue() const final { return true; }

private:
    CSSRectValue(RefPtr<CSSValue> top, RefPtr<CSSValue> right, RefPtr<CSSValue> bottom, RefPtr<CSSValue> left)
        : m_top(top), m_right(right), m_bottom(bottom), m_left(left)
    {}

    RefPtr<CSSValue> m_top;
    RefPtr<CSSValue> m_right;
    RefPtr<CSSValue> m_bottom;
    RefPtr<CSSValue> m_left;
};

template<>
struct is<CSSRectValue> {
    static bool check(const CSSValue& value) { return value.isRectValue(); }
};

class CSSListValue : public CSSValue {
public:
    static RefPtr<CSSListValue> create(CSSValueList values);

    size_t length() const { return m_values.size(); }
    const RefPtr<CSSValue>& front() const { return m_values.front(); }
    const RefPtr<CSSValue>& back() const { return m_values.back(); }
    const RefPtr<CSSValue>& at(size_t index) const { return m_values.at(index); }
    const CSSValueList& values() const { return m_values; }
    bool isListValue() const final { return true; }

protected:
    CSSListValue(CSSValueList values) : m_values(std::move(values)) {}
    CSSValueList m_values;
};

template<>
struct is<CSSListValue> {
    static bool check(const CSSValue& value) { return value.isListValue(); }
};

class CSSFunctionValue final : public CSSListValue {
public:
    static RefPtr<CSSFunctionValue> create(CSSValueID id, CSSValueList values);
    static RefPtr<CSSFunctionValue> create(CSSValueID id, RefPtr<CSSValue> value);

    CSSValueID id() const { return m_id; }
    bool isFunctionValue() const final { return true; }

private:
    CSSFunctionValue(CSSValueID id, CSSValueList values)
        : m_id(id), CSSListValue(std::move(values))
    {}

    CSSValueID m_id;
};

template<>
struct is<CSSFunctionValue> {
    static bool check(const CSSValue& value) { return value.isFunctionValue(); }
};

enum class CSSPropertyID {
    Unknown,
    AlignContent,
    AlignItems,
    AlignSelf,
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
    BorderHorizontalSpacing,
    BorderLeft,
    BorderLeftColor,
    BorderLeftStyle,
    BorderLeftWidth,
    BorderRadius,
    BorderRight,
    BorderRightColor,
    BorderRightStyle,
    BorderRightWidth,
    BorderSpacing,
    BorderStyle,
    BorderTop,
    BorderTopColor,
    BorderTopLeftRadius,
    BorderTopRightRadius,
    BorderTopStyle,
    BorderTopWidth,
    BorderVerticalSpacing,
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
    ColumnFill,
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
    CSSProperty(CSSPropertyID id, bool important, RefPtr<CSSValue> value)
        : m_id(id), m_important(important), m_value(value)
    {}

    CSSPropertyID id() const { return m_id; }
    bool important() const { return m_important; }
    const RefPtr<CSSValue>& value() const { return m_value; }

private:
    CSSPropertyID m_id;
    bool m_important;
    RefPtr<CSSValue> m_value;
};

using CSSPropertyList = std::vector<CSSProperty>;
using CSSPropertyMap = std::map<CSSPropertyID, RefPtr<CSSValue>>;

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

using CSSCompoundSelector = std::pmr::list<CSSSimpleSelector>;
using CSSSelector = std::pmr::list<CSSComplexSelector>;

using CSSCompoundSelectorList = std::pmr::list<CSSCompoundSelector>;
using CSSSelectorList = std::pmr::list<CSSSelector>;

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
    CSSSimpleSelector(MatchType matchType, const HeapString& value) : m_matchType(matchType), m_value(value) {}
    CSSSimpleSelector(MatchType matchType, const MatchPattern& matchPattern) : m_matchType(matchType), m_matchPattern(matchPattern) {}

    CSSSimpleSelector(MatchType matchType, CSSCompoundSelectorList subSelectors)
        : m_matchType(matchType), m_subSelectors(std::move(subSelectors))
    {}

    CSSSimpleSelector(MatchType matchType, AttributeCaseType attributeCaseType, const GlobalString& name, const HeapString& value)
        : m_matchType(matchType), m_attributeCaseType(attributeCaseType), m_name(name), m_value(value)
    {}

    MatchType matchType() const { return m_matchType; }
    AttributeCaseType attributeCaseType() const { return m_attributeCaseType; }
    const MatchPattern& matchPattern() const { return m_matchPattern; }
    const GlobalString& name() const { return m_name; }
    const HeapString& value() const { return m_value; }
    const CSSCompoundSelectorList& subSelectors() const { return m_subSelectors; }
    bool isCaseSensitive() const { return m_attributeCaseType == AttributeCaseType::Sensitive; }

    bool matchnth(int count) const;

private:
    MatchType m_matchType;
    AttributeCaseType m_attributeCaseType;
    MatchPattern m_matchPattern;
    GlobalString m_name;
    HeapString m_value;
    CSSCompoundSelectorList m_subSelectors;
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

    CSSComplexSelector(Combinator combinator, CSSCompoundSelector compoundSelector)
        : m_combinator(combinator), m_compoundSelector(std::move(compoundSelector))
    {}

    Combinator combinator() const { return m_combinator; }
    const CSSCompoundSelector& compoundSelector() const { return m_compoundSelector; }

private:
    Combinator m_combinator;
    CSSCompoundSelector m_compoundSelector;
};

class CSSRule {
public:
    enum class Type {
        Style,
        Import,
        FontFace,
        PageMargin,
        Page
    };

    virtual ~CSSRule() = default;
    virtual Type type() const = 0;

protected:
    CSSRule() = default;
};

using CSSRuleList = std::vector<std::unique_ptr<CSSRule>>;

class CSSStyleRule final : public CSSRule {
public:
    static std::unique_ptr<CSSStyleRule> create(CSSSelectorList selectors, CSSPropertyList properties);

    const CSSSelectorList& selectors() const { return m_selectors; }
    const CSSPropertyList& properties() const { return m_properties; }
    Type type() const final { return Type::Style; }

private:
    CSSStyleRule(CSSSelectorList selectors, CSSPropertyList properties)
        : m_selectors(std::move(selectors)), m_properties(std::move(properties))
    {}

    CSSSelectorList m_selectors;
    CSSPropertyList m_properties;
};

template<>
struct is<CSSStyleRule> {
    static bool check(const CSSRule& value) { return value.type() == CSSRule::Type::Style; }
};

class CSSImportRule final : public CSSRule {
public:
    static std::unique_ptr<CSSImportRule> create(const HeapString& href);

    const HeapString& href() const { return m_href; }
    Type type() const final { return Type::Import; }
    const CSSRuleList& fetch(Document* document) const;

private:
    CSSImportRule(const HeapString& href) : m_href(href) {}
    HeapString m_href;
    mutable CSSRuleList m_rules;
};

template<>
struct is<CSSImportRule> {
    static bool check(const CSSRule& value) { return value.type() == CSSRule::Type::Import; }
};

class CSSFontFaceRule : public CSSRule {
public:
    static std::unique_ptr<CSSFontFaceRule> create(CSSPropertyList properties);

    const CSSPropertyList& properties() const { return m_properties; }
    Type type() const final { return Type::FontFace; }

private:
    CSSFontFaceRule(CSSPropertyList properties)
        : m_properties(std::move(properties))
    {}

    CSSPropertyList m_properties;
};

template<>
struct is<CSSFontFaceRule> {
    static bool check(const CSSRule& value) { return value.type() == CSSRule::Type::FontFace; }
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

    static std::unique_ptr<CSSPageMarginRule> create(MarginType marginType, CSSPropertyList properties);

    MarginType marginType() const { return m_marginType; }
    const CSSPropertyList& properties() const { return m_properties; }
    Type type() const final { return Type::PageMargin; }

private:
    CSSPageMarginRule(MarginType marginType, CSSPropertyList properties)
        : m_marginType(marginType), m_properties(std::move(properties))
    {}

    MarginType m_marginType;
    CSSPropertyList m_properties;
};

template<>
struct is<CSSPageMarginRule> {
    static bool check(const CSSRule& value) { return value.type() == CSSRule::Type::PageMargin; }
};

using CSSPageMarginRuleList = std::vector<std::unique_ptr<CSSPageMarginRule>>;

class CSSPageRule : public CSSRule {
public:
    static std::unique_ptr<CSSPageRule> create(CSSPageSelectorList selectors, CSSPageMarginRuleList margins, CSSPropertyList properties);

    const CSSPageSelectorList& selectors() const { return m_selectors; }
    const CSSPageMarginRuleList& margins() const { return m_margins; }
    const CSSPropertyList& properties() const { return m_properties; }
    Type type() const final { return Type::Page; }

private:
    CSSPageRule(CSSPageSelectorList selectors, CSSPageMarginRuleList margins, CSSPropertyList properties)
        : m_selectors(std::move(selectors)), m_margins(std::move(margins)), m_properties(std::move(properties))
    {}

    CSSPageSelectorList m_selectors;
    CSSPageMarginRuleList m_margins;
    CSSPropertyList m_properties;
};

template<>
struct is<CSSPageRule> {
    static bool check(const CSSRule& value) { return value.type() == CSSRule::Type::Page; }
};

enum class PseudoType : uint8_t {
    None,
    Before,
    After,
    Marker,
    FirstLetter,
    FirstLine
};

class Element;

class CSSRuleData {
public:
    CSSRuleData(const CSSStyleRule* rule, const CSSSelector& selector, uint32_t specificity, uint32_t position)
        : m_rule(rule), m_selector(&selector), m_specificity(specificity), m_position(position)
    {}

    const CSSStyleRule* rule() const { return m_rule; }
    const CSSSelector* selector() const { return m_selector; }
    const CSSPropertyList& properties() const { return m_rule->properties(); }
    const uint32_t& specificity() const { return m_specificity; }
    const uint32_t& position() const { return m_position; }
    bool match(const Element* element, PseudoType pseudoType) const;

private:
    static bool matchCompoundSelectorList(const Element* element, PseudoType pseudoType, const CSSCompoundSelectorList& selectors);
    static bool matchCompoundSelector(const Element* element, PseudoType pseudoType, const CSSCompoundSelector& selector);
    static bool matchSimpleSelector(const Element* element, PseudoType pseudoType, const CSSSimpleSelector& selector);

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

    static bool matchPseudoClassIsSelector(const Element* element, PseudoType seudoType, const CSSSimpleSelector& selector);
    static bool matchPseudoClassNotSelector(const Element* element, PseudoType pseudoType, const CSSSimpleSelector& selector);

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

private:
    const CSSStyleRule* m_rule;
    const CSSSelector* m_selector;
    uint32_t m_specificity;
    uint32_t m_position;
};

inline bool operator<(const CSSRuleData& a, const CSSRuleData& b) { return std::tie(a.specificity(), a.position()) < std::tie(b.specificity(), b.position()); }
inline bool operator>(const CSSRuleData& a, const CSSRuleData& b) { return std::tie(a.specificity(), a.position()) > std::tie(b.specificity(), b.position()); }

using CSSRuleDataList = std::vector<CSSRuleData>;

template<typename T>
class CSSRuleDataMap {
public:
    CSSRuleDataMap() = default;

    void add(const T& name, const CSSRuleData& data);
    const CSSRuleDataList* get(const T& name) const;

private:
    std::map<T, CSSRuleDataList> m_ruleDataMap;
};

template<typename T>
void CSSRuleDataMap<T>::add(const T& name, const CSSRuleData& data)
{
    m_ruleDataMap[name].push_back(data);
}

template<typename T>
const CSSRuleDataList* CSSRuleDataMap<T>::get(const T& name) const
{
    auto it = m_ruleDataMap.find(name);
    if(it == m_ruleDataMap.end())
        return nullptr;
    return &it->second;
}

class CSSPageRuleData {
public:
    CSSPageRuleData(const CSSPageRule* rule, const CSSPageSelector& selector, uint32_t specificity, uint32_t position)
        : m_rule(rule), m_selector(&selector), m_specificity(specificity), m_position(position)
    {}

    const CSSPageRule* rule() const { return m_rule; }
    const CSSPageSelector* selector() const { return m_selector; }
    const uint32_t& specificity() const { return m_specificity; }
    const uint32_t& position() const { return m_position; }
    bool match(const GlobalString& pageName, size_t pageIndex) const;

private:
    const CSSPageRule* m_rule;
    const CSSPageSelector* m_selector;
    uint32_t m_specificity;
    uint32_t m_position;
};

inline bool operator<(const CSSPageRuleData& a, const CSSPageRuleData& b) { return std::tie(a.specificity(), a.position()) < std::tie(b.specificity(), b.position()); }
inline bool operator>(const CSSPageRuleData& a, const CSSPageRuleData& b) { return std::tie(a.specificity(), a.position()) > std::tie(b.specificity(), b.position()); }

using CSSPageRuleDataList = std::multiset<CSSPageRuleData, std::greater<CSSPageRuleData>>;

class FontFace;

class CSSFontFaceCache {
public:
    CSSFontFaceCache() = default;
    RefPtr<FontFace> get(const std::string_view& family, bool italic, bool smallCaps, int weight) const;
    void add(const HeapString& family, bool italic, bool smallCaps, int weight, RefPtr<FontFace> face);

private:
    using FontFaceData = std::tuple<bool, bool, int, RefPtr<FontFace>>;
    using FontFaceDataList = std::vector<FontFaceData>;
    using FontFaceDataMap = std::map<HeapString, FontFaceDataList, std::less<>>;
    FontFaceDataMap m_fontFaceDataMap;
};

class BoxStyle;

class CSSRuleCache {
public:
    static std::unique_ptr<CSSRuleCache> create(Document* document);

    RefPtr<BoxStyle> styleForElement(Element* element, const BoxStyle& parentStyle) const;
    RefPtr<BoxStyle> pseudoStyleForElement(Element* element, const BoxStyle& parentStyle, PseudoType pseudoType) const;
    RefPtr<FontFace> getFontFace(const std::string_view& family, bool italic, bool smallCaps, int weight) const;

private:
    CSSRuleCache(Document* document);

    void addRules(Document* document, uint32_t& position, const CSSRuleList& rules);
    void addStyleRule(uint32_t position, const CSSStyleRule* rule);
    void addPageRule(uint32_t position, const CSSPageRule* rule);
    void addFontFaceRule(Document* document, const CSSFontFaceRule* rule);

    CSSRuleDataMap<HeapString> m_idRules;
    CSSRuleDataMap<HeapString> m_classRules;
    CSSRuleDataMap<GlobalString> m_tagRules;
    CSSRuleDataMap<PseudoType> m_pseudoRules;

    CSSRuleDataList m_universeRules;
    CSSPageRuleDataList m_pageRules;
    CSSFontFaceCache m_fontFaceCache;
};

class CSSStyleBuilder {
public:
    CSSStyleBuilder(Element* element, const BoxStyle& parentStyle, PseudoType pseudoType)
        : m_pseudoType(pseudoType), m_element(element), m_parentStyle(parentStyle)
    {}

    void add(const CSSRuleDataList* rules);
    void add(const CSSPropertyList& properties);

    RefPtr<BoxStyle> build();

private:
    PseudoType m_pseudoType;
    Element* m_element;
    const BoxStyle& m_parentStyle;
    CSSPropertyList m_properties;
    CSSRuleDataList m_rules;
};

} // namespace htmlbook

#endif // CSSRULE_H
