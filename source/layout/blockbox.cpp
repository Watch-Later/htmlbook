#include "blockbox.h"

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

bool BlockFlowBox::containsFloats(Box* box) const
{
    if(!m_floatingBoxes)
        return false;
    auto it = m_floatingBoxes->begin();
    while(it != m_floatingBoxes->end()) {
        if(it->box == box)
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
        if(it->box == box)
            return;
        ++it;
    }

    FloatingBox floatingBox;
    floatingBox.type = box->style()->floating();
    floatingBox.box = box;
    floatingBox.isHidden = false;
    floatingBox.isIntruding = false;
    m_floatingBoxes->push_back(floatingBox);
}

void BlockFlowBox::removeFloatingBox(BoxFrame* box)
{
    if(!m_floatingBoxes)
        return;
    auto it = m_floatingBoxes->begin();
    while(it != m_floatingBoxes->end()) {
        if(it->box == box)
            break;
        ++it;
    }

    assert(it != m_floatingBoxes->end());
    m_floatingBoxes->erase(it);
}

} // namespace htmlbook
