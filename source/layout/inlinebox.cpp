#include "inlinebox.h"
#include "blockbox.h"

namespace htmlbook {

InlineBox::InlineBox(Node* node, const RefPtr<BoxStyle>& style)
    : BoxModel(node, style)
    , m_lines(style->heap())
{
    setInline(true);
}

void InlineBox::addBox(Box* box)
{
    if(m_continuation) {
        m_continuation->addBox(box);
        return;
    }

    if(box->isInline() || box->isFloatingOrPositioned()) {
        BoxModel::addBox(box);
        return;
    }

    auto newBlock = createAnonymousBlock(style());
    BlockBox* preBlock = nullptr;
    BlockBox* postBlock = nullptr;
    auto block = containingBlock();
    if(block->isAnonymous()) {
        preBlock = block;
        postBlock = createAnonymousBlock(block->style());
        block = block->containingBlock();

        assert(block->lastBox() == preBlock);
        block->appendChild(newBlock);
        block->appendChild(postBlock);
    } else {
        preBlock = createAnonymousBlock(block->style());
        postBlock = createAnonymousBlock(block->style());
        block->moveChildrenTo(preBlock);

        block->appendChild(preBlock);
        block->appendChild(newBlock);
        block->appendChild(postBlock);
    }

    auto clone = new (heap()) InlineBox(nullptr, style());
    Box* currentParent = parentBox();
    Box* currentChild = this;
    auto currentClone = clone;
    while(currentParent != preBlock) {
        auto parent = to<InlineBox>(currentParent);
        assert(parent->continuation() == nullptr);
        auto clone = new (heap()) InlineBox(nullptr, parent->style());
        clone->appendChild(currentClone);
        parent->setContinuation(clone);

        assert(currentChild->nextBox() == nullptr);
        currentChild = currentParent;
        currentClone = clone;
        currentParent = currentParent->parentBox();
    }

    assert(currentChild->nextBox() == nullptr);
    postBlock->appendChild(currentClone);
    newBlock->addBox(box);
    newBlock->setContinuation(clone);
    setContinuation(newBlock);
}

void InlineBox::computeMargins(float& top, float& bottom, float& left, float& right) const
{
    auto compute = [this](const auto& margin) {
        float containerWidth = 0;
        if(margin.isPercent())
            containerWidth = containingBlockWidthForContent();
        return margin.calcMin(containerWidth);
    };

    top = compute(style()->marginTop());
    bottom = compute(style()->marginBottom());
    left = compute(style()->marginLeft());
    right = compute(style()->marginRight());
}

void InlineBox::updateMargins()
{
    computeMargins(m_marginTop, m_marginBottom, m_marginLeft, m_marginRight);
}

} // namespace htmlbook
