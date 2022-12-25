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

class CSSValue : public HeapMember, public RefCounted<CSSValue> {
public:
    enum class Type {
        Initial,
        Inherit,
        Ident,
        CustomIdent,
        Integer,
        Number,
        Percent,
        Angle,
        Length,
        String,
        Url,
        Image,
        Color,
        Counter,
        Pair,
        Rect,
        List,
        Function
    };

    virtual ~CSSValue() = default;
    virtual Type type() const = 0;

protected:
    CSSValue() = default;
};

using CSSValueList = std::pmr::vector<RefPtr<CSSValue>>;

class CSSInitialValue final : public CSSValue {
public:
    static RefPtr<CSSInitialValue> create(Heap* heap);

    Type type() const final { return Type::Initial; }

private:
    CSSInitialValue() = default;
};

template<>
struct is_a<CSSInitialValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValue::Type::Initial; }
};

class CSSInheritValue final : public CSSValue {
public:
    static RefPtr<CSSInheritValue> create(Heap* heap);

    Type type() const final { return Type::Inherit; }

private:
    CSSInheritValue() = default;
};

template<>
struct is_a<CSSInheritValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValue::Type::Inherit; }
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
    static RefPtr<CSSIdentValue> create(Heap* heap, CSSValueID value);

    CSSValueID value() const { return m_value; }
    Type type() const final { return Type::Ident; }

private:
    CSSIdentValue(CSSValueID value) : m_value(value) {}
    CSSValueID m_value;
};

template<>
struct is_a<CSSIdentValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValue::Type::Ident; }
};

class CSSCustomIdentValue final : public CSSValue {
public:
    static RefPtr<CSSCustomIdentValue> create(Heap* heap, const HeapString& value);

    const HeapString& value() const { return m_value; }
    Type type() const final { return Type::CustomIdent; }

private:
    CSSCustomIdentValue(const HeapString& value) : m_value(value) {}
    HeapString m_value;
};

template<>
struct is_a<CSSCustomIdentValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValue::Type::CustomIdent; }
};

class CSSIntegerValue final : public CSSValue {
public:
    static RefPtr<CSSIntegerValue> create(Heap* heap, int value);

    int value() const { return m_value; }
    Type type() const final { return Type::Integer; }

private:
    CSSIntegerValue(int value) : m_value(value) {}
    int m_value;
};

template<>
struct is_a<CSSIntegerValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValue::Type::Integer; }
};

class CSSNumberValue final : public CSSValue {
public:
    static RefPtr<CSSNumberValue> create(Heap* heap, double value);

    double value() const { return m_value; }
    Type type() const final { return Type::Number; }

private:
    CSSNumberValue(double value) : m_value(value) {}
    double m_value;
};

template<>
struct is_a<CSSNumberValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValue::Type::Number; }
};

class CSSPercentValue final : public CSSValue {
public:
    static RefPtr<CSSPercentValue> create(Heap* heap, double value);

    double value() const { return m_value; }
    Type type() const final { return Type::Percent; }

private:
    CSSPercentValue(double value) : m_value(value) {}
    double m_value;
};

template<>
struct is_a<CSSPercentValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValue::Type::Percent; }
};

class CSSAngleValue final : public CSSValue {
public:
    enum class Unit {
        Degrees,
        Radians,
        Gradians,
        Turns,
    };

    static RefPtr<CSSAngleValue> create(Heap* heap, double value, Unit unit);

    double value() const { return m_value; }
    Unit unit() const { return m_unit; }
    Type type() const final { return Type::Angle; }

private:
    CSSAngleValue(double value, Unit unit)
        : m_value(value), m_unit(unit)
    {}

    double m_value;
    Unit m_unit;
};

template<>
struct is_a<CSSAngleValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValue::Type::Angle; }
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

    static RefPtr<CSSLengthValue> create(Heap* heap, double value, Unit unit);

    double value() const { return m_value; }
    Unit unit() const { return m_unit; }
    Type type() const final { return Type::Length; }

private:
    CSSLengthValue(double value, Unit unit)
        : m_value(value), m_unit(unit)
    {}

    double m_value;
    Unit m_unit;
};

template<>
struct is_a<CSSLengthValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValue::Type::Length; }
};

class CSSStringValue final : public CSSValue {
public:
    static RefPtr<CSSStringValue> create(Heap* heap, const HeapString& value);

    const HeapString& value() const { return m_value; }
    Type type() const final { return Type::String; }

private:
    CSSStringValue(const HeapString& value) : m_value(value) {}
    HeapString m_value;
};

template<>
struct is_a<CSSStringValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValue::Type::String; }
};

class CSSUrlValue final : public CSSValue {
public:
    static RefPtr<CSSUrlValue> create(Heap* heap, const HeapString& value);

    const HeapString& value() const { return m_value; }
    Type type() const final { return Type::Url; }

private:
    CSSUrlValue(const HeapString& value) : m_value(value) {}
    HeapString m_value;
};

template<>
struct is_a<CSSUrlValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValue::Type::Url; }
};

class Image;

class CSSImageValue final : public CSSValue {
public:
    static RefPtr<CSSImageValue> create(Heap* heap, const HeapString& value);

    const HeapString& value() const { return m_value; }
    const RefPtr<Image>& image() const { return m_image; }
    RefPtr<Image> fetch(Document* document) const;
    Type type() const final { return Type::Image; }

private:
    CSSImageValue(const HeapString& value);
    HeapString m_value;
    mutable RefPtr<Image> m_image;
};

template<>
struct is_a<CSSImageValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValue::Type::Image; }
};

class CSSColorValue final : public CSSValue {
public:
    static RefPtr<CSSColorValue> create(Heap* heap, uint32_t value);
    static RefPtr<CSSColorValue> create(Heap* heap, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

    uint32_t value() const { return m_value; }
    Type type() const final { return Type::Color; }

private:
    CSSColorValue(uint32_t value) : m_value(value) {}
    uint32_t m_value;
};

template<>
struct is_a<CSSColorValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValue::Type::Color; }
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
    static RefPtr<CSSCounterValue> create(Heap* heap, const GlobalString& identifier, ListStyleType listStyle, const HeapString& separator);

    const GlobalString& identifier() const { return m_identifier; }
    ListStyleType listStyle() const { return m_listStyle; }
    const HeapString& separator() const { return m_separator; }
    Type type() const final { return Type::Counter; }

private:
    CSSCounterValue(const GlobalString& identifier, ListStyleType listStyle, const HeapString& separator)
        : m_identifier(identifier), m_listStyle(listStyle), m_separator(separator)
    {}

    GlobalString m_identifier;
    ListStyleType m_listStyle;
    HeapString m_separator;
};

template<>
struct is_a<CSSCounterValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValue::Type::Counter; }
};

class CSSPairValue final : public CSSValue {
public:
    static RefPtr<CSSPairValue> create(Heap* heap, RefPtr<CSSValue> first, RefPtr<CSSValue> second);

    const RefPtr<CSSValue>& first() const { return m_first; }
    const RefPtr<CSSValue>& second() const { return m_second; }
    Type type() const final { return Type::Pair; }

private:
    CSSPairValue(RefPtr<CSSValue> first, RefPtr<CSSValue> second)
        : m_first(first), m_second(second)
    {}

    RefPtr<CSSValue> m_first;
    RefPtr<CSSValue> m_second;
};

template<>
struct is_a<CSSPairValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValue::Type::Pair; }
};

class CSSRectValue final : public CSSValue {
public:
    static RefPtr<CSSRectValue> create(Heap* heap, RefPtr<CSSValue> top, RefPtr<CSSValue> right, RefPtr<CSSValue> bottom, RefPtr<CSSValue> left);

    const RefPtr<CSSValue>& top() const { return m_top; }
    const RefPtr<CSSValue>& right() const { return m_right; }
    const RefPtr<CSSValue>& bottom() const { return m_bottom; }
    const RefPtr<CSSValue>& left() const { return m_left; }
    Type type() const final { return Type::Rect; }

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
struct is_a<CSSRectValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValue::Type::Rect; }
};

class CSSListValue : public CSSValue {
public:
    static RefPtr<CSSListValue> create(Heap* heap, CSSValueList values);

    const RefPtr<CSSValue>& front() const { return m_values.front(); }
    const RefPtr<CSSValue>& back() const { return m_values.back(); }
    const RefPtr<CSSValue>& at(size_t index) const { return m_values.at(index); }
    const CSSValueList& values() const { return m_values; }
    size_t size() const { return m_values.size(); }
    bool empty() const { return m_values.empty(); }
    Type type() const override { return Type::List; }

protected:
    CSSListValue(CSSValueList values) : m_values(std::move(values)) {}
    CSSValueList m_values;
};

template<>
struct is_a<CSSListValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValue::Type::List; }
};

class CSSFunctionValue final : public CSSListValue {
public:
    static RefPtr<CSSFunctionValue> create(Heap* heap, CSSValueID id, CSSValueList values);
    static RefPtr<CSSFunctionValue> create(Heap* heap, CSSValueID id, RefPtr<CSSValue> value);

    CSSValueID id() const { return m_id; }
    Type type() const final { return Type::Function; }

private:
    CSSFunctionValue(CSSValueID id, CSSValueList values)
        : m_id(id), CSSListValue(std::move(values))
    {}

    CSSValueID m_id;
};

template<>
struct is_a<CSSFunctionValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValue::Type::Function; }
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

using CSSPropertyList = std::pmr::list<CSSProperty>;
using CSSPropertyMap = std::pmr::map<CSSPropertyID, RefPtr<CSSValue>>;

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

class CSSRule : public HeapMember, public RefCounted<CSSRule> {
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

using CSSRuleList = std::pmr::list<RefPtr<CSSRule>>;

class CSSStyleRule final : public CSSRule {
public:
    static RefPtr<CSSStyleRule> create(Heap* heap, CSSSelectorList selectors, CSSPropertyList properties);

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
struct is_a<CSSStyleRule> {
    static bool check(const CSSRule& value) { return value.type() == CSSRule::Type::Style; }
};

class CSSImportRule final : public CSSRule {
public:
    static RefPtr<CSSImportRule> create(Heap* heap, const HeapString& href);

    const HeapString& href() const { return m_href; }
    Type type() const final { return Type::Import; }
    const CSSRuleList& fetch(Document* document) const;

private:
    CSSImportRule(Heap* heap, const HeapString& href)
        : m_href(href), m_rules(heap), m_heap(heap)
    {}

    HeapString m_href;
    mutable CSSRuleList m_rules;
    Heap* m_heap;
};

template<>
struct is_a<CSSImportRule> {
    static bool check(const CSSRule& value) { return value.type() == CSSRule::Type::Import; }
};

class CSSFontFaceRule : public CSSRule {
public:
    static RefPtr<CSSFontFaceRule> create(Heap* heap, CSSPropertyList properties);

    const CSSPropertyList& properties() const { return m_properties; }
    Type type() const final { return Type::FontFace; }

private:
    CSSFontFaceRule(CSSPropertyList properties)
        : m_properties(std::move(properties))
    {}

    CSSPropertyList m_properties;
};

template<>
struct is_a<CSSFontFaceRule> {
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

    static RefPtr<CSSPageMarginRule> create(Heap* heap, MarginType marginType, CSSPropertyList properties);

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
struct is_a<CSSPageMarginRule> {
    static bool check(const CSSRule& value) { return value.type() == CSSRule::Type::PageMargin; }
};

using CSSPageMarginRuleList = std::pmr::list<RefPtr<CSSPageMarginRule>>;

class CSSPageRule : public CSSRule {
public:
    static RefPtr<CSSPageRule> create(Heap* heap, CSSPageSelectorList selectors, CSSPageMarginRuleList margins, CSSPropertyList properties);

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
struct is_a<CSSPageRule> {
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
    CSSRuleData(const RefPtr<CSSStyleRule>& rule, const CSSSelector* selector, uint32_t specificity, uint32_t position)
        : m_rule(rule), m_selector(selector), m_specificity(specificity), m_position(position)
    {}

    const RefPtr<CSSStyleRule>& rule() const { return m_rule; }
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
    RefPtr<CSSStyleRule> m_rule;
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
    CSSPageRuleData(const RefPtr<CSSPageRule>& rule, const CSSPageSelector* selector, uint32_t specificity, uint32_t position)
        : m_rule(rule), m_selector(selector), m_specificity(specificity), m_position(position)
    {}

    const RefPtr<CSSPageRule>& rule() const { return m_rule; }
    const CSSPageSelector* selector() const { return m_selector; }
    const uint32_t& specificity() const { return m_specificity; }
    const uint32_t& position() const { return m_position; }

    bool match(const std::string_view& pageName, size_t pageIndex) const;

private:
    RefPtr<CSSPageRule> m_rule;
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

class CSSStyleSheet {
public:
    explicit CSSStyleSheet(Document* document);

    RefPtr<BoxStyle> styleForElement(Element* element, const RefPtr<BoxStyle>& parentStyle) const;
    RefPtr<BoxStyle> pseudoStyleForElement(Element* element, const RefPtr<BoxStyle>& parentStyle, PseudoType pseudoType) const;
    RefPtr<FontFace> getFontFace(const std::string_view& family, bool italic, bool smallCaps, int weight) const;

    void parseStyle(const std::string_view& content);

private:
    void addRules(const CSSRuleList& rules);
    void addStyleRule(const RefPtr<CSSStyleRule>& rule);
    void addPageRule(const RefPtr<CSSPageRule>& rule);
    void addFontFaceRule(const RefPtr<CSSFontFaceRule>& rule);

    Document* m_document;
    CSSRuleDataMap<HeapString> m_idRules;
    CSSRuleDataMap<HeapString> m_classRules;
    CSSRuleDataMap<GlobalString> m_tagRules;
    CSSRuleDataMap<PseudoType> m_pseudoRules;

    CSSRuleDataList m_universeRules;
    CSSPageRuleDataList m_pageRules;
    CSSFontFaceCache m_fontFaceCache;

    uint32_t m_position{0};
};

class CSSStyleBuilder {
public:
    CSSStyleBuilder(Element* element, const RefPtr<BoxStyle>& parentStyle, PseudoType pseudoType);

    void add(const CSSRuleDataList* rules);
    void add(const CSSPropertyList& properties);

    RefPtr<BoxStyle> build();

private:
    PseudoType m_pseudoType;
    Element* m_element;
    RefPtr<BoxStyle> m_parentStyle;
    CSSPropertyList m_properties;
    CSSRuleDataList m_rules;
};

} // namespace htmlbook

#endif // CSSRULE_H
