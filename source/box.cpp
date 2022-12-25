#include "box.h"
#include "document.h"
#include "resource.h"

namespace htmlbook {

Box::Box(Node* node, const RefPtr<BoxStyle>& style)
    : m_node(node), m_style(style)
{
    if(node)
        node->setBox(this);
}

Box::~Box()
{
    if(m_parentBox)
        m_parentBox->removeChild(this);
    if(m_node)
        m_node->setBox(nullptr);
}

void Box::computePreferredWidths(float& minWidth, float& maxWidth) const
{
    minWidth = 0;
    maxWidth = 0;
}

void Box::buildBox(BoxLayer* layer)
{
    m_containingBox = containingBox(*this);
    m_containingBlock = containingBlock(*this);
    for(auto box = firstBox(); box; box = box->nextBox()) {
        box->buildBox(layer);
    }
}

void Box::addBox(Box* box)
{
    appendChild(box);
}

LineBox* Box::addLine(std::unique_ptr<LineBox> line)
{
    auto lines = this->lines();
    assert(lines != nullptr);
    lines->push_back(std::move(line));
    return &*lines->back();
}

std::unique_ptr<LineBox> Box::removeLine(LineBox* line)
{
    auto lines = this->lines();
    assert(lines != nullptr);
    for(auto it = lines->begin(); it != lines->end(); ++it) {
        if(line == &**it) {
            auto value = std::move(*it);
            lines->erase(it);
            return value;
        }
    }

    return nullptr;
}

void Box::insertChild(Box* box, Box* nextBox)
{
    auto children = this->children();
    assert(children != nullptr);
    children->insert(this, box, nextBox);
}

void Box::appendChild(Box* box)
{
    auto children = this->children();
    assert(children != nullptr);
    children->append(this, box);
}

void Box::removeChild(Box* box)
{
    auto children = this->children();
    assert(children != nullptr);
    children->remove(this, box);
}

void Box::moveChildrenTo(Box* to, Box* begin, Box* end)
{
    auto fromChildren = children();
    auto toChildren = to->children();
    assert(fromChildren && toChildren);
    auto child = begin;
    while(child && child != end) {
        auto nextChild = child->nextBox();
        fromChildren->remove(this, child);
        toChildren->append(to, child);
        child = nextChild;
    }
}

void Box::moveChildrenTo(Box* to, Box* begin)
{
    moveChildrenTo(to, begin, nullptr);
}

void Box::moveChildrenTo(Box* to)
{
    moveChildrenTo(to, firstBox(), nullptr);
}

Box* Box::firstBox() const
{
    if(auto children = this->children())
        return children->firstBox();
    return nullptr;
}

Box* Box::lastBox() const
{
    if(auto children = this->children())
        return children->lastBox();
    return nullptr;
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

Box* Box::containingBox(const Box& box)
{
    auto parent = box.parentBox();
    if(!is<TextBox>(box)) {
        if(box.position() == Position::Fixed)
            return box.containingBlockFixed();
        if(box.position() == Position::Absolute) {
            while(parent && parent->position() == Position::Static) {
                if(parent->isRootBox() || (parent->hasTransform() && is<BlockBox>(*parent)))
                    break;
                parent = parent->parentBox();
            }
        }
    }

    return parent;
}

BlockBox* Box::containingBlock(const Box& box)
{
    if(!is<TextBox>(box)) {
        if(box.position() == Position::Fixed)
            return box.containingBlockFixed();
        if(box.position() == Position::Absolute) {
            return box.containingBlockAbsolute();
        }
    }

    auto parent = box.parentBox();
    while(parent && ((parent->isInline() && !parent->isReplaced()) || !is<BlockBox>(*parent))) {
        parent = parent->parentBox();
    }

    return to<BlockBox>(parent);
}

BlockBox* Box::containingBlockFixed() const
{
    auto parent = parentBox();
    while(parent && !(parent->isRootBox() || (parent->hasTransform() && is<BlockBox>(*parent)))) {
        parent = parent->parentBox();
    }

    return to<BlockBox>(parent);
}

BlockBox* Box::containingBlockAbsolute() const
{
    auto parent = parentBox();
    while(parent && parent->position() == Position::Static) {
        if(parent->isRootBox() || (parent->hasTransform() && is<BlockBox>(*parent)))
            break;
        parent = parent->parentBox();
    }

    if(parent && !is<BlockBox>(*parent))
        parent = parent->containingBox();
    while(parent && parent->isAnonymous()) {
        parent = parent->containingBox();
    }

    return to<BlockBox>(parent);
}

BoxList::~BoxList()
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
}

void BoxList::insert(Box* parent, Box* box, Box* nextBox)
{
    if(nextBox == nullptr) {
        append(parent, box);
        return;
    }

    assert(nextBox->parentBox() == parent);
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

    box->setParentBox(parent);
    box->setPrevBox(prevBox);
    box->setNextBox(nextBox);
}

void BoxList::append(Box* parent, Box* box)
{
    assert(box->parentBox() == nullptr);
    assert(box->prevBox() == nullptr);
    assert(box->nextBox() == nullptr);
    box->setParentBox(parent);
    if(m_firstBox == nullptr) {
        m_firstBox = m_lastBox = box;
        return;
    }

    box->setPrevBox(m_lastBox);
    m_lastBox->setNextBox(box);
    m_lastBox = box;
}

void BoxList::remove(Box* parent, Box* box)
{
    assert(box->parentBox() == parent);
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

TextBox::TextBox(Node* node, const RefPtr<BoxStyle>& style)
    : Box(node, style)
    , m_lines(style->heap())
{
    setInline(true);
}

BoxModel::BoxModel(Node* node, const RefPtr<BoxStyle>& style)
    : Box(node, style)
{
    switch(style->floating()) {
    case Float::None:
        setFloating(false);
        break;
    default:
        setFloating(true);
        break;
    }

    switch(style->position()) {
    case Position::Static:
        setPositioned(false);
        break;
    default:
        setPositioned(true);
        break;
    }

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
    auto children = this->children();
    assert(children != nullptr);
    if(!is<TableCellBox>(box) && !is<TableRowBox>(box)
        && !is<TableCaptionBox>(box) && !is<TableColumnBox>(box)
        && !is<TableSectionBox>(box)) {
        children->append(this, box);
        return;
    }

    auto lastChild = children->lastBox();
    if(lastChild && lastChild->isAnonymous() && is<TableBox>(lastChild)) {
        lastChild->addBox(box);
        return;
    }

    auto newTable = createAnonymous(style(), Display::Table);
    children->append(this, newTable);
    newTable->addBox(box);
}

void BoxModel::computeBorder(float& top, float& bottom, float& left, float& right) const
{
    top = style()->borderTopWidth();
    bottom = style()->borderBottomWidth();
    left = style()->borderLeftWidth();
    right = style()->borderRightWidth();
}

void BoxModel::computePadding(float& top, float& bottom, float& left, float& right) const
{
    auto compute = [this](const auto& padding) {
        return padding.calc(containingBlock()->availableWidth());
    };

    top = compute(style()->paddingTop());
    bottom = compute(style()->paddingBottom());
    left = compute(style()->paddingLeft());
    right = compute(style()->paddingRight());
}

float BoxModel::borderTop() const
{
    if(m_borderTop < 0)
        computeBorder(m_borderTop, m_borderBottom, m_borderLeft, m_borderRight);
    return m_borderTop;
}

float BoxModel::borderBottom() const
{
    if(m_borderBottom < 0)
        computeBorder(m_borderTop, m_borderBottom, m_borderLeft, m_borderRight);
    return m_borderBottom;
}

float BoxModel::borderLeft() const
{
    if(m_borderLeft < 0)
        computeBorder(m_borderTop, m_borderBottom, m_borderLeft, m_borderRight);
    return m_borderLeft;
}

float BoxModel::borderRight() const
{
    if(m_borderRight < 0)
        computeBorder(m_borderTop, m_borderBottom, m_borderLeft, m_borderRight);
    return m_borderRight;
}

float BoxModel::paddingTop() const
{
    if(m_paddingTop < 0)
        computePadding(m_paddingTop, m_paddingBottom, m_paddingLeft, m_paddingRight);
    return m_paddingTop;
}

float BoxModel::paddingBottom() const
{
    if(m_paddingBottom < 0)
        computePadding(m_paddingTop, m_paddingBottom, m_paddingLeft, m_paddingRight);
    return m_paddingBottom;
}

float BoxModel::paddingLeft() const
{
    if(m_paddingLeft < 0)
        computePadding(m_paddingTop, m_paddingBottom, m_paddingLeft, m_paddingRight);
    return m_paddingLeft;
}

float BoxModel::paddingRight() const
{
    if(m_paddingRight < 0)
        computePadding(m_paddingTop, m_paddingBottom, m_paddingLeft, m_paddingRight);
    return m_paddingRight;
}

BoxFrame::BoxFrame(Node* node, const RefPtr<BoxStyle>& style)
    : BoxModel(node, style)
{
    setHasTransform(style->hasTransform());
}

float BoxFrame::minPreferredWidth() const
{
    if(m_minPreferredWidth < 0)
        computePreferredWidths(m_minPreferredWidth, m_maxPreferredWidth);
    return m_minPreferredWidth;
}

float BoxFrame::maxPreferredWidth() const
{
    if(m_maxPreferredWidth < 0)
        computePreferredWidths(m_minPreferredWidth, m_maxPreferredWidth);
    return m_maxPreferredWidth;
}

void BoxFrame::updateWidth()
{
    computeWidth(m_x, m_width, m_marginLeft, m_marginRight);
}

void BoxFrame::updateHeight()
{
    computeHeight(m_y, m_height, m_marginTop, m_marginBottom);
}

void BoxFrame::computePositionedWidthReplaced(float& x, float& width, float& marginLeft, float& marginRight) const
{
}

void BoxFrame::computePositionedWidth(float& x, float& width, float& marginLeft, float& marginRight) const
{
    if(isReplaced()) {
        computePositionedWidthReplaced(x, width, marginLeft, marginRight);
        return;
    }
}

void BoxFrame::computeWidth(float& x, float& width, float& marginLeft, float& marginRight) const
{
    if(isPositioned()) {
        computePositionedWidth(x, width, marginLeft, marginRight);
        return;
    }
}

void BoxFrame::computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const
{
}

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

        auto children = block->children();
        assert(children->lastBox() == preBlock);
        children->append(block, newBlock);
        children->append(block, postBlock);
    } else {
        preBlock = createAnonymousBlock(block->style());
        postBlock = createAnonymousBlock(block->style());
        block->moveChildrenTo(preBlock);

        auto children = block->children();
        children->append(block, preBlock);
        children->append(block, newBlock);
        children->append(block, postBlock);
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

FlexibleBox::FlexibleBox(Node* node, const RefPtr<BoxStyle>& style)
    : BlockBox(node, style)
{
    setChildrenInline(false);
}

ReplacedBox::ReplacedBox(Node* node, const RefPtr<BoxStyle>& style)
    : BoxFrame(node, style)
{
    setReplaced(true);
}

ImageBox::ImageBox(Node* node, const RefPtr<BoxStyle>& style)
    : ReplacedBox(node, style)
{
}

void ImageBox::setImage(RefPtr<Image> image)
{
    m_image = std::move(image);
}

ListItemBox::ListItemBox(Node* node, const RefPtr<BoxStyle>& style)
    : BlockFlowBox(node, style)
{
}

InsideListMarkerBox::InsideListMarkerBox(const RefPtr<BoxStyle>& style)
    : InlineBox(nullptr, style)
{
}

OutsideListMarkerBox::OutsideListMarkerBox(const RefPtr<BoxStyle>& style)
    : BlockFlowBox(nullptr, style)
{
}

TableBox::TableBox(Node* node, const RefPtr<BoxStyle>& style)
    : BlockBox(node, style)
{
    setChildrenInline(false);
}

void TableBox::buildBox(BoxLayer* layer)
{
    for(auto child = m_children.firstBox(); child; child = child->nextBox()) {
        if(auto section = to<TableSectionBox>(child)) {
            switch(child->display()) {
            case Display::TableHeaderGroup:
                m_header = section;
                break;
            case Display::TableFooterGroup:
                m_footer = section;
                break;
            default:
                m_sections.push_back(section);
                break;
            }
        } else if(auto column = to<TableColumnBox>(child)) {
            if(auto child = column->firstBox()) {
                do {
                    if(auto column = to<TableColumnBox>(child))
                        m_columns.push_back(column);
                    child = child->nextBox();
                } while(child);
            } else {
                m_columns.push_back(column);
            }
        } else {
            assert(is<TableCaptionBox>(child));
            auto caption = to<TableCaptionBox>(child);
            m_captions.push_back(caption);
        }
    }

    BlockBox::buildBox(layer);
}

void TableBox::addBox(Box* box)
{
    if(is<TableCaptionBox>(box) || is<TableColumnBox>(box)
        || is<TableSectionBox>(box)) {
        m_children.append(this, box);
        return;
    }

    auto lastChild = m_children.lastBox();
    if(lastChild && lastChild->isAnonymous() && is<TableSectionBox>(lastChild)) {
        lastChild->addBox(box);
        return;
    }

    auto newSection = createAnonymous(style(), Display::TableRowGroup);
    m_children.append(this, newSection);
    newSection->addBox(box);
}

TableSectionBox::TableSectionBox(Node* node, const RefPtr<BoxStyle>& style)
    : Box(node, style)
{
}

void TableSectionBox::addBox(Box* box)
{
    if(is<TableRowBox>(box)) {
        m_children.append(this, box);
        return;
    }

    auto lastChild = m_children.lastBox();
    if(lastChild && lastChild->isAnonymous() && is<TableRowBox>(lastChild)) {
        lastChild->addBox(box);
        return;
    }

    auto newRow = createAnonymous(style(), Display::TableRow);
    m_children.append(this, newRow);
    newRow->addBox(box);
}

TableRowBox::TableRowBox(Node* node, const RefPtr<BoxStyle>& style)
    : Box(node, style)
{
}

void TableRowBox::addBox(Box* box)
{
    if(is<TableCellBox>(box)) {
        m_children.append(this, box);
        return;
    }

    auto lastChild = m_children.lastBox();
    if(lastChild && lastChild->isAnonymous() && is<TableCellBox>(lastChild)) {
        lastChild->addBox(box);
        return;
    }

    auto newCell = createAnonymous(style(), Display::TableCell);
    m_children.append(this, newCell);
    newCell->addBox(box);
}

TableCellBox::TableCellBox(Node* node, const RefPtr<BoxStyle>& style)
    : BlockFlowBox(node, style)
{
}

TableColumnBox::TableColumnBox(Node* node, const RefPtr<BoxStyle>& style)
    : Box(node, style)
{
}

TableColumnGroupBox::TableColumnGroupBox(Node* node, const RefPtr<BoxStyle>& style)
    : TableColumnBox(node, style)
{
}

void TableColumnGroupBox::addBox(Box* box)
{
    m_children.append(this, box);
}

TableCaptionBox::TableCaptionBox(Node* node, const RefPtr<BoxStyle>& style)
    : BlockFlowBox(node, style), m_captionSide(style->captionSide())
{
}

} // namespace htmlbook
