#include "box.h"
#include "document.h"
#include "flexiblebox.h"
#include "listitembox.h"
#include "replacedbox.h"
#include "tablebox.h"

namespace htmlbook {

Box::Box(Node* node, const RefPtr<BoxStyle>& style)
    : m_node(node), m_style(style)
{
    if(node) {
        node->setBox(this);
    }
}

Box::~Box()
{
    auto box = m_firstBox;
    while(box) {
        auto nextBox = box->nextBox();
        box->setParentBox(nullptr);
        box->setPrevBox(nullptr);
        box->setNextBox(nullptr);
        delete box;
        box = nextBox;
    }

    if(m_parentBox)
        m_parentBox->removeChild(this);
    if(m_node) {
        m_node->setBox(nullptr);
    }
}

void Box::buildBox(BoxLayer* layer)
{
    auto child = m_firstBox;
    while(child) {
        child->buildBox(layer);
        child = child->nextBox();
    }
}

void Box::layout()
{
    assert(false);
}

void Box::addBox(Box* box)
{
    appendChild(box);
}

void Box::insertChild(Box* box, Box* nextBox)
{
    if(nextBox == nullptr) {
        appendChild(box);
        return;
    }

    assert(nextBox->parentBox() == this);
    assert(box->parentBox() == nullptr);
    assert(box->prevBox() == nullptr);
    assert(box->nextBox() == nullptr);

    auto prevBox = nextBox->prevBox();
    nextBox->setPrevBox(box);
    assert(m_lastBox != prevBox);
    if(prevBox == nullptr) {
        assert(m_firstBox == nextBox);
        m_firstBox = box;
    } else {
        assert(m_firstBox != nextBox);
        prevBox->setNextBox(box);
    }

    box->setParentBox(this);
    box->setPrevBox(prevBox);
    box->setNextBox(nextBox);
}

void Box::appendChild(Box* box)
{
    assert(box->parentBox() == nullptr);
    assert(box->prevBox() == nullptr);
    assert(box->nextBox() == nullptr);
    box->setParentBox(this);
    if(m_firstBox == nullptr) {
        m_firstBox = m_lastBox = box;
        return;
    }

    box->setPrevBox(m_lastBox);
    m_lastBox->setNextBox(box);
    m_lastBox = box;
}

void Box::removeChild(Box* box)
{
    assert(box->parentBox() == this);
    auto nextBox = box->nextBox();
    auto prevBox = box->prevBox();
    if(nextBox)
        nextBox->setPrevBox(prevBox);
    if(prevBox)
        prevBox->setNextBox(nextBox);

    if(m_firstBox == box)
        m_firstBox = nextBox;
    if(m_lastBox == box)
        m_lastBox = prevBox;

    box->setParentBox(nullptr);
    box->setPrevBox(nullptr);
    box->setNextBox(nullptr);
}

void Box::moveChildrenTo(Box* to, Box* begin, Box* end)
{
    auto child = begin;
    while(child && child != end) {
        auto nextChild = child->nextBox();
        removeChild(child);
        to->appendChild(child);
        child = nextChild;
    }
}

void Box::moveChildrenTo(Box* to, Box* begin)
{
    moveChildrenTo(to, begin, nullptr);
}

void Box::moveChildrenTo(Box* to)
{
    moveChildrenTo(to, m_firstBox, nullptr);
}

Box* Box::create(Node* node, const RefPtr<BoxStyle>& style)
{
    if(style->pseudoType() == PseudoType::Marker) {
        if(style->listStylePosition() == ListStylePosition::Inside)
            return new (style->heap()) InsideListMarkerBox(style);
        return new (style->heap()) OutsideListMarkerBox(style);
    }

    switch(style->display()) {
    case Display::Inline:
        return new (style->heap()) InlineBox(node, style);
    case Display::Block:
    case Display::InlineBlock:
        return new (style->heap()) BlockFlowBox(node, style);
    case Display::Flex:
    case Display::InlineFlex:
        return new (style->heap()) FlexibleBox(node, style);
    case Display::Table:
    case Display::InlineTable:
        return new (style->heap()) TableBox(node, style);
    case Display::ListItem:
        return new (style->heap()) ListItemBox(node, style);
    case Display::TableCell:
        return new (style->heap()) TableCellBox(node, style);
    case Display::TableColumn:
        return new (style->heap()) TableColumnBox(node, style);
    case Display::TableColumnGroup:
        return new (style->heap()) TableColumnGroupBox(node, style);
    case Display::TableRow:
        return new (style->heap()) TableRowBox(node, style);
    case Display::TableRowGroup:
    case Display::TableHeaderGroup:
    case Display::TableFooterGroup:
        return new (style->heap()) TableSectionBox(node, style);
    case Display::TableCaption:
        return new (style->heap()) TableCaptionBox(node, style);
    default:
        assert(false);
    }

    return nullptr;
}

Box* Box::createAnonymous(const RefPtr<BoxStyle>& parentStyle, Display display)
{
    auto newBox = create(nullptr, BoxStyle::create(parentStyle, display));
    newBox->setAnonymous(true);
    return newBox;
}

BlockFlowBox* Box::createAnonymousBlock(const RefPtr<BoxStyle>& parentStyle)
{
    auto newStyle = BoxStyle::create(parentStyle, Display::Block);
    auto newBlock = new (newStyle->heap()) BlockFlowBox(nullptr, newStyle);
    newBlock->setAnonymous(true);
    return newBlock;
}

BoxModel* Box::containingBox() const
{
    auto parent = parentBox();
    if(!isTextBox()) {
        if(position() == Position::Fixed)
            return containingBlockFixed();
        if(position() == Position::Absolute) {
            while(parent && parent->position() == Position::Static) {
                if(parent->isBoxView() || (parent->hasTransform() && parent->isBlockBox()))
                    break;
                parent = parent->parentBox();
            }
        }
    }

    return to<BoxModel>(parent);
}

BlockBox* Box::containingBlock() const
{
    if(!isTextBox()) {
        if(position() == Position::Fixed)
            return containingBlockFixed();
        if(position() == Position::Absolute) {
            return containingBlockAbsolute();
        }
    }

    auto parent = parentBox();
    while(parent && ((parent->isInline() && !parent->isReplaced()) || !parent->isBlockBox()))
        parent = parent->parentBox();
    return to<BlockBox>(parent);
}

BlockBox* Box::containingBlockFixed() const
{
    auto parent = parentBox();
    while(parent && !(parent->isBoxView() || (parent->hasTransform() && parent->isBlockBox())))
        parent = parent->parentBox();
    return to<BlockBox>(parent);
}

BlockBox* Box::containingBlockAbsolute() const
{
    auto parent = parentBox();
    while(parent && parent->position() == Position::Static) {
        if(parent->isBoxView() || (parent->hasTransform() && parent->isBlockBox()))
            break;
        parent = parent->parentBox();
    }

    if(parent && !parent->isBlockBox())
        parent = parent->containingBox();
    while(parent && parent->isAnonymous())
        parent = parent->containingBox();
    return to<BlockBox>(parent);
}

bool Box::isBody() const
{
    return m_node && m_node->tagName() == bodyTag;
}

bool Box::isRootBox() const
{
    return m_node && m_node->isRootNode();
}

bool Box::isFlexItem() const
{
    return m_parentBox && m_parentBox->isFlexibleBox();
}

std::unique_ptr<BoxLayer> BoxLayer::create(BoxModel* box, BoxLayer* parent)
{
    return std::unique_ptr<BoxLayer>(new (box->heap()) BoxLayer(box, parent));
}

BoxLayer::BoxLayer(BoxModel* box, BoxLayer* parent)
    : m_box(box), m_parent(parent), m_children(box->heap())
{
    m_index = box->style()->zIndex().value_or(0);
    if(parent == nullptr)
        return;
    auto compare = [](auto a, auto b) { return a->index() < b->index(); };
    auto it = std::upper_bound(parent->m_children.begin(), parent->m_children.end(), this, compare);
    parent->m_children.insert(it, this);
}

BoxModel::BoxModel(Node* node, const RefPtr<BoxStyle>& style)
    : Box(node, style)
{
    switch(style->display()) {
    case Display::Inline:
    case Display::InlineBlock:
    case Display::InlineFlex:
    case Display::InlineTable:
        setInline(true);
        break;
    default:
        setInline(false);
        break;
    }
}

void BoxModel::buildBox(BoxLayer* layer)
{
    if(layer == nullptr || requiresLayer()) {
        m_layer = BoxLayer::create(this, layer);
        layer = m_layer.get();
    }

    Box::buildBox(layer);
}

void BoxModel::addBox(Box* box)
{
    if(!box->isTableCellBox() && !box->isTableRowBox()
        && !box->isTableCaptionBox() && !box->isTableColumnBox()
        && !box->isTableSectionBox()) {
        appendChild(box);
        return;
    }

    auto lastChild = lastBox();
    if(lastChild && lastChild->isAnonymous() && lastChild->isTableBox()) {
        lastChild->addBox(box);
        return;
    }

    auto newTable = createAnonymous(style(), Display::Table);
    appendChild(newTable);
    newTable->addBox(box);
}

void BoxModel::computeBorderWidths(float& top, float& bottom, float& left, float& right) const
{
    switch(style()->borderTopStyle()) {
    case LineStyle::None:
    case LineStyle::Hidden:
        top = 0.f;
        break;
    default:
        top = style()->borderTopWidth();
        break;
    }

    switch(style()->borderBottomStyle()) {
    case LineStyle::None:
    case LineStyle::Hidden:
        bottom = 0.f;
        break;
    default:
        bottom = style()->borderBottomWidth();
        break;
    }

    switch(style()->borderLeftStyle()) {
    case LineStyle::None:
    case LineStyle::Hidden:
        left = 0.f;
        break;
    default:
        left = style()->borderLeftWidth();
        break;
    }

    switch(style()->borderRightStyle()) {
    case LineStyle::None:
    case LineStyle::Hidden:
        right = 0.f;
        break;
    default:
        right = style()->borderRightWidth();
        break;
    }
}

void BoxModel::computePaddingWidths(float& top, float& bottom, float& left, float& right) const
{
    auto compute = [this](const auto& padding) {
        float containerWidth = 0;
        if(padding.isPercent())
            containerWidth = containingBlockWidthForContent();
        return padding.calcMin(containerWidth);
    };

    top = compute(style()->paddingTop());
    bottom = compute(style()->paddingBottom());
    left = compute(style()->paddingLeft());
    right = compute(style()->paddingRight());
}

void BoxModel::updateBorderWidths() const
{
    computeBorderWidths(m_borderTop, m_borderBottom, m_borderLeft, m_borderRight);
}

void BoxModel::updatePaddingWidths() const
{
    computePaddingWidths(m_paddingTop, m_paddingBottom, m_paddingLeft, m_paddingRight);
}

float BoxModel::containingBlockWidthForContent() const
{
    return containingBlock()->availableWidth();
}

float BoxModel::containingBlockHeightForContent() const
{
    return containingBlock()->availableHeight();
}

float BoxModel::borderTop() const
{
    if(m_borderTop < 0)
        updateBorderWidths();
    return m_borderTop;
}

float BoxModel::borderBottom() const
{
    if(m_borderBottom < 0)
        updateBorderWidths();
    return m_borderBottom;
}

float BoxModel::borderLeft() const
{
    if(m_borderLeft < 0)
        updateBorderWidths();
    return m_borderLeft;
}

float BoxModel::borderRight() const
{
    if(m_borderRight < 0)
        updateBorderWidths();
    return m_borderRight;
}

float BoxModel::paddingTop() const
{
    if(m_paddingTop < 0)
        updatePaddingWidths();
    return m_paddingTop;
}

float BoxModel::paddingBottom() const
{
    if(m_paddingBottom < 0)
        updatePaddingWidths();
    return m_paddingBottom;
}

float BoxModel::paddingLeft() const
{
    if(m_paddingLeft < 0)
        updatePaddingWidths();
    return m_paddingLeft;
}

float BoxModel::paddingRight() const
{
    if(m_paddingRight < 0)
        updatePaddingWidths();
    return m_paddingRight;
}

BoxFrame::BoxFrame(Node* node, const RefPtr<BoxStyle>& style)
    : BoxModel(node, style)
{
    setHasTransform(style->hasTransform());
    switch(style->position()) {
    case Position::Static:
    case Position::Relative:
        setPositioned(false);
        break;
    default:
        setPositioned(true);
        break;
    }

    switch(style->floating()) {
    case Float::None:
        setFloating(false);
        break;
    default:
        setFloating(true);
        break;
    }
}

void BoxFrame::computePreferredWidths(float& minWidth, float& maxWidth) const
{
    minWidth = 0;
    maxWidth = 0;
}

void BoxFrame::updatePreferredWidths() const
{
    computePreferredWidths(m_minPreferredWidth, m_maxPreferredWidth);
}

float BoxFrame::minPreferredWidth() const
{
    if(m_minPreferredWidth < 0)
        updatePreferredWidths();
    return m_minPreferredWidth;
}

float BoxFrame::maxPreferredWidth() const
{
    if(m_maxPreferredWidth < 0)
        updatePreferredWidths();
    return m_maxPreferredWidth;
}

float BoxFrame::intrinsicWidth() const
{
    if(auto box = to<ReplacedBox>(this))
        return box->intrinsicWidth();
    return 0;
}

float BoxFrame::intrinsicHeight() const
{
    if(auto box = to<ReplacedBox>(this))
        return box->intrinsicHeight();
    return 0;
}

float BoxFrame::availableHeightUsing(const Length& height) const
{
    if(height.isFixed())
        return adjustContentBoxHeight(height.value());

    if(isBoxView())
        return style()->viewportHeight();

    if(isTableCellBox() && (height.isAuto() || height.isPercent())) {
        if(hasOverrideHeight())
            return overrideHeight();
        return m_height - borderPaddingHeight();
    }

    if(height.isPercent()) {
        auto availableHeight = containingBlockHeightForContent();
        return adjustContentBoxHeight(height.calc(availableHeight));
    }

    if(isPositioned() && isBlockBox() && style()->height().isAuto()
        && !(style()->top().isAuto() || style()->bottom().isAuto())) {
        auto& block = to<BlockBox>(*this);
        float y = 0;
        float computedHeight = block.height();
        float marginTop = 0;
        float marginBottom = 0;
        block.computeHeight(y, computedHeight, marginTop, marginBottom);
        return block.adjustContentBoxHeight(computedHeight - block.borderPaddingHeight());
    }

    return containingBlockHeightForContent();
}

float BoxFrame::containingBlockWidthForPositioned(const BoxModel* containingBox) const
{
    if(auto box = to<BoxFrame>(containingBox))
        return box->width() - box->borderLeft() - box->borderRight();

    auto& lines = to<InlineBox>(*containingBox).lines();
    if(lines.empty())
        return 0;

    auto& firstLine = *lines.front();
    auto& lastLine = *lines.back();

    float fromLeft = 0;
    float fromRight = 0;
    if(containingBox->style()->isLeftToRightDirection()) {
        fromLeft = firstLine.x() + firstLine.borderLeft();
        fromRight = lastLine.x() + lastLine.width() - lastLine.borderRight();
    } else {
        fromRight = firstLine.x() + firstLine.width() - firstLine.borderRight();
        fromLeft = lastLine.x() + lastLine.borderLeft();
    }

    return std::max(0.f, fromRight - fromLeft);
}

float BoxFrame::containingBlockHeightForPositioned(const BoxModel* containingBox) const
{
    if(auto box = to<BoxFrame>(containingBox))
        return box->height() - box->borderTop() - box->borderBottom();

    auto& lines = to<InlineBox>(*containingBox).lines();
    if(lines.empty())
        return 0;

    auto& firstLine = *lines.front();
    auto& lastLine = *lines.back();
    auto lineHeight = lastLine.y() + lastLine.height() - firstLine.y();
    return lineHeight - containingBox->borderTop() - containingBox->borderBottom();
}

bool BoxFrame::shrinkToAvoidFloats() const
{
    if(isInline() || isFloating() || !avoidsFloats())
        return false;
    return style()->width().isAuto();
}

float BoxFrame::shrinkWidthToAvoidFloats(float marginLeft, float marginRight, const BlockFlowBox* container) const
{
    auto availableWidth = container->availableWidthForLine(m_y, false) - marginLeft - marginRight;
    if(marginLeft > 0) {
        auto lineStartOffset = container->startOffsetForLine(m_y, false);
        auto contentStartOffset = container->startOffsetForContent();
        auto marginStartOffset = contentStartOffset + marginLeft;
        if(lineStartOffset > marginStartOffset) {
            availableWidth += marginLeft;
        } else {
            availableWidth += lineStartOffset - contentStartOffset;
        }
    }

    if(marginRight > 0) {
        auto lineEndOffset = container->endOffsetForLine(m_y, false);
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

bool BoxFrame::adjustToFitContent() const
{
    if(isFloating() || (isReplaced() && isInline() && isBlockBox()))
        return true;
    if(!isFlexItem())
        return false;
    auto parentStyle = parentBox()->style();
    if(!parentStyle->isColumnFlexDirection() || parentStyle->flexWrap() != FlexWrap::Nowrap)
        return true;
    if(style()->marginLeft().isAuto() || style()->marginRight().isAuto())
        return true;
    return !(style()->alignSelf() == AlignItems::Stretch || (style()->alignSelf() == AlignItems::Auto && parentStyle->alignItems() == AlignItems::Stretch));
}

float BoxFrame::adjustWidthToFitContent(float width) const
{
    if(adjustToFitContent()) {
        width = std::max(width, minPreferredWidth());
        width = std::min(width, maxPreferredWidth());
    }

    return width;
}

float BoxFrame::adjustBorderBoxWidth(float width) const
{
    if(style()->boxSizing() == BoxSizing::ContentBox)
        return width + borderPaddingWidth();
    return std::max(width, borderPaddingWidth());
}

float BoxFrame::adjustBorderBoxHeight(float height) const
{
    if(style()->boxSizing() == BoxSizing::ContentBox)
        return height + borderPaddingHeight();
    return std::max(height, borderPaddingHeight());
}

float BoxFrame::adjustContentBoxWidth(float width) const
{
    if(style()->boxSizing() == BoxSizing::BorderBox)
        width -= borderPaddingWidth();
    return std::max(0.f, width);
}

float BoxFrame::adjustContentBoxHeight(float height) const
{
    if(style()->boxSizing() == BoxSizing::BorderBox)
        height -= borderPaddingHeight();
    return std::max(0.f, height);
}

float BoxFrame::computeReplacedWidthUsing(const Length& width) const
{
    switch(width.type()) {
    case Length::Type::Fixed:
        return adjustContentBoxWidth(width.value());
    case Length::Type::Percent:
        computePercentageReplacedWidth(width);
    default:
        return intrinsicWidth();
    }
}

float BoxFrame::computeReplacedHeightUsing(const Length& height) const
{
    switch(height.type()) {
    case Length::Type::Fixed:
        return adjustContentBoxHeight(height.value());
    case Length::Type::Percent:
        return computePercentageReplacedHeight(height);
    default:
        return intrinsicHeight();
    }
}

float BoxFrame::computePercentageReplacedWidth(const Length& width) const
{
    auto containerWidth = isPositioned() ? containingBlockWidthForPositioned(containingBox()) : containingBlockWidthForContent();
    if(containerWidth > 0)
        return adjustContentBoxWidth(width.calcMin(containerWidth));
    return intrinsicWidth();
}

float BoxFrame::computePercentageReplacedHeight(const Length& height) const
{
    auto cb = isPositioned() ? containingBox() : containingBlock();
    while(cb->isAnonymous())
        cb = cb->containingBlock();
    if(cb->isPositioned() && cb->style()->height().isAuto()
        && !(cb->style()->top().isAuto() || cb->style()->bottom().isAuto())) {
        auto& block = to<BlockBox>(*cb);
        float y = 0;
        float computedHeight = block.height();
        float marginTop = 0;
        float marginBottom = 0;
        block.computeHeight(y, computedHeight, marginTop, marginBottom);
        auto availableHeight = block.adjustContentBoxHeight(computedHeight - block.borderPaddingHeight());
        return adjustContentBoxHeight(height.calc(availableHeight));
    }

    if(isPositioned()) {
        auto availableHeight = containingBlockHeightForPositioned(cb);
        return adjustContentBoxHeight(height.calc(availableHeight));
    }

    while(cb && !cb->isBoxView() && (cb->style()->height().isAuto() || cb->style()->height().isPercent())) {
        if(cb->isTableCellBox()) {
            auto availableHeight = std::max(intrinsicHeight(), containingBlockHeightForContent());
            return height.calc(availableHeight - borderPaddingHeight());
        }

        cb = cb->containingBlock();
    }

    auto availableHeight = containingBlockHeightForContent();
    return adjustContentBoxHeight(height.calc(availableHeight));
}

float BoxFrame::computeReplacedWidth() const
{
    auto width = computeReplacedWidthUsing(style()->width());
    auto minWidth = computeReplacedWidthUsing(style()->minWidth());
    auto maxWidth = style()->maxWidth().isNone() ? width : computeReplacedWidthUsing(style()->maxWidth());
    return std::max(minWidth, std::min(width, maxWidth));
}

float BoxFrame::computeReplacedHeight() const
{
    auto height = computeReplacedHeightUsing(style()->height());
    auto minHeight = computeReplacedHeightUsing(style()->minHeight());
    auto maxHeight = style()->maxHeight().isNone() ? height : computeReplacedHeightUsing(style()->maxHeight());
    return std::max(minHeight, std::min(height, maxHeight));
}

void BoxFrame::computeHorizontalMargins(float& marginLeft, float& marginRight, float childWidth, const BlockBox* container, float containerWidth) const
{
    auto marginLeftLength = style()->marginLeft();
    auto marginRightLength = style()->marginRight();
    if(isInline() || isFloating()) {
        marginLeft = marginLeftLength.calcMin(containerWidth);
        marginRight = marginRightLength.calcMin(containerWidth);
        return;
    }

    auto containerStyle = container->style();
    auto containerTextAlign = containerStyle->textAlign();
    auto containerDirection = containerStyle->direction();
    if(marginLeftLength.isAuto() && marginRightLength.isAuto() && childWidth < containerWidth
        || (!marginLeftLength.isAuto() && !marginRightLength.isAuto() && containerTextAlign == TextAlign::Center)) {
        marginLeft = std::max(0.f, (containerWidth - childWidth) / 2.f);
        marginRight = containerWidth - childWidth - marginLeft;
        return;
    }

    if(marginRightLength.isAuto() && childWidth < containerWidth
        || (!marginLeftLength.isAuto() && containerDirection == TextDirection::Rtl && containerTextAlign == TextAlign::Left)) {
        marginLeft = marginLeftLength.calc(containerWidth);
        marginRight = containerWidth - childWidth - marginLeft;
        return;
    }

    if(marginLeftLength.isAuto() && childWidth < containerWidth
        || (!marginRightLength.isAuto() && containerDirection == TextDirection::Ltr && containerTextAlign == TextAlign::Right)) {
        marginRight = marginRightLength.calc(containerWidth);
        marginLeft = containerWidth - childWidth - marginRight;
        return;
    }

    marginLeft = marginLeftLength.calcMin(containerWidth);
    marginRight = marginRightLength.calcMin(containerWidth);
}

void BoxFrame::computeVerticalMargins(float& marginTop, float& marginBottom) const
{
    if(isTableCellBox()) {
        marginTop = 0;
        marginBottom = 0;
        return;
    }

    auto containerWidth = containingBlockWidthForContent();
    marginTop = style()->marginTop().calcMin(containerWidth);
    marginBottom = style()->marginBottom().calcMin(containerWidth);
}

float BoxFrame::computeWidthUsing(const Length& width, const BlockBox* container, float containerWidth) const
{
    if(!width.isAuto())
        return adjustBorderBoxWidth(width.calc(containerWidth));
    auto marginLeft = style()->marginLeft().calcMin(containerWidth);
    auto marginRight = style()->marginRight().calcMin(containerWidth);
    auto computedWidth = containerWidth - marginLeft - marginRight;
    auto containerBlockFlow = to<BlockFlowBox>(container);
    if(containerBlockFlow && containerBlockFlow->containsFloats() && shrinkToAvoidFloats())
        computedWidth = std::min(computedWidth, shrinkWidthToAvoidFloats(marginLeft, marginRight, containerBlockFlow));
    return adjustWidthToFitContent(computedWidth);
}

std::optional<float> BoxFrame::computeHeightUsing(const Length& height) const
{
    switch(height.type()) {
    case Length::Type::Fixed:
        return height.value();
    case Length::Type::Percent:
        return computePercentageHeight(height);
    default:
        return std::nullopt;
    }
}

std::optional<float> BoxFrame::computePercentageHeight(const Length& height) const
{
    if(isTableCellBox()) {
        if(!hasOverrideHeight())
            return std::nullopt;
        auto computedHeight = height.calc(overrideHeight());
        computedHeight -= borderPaddingHeight();
        return std::max(0.f, computedHeight);
    }

    auto container = containingBlock();
    auto containerStyle = container->style();
    auto containerStyleHeight = containerStyle->height();
    auto containerStyleTop = containerStyle->top();
    auto containerStyleBottom = containerStyle->bottom();

    float availableHeight = 0;
    if(containerStyleHeight.isFixed()) {
        availableHeight = container->adjustContentBoxHeight(containerStyleHeight.value());
    } else if(container->isPositioned() && (!containerStyleHeight.isAuto() || (!containerStyleTop.isAuto() && !containerStyleBottom.isAuto()))) {
        float y = 0;
        float computedHeight = container->height();
        float marginTop = 0;
        float marginBottom = 0;
        container->computeHeight(y, computedHeight, marginTop, marginBottom);
        availableHeight = computedHeight - container->borderPaddingHeight();
    } else if(containerStyleHeight.isPercent()) {
        auto computedHeight = container->computePercentageHeight(containerStyleHeight);
        if(!computedHeight)
            return std::nullopt;
        availableHeight = container->adjustContentBoxHeight(*computedHeight);
    } else if(container->isBoxView()) {
        availableHeight = containerStyle->viewportHeight();
    } else {
        return std::nullopt;
    }

    if(isTableBox()) {
        if(isPositioned())
            availableHeight += paddingHeight();
        auto computedHeight = height.calc(availableHeight);
        computedHeight -= borderPaddingHeight();
        return std::max(0.f, computedHeight);
    }

    return height.calc(availableHeight);
}

float BoxFrame::constrainWidthByMinMax(float width, const BlockBox* container, float containerWidth) const
{
    auto minWidthLength = style()->minWidth();
    auto maxWidthLength = style()->maxWidth();
    if(!maxWidthLength.isNone())
        width = std::min(width, computeWidthUsing(maxWidthLength, container, containerWidth));
    return std::max(width, adjustBorderBoxWidth(minWidthLength.calcMin(containerWidth)));
}

float BoxFrame::constrainHeightByMinMax(float height) const
{
    if(auto maxHeight = computeHeightUsing(style()->maxHeight()))
        height = std::min(height, *maxHeight);
    if(auto minHeight = computeHeightUsing(style()->minHeight()))
        height = std::max(height, *minHeight);
    return height;
}

static float computePositionedLeftOffset(float left, float marginLeft, const BoxModel* container, TextDirection containerDirection)
{
    if(containerDirection == TextDirection::Rtl && container->isInlineBox()) {
        auto& lines = to<InlineBox>(*container).lines();
        if(lines.size() > 1) {
            auto& firstLine = *lines.front();
            auto& lastLine = *lines.back();
            return left + marginLeft + lastLine.borderLeft() + (lastLine.x() - firstLine.x());
        }
    }

    return left + marginLeft + container->borderLeft();
}

void BoxFrame::computePositionedWidthUsing(const Length& widthLength, const BoxModel* container, TextDirection containerDirection, float containerWidth,
    const Length& leftLength, const Length& rightLength, const Length& marginLeftLength, const Length& marginRightLength,
    float& x, float& width, float& marginLeft, float& marginRight) const
{
    auto widthLenghtIsAuto = widthLength.isAuto();
    auto leftLenghtIsAuto = leftLength.isAuto();
    auto rightLenghtIsAuto = rightLength.isAuto();

    float leftLengthValue = 0;
    if(!widthLenghtIsAuto && !leftLenghtIsAuto && !rightLenghtIsAuto) {
        width = adjustContentBoxWidth(widthLength.calc(containerWidth));
        leftLengthValue = leftLength.calc(containerWidth);

        auto availableSpace = containerWidth - (leftLengthValue + width + borderPaddingWidth() + rightLength.calc(containerWidth));
        if(marginLeftLength.isAuto() && marginRightLength.isAuto()) {
            if(availableSpace >= 0) {
                marginLeft = availableSpace / 2.f;
                marginRight = availableSpace - marginLeft;
            } else {
                if(containerDirection == TextDirection::Ltr) {
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
            if(containerDirection == TextDirection::Rtl) {
                leftLengthValue = (availableSpace + leftLengthValue) - marginLeft - marginRight;
            }
        }
    } else {
        marginLeft = marginLeftLength.calcMin(containerWidth);
        marginRight = marginRightLength.calcMin(containerWidth);

        auto availableSpace = containerWidth - (marginLeft + marginRight + borderPaddingWidth());
        if(leftLenghtIsAuto && widthLenghtIsAuto && !rightLenghtIsAuto) {
            auto rightLengthValue = rightLength.calc(containerWidth);
            auto preferredWidth = maxPreferredWidth() - borderPaddingWidth();
            auto preferredMinWidth = minPreferredWidth() - borderPaddingWidth();
            auto availableWidth = availableSpace - rightLengthValue;
            width = std::min(preferredWidth, std::max(preferredMinWidth, availableWidth));
            leftLengthValue = availableSpace - (width + rightLengthValue);
        } else if(!leftLenghtIsAuto && widthLenghtIsAuto && rightLenghtIsAuto) {
            auto leftLengthValue = leftLength.calc(containerWidth);
            auto preferredWidth = maxPreferredWidth() - borderPaddingWidth();
            auto preferredMinWidth = minPreferredWidth() - borderPaddingWidth();
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

    x = computePositionedLeftOffset(leftLengthValue, marginLeft, container, containerDirection);
}

void BoxFrame::computePositionedWidthReplaced(float& x, float& width, float& marginLeft, float& marginRight) const
{
    auto container = containingBox();
    auto containerWidth = containingBlockWidthForPositioned(container);
    auto containerDirection = container->style()->direction();

    auto marginLeftLength = style()->marginLeft();
    auto marginRightLength = style()->marginRight();

    auto leftLength = style()->left();
    auto rightLength = style()->right();

    width = computeReplacedWidth() + borderPaddingWidth();
    auto availableSpace = containerWidth - width;
    if(leftLength.isAuto() && rightLength.isAuto()) {
        if(containerDirection == TextDirection::Ltr) {
            auto staticPosition = layer()->staticLeft() - container->borderLeft();
            for(auto parent = parentBox(); parent && parent != container; parent = parent->parentBox()) {
                if(auto box = to<BoxFrame>(parent)) {
                    staticPosition += box->x();
                }
            }

            leftLength = Length{Length::Type::Fixed, staticPosition};
        } else {
            auto staticPosition = layer()->staticLeft() + containerWidth + container->borderRight();
            for(auto parent = parentBox(); parent && parent != container; parent = parent->parentBox()) {
                if(auto box = to<BoxFrame>(parent)) {
                    staticPosition += box->x();
                }
            }

            rightLength = Length{Length::Type::Fixed, staticPosition};
        }
    }

    if(leftLength.isAuto() || rightLength.isAuto()) {
        if(marginLeftLength.isAuto()) 
            marginLeftLength = Length::ZeroFixed;
        if(marginRightLength.isAuto()) {
            marginRightLength = Length::ZeroFixed;
        }
    }

    float leftLengthValue = 0;
    float rightLengthValue = 0;
    if(marginLeftLength.isAuto() && marginRightLength.isAuto()) {
        leftLengthValue = leftLength.calc(containerWidth);
        rightLengthValue = rightLength.calc(containerWidth);

        auto availableWidth = availableSpace - (leftLengthValue + rightLengthValue);
        if(availableWidth > 0) {
            marginLeft = availableWidth / 2.f;
            marginRight = availableWidth - marginLeft;
        } else {
            if(containerDirection == TextDirection::Ltr) {
                marginLeft = 0;
                marginRight = availableWidth;
            } else {
                marginLeft = availableWidth;
                marginRight = 0;
            }
        }
    } else if(leftLength.isAuto()) {
        marginLeft = marginLeftLength.calc(containerWidth);
        marginRight = marginRightLength.calc(containerWidth);
        rightLengthValue = rightLength.calc(containerWidth);

        leftLengthValue = availableSpace - (rightLengthValue + marginLeft + marginRight);
    } else if(rightLength.isAuto()) {
        marginLeft = marginLeftLength.calc(containerWidth);
        marginRight = marginRightLength.calc(containerWidth);
        leftLengthValue = leftLength.calc(containerWidth);

        rightLengthValue = availableSpace - (leftLengthValue + marginLeft + marginRight);
    } else if(marginLeftLength.isAuto()) {
        marginRight = marginRightLength.calc(containerWidth);
        leftLengthValue = leftLength.calc(containerWidth);
        rightLengthValue = rightLength.calc(containerWidth);

        marginLeft = availableSpace - (leftLengthValue + rightLengthValue + marginRight);
    } else if(marginRightLength.isAuto()) {
        marginLeft = marginLeftLength.calc(containerWidth);
        leftLengthValue = leftLength.calc(containerWidth);
        rightLengthValue = rightLength.calc(containerWidth);

        marginRight = availableSpace - (leftLengthValue + rightLengthValue + marginLeft);
    } else {
        marginLeft = marginLeftLength.calc(containerWidth);
        marginRight = marginRightLength.calc(containerWidth);
        leftLengthValue = leftLength.calc(containerWidth);
        rightLengthValue = rightLength.calc(containerWidth);
    }

    auto totalWidth = width + leftLengthValue + rightLengthValue +  marginLeft + marginRight;
    if(totalWidth > containerWidth && containerDirection == TextDirection::Rtl) {
        leftLengthValue = containerWidth - (totalWidth - leftLengthValue);
    }

    x = computePositionedLeftOffset(leftLengthValue, marginLeft, container, containerDirection);
}

void BoxFrame::computePositionedWidth(float& x, float& width, float& marginLeft, float& marginRight) const
{
    if(isReplaced()) {
        computePositionedWidthReplaced(x, width, marginLeft, marginRight);
        return;
    }

    auto container = containingBox();
    auto containerWidth = containingBlockWidthForPositioned(container);
    auto containerDirection = container->style()->direction();

    auto marginLeftLength = style()->marginLeft();
    auto marginRightLength = style()->marginRight();

    auto leftLength = style()->left();
    auto rightLength = style()->right();
    if(leftLength.isAuto() && rightLength.isAuto()) {
        if(containerDirection == TextDirection::Ltr) {
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

    if(minWidthLength.isAuto() || !minWidthLength.isZero()) {
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

    width += borderPaddingWidth();
}

void BoxFrame::computePositionedHeightUsing(const Length& heightLength, const BoxModel* container, float containerHeight, float contentHeight,
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

        auto availableSpace = containerHeight - (topLengthValue + height + borderPaddingHeight() + bottomLength.calc(containerHeight));
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

        auto availableSpace = containerHeight - (marginTop + marginBottom + borderPaddingHeight());
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

void BoxFrame::computePositionedHeightReplaced(float& y, float& height, float& marginTop, float& marginBottom) const
{
    auto container = containingBox();
    auto containerHeight = containingBlockHeightForPositioned(container);

    auto marginTopLength = style()->marginTop();
    auto marginBottomLength = style()->marginBottom();

    auto topLength = style()->top();
    auto bottomLength = style()->bottom();

    height = computeReplacedHeight() + borderPaddingHeight();
    auto availableSpace = containerHeight - height;
    if(topLength.isAuto() && bottomLength.isAuto()) {
        auto staticTop = layer()->staticTop() - container->borderTop();
        for(auto parent = parentBox(); parent && parent != container; parent = parent->parentBox()) {
            if(auto box = to<BoxFrame>(parent)) {
                staticTop += box->y();
            }
        }

        topLength = Length{Length::Type::Fixed, staticTop};
    }

    if(topLength.isAuto() || bottomLength.isAuto()) {
        if(marginTopLength.isAuto())
            marginTopLength = Length::ZeroFixed;
        if(marginBottomLength.isAuto()) {
            marginBottomLength = Length::ZeroFixed;
        }
    }

    float topLengthValue = 0;
    float bottomLengthValue = 0;
    if(marginTopLength.isAuto() && marginBottomLength.isAuto()) {
        topLengthValue = topLength.calc(containerHeight);
        bottomLengthValue = bottomLength.calc(containerHeight);

        auto availableHeight = availableSpace - (topLengthValue + bottomLengthValue);
        marginTop = availableHeight / 2.f;
        marginBottom = availableHeight - marginTop;
    } else if(topLength.isAuto()) {
        marginTop = marginTopLength.calc(containerHeight);
        marginBottom = marginBottomLength.calc(containerHeight);
        bottomLengthValue = bottomLength.calc(containerHeight);

        topLengthValue = availableSpace - (bottomLengthValue + marginTop + marginBottom);
    } else if(bottomLength.isAuto()) {
        marginTop = marginTopLength.calc(containerHeight);
        marginBottom = marginBottomLength.calc(containerHeight);
        topLengthValue = topLength.calc(containerHeight);
    } else if(marginTopLength.isAuto()) {
        marginBottom = marginBottomLength.calc(containerHeight);
        topLengthValue = topLength.calc(containerHeight);
        bottomLengthValue = bottomLength.calc(containerHeight);

        marginTop = availableSpace - (topLengthValue + bottomLengthValue + marginBottom);
    } else if(marginBottomLength.isAuto()) {
        marginTop = marginTopLength.calc(containerHeight);
        topLengthValue = topLength.calc(containerHeight);
        bottomLengthValue = bottomLength.calc(containerHeight);

        marginBottom = availableSpace - (topLengthValue + bottomLengthValue + marginTop);
    } else {
        marginTop = marginTopLength.calc(containerHeight);
        marginBottom = marginBottomLength.calc(containerHeight);
        topLengthValue = topLength.calc(containerHeight);
    }

    y = topLengthValue + marginTop + container->borderTop();
}

void BoxFrame::computePositionedHeight(float& y, float& height, float& marginTop, float& marginBottom) const
{
    if(isReplaced()) {
        computePositionedHeightReplaced(y, height, marginTop, marginBottom);
        return;
    }

    auto container = containingBox();
    auto containerHeight = containingBlockHeightForPositioned(container);
    auto contentHeight = height - borderPaddingHeight();

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

    if(minHeightLength.isAuto() || !minHeightLength.isZero()) {
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

    height += borderPaddingHeight();
}

void BoxFrame::computeWidth(float& x, float& width, float& marginLeft, float& marginRight) const
{
    if(isPositioned()) {
        computePositionedWidth(x, width, marginLeft, marginRight);
        return;
    }

    if(hasOverrideWidth() && isFlexItem()) {
        width = overrideWidth() + borderPaddingWidth();
        return;
    }

    auto computeAsReplaced = isReplaced() && !(isInline() && isBlockBox());
    if(computeAsReplaced)
        width = computeReplacedWidth() + borderPaddingWidth();

    auto container = containingBlock();
    auto containerWidth = std::max(0.f, container->availableWidth());
    if(isInline() && !(isReplaced() && isBlockBox())) {
        if(computeAsReplaced)
            width = std::max(width, minPreferredWidth());
        marginLeft = style()->marginLeft().calcMin(containerWidth);
        marginRight = style()->marginRight().calcMin(containerWidth);
        return;
    }

    if(!computeAsReplaced) {
        width = computeWidthUsing(style()->width(), container, containerWidth);
        width = constrainWidthByMinMax(width, container, containerWidth);
    }

    computeHorizontalMargins(marginLeft, marginRight, width, container, containerWidth);
    if(containerWidth && !(containerWidth == (width + marginLeft + marginRight)) && !isInline() && !isFloating() && !isFlexibleBox()) {
        if(style()->isLeftToRightDirection() == container->style()->isLeftToRightDirection()) {
            marginRight = containerWidth - width - marginLeft;
        } else {
            marginLeft = containerWidth - width - marginRight;
        }
    }
}

void BoxFrame::computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const
{
    if(isTableCellBox() || (isInline() && !isReplaced()))
        return;
    if(isPositioned()) {
        computePositionedHeight(y, height, marginTop, marginBottom);
        return;
    }

    if(isTableBox()) {
        computeVerticalMargins(marginTop, marginBottom);
        return;
    }

    if(hasOverrideHeight() && isFlexItem()) {
        height = overrideHeight() + borderPaddingHeight();
    } else if(isReplaced() && !(isInline() && isBlockBox())) {
        height = computeReplacedHeight() + borderPaddingHeight();
    } else {
        if(auto computedHeight = computeHeightUsing(style()->height()))
            height = *computedHeight;
        height = constrainHeightByMinMax(height);
    }

    computeVerticalMargins(marginTop, marginBottom);
}

void BoxFrame::updateWidth()
{
    computeWidth(m_x, m_width, m_marginLeft, m_marginRight);
}

void BoxFrame::updateHeight()
{
    computeHeight(m_y, m_height, m_marginTop, m_marginBottom);
}

void BoxFrame::updateVerticalMargins()
{
    computeVerticalMargins(m_marginTop, m_marginBottom);
}

float BoxFrame::maxMarginTop(bool positive) const
{
    if(auto block = to<BlockFlowBox>(this))
        return positive ? block->maxPositiveMarginTop() : block->maxNegativeMarginTop();
    return positive ? std::max(0.f, m_marginTop) : -std::min(0.f, m_marginTop);
}

float BoxFrame::maxMarginBottom(bool positive) const
{
    if(auto block = to<BlockFlowBox>(this))
        return positive ? block->maxPositiveMarginBottom() : block->maxNegativeMarginBottom();
    return positive ? std::max(0.f, m_marginBottom) : -std::min(0.f, m_marginBottom);
}

float BoxFrame::collapsedMarginTop() const
{
    if(auto block = to<BlockFlowBox>(this))
        return block->maxPositiveMarginTop() - block->maxNegativeMarginTop();
    return m_marginTop;
}

float BoxFrame::collapsedMarginBottom() const
{
    if(auto block = to<BlockFlowBox>(this))
        return block->maxPositiveMarginBottom() - block->maxNegativeMarginBottom();
    return m_marginBottom;
}

} // namespace htmlbook
