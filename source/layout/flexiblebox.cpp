#include "flexiblebox.h"

namespace htmlbook {

std::unique_ptr<FlexItem> FlexItem::create(BoxFrame* box)
{
    return std::unique_ptr<FlexItem>(new (box->heap()) FlexItem(box));
}

FlexibleBox* FlexItem::parentBox() const
{
    return to<FlexibleBox>(m_box->parentBox());
}

const RefPtr<BoxStyle>& FlexItem::parentStyle() const
{
    return parentBox()->style();
}

AlignItem FlexItem::alignSelf() const
{
    if(style()->alignSelf() == AlignItem::Auto)
        return parentStyle()->alignItems();
    return style()->alignSelf();
}

Length FlexItem::flexBasis() const
{
    auto flexBasis = style()->flexBasis();
    if(flexBasis.isAuto())
        return parentStyle()->isRowFlexDirection() ? style()->width() : style()->height();
    return flexBasis;
}

FlexItem::FlexItem(BoxFrame* box)
    : m_box(box)
{
}

std::unique_ptr<FlexLine> FlexLine::create(FlexibleBox* box)
{
    return std::unique_ptr<FlexLine>(new (box->heap()) FlexLine(box));
}

void FlexLine::addItem(FlexItem* item)
{
    m_items.push_back(item);
}

FlexLine::FlexLine(FlexibleBox* box)
    : m_box(box)
    , m_items(box->heap())
{
}

FlexibleBox::FlexibleBox(Node* node, const RefPtr<BoxStyle>& style)
    : BlockBox(node, style)
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
        if(style()->isColumnFlexDirection()) {
            minWidth = std::max(minWidth, childMinWidth);
            maxWidth = std::max(maxWidth, childMaxWidth);
        } else {
            maxWidth += childMaxWidth;
            if(style()->flexWrap() == FlexWrap::Nowrap) {
                minWidth += childMinWidth;
            } else {
                minWidth = std::max(minWidth, childMinWidth);
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
    for(auto child = firstBoxFrame(); child; child = child->nextBoxFrame()) {
        if(child->isPositioned())
            continue;
        m_items.push_back(FlexItem::create(child));
    }

    auto compare = [](auto& a, auto& b) { return a->order() < b->order(); };
    std::stable_sort(m_items.begin(), m_items.end(), compare);

    BlockBox::build(layer);
}

} // namespace htmlbook
