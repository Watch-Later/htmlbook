#include "blockbox.h"
#include "inlinebox.h"
#include "document.h"

namespace htmlbook {

BlockBox::BlockBox(Node* node, const RefPtr<BoxStyle>& style)
    : BoxFrame(node, style)
{
    switch(style->overflow()) {
    case Overflow::Visible:
        setOverflowHidden(false);
        break;
    default:
        setOverflowHidden(true);
        break;
    }
}

void BlockBox::updatePreferredWidths() const
{
    m_minPreferredWidth = 0;
    m_maxPreferredWidth = 0;

    auto widthLength = style()->width();
    if(widthLength.isFixed()) {
        m_minPreferredWidth = m_maxPreferredWidth = adjustBorderBoxWidth(widthLength.value());
    } else {
        computePreferredWidths(m_minPreferredWidth, m_maxPreferredWidth);
    }

    auto minWidthLength = style()->minWidth();
    if(minWidthLength.isFixed() && minWidthLength.value() > 0) {
        m_minPreferredWidth = std::max(m_minPreferredWidth, adjustBorderBoxWidth(minWidthLength.value()));
        m_maxPreferredWidth = std::max(m_maxPreferredWidth, adjustBorderBoxWidth(minWidthLength.value()));
    }

    auto maxWidthLength = style()->maxWidth();
    if(maxWidthLength.isFixed()) {
        m_minPreferredWidth = std::min(m_minPreferredWidth, adjustBorderBoxWidth(maxWidthLength.value()));
        m_maxPreferredWidth = std::min(m_maxPreferredWidth, adjustBorderBoxWidth(maxWidthLength.value()));
    }

    m_minPreferredWidth += borderAndPaddingWidth();
    m_maxPreferredWidth += borderAndPaddingWidth();
}

void BlockBox::insertPositonedBox(BoxFrame* box)
{
    if(!m_positionedBoxes)
        m_positionedBoxes = std::make_unique<PositionedBoxList>(heap());
    m_positionedBoxes->insert(box);
}

void BlockBox::removePositonedBox(BoxFrame* box)
{
    if(m_positionedBoxes) {
        m_positionedBoxes->erase(box);
    }
}

void BlockBox::layoutPositionedBoxes()
{
    if(m_positionedBoxes) {
        for(auto box : *m_positionedBoxes) {
            box->layout();
        }
    }
}

float BlockBox::availableHeight() const
{
    auto availableHeight = availableHeightUsing(style()->height());
    return constrainContentBoxHeightByMinMax(availableHeight);
}

float BlockBox::availableHeightUsing(const Length& heightLength) const
{
    if(isBoxView())
        return style()->viewportHeight();

    if(hasOverrideHeight())
        return overrideHeight() - borderAndPaddingHeight();

    if(heightLength.isPercent() && isPositioned()) {
        auto availableHeight = containingBlockHeightForPositioned(containingBlock());
        return adjustContentBoxHeight(heightLength.calc(availableHeight));
    }

    if(heightLength.isAuto() && isPositioned() && !(style()->top().isAuto() || style()->bottom().isAuto())) {
        float y = 0;
        float computedHeight = height();
        float marginTop = 0;
        float marginBottom = 0;
        computePositionedHeight(y, computedHeight, marginTop, marginBottom);
        return adjustContentBoxHeight(computedHeight - borderAndPaddingHeight());
    }

    if(auto computedHeight = computeHeightUsing(heightLength))
        return adjustContentBoxHeight(*computedHeight);
    return containingBlockHeightForContent();
}

bool BlockBox::shrinkToAvoidFloats() const
{
    if(isInline() || isFloating() || !avoidsFloats())
        return false;
    return style()->width().isAuto();
}

float BlockBox::shrinkWidthToAvoidFloats(float marginLeft, float marginRight, const BlockFlowBox* container) const
{
    auto availableWidth = container->availableWidthForLine(y(), false) - marginLeft - marginRight;
    if(marginLeft > 0) {
        auto lineStartOffset = container->startOffsetForLine(y(), false);
        auto contentStartOffset = container->startOffsetForContent();
        auto marginStartOffset = contentStartOffset + marginLeft;
        if(lineStartOffset > marginStartOffset) {
            availableWidth += marginLeft;
        } else {
            availableWidth += lineStartOffset - contentStartOffset;
        }
    }

    if(marginRight > 0) {
        auto lineEndOffset = container->endOffsetForLine(y(), false);
        auto contentEndOffset = container->endOffsetForContent();
        auto marginEndOffset = contentEndOffset + marginRight;
        if(lineEndOffset > marginEndOffset) {
            availableWidth += marginRight;
        } else {
            availableWidth += lineEndOffset - contentEndOffset;
        }
    }

    return availableWidth;
}

float BlockBox::computeWidthUsing(const Length& widthLength, const BlockBox* container, float containerWidth) const
{
    if(!widthLength.isAuto())
        return adjustBorderBoxWidth(widthLength.calc(containerWidth));
    auto marginLeft = style()->marginLeft().calcMin(containerWidth);
    auto marginRight = style()->marginRight().calcMin(containerWidth);
    auto computedWidth = containerWidth - marginLeft - marginRight;
    auto containerBlock = to<BlockFlowBox>(container);
    if(containerBlock && containerBlock->containsFloats() && shrinkToAvoidFloats())
        computedWidth = std::min(computedWidth, shrinkWidthToAvoidFloats(marginLeft, marginRight, containerBlock));
    if(isFloating() || isInline() || isFlexItem() || isTableBox()) {
        computedWidth = std::min(computedWidth, maxPreferredWidth());
        computedWidth = std::max(computedWidth, minPreferredWidth());
    }

    return computedWidth;
}

float BlockBox::constrainWidthByMinMax(float width, const BlockBox* container, float containerWidth) const
{
    auto minWidthLength = style()->minWidth();
    auto maxWidthLength = style()->maxWidth();

    if(!maxWidthLength.isNone())
        width = std::min(width, computeWidthUsing(maxWidthLength, container, containerWidth));
    if(minWidthLength.isAuto())
        return std::max(width, adjustBorderBoxWidth(0));
    return std::max(width, computeWidthUsing(minWidthLength, container, containerWidth));
}

std::optional<float> BlockBox::computePercentageHeight(const Length& heightLength) const
{
    auto container = containingBlock();
    auto containerStyle = container->style();
    auto containerStyleHeight = containerStyle->height();
    auto containerStyleTop = containerStyle->top();
    auto containerStyleBottom = containerStyle->bottom();

    float availableHeight = 0;
    if(container->hasOverrideHeight()) {
        availableHeight = container->overrideHeight() - container->borderAndPaddingHeight();
    } else if(containerStyleHeight.isFixed()) {
        availableHeight = container->adjustContentBoxHeight(containerStyleHeight.value());
        availableHeight = container->constrainContentBoxHeightByMinMax(availableHeight);
    } else if(container->isPositioned() && (!containerStyleHeight.isAuto() || (!containerStyleTop.isAuto() && !containerStyleBottom.isAuto()))) {
        float y = 0;
        float computedHeight = container->height();
        float marginTop = 0;
        float marginBottom = 0;
        container->computePositionedHeight(y, computedHeight, marginTop, marginBottom);
        availableHeight = computedHeight - container->borderAndPaddingHeight();
    } else if(containerStyleHeight.isPercent()) {
        auto computedHeight = container->computePercentageHeight(containerStyleHeight);
        if(!computedHeight)
            return std::nullopt;
        availableHeight = container->adjustContentBoxHeight(*computedHeight);
        availableHeight = container->constrainContentBoxHeightByMinMax(availableHeight);
    } else if(container->isBoxView()) {
        availableHeight = containerStyle->viewportHeight();
    } else if(container->isAnonymous()) {
        return container->computePercentageHeight(heightLength);
    } else {
        return std::nullopt;
    }

    return heightLength.calc(availableHeight);
}

std::optional<float> BlockBox::computeHeightUsing(const Length& heightLength) const
{
    switch(heightLength.type()) {
    case Length::Type::Fixed:
        return heightLength.value();
    case Length::Type::Percent:
        return computePercentageHeight(heightLength);
    default:
        return std::nullopt;
    }
}

float BlockBox::constrainBorderBoxHeightByMinMax(float height) const
{
    if(auto maxHeight = computeHeightUsing(style()->maxHeight()))
        height = std::min(height, adjustBorderBoxHeight(*maxHeight));
    if(auto minHeight = computeHeightUsing(style()->minHeight()))
        height = std::max(height, adjustBorderBoxHeight(*minHeight));
    return height;
}

float BlockBox::constrainContentBoxHeightByMinMax(float height) const
{
    if(auto maxHeight = computeHeightUsing(style()->maxHeight()))
        height = std::min(height, adjustContentBoxHeight(*maxHeight));
    if(auto minHeight = computeHeightUsing(style()->minHeight()))
        height = std::max(height, adjustContentBoxHeight(*minHeight));
    return height;
}

void BlockBox::computePositionedWidthUsing(const Length& widthLength, const BoxModel* container, Direction containerDirection, float containerWidth,
    const Length& leftLength, const Length& rightLength, const Length& marginLeftLength, const Length& marginRightLength,
    float& x, float& width, float& marginLeft, float& marginRight) const
{
    auto widthLenghtIsAuto = widthLength.isAuto();
    auto leftLenghtIsAuto = leftLength.isAuto();
    auto rightLenghtIsAuto = rightLength.isAuto();

    float leftLengthValue = 0;
    if(!leftLenghtIsAuto && !widthLenghtIsAuto && !rightLenghtIsAuto) {
        leftLengthValue = leftLength.calc(containerWidth);
        width = adjustContentBoxWidth(widthLength.calc(containerWidth));

        auto availableSpace = containerWidth - (leftLengthValue + width + rightLength.calc(containerWidth) + borderAndPaddingWidth());
        if(marginLeftLength.isAuto() && marginRightLength.isAuto()) {
            if(availableSpace >= 0) {
                marginLeft = availableSpace / 2.f;
                marginRight = availableSpace - marginLeft;
            } else {
                if(containerDirection == Direction::Ltr) {
                    marginLeft = 0;
                    marginRight = availableSpace;
                } else {
                    marginLeft = availableSpace;
                    marginRight = 0;
                }
            }
        } else if(marginLeftLength.isAuto()) {
            marginRight = marginRightLength.calc(containerWidth);
            marginLeft = availableSpace - marginRight;
        } else if(marginRightLength.isAuto()) {
            marginLeft = marginLeftLength.calc(containerWidth);
            marginRight = availableSpace - marginLeft;
        } else {
            marginLeft = marginLeftLength.calc(containerWidth);
            marginRight = marginRightLength.calc(containerWidth);
            if(containerDirection == Direction::Rtl) {
                leftLengthValue = (availableSpace + leftLengthValue) - marginLeft - marginRight;
            }
        }
    } else {
        marginLeft = marginLeftLength.calcMin(containerWidth);
        marginRight = marginRightLength.calcMin(containerWidth);

        auto availableSpace = containerWidth - (marginLeft + marginRight + borderAndPaddingWidth());
        if(leftLenghtIsAuto && widthLenghtIsAuto && !rightLenghtIsAuto) {
            auto rightLengthValue = rightLength.calc(containerWidth);

            auto preferredWidth = maxPreferredWidth() - borderAndPaddingWidth();
            auto preferredMinWidth = minPreferredWidth() - borderAndPaddingWidth();
            auto availableWidth = availableSpace - rightLengthValue;
            width = std::min(preferredWidth, std::max(preferredMinWidth, availableWidth));
            leftLengthValue = availableSpace - (width + rightLengthValue);
        } else if(!leftLenghtIsAuto && widthLenghtIsAuto && rightLenghtIsAuto) {
            leftLengthValue = leftLength.calc(containerWidth);

            auto preferredWidth = maxPreferredWidth() - borderAndPaddingWidth();
            auto preferredMinWidth = minPreferredWidth() - borderAndPaddingWidth();
            auto availableWidth = availableSpace - leftLengthValue;
            width = std::min(preferredWidth, std::max(preferredMinWidth, availableWidth));
        } else if(leftLenghtIsAuto && !widthLenghtIsAuto && !rightLenghtIsAuto) {
            width = adjustContentBoxWidth(widthLength.calc(containerWidth));
            leftLengthValue = availableSpace - (width + rightLength.calc(containerWidth));
        } else if(!leftLenghtIsAuto && widthLenghtIsAuto && !rightLenghtIsAuto) {
            leftLengthValue = leftLength.calc(containerWidth);
            width = availableSpace - (leftLengthValue + rightLength.calc(containerWidth));
        } else if (!leftLenghtIsAuto && !widthLenghtIsAuto && rightLenghtIsAuto) {
            leftLengthValue = leftLength.calc(containerWidth);
            width = adjustContentBoxWidth(widthLength.calc(containerWidth));
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

void BlockBox::computePositionedWidth(float& x, float& width, float& marginLeft, float& marginRight) const
{
    auto container = containingBox();
    auto containerWidth = containingBlockWidthForPositioned(container);
    auto containerDirection = container->direction();

    auto marginLeftLength = style()->marginLeft();
    auto marginRightLength = style()->marginRight();

    auto leftLength = style()->left();
    auto rightLength = style()->right();
    if(leftLength.isAuto() && rightLength.isAuto()) {
        if(containerDirection == Direction::Ltr) {
            auto staticPosition = layer()->staticLeft() - borderLeft();
            for(auto parent = parentBox(); parent && parent != container; parent = parent->parentBox()) {
                if(auto box = to<BoxFrame>(parent)) {
                    staticPosition += box->x();
                }
            }

            leftLength = Length{Length::Type::Fixed, staticPosition};
        } else {
            auto staticPosition = layer()->staticLeft() + containerWidth + container->borderRight();
            auto parent = parentBox();
            if(auto box = to<BoxFrame>(parent))
                staticPosition -= box->width();
            for(; parent && parent != container; parent = parent->parentBox()) {
                if(auto box = to<BoxFrame>(parent)) {
                    staticPosition -= box->x();
                }
            }

            rightLength = Length{Length::Type::Fixed, staticPosition};
        }
    }

    auto widthLength = style()->width();
    auto minWidthLength = style()->minWidth();
    auto maxWidthLength = style()->maxWidth();
    computePositionedWidthUsing(widthLength, container, containerDirection, containerWidth,
        leftLength, leftLength, marginLeftLength, marginRightLength, x, width, marginLeft, marginRight);
    if(!maxWidthLength.isNone()) {
        float maxX = 0;
        float maxWidth = 0;
        float maxMarginLeft = 0;
        float maxMarginRight = 0;
        computePositionedWidthUsing(maxWidthLength, container, containerDirection, containerWidth,
            leftLength, rightLength, marginLeftLength, marginRightLength, maxX, maxWidth, maxMarginLeft, maxMarginRight);
        if(width > maxWidth) {
            x = maxX;
            width = maxWidth;
            marginLeft = maxMarginLeft;
            marginRight = maxMarginRight;
        }
    }

    if(!minWidthLength.isZero()) {
        float minX = 0;
        float minWidth = 0;
        float minMarginLeft = 0;
        float minMarginRight = 0;
        computePositionedWidthUsing(minWidthLength, container, containerDirection, containerWidth,
            leftLength, rightLength, marginLeftLength, marginRightLength, minX, minWidth, minMarginLeft, minMarginRight);
        if(width < minWidth) {
            x = minX;
            width = minWidth;
            marginLeft = minMarginLeft;
            marginRight = minMarginRight;
        }
    }

    width += borderAndPaddingWidth();
}

void BlockBox::computePositionedHeightUsing(const Length& heightLength, const BoxModel* container, float containerHeight, float contentHeight,
    const Length& topLength, const Length& bottomLength, const Length& marginTopLength, const Length& marginBottomLength,
    float& y, float& height, float& marginTop, float& marginBottom) const
{
    auto heightLenghtIsAuto = heightLength.isAuto();
    auto topLenghtIsAuto = topLength.isAuto();
    auto bottomLenghtIsAuto = bottomLength.isAuto();

    float topLengthValue = 0;
    float heightLengthValue = 0;
    if(isTableBox()) {
        heightLengthValue = contentHeight;
        heightLenghtIsAuto = true;
    } else {
        heightLengthValue = heightLength.calc(containerHeight);
        heightLengthValue = adjustContentBoxHeight(heightLengthValue);
    }

    if(!topLenghtIsAuto && !heightLenghtIsAuto && !bottomLenghtIsAuto) {
        height = heightLengthValue;
        topLengthValue = topLength.calc(containerHeight);

        auto availableSpace = containerHeight - (height + topLengthValue + bottomLength.calc(containerHeight) + borderAndPaddingHeight());
        if(marginTopLength.isAuto() && marginBottomLength.isAuto()) {
            marginTop = availableSpace / 2.f;
            marginBottom = availableSpace - marginTop;
        } else if(marginTopLength.isAuto()) {
            marginBottom = marginBottomLength.calc(containerHeight);
            marginTop = availableSpace - marginBottom;
        } else if(marginBottomLength.isAuto()) {
            marginTop = marginTopLength.calc(containerHeight);
            marginBottom = availableSpace - marginTop;
        } else {
            marginTop = marginTopLength.calc(containerHeight);
            marginBottom = marginBottomLength.calc(containerHeight);
        }
    } else {
        marginTop = marginTopLength.calcMin(containerHeight);
        marginBottom = marginBottomLength.calcMin(containerHeight);

        auto availableSpace = containerHeight - (marginTop + marginBottom + borderAndPaddingHeight());
        if(topLenghtIsAuto && heightLenghtIsAuto && !bottomLenghtIsAuto) {
            height = contentHeight;
            topLengthValue = availableSpace - (height + bottomLength.calc(containerHeight));
        } else if(!topLenghtIsAuto && heightLenghtIsAuto && bottomLenghtIsAuto) {
            topLengthValue = topLength.calc(containerHeight);
            height = contentHeight;
        } else if(topLenghtIsAuto && !heightLenghtIsAuto && !bottomLenghtIsAuto) {
            height = heightLengthValue;
            topLengthValue = availableSpace - (height + bottomLength.calc(containerHeight));
        } else if(!topLenghtIsAuto && heightLenghtIsAuto && !bottomLenghtIsAuto) {
            topLengthValue = topLength.calc(containerHeight);
            height = std::max(0.f, availableSpace - (topLengthValue + bottomLength.calc(containerHeight)));
        } else if(!topLenghtIsAuto && !heightLenghtIsAuto && bottomLenghtIsAuto) {
            height = heightLengthValue;
            topLengthValue = topLength.calc(containerHeight);
        }
    }

    y = topLengthValue + marginTop + container->borderTop();
}

void BlockBox::computePositionedHeight(float& y, float& height, float& marginTop, float& marginBottom) const
{
    auto container = containingBox();
    auto containerHeight = containingBlockHeightForPositioned(container);
    auto contentHeight = height - borderAndPaddingHeight();

    auto marginTopLength = style()->marginTop();
    auto marginBottomLength = style()->marginBottom();

    auto topLength = style()->top();
    auto bottomLength = style()->bottom();
    if(topLength.isAuto() && bottomLength.isAuto()) {
        auto staticTop = layer()->staticTop() - container->borderTop();
        for(auto parent = parentBox(); parent && parent != container; parent = parent->parentBox()) {
            if(auto box = to<BoxFrame>(parent)) {
                staticTop += box->y();
            }
        }

        topLength = Length{Length::Type::Fixed, staticTop};
    }

    auto heightLength = style()->height();
    auto minHeightLength = style()->minHeight();
    auto maxHeightLength = style()->maxHeight();
    computePositionedHeightUsing(heightLength, container, containerHeight, contentHeight,
        topLength, bottomLength, marginTopLength, marginBottomLength, y, height, marginTop, marginBottom);
    if(!maxHeightLength.isNone()) {
        float maxY = 0;
        float maxHeight = 0;
        float maxMarginTop = 0;
        float maxMarginBottom = 0;
        computePositionedHeightUsing(maxHeightLength, container, containerHeight, contentHeight,
            topLength, bottomLength, marginTopLength, marginBottomLength, maxY, maxHeight, maxMarginTop, maxMarginBottom);
        if(height > maxHeight) {
            y = maxY;
            height = maxHeight;
            marginTop = maxMarginTop;
            marginBottom = maxMarginBottom;
        }
    }

    if(!minHeightLength.isZero()) {
        float minY = 0;
        float minHeight = 0;
        float minMarginTop = 0;
        float minMarginBottom = 0;
        computePositionedHeightUsing(minHeightLength, container, containerHeight, contentHeight,
            topLength, bottomLength, marginTopLength, marginBottomLength, minY, minHeight, minMarginTop, minMarginBottom);
        if(height < minHeight) {
            y = minY;
            height = minHeight;
            marginTop = minMarginTop;
            marginBottom = minMarginBottom;
        }
    }

    height += borderAndPaddingHeight();
}

void BlockBox::computeWidth(float& x, float& width, float& marginLeft, float& marginRight) const
{
    if(hasOverrideWidth()) {
        width = overrideWidth();
        return;
    }

    if(isPositioned()) {
        computePositionedWidth(x, width, marginLeft, marginRight);
        return;
    }

    auto container = containingBlock();
    auto containerWidth = std::max(0.f, container->availableWidth());

    width = computeWidthUsing(style()->width(), container, containerWidth);
    width = constrainWidthByMinMax(width, container, containerWidth);
    if(isTableBox())
        width = std::max(width, minPreferredWidth());
    computeHorizontalMargins(marginLeft, marginRight, width, container, containerWidth);
}

void BlockBox::computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const
{
    if(hasOverrideHeight()) {
        height = overrideHeight();
        return;
    }

    if(isPositioned()) {
        computePositionedHeight(y, height, marginTop, marginBottom);
        return;
    }

    if(auto computedHeight = computeHeightUsing(style()->height()))
        height = adjustBorderBoxHeight(*computedHeight);
    height = constrainBorderBoxHeightByMinMax(height);
    computeVerticalMargins(marginTop, marginBottom);
}

BlockFlowBox::~BlockFlowBox() = default;

BlockFlowBox::BlockFlowBox(Node* node, const RefPtr<BoxStyle>& style)
    : BlockBox(node, style)
{
    setChildrenInline(true);
}

bool BlockFlowBox::avoidsFloats() const
{
    return isInline() || isFloating() || isPositioned() || isOverflowHidden() || isRootBox() || isFlexItem();
}

bool BlockFlowBox::isSelfCollapsingBlock() const
{
    if(height() || avoidsFloats())
        return false;
    if(isChildrenInline())
        return m_lineLayout->empty();
    for(auto child = firstBoxFrame(); child; child = child->nextBoxFrame()) {
        if(child->isFloatingOrPositioned())
            continue;
        if(!child->isSelfCollapsingBlock()) {
            return false;
        }
    }

    return true;
}

void BlockFlowBox::computePreferredWidths(float& minWidth, float& maxWidth) const
{
    if(isChildrenInline()) {
        m_lineLayout->computePreferredWidths(minWidth, maxWidth);
        return;
    }

    float floatLeftWidth = 0;
    float floatRightWidth = 0;
    for(auto child = firstBoxFrame(); child; child = child->nextBoxFrame()) {
        if(child->isPositioned())
            continue;
        auto childStyle = child->style();
        if(child->isFloating() || child->avoidsFloats()) {
            auto floatWidth = floatLeftWidth + floatRightWidth;
            if(childStyle->isClearLeft()) {
                maxWidth = std::max(floatWidth, maxWidth);
                floatLeftWidth = 0;
            }

            if(childStyle->isClearRight()) {
                maxWidth = std::max(floatWidth, maxWidth);
                floatRightWidth = 0;
            }
        }

        auto marginLeftLength = childStyle->marginLeft();
        auto marginRightLength = childStyle->marginRight();

        float marginLeft = 0;
        float marginRight = 0;
        if(marginLeftLength.isFixed())
            marginLeft = marginLeftLength.value();
        if(marginRightLength.isFixed())
            marginRight = marginRightLength.value();

        auto childMinWidth = child->minPreferredWidth();
        auto childMaxWidth = child->maxPreferredWidth();

        auto marginWidth = marginLeft + marginRight;
        auto width = childMinWidth + marginWidth;

        minWidth = std::max(width, minWidth);
        if(style()->whiteSpace() == WhiteSpace::Nowrap)
            maxWidth = std::max(width, maxWidth);
        width = childMaxWidth + marginWidth;

        if(child->isFloating()) {
            if(childStyle->floating() == Float::Left) {
                floatLeftWidth += width;
            } else {
                floatRightWidth += width;
            }
        } else {
            if(child->avoidsFloats()) {
                if(marginLeft > 0)
                    marginLeft = std::max(floatLeftWidth, marginLeft);
                else
                    marginLeft += floatLeftWidth;

                if(marginRight > 0)
                    marginRight = std::max(floatRightWidth, marginRight);
                else
                    marginRight += floatRightWidth;
                width = std::max(childMaxWidth + marginLeft + marginRight, floatLeftWidth + floatRightWidth);
            } else {
                maxWidth = std::max(maxWidth, floatLeftWidth + floatRightWidth);
            }

            maxWidth = std::max(width, maxWidth);
            floatLeftWidth = 0;
            floatRightWidth = 0;
        }
    }

    minWidth = std::max(0.f, minWidth);
    maxWidth = std::max(0.f, maxWidth);

    maxWidth = std::max(maxWidth, floatLeftWidth + floatRightWidth);
    maxWidth = std::max(maxWidth, minWidth);
}

class MarginInfo {
public:
    MarginInfo(const BlockFlowBox* block, float top, float bottom);

    bool atTopOfBlock() const { return m_atTopOfBlock; }
    bool atBottomOfBlock() const { return m_atBottomOfBlock; }

    bool canCollapseWithChildren() const { return m_canCollapseWithChildren; }
    bool canCollapseMarginTopWithChildren() const { return m_canCollapseMarginTopWithChildren; }
    bool canCollapseMarginBottomWithChildren() const { return m_canCollapseMarginBottomWithChildren; }

    bool canCollapseWithMarginTop() const { return m_atTopOfBlock && m_canCollapseMarginTopWithChildren; }
    bool canCollapseWithMarginBottom() const { return m_atBottomOfBlock && m_canCollapseMarginBottomWithChildren; }

    float positiveMargin() const { return m_positiveMargin; }
    float negativeMargin() const { return m_negativeMargin; }
    float margin() const { return m_positiveMargin - m_negativeMargin; }

    void setAtTopOfBlock(bool value) { m_atTopOfBlock = value; }
    void setAtBottomOfBlock(bool value) { m_atBottomOfBlock = value; }

    void setPositiveMargin(float value) { m_positiveMargin = value; }
    void setNegativeMargin(float value) { m_negativeMargin = value; }

    void setPositiveMarginIfLarger(float value) { if(value > m_positiveMargin) { m_positiveMargin = value; } }
    void setNegativeMarginIfLarger(float value) { if(value > m_negativeMargin) { m_negativeMargin = value; } }

private:
    bool m_atTopOfBlock;
    bool m_atBottomOfBlock;

    bool m_canCollapseWithChildren;
    bool m_canCollapseMarginTopWithChildren;
    bool m_canCollapseMarginBottomWithChildren;

    float m_positiveMargin;
    float m_negativeMargin;
};

inline MarginInfo::MarginInfo(const BlockFlowBox* block, float top, float bottom)
    : m_atTopOfBlock(true)
    , m_atBottomOfBlock(false)
{
    m_canCollapseWithChildren = !block->avoidsFloats();
    m_canCollapseMarginTopWithChildren = m_canCollapseWithChildren && !top;
    m_canCollapseMarginBottomWithChildren = m_canCollapseWithChildren && !bottom && block->style()->height().isAuto();

    m_positiveMargin = m_canCollapseMarginTopWithChildren ? block->maxPositiveMarginTop() : 0.f;
    m_negativeMargin = m_canCollapseMarginTopWithChildren ? block->maxNegativeMarginTop() : 0.f;
}

void BlockFlowBox::adjustPositionedBox(BoxFrame* child, const MarginInfo& marginInfo)
{
    auto staticTop = height();
    if(!marginInfo.canCollapseWithMarginTop())
        staticTop += marginInfo.margin();

    auto childLayer = child->layer();
    childLayer->setStaticLeft(startOffsetForContent());
    childLayer->setStaticTop(staticTop);
}

void BlockFlowBox::adjustFloatingBox(const MarginInfo& marginInfo)
{
    float marginOffset = 0;
    if(!marginInfo.canCollapseWithMarginTop())
        marginOffset = marginInfo.margin();
    setHeight(height() + marginOffset);
    positionNewFloats();
    setHeight(height() - marginOffset);
}

void BlockFlowBox::handleBottomOfBlock(float top, float bottom, MarginInfo& marginInfo)
{
    marginInfo.setAtBottomOfBlock(true);
    if(!marginInfo.canCollapseWithMarginBottom() && !marginInfo.canCollapseWithMarginTop())
        setHeight(height() + marginInfo.margin());
    setHeight(bottom + height());
    setHeight(std::max(top + bottom, height()));
    if(marginInfo.canCollapseWithMarginBottom() && !marginInfo.canCollapseWithMarginTop()) {
        m_maxPositiveMarginBottom = std::max(m_maxPositiveMarginBottom, marginInfo.positiveMargin());
        m_maxNegativeMarginBottom = std::max(m_maxNegativeMarginBottom, marginInfo.negativeMargin());
    }
}

float BlockFlowBox::collapseMargins(BoxFrame* child, MarginInfo& marginInfo)
{
    auto posTop = child->maxMarginTop(true);
    auto negTop = child->maxMarginTop(false);
    if(child->isSelfCollapsingBlock()) {
        posTop = std::max(posTop, child->maxMarginBottom(true));
        negTop = std::max(negTop, child->maxMarginBottom(false));
    }

    if(marginInfo.canCollapseWithMarginTop()) {
        m_maxPositiveMarginTop = std::max(posTop, m_maxPositiveMarginTop);
        m_maxNegativeMarginTop = std::max(negTop, m_maxNegativeMarginTop);
    }

    auto top = height();
    if(child->isSelfCollapsingBlock()) {
        auto collapsedPosTop = std::max(marginInfo.positiveMargin(), child->maxMarginTop(true));
        auto collapsedNegTop = std::max(marginInfo.negativeMargin(), child->maxMarginTop(false));
        if(!marginInfo.canCollapseWithMarginTop()) {
            top = height() + collapsedPosTop - collapsedNegTop;
        }

        marginInfo.setPositiveMargin(collapsedPosTop);
        marginInfo.setNegativeMargin(collapsedNegTop);

        marginInfo.setPositiveMarginIfLarger(child->maxMarginBottom(true));
        marginInfo.setNegativeMarginIfLarger(child->maxMarginBottom(false));
    } else {
        if(!marginInfo.atTopOfBlock() || !marginInfo.canCollapseMarginTopWithChildren()) {
            setHeight(height() + std::max(posTop, marginInfo.positiveMargin()) - std::max(negTop, marginInfo.negativeMargin()));
            top = height();
        }

        marginInfo.setPositiveMargin(child->maxMarginBottom(true));
        marginInfo.setNegativeMargin(child->maxMarginBottom(false));
    }

    return top;
}

void BlockFlowBox::layoutBlockChild(BoxFrame* child, MarginInfo& marginInfo)
{
    auto posTop = m_maxPositiveMarginTop;
    auto negTop = m_maxNegativeMarginTop;

    child->updateVerticalMargins();

    auto estimatedTop = height();
    if(!marginInfo.canCollapseWithMarginTop())
        estimatedTop += std::max(marginInfo.margin(), child->marginTop());
    child->setY(estimatedTop + getClearDelta(child, estimatedTop));

    child->layout();

    auto offsetY = collapseMargins(child, marginInfo);
    auto clearDelta = getClearDelta(child, offsetY);
    if(clearDelta && child->isSelfCollapsingBlock()) {
        marginInfo.setPositiveMargin(std::max(child->maxMarginTop(true), child->maxMarginBottom(true)));
        marginInfo.setNegativeMargin(std::max(child->maxMarginTop(false), child->maxMarginBottom(false)));

        setHeight(child->y() + child->maxMarginTop(false));
    } else {
        setHeight(clearDelta + height());
    }

    if(clearDelta && marginInfo.atTopOfBlock()) {
        m_maxPositiveMarginTop = posTop;
        m_maxNegativeMarginTop = negTop;
        marginInfo.setAtTopOfBlock(false);
    }

    child->setY(offsetY + clearDelta);
    if(marginInfo.atTopOfBlock() && !child->isSelfCollapsingBlock())
        marginInfo.setAtTopOfBlock(false);

    if(style()->isLeftToRightDirection()) {
        auto offsetX = borderAndPaddingLeft() + child->marginLeft();
        if(containsFloats() && child->avoidsFloats()) {
            auto childStyle = child->style();
            auto startOffset = startOffsetForLine(child->y(), false);
            if(childStyle->marginLeft().isAuto())
                offsetX = std::max(offsetX, startOffset + child->marginLeft());
            else if(startOffset > borderAndPaddingLeft()) {
                offsetX = std::max(offsetX, startOffset);
            }
        }

        child->setX(offsetX);
    } else {
        auto offsetX = borderAndPaddingRight() + child->marginRight();
        if(containsFloats() && child->avoidsFloats()) {
            auto childStyle = child->style();
            auto startOffset = startOffsetForLine(child->y(), false);
            if(childStyle->marginRight().isAuto())
                offsetX = std::max(offsetX, startOffset + child->marginRight());
            else if(startOffset > borderAndPaddingRight()) {
                offsetX = std::max(offsetX, startOffset);
            }
        }

        child->setX(width() - offsetX - child->width());
    }

    setHeight(height() + child->height());
    if(auto childBlock = to<BlockFlowBox>(child)) {
        addOverhangingFloats(childBlock);
    }
}

void BlockFlowBox::layoutBlockChildren()
{
    auto top = borderTop() + paddingTop();
    auto bottom = borderBottom() + paddingBottom();
    setHeight(top);

    MarginInfo marginInfo(this, top, bottom);
    for(auto child = firstBoxFrame(); child; child = child->nextBoxFrame()) {
        if(child->isPositioned()) {
            child->containingBlock()->insertPositonedBox(child);
            adjustPositionedBox(child, marginInfo);
        } else if(child->isFloating()) {
            insertFloatingBox(child);
            adjustFloatingBox(marginInfo);
        } else {
            layoutBlockChild(child, marginInfo);
        }
    }

    handleBottomOfBlock(top, bottom, marginInfo);
}

void BlockFlowBox::layoutInlineChildren()
{
    m_lineLayout->layout();
}

void BlockFlowBox::layout()
{
    updateWidth();
    buildIntrudingFloats();
    updateMaxMargins();

    if(isChildrenInline())
        layoutInlineChildren();
    else
        layoutBlockChildren();

    auto bottomEdge = borderBottom() + paddingBottom();
    if(avoidsFloats() && floatBottom() > (height() - bottomEdge))
        setHeight(bottomEdge + floatBottom());
    updateHeight();
    buildOverhangingFloats();
    layoutPositionedBoxes();
}

void BlockFlowBox::buildIntrudingFloats()
{
    if(m_floatingBoxes)
        m_floatingBoxes->clear();
    if(isFloating() || isPositioned() || avoidsFloats())
        return;

    auto parentBlock = to<BlockFlowBox>(parentBox());
    if(parentBlock == nullptr)
        return;

    auto parentHasFloats = false;
    auto offsetX = parentBlock->leftOffsetForContent();
    auto offsetY = y();

    auto prev = prevBox();
    while(prev && (prev->avoidsFloats() || !prev->isBlockFlowBox())) {
        if(prev->isFloating())
            parentHasFloats = true;
        prev = prev->prevBox();
    }

    if(parentHasFloats)
        addIntrudingFloats(parentBlock, offsetX, offsetY);

    auto prevBlock = to<BlockFlowBox>(prev);
    if(prevBlock == nullptr) {
        prevBlock = parentBlock;
    } else {
        offsetX = 0;
        offsetY -= prevBlock->y();
    }

    if(prevBlock->floatBottom() > offsetY) {
        addIntrudingFloats(prevBlock, offsetX, offsetY);
    }
}

void BlockFlowBox::buildOverhangingFloats()
{
    if(isChildrenInline())
        return;
    for(auto child = firstBox(); child; child = child->nextBox()) {
        if(child->isFloatingOrPositioned())
            continue;
        if(auto block = to<BlockFlowBox>(child)) {
            if(block->floatBottom() + block->y() > height()) {
                addOverhangingFloats(block);
            }
        }
    }
}

void BlockFlowBox::addIntrudingFloats(BlockFlowBox* prevBlock, float offsetX, float offsetY)
{
    if(!prevBlock->containsFloats() || prevBlock->avoidsFloats())
        return;
    for(auto& item : *prevBlock->floatingBoxes()) {
        if(item.bottom() > offsetY && !containsFloat(item.box())) {
            FloatingBox floatingBox(item.box());
            floatingBox.setX(item.x() - offsetX);
            if(prevBlock != parentBox())
                floatingBox.setX(item.x() + prevBlock->marginLeft());
            floatingBox.setX(item.x() - marginLeft());
            floatingBox.setY(item.y() - offsetY);
            floatingBox.setWidth(item.width());
            floatingBox.setHeight(item.height());
            floatingBox.setIsIntruding(true);
            floatingBox.setIsPlaced(true);
            if(!m_floatingBoxes)
                m_floatingBoxes = std::make_unique<FloatingBoxList>(heap());
            m_floatingBoxes->push_back(floatingBox);
        }
    }
}

void BlockFlowBox::addOverhangingFloats(BlockFlowBox* childBlock)
{
    if(!childBlock->containsFloats() || childBlock->avoidsFloats())
        return;
    for(auto& item : *childBlock->floatingBoxes()) {
        auto floatBottom = item.bottom() + childBlock->y();
        if(floatBottom > height() && !containsFloat(item.box())) {
            FloatingBox floatingBox(item.box());
            floatingBox.setX(item.x() + childBlock->x());
            floatingBox.setY(item.y() + childBlock->y());
            floatingBox.setWidth(item.width());
            floatingBox.setHeight(item.height());
            floatingBox.setIsIntruding(true);
            floatingBox.setIsPlaced(true);
            if(!m_floatingBoxes)
                m_floatingBoxes = std::make_unique<FloatingBoxList>(heap());
            m_floatingBoxes->push_back(floatingBox);
        }
    }
}

void BlockFlowBox::positionNewFloats()
{
    if(!containsFloats())
        return;
    auto floatTop = height();
    for(auto& floatingBox : *m_floatingBoxes) {
        if(floatingBox.isPlaced()) {
            floatTop = std::max(floatTop, floatingBox.y());
            continue;
        }

        auto child = floatingBox.box();
        assert(this == child->containingBlock());
        auto childStyle = child->style();
        if(childStyle->isClearLeft())
            floatTop = std::max(floatTop, leftFloatBottom());
        if(childStyle->isClearRight())
            floatTop = std::max(floatTop, rightFloatBottom());

        auto leftOffset = leftOffsetForContent();
        auto rightOffset = rightOffsetForContent();
        auto floatWidth = std::min(rightOffset - leftOffset, floatingBox.width());

        float floatLeft = 0;
        if(childStyle->floating() == Float::Left) {
            float heightRemainingLeft = 1;
            float heightRemainingRight = 1;
            floatLeft = leftOffsetForFloat(floatTop, leftOffset, false, &heightRemainingLeft);
            while(rightOffsetForFloat(floatTop, rightOffset, false, &heightRemainingRight) - floatLeft < floatWidth) {
                floatTop += std::min(heightRemainingLeft, heightRemainingRight);
                floatLeft = leftOffsetForFloat(floatTop, leftOffset, false, &heightRemainingLeft);
            }

            floatLeft = std::max(floatLeft, leftOffset - borderAndPaddingLeft());
        } else {
            float heightRemainingLeft = 1;
            float heightRemainingRight = 1;
            floatLeft = rightOffsetForFloat(floatTop, rightOffset, false, &heightRemainingRight);
            while(floatLeft - leftOffsetForFloat(floatTop, leftOffset, false, &heightRemainingLeft) < floatWidth) {
                floatTop += std::min(heightRemainingLeft, heightRemainingRight);
                floatLeft = rightOffsetForFloat(floatTop, rightOffset, false, &heightRemainingRight);
            }

            floatLeft -= floatingBox.width();
        }

        child->setX(floatLeft + child->marginLeft());
        child->setY(floatTop + child->marginTop());

        floatingBox.setX(floatLeft);
        floatingBox.setY(floatTop);
        floatingBox.setHeight(child->height() + child->marginTop() + child->marginBottom());
        floatingBox.setIsPlaced(true);
    }
}

bool BlockFlowBox::containsFloat(Box* box) const
{
    if(!m_floatingBoxes)
        return false;
    for(auto& floatingBox : *m_floatingBoxes) {
        if(box == floatingBox.box()) {
            return true;
        }
    }

    return false;
}

float BlockFlowBox::leftFloatBottom() const
{
    if(!m_floatingBoxes)
        return 0;
    float bottom = 0;
    for(auto& floatingBox : *m_floatingBoxes) {
        if(floatingBox.isPlaced() && floatingBox.type() == Float::Left) {
            bottom = std::max(bottom, floatingBox.bottom());
        }
    }

    return bottom;
}

float BlockFlowBox::rightFloatBottom() const
{
    if(!m_floatingBoxes)
        return 0;
    float bottom = 0;
    for(auto& floatingBox : *m_floatingBoxes) {
        if(floatingBox.isPlaced() && floatingBox.type() == Float::Right) {
            bottom = std::max(bottom, floatingBox.bottom());
        }
    }

    return bottom;
}

float BlockFlowBox::floatBottom() const
{
    if(!m_floatingBoxes)
        return 0;
    float bottom = 0;
    for(auto& floatingBox : *m_floatingBoxes) {
        if(floatingBox.isPlaced()) {
            bottom = std::max(bottom, floatingBox.bottom());
        }
    }

    return bottom;
}

float BlockFlowBox::nextFloatBottom(float y) const
{
    if(!m_floatingBoxes)
        return 0;
    std::optional<float> bottom;
    for(auto& floatingBox : *m_floatingBoxes) {
        assert(floatingBox.isPlaced());
        auto floatBottom = floatingBox.bottom();
        if(floatBottom > y) {
            bottom = !bottom ? floatBottom : std::min(*bottom, floatBottom);
        }
    }

    return bottom.value_or(0.f);
}

float BlockFlowBox::leftOffsetForFloat(float y, float offset, bool indent, float* heightRemaining) const
{
    if(heightRemaining) *heightRemaining = 1;
    if(m_floatingBoxes) {
        for(auto& item : *m_floatingBoxes) {
            if(item.type() != Float::Left || !item.isPlaced())
                continue;
            if(item.y() <= y && item.bottom() > y && item.right() > offset) {
                if(heightRemaining) *heightRemaining = item.bottom() - y;
                offset = item.right();
            }
        }
    }

    if(indent && style()->isLeftToRightDirection()) {
        float availableWidth = 0;
        auto textIndentLength = style()->textIndent();
        if(textIndentLength.isPercent())
            availableWidth = containingBlock()->availableWidth();
        offset += textIndentLength.calcMin(availableWidth);
    }

    return offset;
}

float BlockFlowBox::rightOffsetForFloat(float y, float offset, bool indent, float* heightRemaining) const
{
    if(heightRemaining) *heightRemaining = 1;
    if(m_floatingBoxes) {
        for(auto& item : *m_floatingBoxes) {
            if(item.type() != Float::Right || !item.isPlaced())
                continue;
            if(item.y() <= y && item.bottom() > y && item.x() < offset) {
                if(heightRemaining) *heightRemaining = item.bottom() - y;
                offset = item.x();
            }
        }
    }

    if(indent && style()->isRightToLeftDirection()) {
        float availableWidth = 0;
        auto textIndentLength = style()->textIndent();
        if(textIndentLength.isPercent())
            availableWidth = containingBlock()->availableWidth();
        offset -= textIndentLength.calcMin(availableWidth);
    }

    return offset;
}

float BlockFlowBox::getClearDelta(BoxFrame* child, float y) const
{
    if(!containsFloats())
        return 0;

    float delta = 0;
    switch(child->style()->clear()) {
    case Clear::Left:
        delta = std::max(0.f, leftFloatBottom() - y);
        break;
    case Clear::Right:
        delta = std::max(0.f, rightFloatBottom() - y);
        break;
    case Clear::Both:
        delta = std::max(0.f, floatBottom() - y);
        break;
    case Clear::None:
        break;
    }

    if(!delta && child->avoidsFloats()) {
        auto top = y;
        while(true) {
            auto availableWidth = availableWidthForLine(top, false);
            if(availableWidth == availableWidthForContent())
                return top - y;

            auto childX = child->x();
            auto childY = child->y();
            auto childWidth = child->width();
            auto childMarginLeft = child->marginLeft();
            auto childMarginRight = child->marginRight();

            child->setY(top);
            child->computeWidth(childX, childWidth, childMarginLeft, childMarginRight);
            child->setY(childY);
            if(childWidth <= availableWidth)
                return top - y;
            top = nextFloatBottom(top);
        }
    }

    return delta;
}

void BlockFlowBox::updateMaxMargins()
{
    if(isTableCellBox())
        return;
    m_maxPositiveMarginTop = std::max(0.f, m_marginTop);
    m_maxNegativeMarginTop = std::max(0.f, -m_marginTop);
    m_maxPositiveMarginBottom = std::max(0.f, m_marginBottom);
    m_maxNegativeMarginBottom = std::max(0.f, -m_marginBottom);
}

float BlockFlowBox::maxMarginTop(bool positive) const
{
    return positive ? m_maxPositiveMarginTop : m_maxNegativeMarginTop;
}

float BlockFlowBox::maxMarginBottom(bool positive) const
{
    return positive ? m_maxPositiveMarginBottom : m_maxNegativeMarginBottom;
}

void BlockFlowBox::insertFloatingBox(BoxFrame* box)
{
    if(containsFloat(box))
        return;

    box->layout();

    FloatingBox floatingBox(box);
    floatingBox.setWidth(box->width() + box->marginLeft() + box->marginRight());
    floatingBox.setIsIntruding(false);
    floatingBox.setIsPlaced(false);
    if(!m_floatingBoxes)
        m_floatingBoxes = std::make_unique<FloatingBoxList>(heap());
    m_floatingBoxes->push_back(floatingBox);
}

void BlockFlowBox::removeFloatingBox(BoxFrame* box)
{
    if(!m_floatingBoxes)
        return;
    auto it = m_floatingBoxes->begin();
    while(it != m_floatingBoxes->end()) {
        if(box == it->box())
            break;
        ++it;
    }

    assert(it != m_floatingBoxes->end());
    m_floatingBoxes->erase(it);
}

void BlockFlowBox::addBox(Box* box)
{
    if(m_continuation) {
        m_continuation->addBox(box);
        return;
    }

    if(isChildrenInline() && !box->isInline() && !box->isFloatingOrPositioned()) {
        for(auto child = firstBox(); child; child = child->nextBox()) {
            if(child->isFloatingOrPositioned())
                continue;
            auto newBlock = createAnonymousBlock(style());
            moveChildrenTo(newBlock);
            appendChild(newBlock);
            break;
        }

        setChildrenInline(false);
    } else if(!isChildrenInline() && (box->isInline() || box->isFloatingOrPositioned())) {
        auto lastChild = lastBox();
        if(lastChild && lastChild->isAnonymous() && lastChild->isBlockBox()) {
            lastChild->addBox(box);
            return;
        }

        if(box->isInline()) {
            auto newBlock = createAnonymousBlock(style());
            appendChild(newBlock);

            auto child = newBlock->prevBox();
            while(child && child->isFloatingOrPositioned()) {
                auto prevBox = child->prevBox();
                removeChild(child);
                newBlock->insertChild(child, newBlock->firstBox());
                child = prevBox;
            }

            newBlock->addBox(box);
            return;
        }
    }

    BlockBox::addBox(box);
}

void BlockFlowBox::build(BoxLayer* layer)
{
    if(isChildrenInline()) {
        m_lineLayout = LineLayout::create(this);
        m_lineLayout->build();
    }

    BlockBox::build(layer);
}

} // namespace htmlbook
