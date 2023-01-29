#ifndef BOXSTYLE_H
#define BOXSTYLE_H

#include "cssrule.h"

#include <optional>

namespace htmlbook {

enum class Display : uint8_t {
    None,
    Block,
    Flex,
    Inline,
    InlineBlock,
    InlineFlex,
    InlineTable,
    ListItem,
    Table,
    TableCaption,
    TableCell,
    TableColumn,
    TableColumnGroup,
    TableFooterGroup,
    TableHeaderGroup,
    TableRow,
    TableRowGroup
};

enum class Visibility : uint8_t {
    Visible,
    Hidden,
    Collapse
};

enum class Float : uint8_t {
    None,
    Left,
    Right
};

enum class Clear : uint8_t {
    None,
    Left,
    Right,
    Both
};

enum class Position : uint8_t {
    Static,
    Relative,
    Absolute,
    Fixed
};

enum class Overflow : uint8_t {
    Auto,
    Visible,
    Hidden,
    Scroll
};

enum class LineStyle : uint8_t {
    None,
    Auto,
    Hidden,
    Inset,
    Groove,
    Outset,
    Ridge,
    Dotted,
    Dashed,
    Solid,
    Double
};

enum class ListStylePosition : uint8_t {
    Outside,
    Inside
};

enum class BackgroundRepeat : uint8_t {
    Repeat,
    RepeatX,
    RepeatY,
    NoRepeat
};

enum class BackgroundBox : uint8_t {
    BorderBox,
    PaddingBox,
    ContentBox
};

enum class BackgroundAttachment : uint8_t {
    Scroll,
    Fixed,
    Local
};

enum class FontStyle : uint8_t {
    Normal,
    Italic,
    Oblique
};

enum class FontVariant : uint8_t {
    Normal,
    SmallCaps
};

enum class TextDirection : uint8_t {
    Ltr,
    Rtl
};

enum class TextAlign : uint8_t {
    Left,
    Center,
    Right,
    Justify
};

enum class TextTransform : uint8_t {
    None,
    Capitalize,
    Uppercase,
    Lowercase
};

enum class TextOverflow : uint8_t {
    Clip,
    Ellipsis
};

enum class TextDecorationLine : uint8_t {
    None = 0x0,
    Underline = 0x1,
    Overline = 0x2,
    LineThrough = 0x4
};

enum class TextDecorationStyle : uint8_t {
    Solid,
    Double,
    Dotted,
    Dashed,
    Wavy
};

enum class WhiteSpace : uint8_t {
    Normal,
    Pre,
    Nowrap,
    PreLine,
    PreWrap,
    BreakSpaces
};

enum class LineBreak : uint8_t {
    Auto,
    Loose,
    Normal,
    Strict,
    Anywhere
};

enum class WordBreak : uint8_t {
    Normal,
    BreakAll,
    KeepAll,
    BreakWord
};

enum class WordWrap : uint8_t {
    Normal,
    BreakWord,
    Anywhere
};

enum class Hyphens : uint8_t {
    Auto,
    None,
    Manual
};

enum class TableLayout : uint8_t {
    Auto,
    Fixed
};

enum class CaptionSide : uint8_t {
    Top,
    Bottom
};

enum class EmptyCells : uint8_t {
    Show,
    Hide
};

enum class BorderCollapse : uint8_t {
    Separate,
    Collapse
};

enum class BoxSizing : uint8_t {
    ContentBox,
    BorderBox
};

enum class FlexDirection : uint8_t {
    Row,
    RowReverse,
    Column,
    ColumnReverse
};

enum class FlexWrap : uint8_t {
    Nowrap,
    Wrap,
    WrapReverse
};

enum class AlignContent : uint8_t {
    FlexStart,
    FlexEnd,
    Center,
    SpaceBetween,
    SpaceAround,
    Stretch
};

enum class AlignItem : uint8_t {
    Auto,
    FlexStart,
    FlexEnd,
    Center,
    Baseline,
    Stretch
};

enum class ColumnSpan : uint8_t {
    None,
    All
};

enum class ColumnFill : uint8_t {
    Auto,
    Balance
};

enum class QuoteType : uint8_t {
    Open,
    Close,
    NoOpen,
    NoClose
};

class Color {
public:
    Color() = default;
    explicit Color(uint32_t value) : m_value(value) {}

    uint8_t alpha() const { return (m_value >> 24) & 0xff; }
    uint8_t red() const { return (m_value >> 16) & 0xff; }
    uint8_t green() const { return (m_value >> 8) & 0xff; }
    uint8_t blue() const { return (m_value >> 0) & 0xff; }

    uint32_t value() const { return m_value; }

    static const Color Transparent;
    static const Color Black;
    static const Color White;

private:
    uint32_t m_value{0};
};

class Length {
public:
    enum class Type {
        None,
        Auto,
        Percent,
        Fixed
    };

    Length(Type type, float value) : m_type(type), m_value(value) {}

    bool isNone() const { return m_type == Type::None; }
    bool isAuto() const { return m_type == Type::Auto; }
    bool isFixed() const { return m_type == Type::Fixed; }
    bool isPercent() const { return m_type == Type::Percent; }
    bool isZero() const { return m_value == 0; }

    float value() const { return m_value; }
    Type type() const { return m_type; }

    static const Length None;
    static const Length Auto;
    static const Length ZeroPercent;
    static const Length ZeroFixed;

    float calc(float maximum) const;
    float calcMin(float maximum) const;

private:
    Type m_type;
    float m_value;
};

inline float Length::calc(float maximum) const
{
    switch(m_type) {
    case Type::Fixed:
        return m_value;
    case Type::Percent:
        return m_value * maximum / 100.f;
    default:
        return maximum;
    }
}

inline float Length::calcMin(float maximum) const
{
    switch(m_type) {
    case Type::Fixed:
        return m_value;
    case Type::Percent:
        return m_value * maximum / 100.f;
    default:
        return 0;
    }
}

class LengthSize {
public:
    LengthSize(const Length& width, const Length& height)
        : m_width(width), m_height(height)
    {}

    const Length& width() const { return m_width; }
    const Length& height() const { return m_height; }

    static const LengthSize None;
    static const LengthSize Auto;
    static const LengthSize ZeroPercent;
    static const LengthSize ZeroFixed;

private:
    Length m_width;
    Length m_height;
};

class LengthRect {
public:
    LengthRect(const Length& left, const Length& right, const Length& top, const Length& bottom)
        : m_left(left), m_right(right), m_top(top), m_bottom(bottom)
    {}

    const Length& left() const { return m_left; }
    const Length& right() const { return m_right; }
    const Length& top() const { return m_top; }
    const Length& bottom() const { return m_bottom; }

    static const LengthRect None;
    static const LengthRect Auto;
    static const LengthRect ZeroPercent;
    static const LengthRect ZeroFixed;

private:
    Length m_left;
    Length m_right;
    Length m_top;
    Length m_bottom;
};

class BackgroundSize {
public:
    enum class Type {
        Contain,
        Cover,
        Length
    };

    BackgroundSize(Type type, const Length& width = Length::Auto, const Length& height = Length::Auto)
        : m_type(type), m_width(width), m_height(height)
    {}

    Type type() const { return m_type; }
    const Length& width() const { return m_width; }
    const Length& height() const { return m_height; }

private:
    Type m_type;
    Length m_width;
    Length m_height;
};

class VerticalAlign {
public:
    enum class Type {
        Baseline,
        Middle,
        Sub,
        Super,
        TextTop,
        TextBottom,
        Top,
        Bottom,
        BaselineMiddle,
        Length
    };

    VerticalAlign(Type type, const Length& value = Length::Auto)
        : m_type(type), m_value(value)
    {}

    Type type() const { return m_type; }
    const Length& value() const { return m_value; }

private:
    Type m_type;
    Length m_value;
};

class Node;

class BoxStyle : public HeapMember, public RefCounted<BoxStyle> {
public:
    static RefPtr<BoxStyle> create(Node* node, PseudoType pseudoType, Display display);
    static RefPtr<BoxStyle> create(const RefPtr<BoxStyle>& parentStyle, Display display);

    Document* document() const;
    Heap* heap() const;

    Node* node() const { return m_node; }
    PseudoType pseudoType() const { return m_pseudoType; }
    const CSSPropertyMap& properties() const { return m_properties; }
    RefPtr<FontFace> fontFace() const;

    float fontSize() const { return m_fontSize; }
    int fontWeight() const { return m_fontWeight; }
    FontStyle fontStyle() const { return m_fontStyle; }
    FontVariant fontVariant() const { return m_fontVariant; }

    Display display() const;
    Float floating() const;
    Position position() const { return m_position; }
    Clear clear() const { return m_clear; }
    Overflow overflow() const { return m_overflow; }
    Visibility visibility() const { return m_visibility; }
    const Color& color() const { return m_color; }

    Length left() const;
    Length right() const;
    Length top() const;
    Length bottom() const;
    Length width() const;
    Length height() const;
    Length minWidth() const;
    Length minHeight() const;
    Length maxWidth() const;
    Length maxHeight() const;

    Length marginLeft() const;
    Length marginRight() const;
    Length marginTop() const;
    Length marginBottom() const;

    Length paddingLeft() const;
    Length paddingRight() const;
    Length paddingTop() const;
    Length paddingBottom() const;

    LineStyle borderLeftStyle() const;
    LineStyle borderRightStyle() const;
    LineStyle borderTopStyle() const;
    LineStyle borderBottomStyle() const;

    Color borderLeftColor() const;
    Color borderRightColor() const;
    Color borderTopColor() const;
    Color borderBottomColor() const;

    float borderLeftWidth() const;
    float borderRightWidth() const;
    float borderTopWidth() const;
    float borderBottomWidth() const;

    LengthSize borderTopLeftRadius() const;
    LengthSize borderTopRightRadius() const;
    LengthSize borderBottomLeftRadius() const;
    LengthSize borderBottomRightRadius() const;

    ListStyleType listStyleType() const;
    ListStylePosition listStylePosition() const;
    RefPtr<Image> listStyleImage() const;

    RefPtr<Image> backgroundImage() const;
    Color backgroundColor() const;
    BackgroundRepeat backgroundRepeat() const;
    BackgroundBox backgroundOrigin() const;
    BackgroundBox backgroundClip() const;
    BackgroundAttachment backgroundAttachment() const;
    BackgroundSize backgroundSize() const;
    LengthSize backgroundPosition() const;

    TableLayout tableLayout() const;
    CaptionSide captionSide() const;
    EmptyCells emptyCells() const;
    BorderCollapse borderCollapse() const;
    float borderHorizontalSpacing() const;
    float borderVerticalSpacing() const;

    TextDirection direction() const { return m_direction; }
    TextAlign textAlign() const { return m_textAlign; }
    TextTransform textTransform() const;
    TextOverflow textOverflow() const;
    TextDecorationLine textDecorationLine() const;
    TextDecorationStyle textDecorationStyle() const;
    Color textDecorationColor() const;
    WhiteSpace whiteSpace() const { return m_whiteSpace; }
    LineBreak lineBreak() const;
    WordBreak wordBreak() const;
    WordWrap wordWrap() const;
    Hyphens hyphens() const;
    float tabSize() const;
    Length textIndent() const;

    BoxSizing boxSizing() const { return m_boxSizing; }
    std::optional<int> zIndex() const;
    VerticalAlign verticalAlign() const;
    LengthRect clip() const;

    Length flexBasis() const;
    float flexGrow() const;
    float flexShrink() const;
    int order() const;
    FlexDirection flexDirection() const;
    FlexWrap flexWrap() const;
    AlignContent justifyContent() const;
    AlignContent alignContent() const;
    AlignItem alignItems() const;
    AlignItem alignSelf() const;

    float outlineOffset() const;
    Color outlineColor() const;
    float outlineWidth() const;
    LineStyle outlineStyle() const;

    Color columnRuleColor() const;
    LineStyle columnRuleStyle() const;
    ColumnSpan columnSpan() const;
    ColumnFill columnFill() const;
    std::optional<float> columnWidth() const;
    std::optional<float> columnGap() const;
    std::optional<int> columnCount() const;

    int widows() const;
    int orphans() const;

    bool isLeftToRightDirection() const { return m_direction == TextDirection::Ltr; }
    bool isRightToLeftDirection() const { return m_direction == TextDirection::Rtl; }
    bool isRowFlexDirection() const { return flexDirection() == FlexDirection::Row || flexDirection() == FlexDirection::RowReverse; }
    bool isColumnFlexDirection() const { return flexDirection() == FlexDirection::Column || flexDirection() == FlexDirection::ColumnReverse; }
    bool isReverseFlexDirection() const { return flexDirection() == FlexDirection::RowReverse || flexDirection() == FlexDirection::ColumnReverse; }
    bool isClearLeft() const { return m_clear == Clear::Left || m_clear == Clear::Both; }
    bool isClearRight() const { return m_clear == Clear::Right || m_clear == Clear::Both; }
    bool hasTransform() const;

    const HeapString& getQuote(bool open, size_t depth) const;

    RefPtr<CSSValue> get(CSSPropertyID id) const;
    void set(CSSPropertyID id, RefPtr<CSSValue> value);
    void remove(CSSPropertyID id);
    bool has(CSSPropertyID id) const { return get(id); }

    float exFontSize() const;
    float chFontSize() const;
    float remFontSize() const;

    float viewportWidth() const;
    float viewportHeight() const;
    float viewportMin() const;
    float viewportMax() const;

    float convertLengthValue(const CSSValue& value) const;
    float convertLineWidth(const CSSValue& value) const;
    float convertFontSize(const CSSValue& value) const;
    int convertFontWeight(const CSSValue& value) const;
    std::optional<float> convertLengthOrAuto(const CSSValue& value) const;
    std::optional<float> convertLengthOrNormal(const CSSValue& value) const;
    Length convertLength(const CSSValue& value) const;
    Length convertLengthOrPercent(const CSSValue& value) const;
    Length convertLengthOrPercentOrAuto(const CSSValue& value) const;
    Length convertLengthOrPercentOrNone(const CSSValue& value) const;
    Length convertPositionLength(CSSValueID min, CSSValueID max, const CSSValue& value) const;
    LengthSize convertBorderRadius(const CSSValue& value) const;
    Color convertColor(const CSSValue& value) const;
    RefPtr<Image> convertImage(const CSSValue& value) const;
    RefPtr<Image> convertImageOrNone(const CSSValue& value) const;

    static Display convertDisplay(const CSSValue& value);
    static Position convertPosition(const CSSValue& value);
    static Float convertFloat(const CSSValue& value);
    static Clear convertClear(const CSSValue& value);
    static Overflow convertOverflow(const CSSValue& value);
    static Visibility convertVisibility(const CSSValue& value);
    static BoxSizing convertBoxSizing(const CSSValue& value);
    static WhiteSpace convertWhiteSpace(const CSSValue& value);
    static TextDirection convertTextDirection(const CSSValue& value);
    static TextAlign convertTextAlign(const CSSValue& value);
    static BackgroundBox convertBackgroundBox(const CSSValue& value);
    static LineStyle convertLineStyle(const CSSValue& value);
    static FontStyle convertFontStyle(const CSSValue& value);
    static FontVariant convertFontVariant(const CSSValue& value);
    static AlignContent convertAlignContent(const CSSValue& value);
    static AlignItem convertAlignItem(const CSSValue& value);
    static int convertInteger(const CSSValue& value);
    static std::optional<int> convertIntegerOrAuto(const CSSValue& value);
    static float convertNumber(const CSSValue& value);

    void inheritFrom(const BoxStyle& parentStyle);

    ~BoxStyle();

private:
    BoxStyle(Node* node, PseudoType pseudoType, Display display);
    Node* m_node;
    CSSPropertyMap m_properties;
    mutable RefPtr<FontFace> m_fontFace;
    PseudoType m_pseudoType;
    Display m_display;
    Position m_position{Position::Static};
    Float m_floating{Float::None};
    Clear m_clear{Clear::None};
    Overflow m_overflow{Overflow::Visible};
    Visibility m_visibility{Visibility::Visible};
    BoxSizing m_boxSizing{BoxSizing::ContentBox};
    TextDirection m_direction{TextDirection::Ltr};
    TextAlign m_textAlign{TextAlign::Left};
    WhiteSpace m_whiteSpace{WhiteSpace::Normal};
    FontStyle m_fontStyle{FontStyle::Normal};
    FontVariant m_fontVariant{FontVariant::Normal};
    float m_fontSize{12.0};
    int m_fontWeight{400};
    Color m_color{Color::Black};
};

} // namespace htmlbook

#endif // BOXSTYLE_H
