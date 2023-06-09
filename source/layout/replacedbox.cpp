#include "replacedbox.h"
#include "inlinebox.h"
#include "blockbox.h"
#include "resource.h"

namespace htmlbook {

ReplacedBox::ReplacedBox(Node* node, const RefPtr<BoxStyle>& style)
    : BoxFrame(node, style)
{
}

void ReplacedBox::updateIntrinsicSize() const
{
    m_intrinsicWidth = 300;
    m_intrinsicHeight = 150;
    m_intrinsicRatio = m_intrinsicWidth / m_intrinsicHeight;
}

float ReplacedBox::intrinsicWidth() const
{
    if(m_intrinsicWidth < 0)
        updateIntrinsicSize();
    return m_intrinsicWidth;
}

float ReplacedBox::intrinsicHeight() const
{
    if(m_intrinsicHeight < 0)
        updateIntrinsicSize();
    return m_intrinsicHeight;
}

double ReplacedBox::intrinsicRatio() const
{
    if(m_intrinsicRatio < 0)
        updateIntrinsicSize();
    return m_intrinsicRatio;
}

void ReplacedBox::computePositionedReplacedWidth(float& x, float& width, float& marginLeft, float& marginRight) const
{
    auto container = containingBox();
    auto containerWidth = container->availableWidthForPositioned();
    auto containerDirection = container->direction();

    auto marginLeftLength = style()->marginLeft();
    auto marginRightLength = style()->marginRight();

    auto leftLength = style()->left();
    auto rightLength = style()->right();

    width = computeReplacedWidth() + borderAndPaddingWidth();
    auto availableSpace = containerWidth - width;
    if(leftLength.isAuto() && rightLength.isAuto()) {
        if(containerDirection == Direction::Ltr) {
            auto staticPosition = layer()->staticLeft() - container->borderLeft();
            for(auto parent = parentBox(); parent && parent != container; parent = parent->parentBox()) {
                if(auto box = to<BoxFrame>(parent)) {
                    staticPosition += box->x();
                }
            }

            leftLength = Length{Length::Type::Fixed, staticPosition};
        } else {
            auto staticPosition = layer()->staticLeft() + containerWidth + container->borderRight();
            for(auto parent = parentBox(); parent && parent != container; parent = parent->parentBox()) {
                if(auto box = to<BoxFrame>(parent)) {
                    staticPosition += box->x();
                }
            }

            rightLength = Length{Length::Type::Fixed, staticPosition};
        }
    }

    if(leftLength.isAuto() || rightLength.isAuto()) {
        if(marginLeftLength.isAuto())
            marginLeftLength = Length::ZeroFixed;
        if(marginRightLength.isAuto()) {
            marginRightLength = Length::ZeroFixed;
        }
    }

    float leftLengthValue = 0;
    float rightLengthValue = 0;
    if(marginLeftLength.isAuto() && marginRightLength.isAuto()) {
        leftLengthValue = leftLength.calc(containerWidth);
        rightLengthValue = rightLength.calc(containerWidth);

        auto availableWidth = availableSpace - (leftLengthValue + rightLengthValue);
        if(availableWidth > 0) {
            marginLeft = availableWidth / 2.f;
            marginRight = availableWidth - marginLeft;
        } else {
            if(containerDirection == Direction::Ltr) {
                marginLeft = 0;
                marginRight = availableWidth;
            } else {
                marginLeft = availableWidth;
                marginRight = 0;
            }
        }
    } else if(leftLength.isAuto()) {
        marginLeft = marginLeftLength.calc(containerWidth);
        marginRight = marginRightLength.calc(containerWidth);
        rightLengthValue = rightLength.calc(containerWidth);

        leftLengthValue = availableSpace - (rightLengthValue + marginLeft + marginRight);
    } else if(rightLength.isAuto()) {
        marginLeft = marginLeftLength.calc(containerWidth);
        marginRight = marginRightLength.calc(containerWidth);
        leftLengthValue = leftLength.calc(containerWidth);
    } else if(marginLeftLength.isAuto()) {
        marginRight = marginRightLength.calc(containerWidth);
        leftLengthValue = leftLength.calc(containerWidth);
        rightLengthValue = rightLength.calc(containerWidth);

        marginLeft = availableSpace - (leftLengthValue + rightLengthValue + marginRight);
    } else if(marginRightLength.isAuto()) {
        marginLeft = marginLeftLength.calc(containerWidth);
        leftLengthValue = leftLength.calc(containerWidth);
        rightLengthValue = rightLength.calc(containerWidth);

        marginRight = availableSpace - (leftLengthValue + rightLengthValue + marginLeft);
    } else {
        marginLeft = marginLeftLength.calc(containerWidth);
        marginRight = marginRightLength.calc(containerWidth);
        leftLengthValue = leftLength.calc(containerWidth);
        rightLengthValue = rightLength.calc(containerWidth);
        if(containerDirection == Direction::Rtl) {
            auto totalWidth = width + leftLengthValue + rightLengthValue +  marginLeft + marginRight;
            leftLengthValue = containerWidth - (totalWidth - leftLengthValue);
        }
    }

    if(containerDirection == Direction::Rtl && container->isInlineBox()) {
        auto& lines = to<InlineBox>(*container).lines();
        if(lines.size() > 1) {
            auto& firstLine = *lines.front();
            auto& lastLine = *lines.back();
            x = leftLengthValue + marginLeft + lastLine.borderLeft() + (lastLine.x() - firstLine.x());
            return;
        }
    }

    x = leftLengthValue + marginLeft + container->borderLeft();
}

void ReplacedBox::computePositionedReplacedHeight(float& y, float& height, float& marginTop, float& marginBottom) const
{
    auto container = containingBox();
    auto containerHeight = container->availableHeightForPositioned();

    auto marginTopLength = style()->marginTop();
    auto marginBottomLength = style()->marginBottom();

    auto topLength = style()->top();
    auto bottomLength = style()->bottom();

    height = computeReplacedHeight() + borderAndPaddingHeight();
    auto availableSpace = containerHeight - height;
    if(topLength.isAuto() && bottomLength.isAuto()) {
        auto staticTop = layer()->staticTop() - container->borderTop();
        for(auto parent = parentBox(); parent && parent != container; parent = parent->parentBox()) {
            if(auto box = to<BoxFrame>(parent)) {
                staticTop += box->y();
            }
        }

        topLength = Length{Length::Type::Fixed, staticTop};
    }

    if(topLength.isAuto() || bottomLength.isAuto()) {
        if(marginTopLength.isAuto())
            marginTopLength = Length::ZeroFixed;
        if(marginBottomLength.isAuto()) {
            marginBottomLength = Length::ZeroFixed;
        }
    }

    float topLengthValue = 0;
    float bottomLengthValue = 0;
    if(marginTopLength.isAuto() && marginBottomLength.isAuto()) {
        topLengthValue = topLength.calc(containerHeight);
        bottomLengthValue = bottomLength.calc(containerHeight);

        auto availableHeight = availableSpace - (topLengthValue + bottomLengthValue);
        marginTop = availableHeight / 2.f;
        marginBottom = availableHeight - marginTop;
    } else if(topLength.isAuto()) {
        marginTop = marginTopLength.calc(containerHeight);
        marginBottom = marginBottomLength.calc(containerHeight);
        bottomLengthValue = bottomLength.calc(containerHeight);

        topLengthValue = availableSpace - (bottomLengthValue + marginTop + marginBottom);
    } else if(bottomLength.isAuto()) {
        marginTop = marginTopLength.calc(containerHeight);
        marginBottom = marginBottomLength.calc(containerHeight);
        topLengthValue = topLength.calc(containerHeight);
    } else if(marginTopLength.isAuto()) {
        marginBottom = marginBottomLength.calc(containerHeight);
        topLengthValue = topLength.calc(containerHeight);
        bottomLengthValue = bottomLength.calc(containerHeight);

        marginTop = availableSpace - (topLengthValue + bottomLengthValue + marginBottom);
    } else if(marginBottomLength.isAuto()) {
        marginTop = marginTopLength.calc(containerHeight);
        topLengthValue = topLength.calc(containerHeight);
        bottomLengthValue = bottomLength.calc(containerHeight);

        marginBottom = availableSpace - (topLengthValue + bottomLengthValue + marginTop);
    } else {
        marginTop = marginTopLength.calc(containerHeight);
        marginBottom = marginBottomLength.calc(containerHeight);
        topLengthValue = topLength.calc(containerHeight);
    }

    y = topLengthValue + marginTop + container->borderTop();
}

float ReplacedBox::computePercentageReplacedWidth(const Length& widthLength) const
{
    float containerWidth = 0;
    if(isPositioned())
        containerWidth = containingBox()->availableWidthForPositioned();
    else
        containerWidth = containingBlockWidthForContent();

    return adjustContentBoxWidth(widthLength.calcMin(containerWidth));
}

std::optional<float> ReplacedBox::computePercentageReplacedHeight(const Length& heightLength) const
{
    if(isPositioned()) {
        auto availableHeight = containingBox()->availableHeightForPositioned();
        return adjustContentBoxHeight(heightLength.calc(availableHeight));
    }

    if(auto availableHeight = containingBlockHeightForContent())
        return adjustContentBoxHeight(heightLength.calc(*availableHeight));
    return std::nullopt;
}

std::optional<float> ReplacedBox::computeReplacedWidthUsing(const Length& widthLength) const
{
    switch(widthLength.type()) {
    case Length::Type::Fixed:
        return adjustContentBoxWidth(widthLength.value());
    case Length::Type::Percent:
        return computePercentageReplacedWidth(widthLength);
    default:
        return std::nullopt;
    }
}

std::optional<float> ReplacedBox::computeReplacedHeightUsing(const Length& heightLength) const
{
    switch(heightLength.type()) {
    case Length::Type::Fixed:
        return adjustContentBoxHeight(heightLength.value());
    case Length::Type::Percent:
        return computePercentageReplacedHeight(heightLength);
    default:
        return std::nullopt;
    }
}

float ReplacedBox::constrainReplacedWidthByMinMax(float width) const
{
    if(auto maxWidth = computeReplacedWidthUsing(style()->maxWidth()))
        width = std::min(width, *maxWidth);
    if(auto minWidth = computeReplacedWidthUsing(style()->minWidth()))
        width = std::max(width, *minWidth);
    return width;
}

float ReplacedBox::constrainReplacedHeightByMinMax(float height) const
{
    if(auto maxHeight = computeReplacedHeightUsing(style()->maxHeight()))
        height = std::min(height, *maxHeight);
    if(auto minHeight = computeReplacedHeightUsing(style()->minHeight()))
        height = std::max(height, *minHeight);
    return height;
}

float ReplacedBox::computeReplacedWidth() const
{
    if(auto width = computeReplacedWidthUsing(style()->width()))
        return constrainReplacedWidthByMinMax(*width);

    float width = 300;

    auto height = computeReplacedHeightUsing(style()->height());
    if(!height && intrinsicWidth()) {
        width = intrinsicWidth();
    } else if(height && intrinsicRatio()) {
        width = constrainReplacedHeightByMinMax(*height) * intrinsicRatio();
    } else if(intrinsicRatio() && (!intrinsicWidth() && intrinsicHeight())) {
        width = intrinsicHeight() * intrinsicRatio();
    } else if(intrinsicWidth()) {
        width = intrinsicWidth();
    }

    return constrainReplacedWidthByMinMax(width);
}

float ReplacedBox::computeReplacedHeight() const
{
    if(auto height = computeReplacedHeightUsing(style()->height()))
        return constrainReplacedHeightByMinMax(*height);

    float height = 150;

    auto width = computeReplacedWidthUsing(style()->width());
    if(!width && intrinsicHeight()) {
        height = intrinsicHeight();
    } else if(width && intrinsicRatio()) {
        height = constrainReplacedWidthByMinMax(*width) / intrinsicRatio();
    } else if(intrinsicRatio() && (!intrinsicHeight() && intrinsicWidth())) {
        height = intrinsicWidth() / intrinsicRatio();
    } else if(intrinsicHeight()) {
        height = intrinsicHeight();
    }

    return constrainReplacedHeightByMinMax(height);
}

void ReplacedBox::computeWidth(float& x, float& width, float& marginLeft, float& marginRight) const
{
    if(isPositioned()) {
        computePositionedReplacedWidth(x, width, marginLeft, marginRight);
        return;
    }

    auto container = containingBlock();
    auto containerWidth = std::max(0.f, container->availableWidth());

    width = computeReplacedWidth() + borderAndPaddingWidth();
    if(isInline())
        width = std::max(width, minPreferredWidth());
    computeHorizontalMargins(marginLeft, marginRight, width, container, containerWidth);
}

void ReplacedBox::computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const
{
    if(isPositioned()) {
        computePositionedReplacedHeight(y, height, marginTop, marginBottom);
        return;
    }

    height = computeReplacedHeight() + borderAndPaddingHeight();
    computeVerticalMargins(marginTop, marginBottom);
}

ImageBox::ImageBox(Node* node, const RefPtr<BoxStyle>& style)
    : ReplacedBox(node, style)
{
}

void ImageBox::setImage(RefPtr<Image> image)
{
    m_image = std::move(image);
}

} // namespace htmlbook
