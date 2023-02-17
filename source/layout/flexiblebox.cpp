#include "flexiblebox.h"

namespace htmlbook {

FlexItem::FlexItem(BlockBox* box, int order, float flexGrow, float flexShrink, AlignItem alignSelf)
    : m_box(box)
    , m_order(order)
    , m_flexGrow(flexGrow)
    , m_flexShrink(flexShrink)
    , m_alignSelf(alignSelf)
{
}

float FlexItem::constrainMainSizeByMinMax(float size) const
{
    const auto& block = flexBox();
    if(auto maxSize = block.computeMaxMainSize(m_box))
        size = std::min(size, *maxSize);
    if(auto minSize = block.computeMinMainSize(m_box))
        size = std::max(size, *minSize);
    return size;
}

float FlexItem::constrainCrossSizeByMinMax(float size) const
{
    const auto& block = flexBox();
    if(auto maxSize = block.computeMaxCrossSize(m_box))
        size = std::min(size, *maxSize);
    if(auto minSize = block.computeMinCrossSize(m_box))
        size = std::max(size, *minSize);
    return size;
}

float FlexItem::flexBaseMarginBoxSize() const
{
    if(isHorizontalFlow())
        return m_flexBaseSize + m_box->marginWidth() + m_box->borderAndPaddingWidth();
    return m_flexBaseSize + m_box->marginHeight() + m_box->borderAndPaddingHeight();
}

float FlexItem::flexBaseBorderBoxSize() const
{
    if(isHorizontalFlow())
        return m_flexBaseSize + m_box->borderAndPaddingWidth();
    return m_flexBaseSize + m_box->borderAndPaddingHeight();
}

float FlexItem::targetMainMarginBoxSize() const
{
    if(isHorizontalFlow())
        return m_targetMainSize + m_box->marginWidth() + m_box->borderAndPaddingWidth();
    return m_targetMainSize + m_box->marginHeight() + m_box->borderAndPaddingHeight();
}

float FlexItem::targetMainBorderBoxSize() const
{
    if(isHorizontalFlow())
        return m_targetMainSize + m_box->marginWidth() + m_box->borderAndPaddingWidth();
    return m_targetMainSize + m_box->marginHeight() + m_box->borderAndPaddingHeight();
}

float FlexItem::marginBoxMainSize() const
{
    if(isHorizontalFlow())
        return m_box->width() + m_box->marginWidth();
    return m_box->height() + m_box->marginHeight();
}

float FlexItem::marginBoxCrossSize() const
{
    if(isHorizontalFlow())
        return m_box->height() + m_box->marginHeight();
    return m_box->width() + m_box->marginWidth();
}

float FlexItem::borderBoxMainSize() const
{
    if(isHorizontalFlow())
        return m_box->width();
    return m_box->height();
}

float FlexItem::borderBoxCrossSize() const
{
    if(isHorizontalFlow())
        return m_box->height();
    return m_box->width();
}

float FlexItem::marginStart() const
{
    if(isHorizontalFlow())
        return m_box->marginLeft();
    return m_box->marginTop();
}

float FlexItem::marginEnd() const
{
    if(isHorizontalFlow())
        return m_box->marginRight();
    return m_box->marginBottom();
}

float FlexItem::marginBefore() const
{
    if(isHorizontalFlow())
        return m_box->marginTop();
    return m_box->marginLeft();
}

float FlexItem::marginAfter() const
{
    if(isHorizontalFlow())
        return m_box->marginBottom();
    return m_box->marginRight();
}

FlexibleBox::FlexibleBox(Node* node, const RefPtr<BoxStyle>& style)
    : BlockBox(node, style)
    , m_flexDirection(style->flexDirection())
    , m_flexWrap(style->flexWrap())
    , m_justifyContent(style->justifyContent())
    , m_alignContent(style->alignContent())
    , m_items(style->heap())
    , m_lines(style->heap())
{
    setChildrenInline(false);
}

void FlexibleBox::computePreferredWidths(float& minWidth, float& maxWidth) const
{
    for(auto child = firstBoxFrame(); child; child = child->nextBoxFrame()) {
        if(child->isPositioned())
            continue;
        auto childStyle = child->style();
        auto marginLeftLength = childStyle->marginLeft();
        auto marginRightLength = childStyle->marginRight();

        float marginWidth = 0;
        if(marginLeftLength.isFixed())
            marginWidth += marginLeftLength.value();
        if(marginRightLength.isFixed())
            marginWidth += marginRightLength.value();

        auto childMinWidth = child->minPreferredWidth();
        auto childMaxWidth = child->maxPreferredWidth();

        childMinWidth += marginWidth;
        childMaxWidth += marginWidth;
        if(isVerticalFlow()) {
            minWidth = std::max(minWidth, childMinWidth);
            maxWidth = std::max(maxWidth, childMaxWidth);
        } else {
            maxWidth += childMaxWidth;
            if(isMultiLine()) {
                minWidth = std::max(minWidth, childMinWidth);
            } else {
                minWidth += childMinWidth;
            }
        }
    }

    minWidth = std::max(0.f, minWidth);
    maxWidth = std::max(minWidth, maxWidth);
}

void FlexibleBox::addBox(Box* box)
{
    if(box->isPositioned() || box->isBlockBox()) {
        BlockBox::addBox(box);
        return;
    }

    auto lastChild = lastBox();
    if(lastChild && lastChild->isAnonymous() && lastChild->isBlockBox()) {
        lastChild->addBox(box);
        return;
    }

    auto newBlock = createAnonymousBlock(style());
    appendChild(newBlock);
    newBlock->addBox(box);
}

void FlexibleBox::build(BoxLayer* layer)
{
    auto alignItems = style()->alignItems();
    for(auto box = firstBox(); box; box = box->nextBox()) {
        if(box->isPositioned())
            continue;
        auto child = to<BlockBox>(box);
        auto childStyle = child->style();
        auto order = childStyle->order();
        auto flexGlow = childStyle->flexGrow();
        auto flexShrink = childStyle->flexGrow();
        auto alignSelf = childStyle->alignSelf();
        if(alignSelf == AlignItem::Auto)
            alignSelf = alignItems;
        m_items.emplace_back(child, order, flexGlow, flexShrink, alignSelf);
    }

    auto compare = [](auto& a, auto& b) { return a.order() < b.order(); };
    std::stable_sort(m_items.begin(), m_items.end(), compare);

    BlockBox::build(layer);
}

float FlexibleBox::computeFlexBaseSize(BlockBox* child) const
{
    auto childStyle = child->style();
    auto flexBasis = childStyle->flexBasis();
    if(isHorizontalFlow()) {
        if(flexBasis.isAuto())
            flexBasis = childStyle->width();
        if(flexBasis.isAuto())
            return child->maxPreferredWidth() - child->borderAndPaddingWidth();
        return child->computeWidthUsing(flexBasis, this, availableWidth()) - child->borderAndPaddingWidth();
    }

    if(flexBasis.isAuto())
        flexBasis = childStyle->height();
    if(auto height = child->computeHeightUsing(flexBasis))
        return child->adjustBorderBoxHeight(*height) - child->borderAndPaddingHeight();

    child->layout();

    return child->height() - child->borderAndPaddingHeight();
}

std::optional<float> FlexibleBox::computeMinMainSize(const BlockBox* child) const
{
    auto childStyle = child->style();
    if(isHorizontalFlow()) {
        auto minWidthLength = childStyle->minWidth();
        if(minWidthLength.isAuto())
            return std::nullopt;
        return child->computeWidthUsing(minWidthLength, this, availableWidth()) - child->borderAndPaddingWidth();
    }

    if(auto height = child->computeHeightUsing(childStyle->minHeight()))
        return child->adjustBorderBoxHeight(*height) - child->borderAndPaddingHeight();
    return std::nullopt;
}

std::optional<float> FlexibleBox::computeMaxMainSize(const BlockBox* child) const
{
    auto childStyle = child->style();
    if(isHorizontalFlow()) {
        auto maxWidthLength = childStyle->maxWidth();
        if(maxWidthLength.isNone())
            return std::nullopt;
        return child->computeWidthUsing(maxWidthLength, this, availableWidth()) - child->borderAndPaddingWidth();
    }

    if(auto height = child->computeHeightUsing(childStyle->maxHeight()))
        return child->adjustBorderBoxHeight(*height) - child->borderAndPaddingHeight();
    return std::nullopt;
}

std::optional<float> FlexibleBox::computeMinCrossSize(const BlockBox* child) const
{
    auto childStyle = child->style();
    if(isVerticalFlow()) {
        auto minWidthLength = childStyle->minWidth();
        if(minWidthLength.isAuto())
            return std::nullopt;
        return child->computeWidthUsing(minWidthLength, this, availableWidth()) - child->borderAndPaddingWidth();
    }

    if(auto height = child->computeHeightUsing(childStyle->minHeight()))
        return child->adjustBorderBoxHeight(*height) - child->borderAndPaddingHeight();
    return std::nullopt;
}

std::optional<float> FlexibleBox::computeMaxCrossSize(const BlockBox* child) const
{
    auto childStyle = child->style();
    if(isVerticalFlow()) {
        auto maxWidthLength = childStyle->maxWidth();
        if(maxWidthLength.isNone())
            return std::nullopt;
        return child->computeWidthUsing(maxWidthLength, this, availableWidth()) - child->borderAndPaddingWidth();
    }

    if(auto height = child->computeHeightUsing(childStyle->maxHeight()))
        return child->adjustBorderBoxHeight(*height) - child->borderAndPaddingHeight();
    return std::nullopt;
}

float FlexibleBox::availableMainSize() const
{
    if(isHorizontalFlow())
        return contentWidth();
    return availableHeight();
}

float FlexibleBox::availableCrossSize() const
{
    if(isHorizontalFlow())
        return contentHeight();
    return availableWidth();
}

float FlexibleBox::borderStart() const
{
    if(isHorizontalFlow())
        return borderLeft();
    return borderTop();
}

float FlexibleBox::borderEnd() const
{
    if(isHorizontalFlow())
        return borderRight();
    return borderBottom();
}

float FlexibleBox::borderBefore() const
{
    if(isHorizontalFlow())
        return borderTop();
    return borderLeft();
}

float FlexibleBox::borderAfter() const
{
    if(isHorizontalFlow())
        return borderBottom();
    return borderRight();
}

float FlexibleBox::paddingStart() const
{
    if(isHorizontalFlow())
        return paddingLeft();
    return paddingTop();
}

float FlexibleBox::paddingEnd() const
{
    if(isHorizontalFlow())
        return paddingRight();
    return paddingBottom();
}

float FlexibleBox::paddingBefore() const
{
    if(isHorizontalFlow())
        return paddingTop();
    return paddingLeft();
}

float FlexibleBox::paddingAfter() const
{
    if(isHorizontalFlow())
        return paddingBottom();
    return paddingRight();
}

void FlexibleBox::layout()
{
    updateWidth();
    setHeight(borderHeight() + paddingHeight());

    m_lines.clear();
    for(auto& item : m_items) {
        auto child = item.box();

        child->clearOverrideSize();
        child->updateMarginWidths();

        item.setFlexBaseSize(computeFlexBaseSize(child));
        item.setTargetMainSize(item.constrainMainSizeByMinMax(item.flexBaseSize()));
    }

    auto containerMainSize = availableMainSize();
    for(auto it = m_items.begin(); it != m_items.end(); ++it) {
        float mainSize = 0;

        auto begin = it;
        while(it != m_items.end()) {
            FlexItem& item = *it;
            auto itemMainSize = item.targetMainMarginBoxSize();
            if(isMultiLine() && it != begin && itemMainSize + mainSize > containerMainSize)
                break;
            mainSize += itemMainSize;
            ++it;
        }

        const auto sign = mainSize < containerMainSize ? FlexSign::Positive : FlexSign::Negative;

        float frozenSpace = 0;
        float unfrozenSpace = 0;
        std::list<FlexItem*> unfrozenItems;

        FlexItemSpan items(begin, it);
        for(auto& item : items) {
            if(item.flexFactor(sign) == 0 || (sign == FlexSign::Positive && item.flexBaseSize() > item.targetMainSize())
                || (sign == FlexSign::Negative && item.flexBaseSize() < item.targetMainSize())) {
                frozenSpace += item.targetMainMarginBoxSize();
            } else {
                item.setTargetMainSize(item.flexBaseSize());
                unfrozenSpace += item.flexBaseMarginBoxSize();
                unfrozenItems.push_back(&item);
            }
        }

        auto initialFreeSpace = containerMainSize - frozenSpace - unfrozenSpace;
        while(!unfrozenItems.empty()) {
            float totalFlexFactor = 0;
            for(auto& item : unfrozenItems) {
                totalFlexFactor += item->flexFactor(sign);
            }

            auto remainingFreeSpace = containerMainSize - frozenSpace - unfrozenSpace;
            if(totalFlexFactor < 1.f) {
                auto scaledInitialFreeSpace = initialFreeSpace * totalFlexFactor;
                if(std::abs(scaledInitialFreeSpace) < std::abs(remainingFreeSpace)) {
                    remainingFreeSpace = scaledInitialFreeSpace;
                }
            }

            if(remainingFreeSpace != 0.f) {
                if(sign == FlexSign::Positive) {
                    for(auto& item : unfrozenItems) {
                        auto distributeRatio = item->flexGrow() / totalFlexFactor;
                        item->setTargetMainSize(item->flexBaseSize() + remainingFreeSpace * distributeRatio);
                    }
                } else {
                    float totalScaledFlexShrinkFactor = 0;
                    for(auto& item : unfrozenItems)
                        totalScaledFlexShrinkFactor += item->flexBaseSize() * item->flexShrink();
                    for(auto& item : unfrozenItems) {
                        auto scaledFlexShrinkFactor = item->flexBaseSize() * item->flexShrink();
                        auto distributeRatio = scaledFlexShrinkFactor / totalScaledFlexShrinkFactor;
                        item->setTargetMainSize(item->flexBaseSize() + remainingFreeSpace * distributeRatio);
                    }
                }
            }

            float totalViolation = 0;
            for(auto& item : unfrozenItems) {
                auto unclampedSize = item->targetMainSize();
                auto clampedSize = item->constrainMainSizeByMinMax(unclampedSize);

                auto violation = clampedSize - unclampedSize;
                if(violation > 0.f) {
                    item->setViolation(FlexItem::Violation::Min);
                } else if(violation < 0.f){
                    item->setViolation(FlexItem::Violation::Max);
                } else {
                    item->setViolation(FlexItem::Violation::None);
                }

                item->setTargetMainSize(clampedSize);
                totalViolation += violation;
            }

            auto freezeMinViolations = totalViolation > 0.f;
            auto freezeMaxViolations = totalViolation < 0.f;
            auto freezeAllViolations = totalViolation == 0.f;

            auto itemIterator = unfrozenItems.begin();
            while(itemIterator != unfrozenItems.end()) {
                auto currentIterator = itemIterator++;
                auto item = *currentIterator;
                if(freezeAllViolations || (freezeMinViolations && item->minViolation()) || (freezeMaxViolations && item->maxViolation())) {
                    frozenSpace += item->targetMainMarginBoxSize();
                    unfrozenSpace -= item->flexBaseMarginBoxSize();
                    unfrozenItems.erase(currentIterator);
                }
            }
        }

        size_t autoMarginCount = 0;
        for(auto& item : items) {
            auto child = item.box();
            auto childStyle = child->style();
            if(isHorizontalFlow()) {
                if(childStyle->marginLeft().isAuto())
                    ++autoMarginCount;
                if(childStyle->marginRight().isAuto()) {
                    ++autoMarginCount;
                }
            } else {
                if(childStyle->marginTop().isAuto())
                    ++autoMarginCount;
                if(childStyle->marginBottom().isAuto()) {
                    ++autoMarginCount;
                }
            }
        }

        auto availableSpace = containerMainSize - frozenSpace;

        float autoMarginOffset = 0;
        if(autoMarginCount > 0) {
            autoMarginOffset = availableSpace / autoMarginCount;
            availableSpace = 0.f;
        }

        auto mainOffset = borderStart() + paddingStart();
        switch(m_justifyContent) {
        case AlignContent::FlexEnd:
            mainOffset += availableSpace;
            break;
        case AlignContent::Center:
            mainOffset += availableSpace / 2.f;
            break;
        case AlignContent::SpaceAround:
            if(availableSpace > 0)
                mainOffset += availableSpace / (2.f * items.size());
            else
                mainOffset += availableSpace / 2.f;
            break;
        default:
            break;
        }

        for(auto& item : items) {
            auto child = item.box();
            auto childStyle = child->style();
            if(isHorizontalFlow())
                child->setOverrideWidth(item.targetMainBorderBoxSize());
            else
                child->setOverrideHeight(item.targetMainBorderBoxSize());

            child->layout();

            if(autoMarginCount > 0) {
                if(isHorizontalFlow()) {
                    if(childStyle->marginLeft().isAuto())
                        child->setMarginLeft(autoMarginOffset);
                    if(childStyle->marginRight().isAuto()) {
                        child->setMarginRight(autoMarginOffset);
                    }
                } else {
                    if(childStyle->marginTop().isAuto())
                        child->setMarginTop(autoMarginOffset);
                    if(childStyle->marginBottom().isAuto()) {
                        child->setMarginBottom(autoMarginOffset);
                    }
                }
            }

            mainOffset += item.marginStart();
            if(isHorizontalFlow()) {
                if(isReverseDirection()) {
                    child->setX(containerMainSize - mainOffset - item.borderBoxMainSize());
                } else {
                    child->setX(mainOffset);
                }
            } else {
                if(isReverseDirection()) {
                    child->setY(containerMainSize - mainOffset - item.borderBoxMainSize());
                } else {
                    child->setY(mainOffset);
                }
            }

            mainOffset += item.borderBoxMainSize();
            mainOffset += item.marginEnd();
            if(availableSpace > 0 && items.size() > 1) {
                switch(m_justifyContent) {
                case AlignContent::SpaceAround:
                    mainOffset += availableSpace / items.size();
                    break;
                case AlignContent::SpaceBetween:
                    mainOffset += availableSpace / (items.size() - 1);
                    break;
                default:
                    break;
                }
            }
        }

        mainOffset += borderEnd() + paddingEnd();
        if(isVerticalFlow())
            setHeight(std::max(mainOffset, height()));
        m_lines.emplace_back(items);
    }

    auto crossOffset = borderBefore() + paddingBefore();
    for(auto& line : m_lines) {
        float crossSize = 0;
        for(auto& item : line.items()) {
            auto child = item.box();
            if(isHorizontalFlow())
                child->setY(crossOffset + item.marginBefore());
            else
                child->setX(crossOffset + item.marginBefore());
            crossSize = std::max(crossSize, item.marginBoxCrossSize());
        }

        line.setCrossOffset(crossOffset);
        line.setCrossSize(crossSize);
        crossOffset += crossSize;
    }

    crossOffset += borderAfter() + paddingAfter();
    if(isHorizontalFlow())
        setHeight(std::max(crossOffset, height()));
    updateHeight();

    if(!isMultiLine() && !m_lines.empty())
        m_lines.front().setCrossSize(availableCrossSize());

    if(isMultiLine() && !m_lines.empty()) {
        auto availableSpace = availableCrossSize();
        for(auto& line : m_lines) 
            availableSpace -= line.crossSize();

        float lineOffset = 0;
        switch(m_alignContent) {
        case AlignContent::FlexEnd:
            lineOffset += availableSpace;
            break;
        case AlignContent::Center:
            lineOffset += availableSpace / 2.f;
            break;
        case AlignContent::SpaceAround:
            if(availableSpace > 0)
                lineOffset += availableSpace / (2.f * m_lines.size());
            else
                lineOffset += availableSpace / 2.f;
            break;
        default:
            break;
        }

        for(auto& line : m_lines) {
            line.setCrossOffset(lineOffset + line.crossOffset());
            for(auto& item : line.items()) {
                auto child = item.box();
                if(isHorizontalFlow())
                    child->setY(lineOffset + child->y());
                else {
                    child->setX(lineOffset + child->x());
                }
            }

            if(m_alignContent == AlignContent::Stretch && availableSpace > 0) {
                auto lineSize = availableSpace / m_lines.size();
                line.setCrossSize(lineSize + line.crossSize());
                lineOffset += lineSize;
            }

            if(availableSpace > 0 && m_lines.size() > 1) {
                switch(m_alignContent) {
                case AlignContent::SpaceAround:
                    lineOffset += availableSpace / m_lines.size();
                    break;
                case AlignContent::SpaceBetween:
                    lineOffset += availableSpace / (m_lines.size() - 1);
                    break;
                default:
                    break;
                }
            }
        }
    }
}

} // namespace htmlbook
