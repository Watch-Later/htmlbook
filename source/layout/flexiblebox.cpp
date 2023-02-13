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

FlexibleBox& FlexItem::flexBox() const
{
    return to<FlexibleBox>(*m_box->parentBox());
}

FlexLine& FlexItem::flexLine() const
{
    return flexBox().lines()[m_lineIndex];
}

float FlexItem::marginBoxMainSize() const
{
    if(flexBox().isHorizontalFlow())
        return m_box->marginWidth() + m_box->borderAndPaddingWidth();
    return m_box->marginHeight() + m_box->borderAndPaddingHeight();
}

float FlexItem::marginBoxCrossSize() const
{
    if(flexBox().isHorizontalFlow())
        return m_box->marginHeight() + m_box->borderAndPaddingHeight();
    return m_box->marginWidth() + m_box->borderAndPaddingWidth();
}

float FlexItem::borderBoxMainSize() const
{
    if(flexBox().isHorizontalFlow())
        return m_targetMainSize + m_box->borderAndPaddingWidth();
    return m_targetMainSize + m_box->borderAndPaddingHeight();
}

float FlexItem::borderBoxCrossSize() const
{
    if(flexBox().isHorizontalFlow())
        return m_targetMainSize + m_box->borderAndPaddingHeight();
    return m_targetMainSize + m_box->borderAndPaddingWidth();
}

FlexLine::FlexLine(const FlexItemSpan& items, float mainSize, float mainOffset, float crossSize, float crossOffset)
    : m_items(items)
    , m_mainSize(mainSize)
    , m_mainOffset(mainOffset)
    , m_crossSize(crossSize)
    , m_crossOffset(crossOffset)
{
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
        return availableWidth();
    return availableHeight();
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
    if(isVerticalFlow())
        return borderLeft();
    return borderTop();
}

float FlexibleBox::borderAfter() const
{
    if(isVerticalFlow())
        return borderRight();
    return borderBottom();
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
    if(isVerticalFlow())
        return paddingLeft();
    return paddingTop();
}

float FlexibleBox::paddingAfter() const
{
    if(isVerticalFlow())
        return paddingRight();
    return paddingBottom();
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
        item.setHypotheticalMainSize(item.constrainMainSizeByMinMax(item.flexBaseSize()));
    }

    auto crossOffset = borderBefore() + paddingBefore();
    auto containerMainSize = availableMainSize();
    for(auto it = m_items.begin(); it != m_items.end(); ++it) {
        float mainSize = 0;

        auto begin = it;
        while(it != m_items.end()) {
            FlexItem& item = *it;
            auto itemMainSize = item.hypotheticalMainSize() + item.marginBoxMainSize();
            if(isMultiLine() && it > begin && itemMainSize + mainSize > containerMainSize)
                break;

            item.setLineIndex(m_lines.size());
            mainSize += itemMainSize;
            it += 1;
        }

        const auto sign = mainSize < containerMainSize ? FlexSign::Positive : FlexSign::Negative;

        float frozenSpace = 0;
        float unfrozenSpace = 0;
        std::list<FlexItem*> unfrozenItems;

        FlexItemSpan items(begin, it);
        for(auto& item : items) {
            if(item.flexFactor(sign) == 0 || (sign == FlexSign::Positive && item.flexBaseSize() > item.hypotheticalMainSize())
                || (sign == FlexSign::Negative && item.flexBaseSize() < item.hypotheticalMainSize())) {
                item.setTargetMainSize(item.hypotheticalMainSize());
                frozenSpace += item.targetMainSize() + item.marginBoxMainSize();
            } else {
                item.setTargetMainSize(item.flexBaseSize());
                unfrozenSpace += item.targetMainSize() + item.marginBoxMainSize();
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
                    frozenSpace += item->targetMainSize() + item->marginBoxMainSize();
                    unfrozenSpace -= item->flexBaseSize() + item->marginBoxMainSize();
                    unfrozenItems.erase(currentIterator);
                }
            }
        }

        auto mainOffset = borderStart() + paddingStart();
        auto availableSpace = containerMainSize - frozenSpace;
        switch(m_justifyContent) {
        case AlignContent::Center:
            mainOffset += availableSpace / 2.f;
            break;
        case AlignContent::FlexEnd:
            mainOffset += availableSpace;
            break;
        case AlignContent::SpaceAround:
            if(availableSpace > 0 && !items.empty())
                mainOffset += availableSpace / (2.f * items.size());
            else
                mainOffset += availableSpace / 2.f;
            break;
        default:
            break;
        }

        float crossSize = 0;
        for(size_t index = 0; index < items.size(); ++index) {
            FlexItem& item = items[index];
            auto child = item.box();
            if(isHorizontalFlow())
                child->setOverrideWidth(item.borderBoxMainSize());
            else
                child->setOverrideHeight(item.borderBoxMainSize());

            child->layout();

            mainOffset += item.borderBoxMainSize();

            crossSize = std::max(crossSize, item.borderBoxCrossSize());
            if(index != items.size() - 1) {
                switch(m_justifyContent) {
                case AlignContent::SpaceBetween:
                    mainOffset += availableSpace / (items.size() - 1);
                    break;
                case AlignContent::SpaceAround:
                case AlignContent::Stretch:
                    mainOffset += availableSpace / items.size();
                    break;
                default:
                    break;
                }
            }
        }

        if(isHorizontalFlow())
            setHeight(std::max(height(), crossSize + crossOffset + borderAfter() + paddingAfter()));
        else
            setHeight(std::max(height(), mainOffset + borderEnd() + paddingEnd()));

        FlexLine line(items, mainSize, mainOffset, crossSize, crossOffset);
        crossOffset += crossSize;
        m_lines.push_back(line);
    }
}

} // namespace htmlbook
