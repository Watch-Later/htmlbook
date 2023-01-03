#include "blockbox.h"
#include "tablebox.h"

namespace htmlbook {

BlockBox::BlockBox(Node* node, const RefPtr<BoxStyle>& style)
    : BoxFrame(node, style)
{
    switch(style->display()) {
    case Display::Inline:
    case Display::InlineBlock:
    case Display::InlineFlex:
    case Display::InlineTable:
        setReplaced(true);
        break;
    default:
        setReplaced(false);
        break;
    }
}

void BlockBox::computeInlinePreferredWidths(float& minWidth, float& maxWidth) const
{
    minWidth = 0;
    maxWidth = 0;
}

void BlockBox::computeBlockPreferredWidths(float& minWidth, float& maxWidth) const
{
    float floatLeftWidth = 0;
    float floatRightWidth = 0;

    auto nowrap = style()->whiteSpace() == WhiteSpace::Nowrap;
    for(auto child = m_children.firstBox(); child; child = child->nextBox()) {
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

        float childMinWidth = 0;
        float childMaxWidth = 0;
        if(auto box = to<BoxFrame>(child)) {
            childMinWidth = box->minPreferredWidth();
            childMaxWidth = box->maxPreferredWidth();
        }

        auto marginWidth = marginLeft + marginRight;
        auto width = childMinWidth + marginWidth;

        minWidth = std::max(width, minWidth);
        if(nowrap && !is<TableBox>(*this))
            maxWidth = std::max(width, maxWidth);
        width = childMinWidth + marginWidth;

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
    maxWidth = std::max(minWidth, maxWidth);
}

void BlockBox::computePreferredWidths(float& minWidth, float& maxWidth) const
{
    minWidth = 0;
    maxWidth = 0;

    auto styleWidth = style()->width();
    if(styleWidth.isFixed() && !is<TableCellBox>(*this)) {
        minWidth = maxWidth = computeContentBoxWidth(styleWidth.value());
    } else if(isChildrenInline()) {
        computeInlinePreferredWidths(minWidth, maxWidth);
    } else {
        computeBlockPreferredWidths(minWidth, maxWidth);
    }

    auto styleMinWidth = style()->minWidth();
    if(styleMinWidth.isFixed() && styleMinWidth.value() > 0) {
        minWidth = std::max(minWidth, computeContentBoxWidth(styleMinWidth.value()));
        maxWidth = std::max(maxWidth, computeContentBoxWidth(styleMinWidth.value()));
    }

    auto styleMaxWidth = style()->maxWidth();
    if(styleMaxWidth.isFixed()) {
        minWidth = std::min(minWidth, computeContentBoxWidth(styleMaxWidth.value()));
        maxWidth = std::min(maxWidth, computeContentBoxWidth(styleMaxWidth.value()));
    }

    minWidth += borderWidth() + paddingWidth();
    maxWidth += borderWidth() + paddingWidth();
}

void BlockBox::addBox(Box* box)
{
    if(m_continuation) {
        m_continuation->addBox(box);
        return;
    }

    if(isChildrenInline() && !box->isInline() && !box->isFloatingOrPositioned()) {
        if(!m_children.empty()) {
            auto newBlock = createAnonymousBlock(style());
            moveChildrenTo(newBlock);
            m_children.append(this, newBlock);
        }

        setChildrenInline(false);
    } else if(!isChildrenInline() && (box->isInline() || box->isFloatingOrPositioned())) {
        auto lastChild = m_children.lastBox();
        if(lastChild && lastChild->isAnonymous() && is<BlockBox>(lastChild)) {
            lastChild->addBox(box);
            return;
        }

        if(box->isInline()) {
            auto newBlock = createAnonymousBlock(style());
            m_children.append(this, newBlock);

            auto children = newBlock->children();
            auto child = newBlock->prevBox();
            while(child && child->isFloatingOrPositioned()) {
                auto prevBox = child->prevBox();
                m_children.remove(this, child);
                children->insert(newBlock, child, children->firstBox());
                child = prevBox;
            }

            newBlock->addBox(box);
            return;
        }
    }

    BoxFrame::addBox(box);
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

BlockFlowBox::~BlockFlowBox() = default;

BlockFlowBox::BlockFlowBox(Node* node, const RefPtr<BoxStyle>& style)
    : BlockBox(node, style)
   , m_lines(style->heap())
{
    setChildrenInline(true);
}

void BlockFlowBox::setFirstLineStyle(RefPtr<BoxStyle> firstLineStyle)
{
    m_firstLineStyle = std::move(firstLineStyle);
}

bool BlockFlowBox::containsFloat(Box* box) const
{
    if(!m_floatingBoxes)
        return false;
    auto it = m_floatingBoxes->begin();
    while(it != m_floatingBoxes->end()) {
        if(box == it->box())
            return false;
        ++it;
    }

    return false;
}

void BlockFlowBox::insertFloatingBox(BoxFrame* box)
{
    assert(box->isFloating());
    if(!m_floatingBoxes)
        m_floatingBoxes = std::make_unique<FloatingBoxList>(heap());
    auto it = m_floatingBoxes->begin();
    while(it != m_floatingBoxes->end()) {
        if(box == it->box())
            return;
        ++it;
    }

    FloatingBox floatingBox(box);
    floatingBox.setIsIntruding(false);
    floatingBox.setIsHidden(false);
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

} // namespace htmlbook
