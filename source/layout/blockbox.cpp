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
    for(auto child = firstBox(); child; child = child->nextBox()) {
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

void BlockBox::addBox(Box* box)
{
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
        if(lastChild && lastChild->isAnonymous() && is<BlockBox>(lastChild)) {
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

    BoxFrame::addBox(box);
}

BlockFlowBox::~BlockFlowBox() = default;

BlockFlowBox::BlockFlowBox(Node* node, const RefPtr<BoxStyle>& style)
    : BlockBox(node, style)
    , m_lines(style->heap())
{
    setChildrenInline(true);
}

void BlockFlowBox::computeInlinePreferredWidths(float& minWidth, float& maxWidth) const
{
    minWidth = 0;
    maxWidth = 0;
}

class MarginInfo {
public:
    MarginInfo(BlockFlowBox* block, float top, float bottom)
        : m_block(block), m_top(top), m_bottom(bottom)
    {}

    BlockFlowBox* block() const { return m_block; }
    float top() const { return m_top; }
    float bottom() const { return m_bottom; }

private:
    BlockFlowBox* m_block;
    float m_top;
    float m_bottom;
};

void BlockFlowBox::adjustPositionedBox(BoxFrame* box, const MarginInfo& info)
{
}

void BlockFlowBox::adjustFloatingBox(const MarginInfo& info)
{
}

void BlockFlowBox::handleBottomOfBlock(float top, float bottom, MarginInfo& info)
{
}

void BlockFlowBox::layoutBlockChild(BoxFrame* child, MarginInfo& info)
{
}

void BlockFlowBox::layoutBlockChildren()
{
    auto top = borderTop() + paddingTop();
    auto bottom = borderBottom() + paddingBottom();
    setHeight(top);

    MarginInfo info(this, top, bottom);
    for(auto box = firstBox(); box; box = box->nextBox()) {
        auto child = to<BoxFrame>(box);
        if(child->isPositioned()) {
            child->containingBlock()->insertPositonedBox(child);
            adjustPositionedBox(child, info);
        } else if(box->isFloating()) {
            insertFloatingBox(child);
            adjustFloatingBox(info);
        } else {
            layoutBlockChild(child, info);
        }
    }

    handleBottomOfBlock(top, bottom, info);
}

void BlockFlowBox::layoutInlineChildren()
{
}

void BlockFlowBox::layoutPositionedBoxes()
{
    if(!m_positionedBoxes)
        return;
    for(auto box : *m_positionedBoxes) {
        box->layout();
    }
}

void BlockFlowBox::layout()
{
    updateWidth();
    buildIntrudingFloats();
    if(!is<TableCellBox>(*this)) {
        m_maxPositiveMarginTop = std::max(0.f, m_marginTop);
        m_maxNegativeMarginTop = std::max(0.f, -m_marginTop);
        m_maxPositiveMarginBottom = std::max(0.f, m_marginBottom);
        m_maxNegativeMarginBottom = std::max(0.f, -m_marginBottom);
    }

    if(isChildrenInline())
        layoutInlineChildren();
    else
        layoutBlockChildren();

    updateHeight();
    buildOverhangingFloats();
    layoutPositionedBoxes();
}

void BlockFlowBox::setFirstLineStyle(RefPtr<BoxStyle> firstLineStyle)
{
    m_firstLineStyle = std::move(firstLineStyle);
}

void BlockFlowBox::buildIntrudingFloats()
{
    if(m_floatingBoxes)
        m_floatingBoxes->clear();
    if(isFloatingOrPositioned() || isRootBox() || avoidsFloats() || is<TableCellBox>(*this))
        return;

    auto parentBlock = to<BlockFlowBox>(parentBox());
    if(parentBlock == nullptr)
        return;

    auto parentHasFloats = false;
    auto xOffset = parentBlock->borderLeft() + parentBlock->paddingLeft();
    auto yOffset = y();

    auto prev = prevBox();
    while(prev && (prev->isFloatingOrPositioned() || prev->avoidsFloats() || !is<BlockBox>(*prev))) {
        if(prev->isFloating())
            parentHasFloats = true;
        prev = prev->prevBox();
    }

    if(parentHasFloats) 
        addIntrudingFloats(parentBlock, xOffset, yOffset);

    auto prevBlock = to<BlockFlowBox>(prev);
    if(prevBlock == nullptr) {
        prevBlock = parentBlock;
    } else {
        xOffset = 0;
        yOffset -= prevBlock->y();
    }

    if(prevBlock->containsFloats() && prevBlock->floatBottom() > yOffset) {
        addIntrudingFloats(prevBlock, xOffset, yOffset);
    }
}

void BlockFlowBox::buildOverhangingFloats()
{
    if(isChildrenInline())
        return;
    for(auto child = firstBox(); child; child = child->nextBox()) {
        if(!child->isFloatingOrPositioned() && is<BlockFlowBox>(child)) {
            auto block = to<BlockFlowBox>(child);
            if(block->floatBottom() + block->y() > height()) {
                addOverhangingFloats(block);
            }
        }
    }
}

void BlockFlowBox::addIntrudingFloats(BlockFlowBox* prevBlock, float xOffset, float yOffset)
{
    if(!prevBlock->containsFloats())
        return;
    for(auto& item : *prevBlock->floatingBoxes()) {
        if(item.bottom() > yOffset && !containsFloat(item.box())) {
            FloatingBox floatingBox(item.box());
            floatingBox.setIsIntruding(true);
            floatingBox.setIsHidden(true);
            floatingBox.setIsPlaced(true);
            if(prevBlock == parentBox())
                floatingBox.setX(item.x() - xOffset);
            else
                floatingBox.setX(item.x() - xOffset - prevBlock->marginLeft());
            floatingBox.setY(item.y() - yOffset);
            floatingBox.setWidth(item.width());
            floatingBox.setHeight(item.height());
            if(m_floatingBoxes == nullptr)
                m_floatingBoxes = std::make_unique<FloatingBoxList>(heap());
            m_floatingBoxes->push_back(floatingBox);
        }
    }
}

void BlockFlowBox::addOverhangingFloats(BlockFlowBox* childBlock)
{
    if(childBlock->isOverflowHidden() || !childBlock->containsFloats())
        return;
    for(auto& item : *childBlock->floatingBoxes()) {
        auto floatBottom = item.bottom() + childBlock->x();
        if(floatBottom > height() && !containsFloat(item.box())) {
            FloatingBox floatingBox(item.box());
            floatingBox.setIsIntruding(true);
            floatingBox.setIsHidden(true);
            floatingBox.setIsPlaced(true);
            floatingBox.setX(item.x() + childBlock->x());
            floatingBox.setY(item.y() + childBlock->y());
            floatingBox.setWidth(item.width());
            floatingBox.setHeight(item.height());
            if(m_floatingBoxes == nullptr)
                m_floatingBoxes = std::make_unique<FloatingBoxList>(heap());
            m_floatingBoxes->push_back(floatingBox);
        }
    }
}

void BlockFlowBox::positionNewFloats()
{
    if(!containsFloats())
        return;
    auto it = std::prev(m_floatingBoxes->end());
    if(it->isPlaced())
        return;

    auto y = height();
    auto begin = m_floatingBoxes->begin();
    for(; it != begin; --it) {
        if(it->isPlaced()) {
            y = std::max(y, it->y());
            ++it;
            break;
        }
    }

    auto end = m_floatingBoxes->end();
    for(; it != end; ++it) {
        auto box = it->box();
        if(this != box->containingBlock())
            continue;
        if(box->style()->isClearLeft())
            y = std::max(y, leftFloatBottom());
        if(box->style()->isClearRight())
            y = std::max(y, rightFloatBottom());

        it->setIsPlaced(true);
    }
}

bool BlockFlowBox::containsFloat(Box* box) const
{
    if(!m_floatingBoxes)
        return false;
    auto it = m_floatingBoxes->begin();
    while(it != m_floatingBoxes->end()) {
        if(box == it->box())
            return true;
        ++it;
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
    floatingBox.setIsPlaced(false);
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

    BlockBox::addBox(box);
}

} // namespace htmlbook
