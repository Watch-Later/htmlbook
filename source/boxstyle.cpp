#include "boxstyle.h"

#include <cassert>

namespace htmlbook {

const Color Color::Transparent(0x00000000);
const Color Color::Black(0x000000FF);
const Color Color::White(0xFFFFFFFF);

const Length Length::None(Length::Type::None, 0);
const Length Length::Auto(Length::Type::Auto,  0);
const Length Length::ZeroPercent(Length::Type::Percent, 0);
const Length Length::ZeroFixed(Length::Type::Fixed, 0);

const LengthSize LengthSize::None(Length::None, Length::None);
const LengthSize LengthSize::Auto(Length::Auto, Length::Auto);
const LengthSize LengthSize::ZeroPercent(Length::ZeroPercent, Length::ZeroPercent);
const LengthSize LengthSize::ZeroFixed(Length::ZeroFixed, Length::ZeroFixed);

const LengthRect LengthRect::None(Length::None, Length::None, Length::None, Length::None);
const LengthRect LengthRect::Auto(Length::Auto, Length::Auto, Length::Auto, Length::Auto);
const LengthRect LengthRect::ZeroPercent(Length::ZeroPercent, Length::ZeroPercent, Length::ZeroPercent, Length::ZeroPercent);
const LengthRect LengthRect::ZeroFixed(Length::ZeroFixed, Length::ZeroFixed, Length::ZeroFixed, Length::ZeroFixed);

std::shared_ptr<BoxStyle> BoxStyle::create(const Element* element)
{
    return std::shared_ptr<BoxStyle>(new BoxStyle(element));
}

std::shared_ptr<BoxStyle> BoxStyle::create(const BoxStyle& parentStyle)
{
    std::shared_ptr<BoxStyle> newStyle(new BoxStyle(parentStyle.element()));
    newStyle->inheritFrom(parentStyle);
    return newStyle;
}

Display BoxStyle::display() const
{
    auto value = get(CSSPropertyID::Display);
    if(value == nullptr)
        return Display::Inline;
    assert(value->isIdentValue());
    auto ident = value->toIdentValue();
    switch(ident->value()) {
    case CSSValueID::None:
        return Display::None;
    case CSSValueID::Block:
        return Display::Block;
    case CSSValueID::Flex:
        return Display::Flex;
    case CSSValueID::Inline:
        return Display::Inline;
    case CSSValueID::InlineBlock:
        return Display::InlineBlock;
    case CSSValueID::InlineFlex:
        return Display::InlineFlex;
    case CSSValueID::InlineTable:
        return Display::InlineTable;
    case CSSValueID::ListItem:
        return Display::ListItem;
    case CSSValueID::Table:
        return Display::Table;
    case CSSValueID::TableCaption:
        return Display::TableCaption;
    case CSSValueID::TableCell:
        return Display::TableCell;
    case CSSValueID::TableColumn:
        return Display::TableColumn;
    case CSSValueID::TableColumnGroup:
        return Display::TableColumnGroup;
    case CSSValueID::TableFooterGroup:
        return Display::TableFooterGroup;
    case CSSValueID::TableHeaderGroup:
        return Display::TableHeaderGroup;
    case CSSValueID::TableRow:
        return Display::TableRow;
    case CSSValueID::TableRowGroup:
        return Display::TableRowGroup;
    default:
        assert(false);
    }
}

Visibility BoxStyle::visibility() const
{
    auto value = get(CSSPropertyID::Visibility);
    if(value == nullptr)
        return Visibility::Visible;
    assert(value->isIdentValue());
    auto ident = value->toIdentValue();
    switch(ident->value()) {
    case CSSValueID::Visible:
        return Visibility::Visible;
    case CSSValueID::Hidden:
        return Visibility::Hidden;
    case CSSValueID::Collapse:
        return Visibility::Collapse;
    default:
        assert(false);
    }
}

Float BoxStyle::floating() const
{
    auto value = get(CSSPropertyID::Float);
    if(value == nullptr)
        return Float::None;
    assert(value->isIdentValue());
    auto ident = value->toIdentValue();
    switch(ident->value()) {
    case CSSValueID::None:
        return Float::None;
    case CSSValueID::Left:
        return Float::Left;
    case CSSValueID::Right:
        return Float::Right;
    default:
        assert(false);
    }
}

Clear BoxStyle::clear() const
{
    auto value = get(CSSPropertyID::Clear);
    if(value == nullptr)
        return Clear::None;
    assert(value->isIdentValue());
    auto ident = value->toIdentValue();
    switch(ident->value()) {
    case CSSValueID::None:
        return Clear::None;
    case CSSValueID::Left:
        return Clear::Left;
    case CSSValueID::Right:
        return Clear::Right;
    case CSSValueID::Both:
        return Clear::Both;
    default:
        assert(false);
    }
}

Position BoxStyle::position() const
{
    auto value = get(CSSPropertyID::Clear);
    if(value == nullptr)
        return Position::Static;
    assert(value->isIdentValue());
    auto ident = value->toIdentValue();
    switch(ident->value()) {
    case CSSValueID::Static:
        return Position::Static;
    case CSSValueID::Relative:
        return Position::Relative;
    case CSSValueID::Absolute:
        return Position::Absolute;
    case CSSValueID::Fixed:
        return Position::Fixed;
    default:
        assert(false);
    }
}

Overflow BoxStyle::overflowX() const
{
    auto value = get(CSSPropertyID::OverflowX);
    if(value == nullptr)
        return Overflow::Visible;
    return convertOverflow(*value);
}

Overflow BoxStyle::overflowY() const
{
    auto value = get(CSSPropertyID::OverflowY);
    if(value == nullptr)
        return Overflow::Visible;
    return convertOverflow(*value);
}

Color BoxStyle::color() const
{
    auto value = get(CSSPropertyID::Color);
    if(value == nullptr)
        return m_currentColor;
    return convertColor(*value);
}

Length BoxStyle::left() const
{
    auto value = get(CSSPropertyID::Left);
    if(value == nullptr)
        return Length::Auto;
    return convertLengthOrPercentOrAuto(*value);
}

Length BoxStyle::right() const
{
    auto value = get(CSSPropertyID::Right);
    if(value == nullptr)
        return Length::Auto;
    return convertLengthOrPercentOrAuto(*value);
}

Length BoxStyle::top() const
{
    auto value = get(CSSPropertyID::Top);
    if(value == nullptr)
        return Length::Auto;
    return convertLengthOrPercentOrAuto(*value);
}

Length BoxStyle::bottom() const
{
    auto value = get(CSSPropertyID::Bottom);
    if(value == nullptr)
        return Length::Auto;
    return convertLengthOrPercentOrAuto(*value);
}

Length BoxStyle::width() const
{
    auto value = get(CSSPropertyID::Width);
    if(value == nullptr)
        return Length::Auto;
    return convertLengthOrPercentOrAuto(*value);
}

Length BoxStyle::height() const
{
    auto value = get(CSSPropertyID::Height);
    if(value == nullptr)
        return Length::Auto;
    return convertLengthOrPercentOrAuto(*value);
}

Length BoxStyle::minWidth() const
{
    auto value = get(CSSPropertyID::MinWidth);
    if(value == nullptr)
        return Length::Auto;
    return convertLengthOrPercentOrAuto(*value);
}

Length BoxStyle::minHeight() const
{
    auto value = get(CSSPropertyID::MinHeight);
    if(value == nullptr)
        return Length::Auto;
    return convertLengthOrPercentOrAuto(*value);
}

Length BoxStyle::maxWidth() const
{
    auto value = get(CSSPropertyID::MaxWidth);
    if(value == nullptr)
        return Length::None;
    return convertLengthOrPercentOrNone(*value);
}

Length BoxStyle::maxHeight() const
{
    auto value = get(CSSPropertyID::MaxHeight);
    if(value == nullptr)
        return Length::None;
    return convertLengthOrPercentOrNone(*value);
}

Length BoxStyle::paddingLeft() const
{
    auto value = get(CSSPropertyID::PaddingLeft);
    if(value == nullptr)
        return Length::ZeroFixed;
    return convertLengthOrPercent(*value);
}

Length BoxStyle::paddingRight() const
{
    auto value = get(CSSPropertyID::PaddingRight);
    if(value == nullptr)
        return Length::ZeroFixed;
    return convertLengthOrPercent(*value);
}

Length BoxStyle::paddingTop() const
{
    auto value = get(CSSPropertyID::PaddingTop);
    if(value == nullptr)
        return Length::ZeroFixed;
    return convertLengthOrPercent(*value);
}

Length BoxStyle::paddingBottom() const
{
    auto value = get(CSSPropertyID::PaddingBottom);
    if(value == nullptr)
        return Length::ZeroFixed;
    return convertLengthOrPercent(*value);
}

Length BoxStyle::marginLeft() const
{
    auto value = get(CSSPropertyID::MarginLeft);
    if(value == nullptr)
        return Length::ZeroFixed;
    return convertLengthOrPercentOrAuto(*value);
}

Length BoxStyle::marginRight() const
{
    auto value = get(CSSPropertyID::MarginRight);
    if(value == nullptr)
        return Length::ZeroFixed;
    return convertLengthOrPercentOrAuto(*value);
}

Length BoxStyle::marginTop() const
{
    auto value = get(CSSPropertyID::MarginTop);
    if(value == nullptr)
        return Length::ZeroFixed;
    return convertLengthOrPercentOrAuto(*value);
}

Length BoxStyle::marginBottom() const
{
    auto value = get(CSSPropertyID::MarginBottom);
    if(value == nullptr)
        return Length::ZeroFixed;
    return convertLengthOrPercentOrAuto(*value);
}

LineStyle BoxStyle::borderLeftStyle() const
{
    auto value = get(CSSPropertyID::BorderLeftStyle);
    if(value == nullptr)
        return LineStyle::None;
    return convertLineStyle(*value);
}

LineStyle BoxStyle::borderRightStyle() const
{
    auto value = get(CSSPropertyID::BorderRightStyle);
    if(value == nullptr)
        return LineStyle::None;
    return convertLineStyle(*value);
}

LineStyle BoxStyle::borderTopStyle() const
{
    auto value = get(CSSPropertyID::BorderTopStyle);
    if(value == nullptr)
        return LineStyle::None;
    return convertLineStyle(*value);
}

LineStyle BoxStyle::borderBottomStyle() const
{
    auto value = get(CSSPropertyID::BorderBottomStyle);
    if(value == nullptr)
        return LineStyle::None;
    return convertLineStyle(*value);
}

Color BoxStyle::borderLeftColor() const
{
    auto value = get(CSSPropertyID::BorderLeftColor);
    if(value == nullptr)
        return m_currentColor;
    return convertColor(*value);
}

Color BoxStyle::borderRightColor() const
{
    auto value = get(CSSPropertyID::BorderRightColor);
    if(value == nullptr)
        return m_currentColor;
    return convertColor(*value);
}

Color BoxStyle::borderTopColor() const
{
    auto value = get(CSSPropertyID::BorderTopColor);
    if(value == nullptr)
        return m_currentColor;
    return convertColor(*value);
}

Color BoxStyle::borderBottomColor() const
{
    auto value = get(CSSPropertyID::BorderBottomColor);
    if(value == nullptr)
        return m_currentColor;
    return convertColor(*value);
}

float BoxStyle::borderLeftWidth() const
{
    auto value = get(CSSPropertyID::BorderLeftWidth);
    if(value == nullptr)
        return 3.0;
    return convertLineWidth(*value);
}

float BoxStyle::borderRightWidth() const
{
    auto value = get(CSSPropertyID::BorderRightWidth);
    if(value == nullptr)
        return 3.0;
    return convertLineWidth(*value);
}

float BoxStyle::borderTopWidth() const
{
    auto value = get(CSSPropertyID::BorderTopWidth);
    if(value == nullptr)
        return 3.0;
    return convertLineWidth(*value);
}

float BoxStyle::borderBottomWidth() const
{
    auto value = get(CSSPropertyID::BorderBottomWidth);
    if(value == nullptr)
        return 3.0;
    return convertLineWidth(*value);
}

LengthSize BoxStyle::borderTopLeftRadius() const
{
    auto value = get(CSSPropertyID::BorderTopLeftRadius);
    if(value == nullptr)
        return LengthSize::ZeroFixed;
    return convertBorderRadius(*value);
}

LengthSize BoxStyle::borderTopRightRadius() const
{
    auto value = get(CSSPropertyID::BorderTopRightRadius);
    if(value == nullptr)
        return LengthSize::ZeroFixed;
    return convertBorderRadius(*value);
}

LengthSize BoxStyle::borderBottomLeftRadius() const
{
    auto value = get(CSSPropertyID::BorderBottomLeftRadius);
    if(value == nullptr)
        return LengthSize::ZeroFixed;
    return convertBorderRadius(*value);
}

LengthSize BoxStyle::borderBottomRightRadius() const
{
    auto value = get(CSSPropertyID::BorderBottomRightRadius);
    if(value == nullptr)
        return LengthSize::ZeroFixed;
    return convertBorderRadius(*value);
}

ListStyleType BoxStyle::listStyleType() const
{
    auto value = get(CSSPropertyID::ListStyleType);
    if(value == nullptr)
        return ListStyleType::None;
    assert(value->isIdentValue());
    auto ident = value->toIdentValue();
    switch(ident->value()) {
    case CSSValueID::None:
        return ListStyleType::None;
    case CSSValueID::Disc:
        return ListStyleType::Disc;
    case CSSValueID::Circle:
        return ListStyleType::Circle;
    case CSSValueID::Square:
        return ListStyleType::Square;
    case CSSValueID::Decimal:
        return ListStyleType::Decimal;
    case CSSValueID::DecimalLeadingZero:
        return ListStyleType::DecimalLeadingZero;
    case CSSValueID::LowerAlpha:
        return ListStyleType::LowerAlpha;
    case CSSValueID::LowerLatin:
        return ListStyleType::LowerLatin;
    case CSSValueID::LowerRoman:
        return ListStyleType::LowerRoman;
    case CSSValueID::UpperAlpha:
        return ListStyleType::UpperAlpha;
    case CSSValueID::UpperLatin:
        return ListStyleType::UpperLatin;
    case CSSValueID::UpperRoman:
        return ListStyleType::UpperRoman;
    default:
        assert(false);
    }
}

ListStylePosition BoxStyle::listStylePosition() const
{
    auto value = get(CSSPropertyID::ListStylePosition);
    if(value == nullptr)
        return ListStylePosition::Inside;
    assert(value->isIdentValue());
    auto ident = value->toIdentValue();
    switch(ident->value()) {
    case CSSValueID::Inside:
        return ListStylePosition::Inside;
    case CSSValueID::Outside:
        return ListStylePosition::Outside;
    default:
        assert(false);
    }
}

Color BoxStyle::backgroundColor() const
{
    auto value = get(CSSPropertyID::BackgroundColor);
    if(value == nullptr)
        return Color::Transparent;
    return convertColor(*value);
}

BackgroundRepeat BoxStyle::backgroundRepeat() const
{
    auto value = get(CSSPropertyID::BackgroundRepeat);
    if(value == nullptr)
        return BackgroundRepeat::Repeat;
    assert(value->isIdentValue());
    auto ident = value->toIdentValue();
    switch(ident->value()) {
    case CSSValueID::Repeat:
        return BackgroundRepeat::Repeat;
    case CSSValueID::RepeatX:
        return BackgroundRepeat::RepeatX;
    case CSSValueID::RepeatY:
        return BackgroundRepeat::RepeatY;
    case CSSValueID::NoRepeat:
        return BackgroundRepeat::NoRepeat;
    default:
        assert(false);
    }
}

BackgroundBox BoxStyle::backgroundOrigin() const
{
    auto value = get(CSSPropertyID::BackgroundOrigin);
    if(value == nullptr)
        return BackgroundBox::PaddingBox;
    return convertBackgroundBox(*value);
}

BackgroundBox BoxStyle::backgroundClip() const
{
    auto value = get(CSSPropertyID::BackgroundOrigin);
    if(value == nullptr)
        return BackgroundBox::BorderBox;
    return convertBackgroundBox(*value);
}

BackgroundAttachment BoxStyle::backgroundAttachment() const
{
    auto value = get(CSSPropertyID::BackgroundAttachment);
    if(value == nullptr)
        return BackgroundAttachment::Scroll;
    assert(value->isIdentValue());
    auto ident = value->toIdentValue();
    switch(ident->value()) {
    case CSSValueID::Scroll:
        return BackgroundAttachment::Scroll;
    case CSSValueID::Fixed:
        return BackgroundAttachment::Fixed;
    case CSSValueID::Local:
        return BackgroundAttachment::Local;
    default:
        assert(false);
    }
}

BackgroundSize BoxStyle::backgroundSize() const
{
    auto value = get(CSSPropertyID::BackgroundSize);
    if(value == nullptr)
        return BackgroundSize(BackgroundSize::Type::Length, Length::Auto, Length::Auto);
    if(auto ident = value->toIdentValue()) {
        switch(ident->value()) {
        case CSSValueID::Contain:
            return BackgroundSize(BackgroundSize::Type::Contain);
        case CSSValueID::Cover:
            return BackgroundSize(BackgroundSize::Type::Cover);
        default:
            assert(false);
        }
    }

    assert(value->isPairValue());
    auto pair = value->toPairValue();
    auto width = convertLengthOrPercentOrAuto(*pair->first());
    auto height = convertLengthOrPercentOrAuto(*pair->second());
    return BackgroundSize(BackgroundSize::Type::Length, width, height);
}

LengthSize BoxStyle::backgroundPosition() const
{
    auto value = get(CSSPropertyID::BackgroundPosition);
    if(value == nullptr)
        return LengthSize::ZeroFixed;
    assert(value->isPairValue());
    auto pair = value->toPairValue();
    auto horizontal = convertPositionLength(CSSValueID::Left, CSSValueID::Right, *pair->first());
    auto vertical = convertPositionLength(CSSValueID::Top, CSSValueID::Bottom, *pair->second());
    return LengthSize(horizontal, vertical);
}

TableLayout BoxStyle::tableLayout() const
{
    auto value = get(CSSPropertyID::TableLayout);
    if(value == nullptr)
        return TableLayout::Auto;
    assert(value->isIdentValue());
    auto ident = value->toIdentValue();
    switch(ident->value()) {
    case CSSValueID::Auto:
        return TableLayout::Auto;
    case CSSValueID::Fixed:
        return TableLayout::Fixed;
    default:
        assert(false);
    }
}

CaptionSide BoxStyle::captionSide() const
{
    auto value = get(CSSPropertyID::CaptionSide);
    if(value == nullptr)
        return CaptionSide::Top;
    assert(value->isIdentValue());
    auto ident = value->toIdentValue();
    switch(ident->value()) {
    case CSSValueID::Top:
        return CaptionSide::Top;
    case CSSValueID::Bottom:
        return CaptionSide::Bottom;
    default:
        assert(false);
    }
}

EmptyCells BoxStyle::emptyCells() const
{
    auto value = get(CSSPropertyID::EmptyCells);
    if(value == nullptr)
        return EmptyCells::Show;
    assert(value->isIdentValue());
    auto ident = value->toIdentValue();
    switch(ident->value()) {
    case CSSValueID::Show:
        return EmptyCells::Show;
    case CSSValueID::Hide:
        return EmptyCells::Hide;
    default:
        assert(false);
    }
}

BorderCollapse BoxStyle::borderCollapse() const
{
    auto value = get(CSSPropertyID::BorderCollapse);
    if(value == nullptr)
        return BorderCollapse::Separate;
    assert(value->isIdentValue());
    auto ident = value->toIdentValue();
    switch(ident->value()) {
    case CSSValueID::Separate:
        return BorderCollapse::Separate;
    case CSSValueID::Collapse:
        return BorderCollapse::Collapse;
    default:
        assert(false);
    }
}

float BoxStyle::borderHorizontalSpacing() const
{
    auto value = get(CSSPropertyID::BorderHorizontalSpacing);
    if(value == nullptr)
        return 0.0;
    return convertLengthValue(*value);
}

float BoxStyle::borderVerticalSpacing() const
{
    auto value = get(CSSPropertyID::BorderVerticalSpacing);
    if(value == nullptr)
        return 0.0;
    return convertLengthValue(*value);
}

TextAlign BoxStyle::textAlign() const
{
    auto value = get(CSSPropertyID::TextAlign);
    if(value == nullptr)
        return TextAlign::Left;
    assert(value->isIdentValue());
    auto ident = value->toIdentValue();
    switch(ident->value()) {
    case CSSValueID::Left:
        return TextAlign::Left;
    case CSSValueID::Right:
        return TextAlign::Right;
    case CSSValueID::Center:
        return TextAlign::Center;
    case CSSValueID::Justify:
        return TextAlign::Justify;
    default:
        assert(false);
    }
}

TextTransform BoxStyle::textTransform() const
{
    auto value = get(CSSPropertyID::TextTransform);
    if(value == nullptr)
        return TextTransform::None;
    assert(value->isIdentValue());
    auto ident = value->toIdentValue();
    switch(ident->value()) {
    case CSSValueID::None:
        return TextTransform::None;
    case CSSValueID::Capitalize:
        return TextTransform::Capitalize;
    case CSSValueID::Uppercase:
        return TextTransform::Uppercase;
    case CSSValueID::Lowercase:
        return TextTransform::Lowercase;
    default:
        assert(false);
    }
}

TextOverflow BoxStyle::textOverflow() const
{
    auto value = get(CSSPropertyID::TextOverflow);
    if(value == nullptr)
        return TextOverflow::Clip;
    assert(value->isIdentValue());
    auto ident = value->toIdentValue();
    switch(ident->value()) {
    case CSSValueID::Clip:
        return TextOverflow::Clip;
    case CSSValueID::Ellipsis:
        return TextOverflow::Ellipsis;
    default:
        assert(false);
    }
}

TextDecorationStyle BoxStyle::textDecorationStyle() const
{
    auto value = get(CSSPropertyID::TextDecorationStyle);
    if(value == nullptr)
        return TextDecorationStyle::Solid;
    assert(value->isIdentValue());
    auto ident = value->toIdentValue();
    switch(ident->value()) {
    case CSSValueID::Solid:
        return TextDecorationStyle::Solid;
    case CSSValueID::Double:
        return TextDecorationStyle::Double;
    case CSSValueID::Dotted:
        return TextDecorationStyle::Dotted;
    case CSSValueID::Dashed:
        return TextDecorationStyle::Dashed;
    case CSSValueID::Wavy:
        return TextDecorationStyle::Wavy;
    default:
        assert(false);
    }
}

Color BoxStyle::textDecorationColor() const
{
    auto value = get(CSSPropertyID::TextDecorationColor);
    if(value == nullptr)
        return m_currentColor;
    return convertColor(*value);
}

WhiteSpace BoxStyle::whiteSpace() const
{
    auto value = get(CSSPropertyID::WhiteSpace);
    if(value == nullptr)
        return WhiteSpace::Normal;
    assert(value->isIdentValue());
    auto ident = value->toIdentValue();
    switch(ident->value()) {
    case CSSValueID::Normal:
        return WhiteSpace::Normal;
    case CSSValueID::Pre:
        return WhiteSpace::Pre;
    case CSSValueID::PreWrap:
        return WhiteSpace::PreWrap;
    case CSSValueID::PreLine:
        return WhiteSpace::PreLine;
    case CSSValueID::Nowrap:
        return WhiteSpace::Nowrap;
    case CSSValueID::BreakSpaces:
        return WhiteSpace::BreakSpaces;
    default:
        assert(false);
    }
}

LineBreak BoxStyle::lineBreak() const
{
    auto value = get(CSSPropertyID::LineBreak);
    if(value == nullptr)
        return LineBreak::Normal;
    assert(value->isIdentValue());
    auto ident = value->toIdentValue();
    switch(ident->value()) {
    case CSSValueID::Auto:
        return LineBreak::Auto;
    case CSSValueID::Normal:
        return LineBreak::Normal;
    case CSSValueID::Loose:
        return LineBreak::Loose;
    case CSSValueID::Strict:
        return LineBreak::Strict;
    case CSSValueID::Anywhere:
        return LineBreak::Anywhere;
    default:
        assert(false);
    }
}

WordBreak BoxStyle::wordBreak() const
{
    auto value = get(CSSPropertyID::WordBreak);
    if(value == nullptr)
        return WordBreak::Normal;
    assert(value->isIdentValue());
    auto ident = value->toIdentValue();
    switch(ident->value()) {
    case CSSValueID::Normal:
        return WordBreak::Normal;
    case CSSValueID::BreakAll:
        return WordBreak::BreakAll;
    case CSSValueID::KeepAll:
        return WordBreak::KeepAll;
    case CSSValueID::BreakWord:
        return WordBreak::BreakWord;
    default:
        assert(false);
    }
}

WordWrap BoxStyle::wordWrap() const
{
    auto value = get(CSSPropertyID::WordWrap);
    if(value == nullptr)
        return WordWrap::Normal;
    assert(value->isIdentValue());
    auto ident = value->toIdentValue();
    switch(ident->value()) {
    case CSSValueID::Normal:
        return WordWrap::Normal;
    case CSSValueID::BreakWord:
        return WordWrap::BreakWord;
    case CSSValueID::Anywhere:
        return WordWrap::Anywhere;
    default:
        assert(false);
    }
}

Hyphens BoxStyle::hyphens() const
{
    auto value = get(CSSPropertyID::Hyphens);
    if(value == nullptr)
        return Hyphens::Manual;
    assert(value->isIdentValue());
    auto ident = value->toIdentValue();
    switch(ident->value()) {
    case CSSValueID::None:
        return Hyphens::None;
    case CSSValueID::Auto:
        return Hyphens::Auto;
    case CSSValueID::Manual:
        return Hyphens::Manual;
    default:
        assert(false);
    }
}

float BoxStyle::tabSize() const
{
    auto value = get(CSSPropertyID::TabSize);
    if(value == nullptr)
        return 8.0;
    return convertLengthValue(*value);
}

Length BoxStyle::textIndent() const
{
    auto value = get(CSSPropertyID::TextIndent);
    if(value == nullptr)
        return Length::ZeroFixed;
    return convertLengthOrPercent(*value);
}

BoxSizing BoxStyle::boxSizing() const
{
    auto value = get(CSSPropertyID::BoxSizing);
    if(value == nullptr)
        return BoxSizing::ContentBox;
    assert(value->isIdentValue());
    auto ident = value->toIdentValue();
    switch(ident->value()) {
    case CSSValueID::BorderBox:
        return BoxSizing::BorderBox;
    case CSSValueID::ContentBox:
        return BoxSizing::ContentBox;
    default:
        assert(false);
    }
}

std::optional<int> BoxStyle::zIndex() const
{
    auto value = get(CSSPropertyID::ZIndex);
    if(value == nullptr)
        return std::nullopt;
    return convertIntegerOrAuto(*value);
}

VerticalAlign BoxStyle::verticalAlign() const
{
    auto value = get(CSSPropertyID::VerticalAlign);
    if(value == nullptr)
        return VerticalAlign(VerticalAlign::Type::Baseline);
    if(auto ident = value->toIdentValue()) {
        switch(ident->value()) {
        case CSSValueID::Baseline:
            return VerticalAlign(VerticalAlign::Type::Baseline);
        case CSSValueID::Sub:
            return VerticalAlign(VerticalAlign::Type::Sub);
        case CSSValueID::TextTop:
            return VerticalAlign(VerticalAlign::Type::TextTop);
        case CSSValueID::Middle:
            return VerticalAlign(VerticalAlign::Type::Middle);
        case CSSValueID::Bottom:
            return VerticalAlign(VerticalAlign::Type::Bottom);
        case CSSValueID::TextBottom:
            return VerticalAlign(VerticalAlign::Type::TextBottom);
        default:
            assert(false);
        }
    }

    return VerticalAlign(VerticalAlign::Type::Length, convertLengthOrPercent(*value));
}

LengthRect BoxStyle::clip() const
{
    auto value = get(CSSPropertyID::Clip);
    if(value == nullptr)
        return LengthRect::Auto;
    if(auto ident = value->toIdentValue()) {
        assert(ident->value() == CSSValueID::Auto);
        return LengthRect::Auto;
    }

    assert(value->isRectValue());
    auto rect = value->toRectValue();
    auto left = convertLengthOrPercentOrAuto(*rect->left());
    auto right = convertLengthOrPercentOrAuto(*rect->right());
    auto top = convertLengthOrPercentOrAuto(*rect->top());
    auto bottom = convertLengthOrPercentOrAuto(*rect->bottom());
    return LengthRect(left, right, top, bottom);
}

Length BoxStyle::flexBasis() const
{
    auto value = get(CSSPropertyID::FlexBasis);
    if(value == nullptr)
        return Length::Auto;
    return convertLengthOrPercentOrAuto(*value);
}

float BoxStyle::flexGrow() const
{
    auto value = get(CSSPropertyID::FlexGrow);
    if(value == nullptr)
        return 1.0;
    return convertNumber(*value);
}

float BoxStyle::flexShrink() const
{
    auto value = get(CSSPropertyID::FlexShrink);
    if(value == nullptr)
        return 0.0;
    return convertNumber(*value);
}

int BoxStyle::order() const
{
    auto value = get(CSSPropertyID::Order);
    if(value == nullptr)
        return 0;
    return convertInteger(*value);
}

FlexDirection BoxStyle::flexDirection() const
{
    auto value = get(CSSPropertyID::FlexDirection);
    if(value == nullptr)
        return FlexDirection::Row;
    assert(value->isIdentValue());
    auto ident = value->toIdentValue();
    switch(ident->value()) {
    case CSSValueID::Row:
        return FlexDirection::Row;
    case CSSValueID::RowReverse:
        return FlexDirection::RowReverse;
    case CSSValueID::Column:
        return FlexDirection::Column;
    case CSSValueID::ColumnReverse:
        return FlexDirection::ColumnReverse;
    default:
        assert(false);
    }
}

FlexWrap BoxStyle::flexWrap() const
{
    auto value = get(CSSPropertyID::FlexWrap);
    if(value == nullptr)
        return FlexWrap::Nowrap;
    assert(value->isIdentValue());
    auto ident = value->toIdentValue();
    switch(ident->value()) {
    case CSSValueID::Nowrap:
        return FlexWrap::Nowrap;
    case CSSValueID::Wrap:
        return FlexWrap::Wrap;
    case CSSValueID::WrapReverse:
        return FlexWrap::WrapReverse;
    default:
        assert(false);
    }
}

AlignContent BoxStyle::alignContent() const
{
    auto value = get(CSSPropertyID::AlignContent);
    if(value == nullptr)
        return AlignContent::Stretch;
    assert(value->isIdentValue());
    auto ident = value->toIdentValue();
    switch(ident->value()) {
    case CSSValueID::FlexStart:
        return AlignContent::FlexStart;
    case CSSValueID::FlexEnd:
        return AlignContent::FlexEnd;
    case CSSValueID::Center:
        return AlignContent::Center;
    case CSSValueID::Stretch:
        return AlignContent::Stretch;
    case CSSValueID::SpaceBetween:
        return AlignContent::SpaceBetween;
    case CSSValueID::SpaceAround:
        return AlignContent::SpaceAround;
    default:
        assert(false);
    }
}

AlignItems BoxStyle::alignItems() const
{
    auto value = get(CSSPropertyID::AlignItems);
    if(value == nullptr)
        return AlignItems::Stretch;
    assert(value->isIdentValue());
    auto ident = value->toIdentValue();
    switch(ident->value()) {
    case CSSValueID::FlexStart:
        return AlignItems::FlexStart;
    case CSSValueID::FlexEnd:
        return AlignItems::FlexEnd;
    case CSSValueID::Center:
        return AlignItems::Center;
    case CSSValueID::Stretch:
        return AlignItems::Stretch;
    case CSSValueID::Baseline:
        return AlignItems::Baseline;
    default:
        assert(false);
    }
}

AlignSelf BoxStyle::alignSelf() const
{
    auto value = get(CSSPropertyID::AlignSelf);
    if(value == nullptr)
        return AlignSelf::Auto;
    assert(value->isIdentValue());
    auto ident = value->toIdentValue();
    switch(ident->value()) {
    case CSSValueID::Auto:
        return AlignSelf::Auto;
    case CSSValueID::FlexStart:
        return AlignSelf::FlexStart;
    case CSSValueID::FlexEnd:
        return AlignSelf::FlexEnd;
    case CSSValueID::Center:
        return AlignSelf::Center;
    case CSSValueID::Stretch:
        return AlignSelf::Stretch;
    case CSSValueID::Baseline:
        return AlignSelf::Baseline;
    default:
        assert(false);
    }
}

JustifyContent BoxStyle::justifyContent() const
{
    auto value = get(CSSPropertyID::JustifyContent);
    if(value == nullptr)
        return JustifyContent::FlexStart;
    assert(value->isIdentValue());
    auto ident = value->toIdentValue();
    switch(ident->value()) {
    case CSSValueID::FlexStart:
        return JustifyContent::FlexStart;
    case CSSValueID::FlexEnd:
        return JustifyContent::FlexEnd;
    case CSSValueID::Center:
        return JustifyContent::Center;
    case CSSValueID::Stretch:
        return JustifyContent::SpaceBetween;
    case CSSValueID::Baseline:
        return JustifyContent::SpaceAround;
    default:
        assert(false);
    }
}

float BoxStyle::outlineOffset() const
{
    auto value = get(CSSPropertyID::OutlineOffset);
    if(value == nullptr)
        return 0.0;
    return convertLengthValue(*value);
}

Color BoxStyle::outlineColor() const
{
    auto value = get(CSSPropertyID::OutlineColor);
    if(value == nullptr)
        return m_currentColor;
    return convertColor(*value);
}

float BoxStyle::outlineWidth() const
{
    auto value = get(CSSPropertyID::OutlineWidth);
    if(value == nullptr)
        return 3.0;
    return convertLineWidth(*value);
}

LineStyle BoxStyle::outlineStyle() const
{
    auto value = get(CSSPropertyID::OutlineStyle);
    if(value == nullptr)
        return LineStyle::Auto;
    return convertLineStyle(*value);
}

Color BoxStyle::columnRuleColor() const
{
    auto value = get(CSSPropertyID::OutlineColor);
    if(value == nullptr)
        return m_currentColor;
    return convertColor(*value);
}

LineStyle BoxStyle::columnRuleStyle() const
{
    auto value = get(CSSPropertyID::ColumnRuleStyle);
    if(value == nullptr)
        return LineStyle::None;
    return convertLineStyle(*value);
}

ColumnSpan BoxStyle::columnSpan() const
{
    auto value = get(CSSPropertyID::ColumnSpan);
    if(value == nullptr)
        return ColumnSpan::None;
    assert(value->isIdentValue());
    auto ident = value->toIdentValue();
    switch(ident->value()) {
    case CSSValueID::None:
        return ColumnSpan::None;
    case CSSValueID::All:
        return ColumnSpan::All;
    default:
        assert(false);
    }
}

ColumnFill BoxStyle::columnFill() const
{
    auto value = get(CSSPropertyID::ColumnFill);
    if(value == nullptr)
        return ColumnFill::Auto;
    assert(value->isIdentValue());
    auto ident = value->toIdentValue();
    switch(ident->value()) {
    case CSSValueID::Auto:
        return ColumnFill::Auto;
    case CSSValueID::Balance:
        return ColumnFill::Balance;
    default:
        assert(false);
    }
}

std::optional<float> BoxStyle::columnWidth() const
{
    auto value = get(CSSPropertyID::ColumnWidth);
    if(value == nullptr)
        return std::nullopt;
    return convertLengthOrAuto(*value);
}

std::optional<float> BoxStyle::columnGap() const
{
    auto value = get(CSSPropertyID::ColumnGap);
    if(value == nullptr)
        return std::nullopt;
    return convertLengthOrNormal(*value);
}

std::optional<int> BoxStyle::columnCount() const
{
    auto value = get(CSSPropertyID::ColumnCount);
    if(value == nullptr)
        return std::nullopt;
    return convertIntegerOrAuto(*value);
}

int BoxStyle::widows() const
{
    auto value = get(CSSPropertyID::Widows);
    if(value == nullptr)
        return 2;
    return convertInteger(*value);
}

int BoxStyle::orphans() const
{
    auto value = get(CSSPropertyID::Orphans);
    if(value == nullptr)
        return 2;
    return convertInteger(*value);
}

std::shared_ptr<CSSValue> BoxStyle::get(CSSPropertyID id) const
{
    auto it = m_properties.find(id);
    if(it == m_properties.end())
        return nullptr;
    return it->second;
}

void BoxStyle::set(CSSPropertyID id, std::shared_ptr<CSSValue> value)
{
    assert(!value->isInitialValue() && !value->isInheritValue());
    m_properties.insert_or_assign(id, std::move(value));
}

float BoxStyle::emFontSize() const
{
    return 0.0;
}

float BoxStyle::exFontSize() const
{
    return 0.0;
}

float BoxStyle::remFontSize() const
{
    return 0.0;
}

float BoxStyle::chFontSize() const
{
    return 0.0;
}

float BoxStyle::viewportWidth() const
{
    return 0.0;
}

float BoxStyle::viewportHeight() const
{
    return 0.0;
}

float BoxStyle::viewportMin() const
{
    return 0.0;
}

float BoxStyle::viewportMax() const
{
    return 0.0;
}

float BoxStyle::convertLengthValue(const CSSValue& value) const
{
    assert(value.isLengthValue());
    constexpr auto dpi = 96.0;
    auto length = value.toLengthValue();
    switch(length->unit()) {
    case CSSLengthValue::Unit::None:
    case CSSLengthValue::Unit::Pixels:
        return length->value();
    case CSSLengthValue::Unit::Inches:
        return length->value() * dpi;
    case CSSLengthValue::Unit::Centimeters:
        return length->value() * dpi / 2.54;
    case CSSLengthValue::Unit::Millimeters:
        return length->value() * dpi / 25.4;
    case CSSLengthValue::Unit::Points:
        return length->value() * dpi / 72.0;
    case CSSLengthValue::Unit::Picas:
        return length->value() * dpi / 6.0;
    case CSSLengthValue::Unit::Ems:
        return length->value() * emFontSize();
    case CSSLengthValue::Unit::Exs:
        return length->value() * exFontSize();
    case CSSLengthValue::Unit::Rems:
        return length->value() * remFontSize();
    case CSSLengthValue::Unit::Chs:
        return length->value() * chFontSize();
    case CSSLengthValue::Unit::ViewportWidth:
        return length->value() * viewportWidth() / 100.0;
    case CSSLengthValue::Unit::ViewportHeight:
        return length->value() * viewportHeight() / 100.0;
    case CSSLengthValue::Unit::ViewportMin:
        return length->value() * viewportMin() / 100.0;
    case CSSLengthValue::Unit::ViewportMax:
        return length->value() * viewportMax() / 100.0;
    }

    return 0.0;
}

float BoxStyle::convertLineWidth(const CSSValue& value) const
{
    if(auto ident = value.toIdentValue()) {
        switch(ident->value()) {
        case CSSValueID::Thin:
            return 1.0;
        case CSSValueID::Medium:
            return 3.0;
        case CSSValueID::Thick:
            return 5.0;
        default:
            assert(false);
        }
    }

    return convertLengthValue(value);
}

std::optional<float> BoxStyle::convertLengthOrAuto(const CSSValue& value) const
{
    if(auto ident = value.toIdentValue()) {
        assert(ident->value() == CSSValueID::Auto);
        return std::nullopt;
    }

    return convertLengthValue(value);
}

std::optional<float> BoxStyle::convertLengthOrNormal(const CSSValue& value) const
{
    if(auto ident = value.toIdentValue()) {
        assert(ident->value() == CSSValueID::Normal);
        return std::nullopt;
    }

    return convertLengthValue(value);
}

Length BoxStyle::convertLength(const CSSValue& value) const
{
    return Length(Length::Type::Fixed, convertLengthValue(value));
}

Length BoxStyle::convertLengthOrPercent(const CSSValue& value) const
{
    if(auto percent = value.toPercentValue())
        return Length(Length::Type::Percent, percent->value());
    return convertLength(value);
}

Length BoxStyle::convertLengthOrPercentOrAuto(const CSSValue& value) const
{
    if(auto ident = value.toIdentValue()) {
        assert(ident->value() == CSSValueID::Auto);
        return Length::Auto;
    }

    return convertLengthOrPercent(value);
}

Length BoxStyle::convertLengthOrPercentOrNone(const CSSValue& value) const
{
    if(auto ident = value.toIdentValue()) {
        assert(ident->value() == CSSValueID::None);
        return Length::None;
    }

    return convertLengthOrPercent(value);
}

Length BoxStyle::convertPositionLength(CSSValueID min, CSSValueID max, const CSSValue& value) const
{
    if(auto ident = value.toIdentValue()) {
        constexpr auto mid = CSSValueID::Center;
        if(min == ident->value())
            return Length(Length::Type::Percent, 0);
        if(mid == ident->value())
            return Length(Length::Type::Percent, 50);
        if(max == ident->value())
            return Length(Length::Type::Percent, 100);
        assert(false);
    }

    return convertLengthOrPercent(value);
}

LengthSize BoxStyle::convertBorderRadius(const CSSValue& value) const
{
    assert(value.isPairValue());
    auto pair = value.toPairValue();
    auto horizontal = convertLengthOrPercent(*pair->first());
    auto vertical = convertLengthOrPercent(*pair->second());
    return LengthSize(horizontal, vertical);
}

Color BoxStyle::convertColor(const CSSValue& value) const
{
    if(auto ident = value.toIdentValue()) {
        assert(ident->value() == CSSValueID::CurrentColor);
        return m_currentColor;
    }

    assert(value.isColorValue());
    auto color = value.toColorValue();
    return Color(color->value());
}

Overflow BoxStyle::convertOverflow(const CSSValue& value)
{
    assert(value.isIdentValue());
    auto ident = value.toIdentValue();
    switch(ident->value()) {
    case CSSValueID::Auto:
        return Overflow::Auto;
    case CSSValueID::Visible:
        return Overflow::Visible;
    case CSSValueID::Hidden:
        return Overflow::Hidden;
    case CSSValueID::Scroll:
        return Overflow::Scroll;
    case CSSValueID::Overlay:
        return Overflow::Overlay;
    default:
        assert(false);
    }
}

BackgroundBox BoxStyle::convertBackgroundBox(const CSSValue& value)
{
    assert(value.isIdentValue());
    auto ident = value.toIdentValue();
    switch(ident->value()) {
    case CSSValueID::BorderBox:
        return BackgroundBox::BorderBox;
    case CSSValueID::PaddingBox:
        return BackgroundBox::PaddingBox;
    case CSSValueID::ContentBox:
        return BackgroundBox::ContentBox;
    default:
        assert(false);
    }
}

LineStyle BoxStyle::convertLineStyle(const CSSValue& value)
{
    assert(value.isIdentValue());
    auto ident = value.toIdentValue();
    switch(ident->value()) {
    case CSSValueID::None:
        return LineStyle::None;
    case CSSValueID::Auto:
        return LineStyle::Auto;
    case CSSValueID::Hidden:
        return LineStyle::Hidden;
    case CSSValueID::Inset:
        return LineStyle::Inset;
    case CSSValueID::Groove:
        return LineStyle::Groove;
    case CSSValueID::Outset:
        return LineStyle::Outset;
    case CSSValueID::Ridge:
        return LineStyle::Ridge;
    case CSSValueID::Dotted:
        return LineStyle::Dotted;
    case CSSValueID::Dashed:
        return LineStyle::Dashed;
    case CSSValueID::Solid:
        return LineStyle::Solid;
    case CSSValueID::Double:
        return LineStyle::Double;
    default:
        assert(false);
    }
}

int BoxStyle::convertInteger(const CSSValue& value)
{
    assert(value.isIntegerValue());
    auto integer = value.toIntegerValue();
    return integer->value();
}

std::optional<int> BoxStyle::convertIntegerOrAuto(const CSSValue& value)
{
    if(auto ident = value.toIdentValue()) {
        assert(ident->value() == CSSValueID::Auto);
        return std::nullopt;
    }

    return convertInteger(value);
}

float BoxStyle::convertNumber(const CSSValue& value)
{
    assert(value.isNumberValue());
    auto number = value.toNumberValue();
    return number->value();
}

void BoxStyle::inheritFrom(const BoxStyle& parentStyle)
{
}

} // namespace htmlbook
