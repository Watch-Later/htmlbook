#include "box.h"
#include "flexiblebox.h"
#include "listitembox.h"
#include "tablebox.h"
#include "document.h"

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

void Box::build(BoxLayer* layer)
{
    if(layer == nullptr || requiresLayer()) {
        m_layer = BoxLayer::create(this, layer);
        layer = m_layer.get();
    }

    auto child = m_firstBox;
    while(child) {
        child->build(layer);
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

BlockBox* Box::containingBlockForPosition(Position position) const
{
    auto parent = parentBox();
    if(position == Position::Static || position == Position::Relative) {
        while(parent && !parent->isBlockBox())
            parent = parent->parentBox();
        return to<BlockBox>(parent);
    }

    if(position == Position::Fixed) {
        while(parent && !(parent->isBoxView() || (parent->hasTransform() && parent->isBlockBox())))
            parent = parent->parentBox();
        return to<BlockBox>(parent);
    }

    while(parent && parent->position() == Position::Static) {
        if(parent->hasTransform() && parent->isBlockBox())
            break;
        parent = parent->parentBox();
    }

    if(parent && !parent->isBlockBox())
        parent = parent->containingBlock();
    while(parent && parent->isAnonymous())
        parent = parent->containingBlock();
    return to<BlockBox>(parent);
}

BlockBox* Box::containingBlock() const
{
    if(isTextBox())
        return containingBlockForPosition(Position::Static);
    return containingBlockForPosition(position());
}

BoxModel* Box::containingBox() const
{
    auto parent = parentBox();
    if(!isTextBox()) {
        if(position() == Position::Fixed)
            return containingBlockForPosition(Position::Fixed);
        if(position() == Position::Absolute) {
            while(parent && parent->position() == Position::Static) {
                if(parent->hasTransform() && parent->isBlockBox())
                    break;
                parent = parent->parentBox();
            }
        }
    }

    return to<BoxModel>(parent);
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

std::unique_ptr<BoxLayer> BoxLayer::create(Box* box, BoxLayer* parent)
{
    return std::unique_ptr<BoxLayer>(new (box->heap()) BoxLayer(box, parent));
}

BoxLayer::BoxLayer(Box* box, BoxLayer* parent)
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

float BoxModel::containingBlockWidthForContent() const
{
    return containingBlock()->availableWidth();
}

float BoxModel::containingBlockHeightForContent() const
{
    return containingBlock()->availableHeight();
}

void BoxModel::updateMarginWidths()
{
    auto compute = [this](const auto& margin) {
        float containerWidth = 0;
        if(margin.isPercent())
            containerWidth = containingBlockWidthForContent();
        return margin.calcMin(containerWidth);
    };

    m_marginTop = compute(style()->marginTop());
    m_marginBottom = compute(style()->marginBottom());
    m_marginLeft = compute(style()->marginLeft());
    m_marginRight = compute(style()->marginRight());
}

void BoxModel::updateBorderWidths() const
{
    switch(style()->borderTopStyle()) {
    case LineStyle::None:
    case LineStyle::Hidden:
        m_borderTop = 0.f;
        break;
    default:
        m_borderTop = style()->borderTopWidth();
        break;
    }

    switch(style()->borderBottomStyle()) {
    case LineStyle::None:
    case LineStyle::Hidden:
        m_borderBottom = 0.f;
        break;
    default:
        m_borderBottom = style()->borderBottomWidth();
        break;
    }

    switch(style()->borderLeftStyle()) {
    case LineStyle::None:
    case LineStyle::Hidden:
        m_borderLeft = 0.f;
        break;
    default:
        m_borderLeft = style()->borderLeftWidth();
        break;
    }

    switch(style()->borderRightStyle()) {
    case LineStyle::None:
    case LineStyle::Hidden:
        m_borderRight = 0.f;
        break;
    default:
        m_borderRight = style()->borderRightWidth();
        break;
    }
}

void BoxModel::updatePaddingWidths() const
{
    auto compute = [this](const auto& padding) {
        float containerWidth = 0;
        if(padding.isPercent())
            containerWidth = containingBlockWidthForContent();
        return padding.calcMin(containerWidth);
    };

    m_paddingTop = compute(style()->paddingTop());
    m_paddingBottom = compute(style()->paddingBottom());
    m_paddingLeft = compute(style()->paddingLeft());
    m_paddingRight = compute(style()->paddingRight());
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

void BoxFrame::updatePreferredWidths() const
{
    m_minPreferredWidth = 0;
    m_maxPreferredWidth = 0;
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

float BoxFrame::adjustBorderBoxWidth(float width) const
{
    if(style()->boxSizing() == BoxSizing::ContentBox)
        return width + borderAndPaddingWidth();
    return std::max(width, borderAndPaddingWidth());
}

float BoxFrame::adjustBorderBoxHeight(float height) const
{
    if(style()->boxSizing() == BoxSizing::ContentBox)
        return height + borderAndPaddingHeight();
    return std::max(height, borderAndPaddingHeight());
}

float BoxFrame::adjustContentBoxWidth(float width) const
{
    if(style()->boxSizing() == BoxSizing::BorderBox)
        width -= borderAndPaddingWidth();
    return std::max(0.f, width);
}

float BoxFrame::adjustContentBoxHeight(float height) const
{
    if(style()->boxSizing() == BoxSizing::BorderBox)
        height -= borderAndPaddingHeight();
    return std::max(0.f, height);
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

    if(container->isFlexibleBox()) {
        if(marginLeftLength.isAuto())
            marginLeftLength = Length::ZeroFixed;
        if(marginRightLength.isAuto()) {
            marginRightLength = Length::ZeroFixed;
        }
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
    return positive ? std::max(0.f, m_marginTop) : -std::min(0.f, m_marginTop);
}

float BoxFrame::maxMarginBottom(bool positive) const
{
    return positive ? std::max(0.f, m_marginBottom) : -std::min(0.f, m_marginBottom);
}

float BoxFrame::collapsedMarginTop() const
{
    return maxMarginTop(true) - maxMarginTop(false);
}

float BoxFrame::collapsedMarginBottom() const
{
    return maxMarginBottom(true) - maxMarginBottom(false);
}

} // namespace htmlbook
