#include "flexiblebox.h"

#include <limits>

namespace htmlbook {

FlexItem::FlexItem(BlockBox* box, int order, float flexGrow, float flexShrink)
    : m_box(box)
    , m_order(order)
    , m_flexGrow(flexGrow)
    , m_flexShrink(flexShrink)
{
}

FlexLine::FlexLine(FlexibleBox* flexBox, const FlexItemSpan& items, float mainSize, float containerMainSize)
    : m_flexBox(flexBox)
    , m_items(items)
    , m_mainSize(mainSize)
    , m_containerMainSize(containerMainSize)
{
}

FlexibleBox::FlexibleBox(Node* node, const RefPtr<BoxStyle>& style)
    : BlockBox(node, style)
    , m_flexDirection(style->flexDirection())
    , m_flexWrap(style->flexWrap())
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
    for(auto box = firstBox(); box; box = box->nextBox()) {
        if(box->isPositioned())
            continue;
        auto child = to<BlockBox>(box);
        auto childStyle = child->style();
        m_items.emplace_back(child, childStyle->order(), childStyle->flexGrow(), childStyle->flexShrink());
    }

    auto compare = [](auto& a, auto& b) { return a.order() < b.order(); };
    std::stable_sort(m_items.begin(), m_items.end(), compare);

    BlockBox::build(layer);
}

float FlexibleBox::computeFlexBaseSize(const BlockBox* child) const
{
    auto childStyle = child->style();
    auto flexBasis = childStyle->flexBasis();
    if(isHorizontalFlow()) {
        if(flexBasis.isAuto())
            flexBasis = childStyle->width();
        if(flexBasis.isAuto())
            return child->width();
        return child->computeWidthUsing(flexBasis, this, availableWidth());
    }

    if(flexBasis.isAuto())
        flexBasis = childStyle->height();
    if(auto height = child->computeHeightUsing(flexBasis))
        return child->adjustBorderBoxHeight(*height);
    return child->height();
}

float FlexibleBox::computeMinMainSize(const BlockBox* child) const
{
    auto childStyle = child->style();
    if(isHorizontalFlow()) {
        auto minWidthLength = childStyle->minWidth();
        if(minWidthLength.isAuto())
            return 0;
        return child->computeWidthUsing(minWidthLength, this, availableWidth());
    }

    if(auto height = child->computeHeightUsing(childStyle->minHeight()))
        return child->adjustBorderBoxHeight(*height);
    return 0;
}

float FlexibleBox::computeMaxMainSize(const BlockBox* child) const
{
    constexpr auto maxSize = std::numeric_limits<float>::max();

    auto childStyle = child->style();
    if(isHorizontalFlow()) {
        auto maxWidthLength = childStyle->maxWidth();
        if(maxWidthLength.isNone())
            return maxSize;
        return child->computeWidthUsing(maxWidthLength, this, availableWidth());
    }

    if(auto height = child->computeHeightUsing(childStyle->maxHeight()))
        return child->adjustBorderBoxHeight(*height);
    return maxSize;
}

float FlexibleBox::availableMainSize() const
{
    if(isHorizontalFlow())
        return availableWidth();
    return availableHeight();
}

void FlexibleBox::layout()
{
    m_lines.clear();
    for(auto& item : m_items) {
        auto child = item.box();

        child->clearOverrideSize();
        child->layout();

        item.setFlexBaseSize(computeFlexBaseSize(child));
        item.setMinMainSize(computeMinMainSize(child));
        item.setMaxMainSize(computeMaxMainSize(child));
    }

    auto containerMainSize = availableMainSize();
    for(auto it = m_items.begin(); it != m_items.end(); ++it) {
        float lineMainSize = 0;

        auto begin = it;
        while(it != m_items.end()) {
            FlexItem& item = *it;
            auto child = item.box();
            auto itemMainSize = item.hypotheticalMainSize();
            if(isHorizontalFlow())
                itemMainSize += child->marginWidth();
            else
                itemMainSize += child->marginHeight();
            if(isMultiLine() && it > begin && itemMainSize + lineMainSize > containerMainSize)
                break;

            item.setLineIndex(m_lines.size());
            item.setTargetMainSize(0);
            item.setMinViolation(false);
            item.setMaxViolation(false);
            lineMainSize += itemMainSize;
            it += 1;
        }

        FlexItemSpan items(begin, it);
        assert(!items.empty());
        m_lines.emplace_back(this, items, lineMainSize, containerMainSize);
    }
}

} // namespace htmlbook
