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
    maxWidth = std::max(maxWidth, minWidth);
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
    MarginInfo(const BlockFlowBox& block, float top, float bottom);

    bool canCollapseWithChildren() const { return m_canCollapseWithChildren; }
    bool canCollapseMarginTopWithChildren() const { return m_canCollapseMarginTopWithChildren; }
    bool canCollapseMarginBottomWithChildren() const { return m_canCollapseMarginBottomWithChildren; }

    bool canCollapseWithMarginTop() const { return m_atTopOfBlock && m_canCollapseMarginTopWithChildren; }
    bool canCollapseWithMarginBottom() const { return m_atBottomOfBlock && m_canCollapseMarginBottomWithChildren; }

    bool atTopOfBlock() const { return m_atTopOfBlock; }
    bool atBottomOfBlock() const { return m_atBottomOfBlock; }

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

inline MarginInfo::MarginInfo(const BlockFlowBox& block, float top, float bottom)
    : m_atTopOfBlock(true)
    , m_atBottomOfBlock(false)
{
    m_canCollapseWithChildren = !block.isRootBox() && !block.isReplaced() && !block.isInline() && !block.isFloating() && !block.isPositioned() && !block.isOverflowHidden() && !is<TableCellBox>(block);
    m_canCollapseMarginTopWithChildren = m_canCollapseWithChildren && top == 0;
    m_canCollapseMarginBottomWithChildren = m_canCollapseWithChildren && bottom == 0;

    m_positiveMargin = m_canCollapseMarginTopWithChildren ? block.maxPositiveMarginTop() : 0.f;
    m_negativeMargin = m_canCollapseMarginTopWithChildren ? block.maxNegativeMarginTop() : 0.f;
}

void BlockFlowBox::adjustPositionedBox(BoxFrame* child, const MarginInfo& marginInfo)
{
    auto staticTop = height();
    if(!marginInfo.canCollapseWithMarginTop())
        staticTop -= marginInfo.margin();

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
    setCollapsedBottomMargin(marginInfo);
}

void BlockFlowBox::setCollapsedBottomMargin(const MarginInfo& marginInfo)
{
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
}

void BlockFlowBox::layoutBlockChildren()
{
    auto top = borderTop() + paddingTop();
    auto bottom = borderBottom() + paddingBottom();
    setHeight(top);

    MarginInfo marginInfo(*this, top, bottom);
    for(auto box = firstBox(); box; box = box->nextBox()) {
        auto child = to<BoxFrame>(box);
        if(child->isPositioned()) {
            child->containingBlock()->insertPositonedBox(child);
            adjustPositionedBox(child, marginInfo);
        } else if(box->isFloating()) {
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
    auto xOffset = parentBlock->leftOffsetForContent();
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

    auto floatTop = height();
    auto begin = m_floatingBoxes->begin();
    for(; it != begin; --it) {
        if(it->isPlaced()) {
            floatTop = std::max(floatTop, it->y());
            ++it;
            break;
        }
    }

    auto end = m_floatingBoxes->end();
    for(; it != end; ++it) {
        auto& floatingBox = *it;
        auto child = floatingBox.box();
        if(this != child->containingBlock())
            continue;
        if(child->style()->isClearLeft())
            floatTop = std::max(floatTop, leftFloatBottom());
        if(child->style()->isClearRight())
            floatTop = std::max(floatTop, rightFloatBottom());

        auto leftOffset = leftOffsetForContent();
        auto rightOffset = rightOffsetForContent();
        auto floatWidth = std::min(rightOffset - leftOffset, floatingBox.width());

        float floatLeft = 0;
        if(child->style()->floating() == Float::Left) {
            float heightRemainingLeft = 1;
            float heightRemainingRight = 1;
            floatLeft = leftOffsetForFloat(floatTop, leftOffset, false, &heightRemainingLeft);
            while(rightOffsetForFloat(floatTop, rightOffset, false, &heightRemainingRight) - floatLeft < floatWidth) {
                floatTop += std::min(heightRemainingLeft, heightRemainingRight);
                floatLeft = leftOffsetForFloat(floatTop, leftOffset, false, &heightRemainingLeft);
            }

            floatLeft = std::max(floatLeft, leftOffset - borderLeft() + paddingLeft());
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

float BlockFlowBox::leftOffsetForFloat(float y, float leftOffset, bool applyTextIndent, float* heightRemaining) const
{
    if(heightRemaining) *heightRemaining = 1;
    if(m_floatingBoxes) {
        for(auto& item : *m_floatingBoxes) {
            if(item.type() != Float::Left || !item.isPlaced())
                continue;
            if(item.y() <= y && item.bottom() > y && item.right() > leftOffset) {
                if(heightRemaining) *heightRemaining = item.bottom() - y;
                leftOffset = item.right();
            }
        }
    }

    if(applyTextIndent && style()->isLeftToRightDirection()) {
        float availableWidth = 0;
        auto textIndentLength = style()->textIndent();
        if(textIndentLength.isPercent())
            availableWidth = containingBlock()->availableWidth();
        leftOffset += textIndentLength.calcMin(availableWidth);
    }

    return leftOffset;
}

float BlockFlowBox::rightOffsetForFloat(float y, float rightOffset, bool applyTextIndent, float* heightRemaining) const
{
    if(heightRemaining) *heightRemaining = 1;
    if(m_floatingBoxes) {
        for(auto& item : *m_floatingBoxes) {
            if(item.type() != Float::Right || !item.isPlaced())
                continue;
            if(item.y() <= y && item.bottom() > y && item.x() < rightOffset) {
                if(heightRemaining) *heightRemaining = item.bottom() - y;
                rightOffset = item.x();
            }
        }
    }

    if(applyTextIndent && !style()->isLeftToRightDirection()) {
        float availableWidth = 0;
        auto textIndentLength = style()->textIndent();
        if(textIndentLength.isPercent())
            availableWidth = containingBlock()->availableWidth();
        rightOffset -= textIndentLength.calcMin(availableWidth);
    }

    return rightOffset;
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

    box->layout();

    FloatingBox floatingBox(box);
    floatingBox.setIsIntruding(false);
    floatingBox.setIsHidden(false);
    floatingBox.setIsPlaced(false);
    floatingBox.setWidth(box->width() + box->marginLeft() + box->marginTop());
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
