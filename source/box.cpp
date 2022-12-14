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

void Box::buildBox(BoxLayer* parent)
{
    auto children = this->children();
    if(children == nullptr)
        return;
    auto box = children->firstBox();
    while(box) {
        box->buildBox(parent);
        box = box->nextBox();
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
            return new InsideListMarkerBox(style);
        return new OutsideListMarkerBox(style);
    }

    switch(style->display()) {
    case Display::Inline:
        return new InlineBox(node, style);
    case Display::Block:
    case Display::InlineBlock:
        return new BlockFlowBox(node, style);
    case Display::Flex:
    case Display::InlineFlex:
        return new FlexibleBox(node, style);
    case Display::Table:
    case Display::InlineTable:
        return new TableBox(node, style);
    case Display::ListItem:
        return new ListItemBox(node, style);
    case Display::TableCell:
        return new TableCellBox(node, style);
    case Display::TableColumn:
        return new TableColumnBox(node, style);
    case Display::TableColumnGroup:
        return new TableColumnGroupBox(node, style);
    case Display::TableRow:
        return new TableRowBox(node, style);
    case Display::TableRowGroup:
    case Display::TableHeaderGroup:
    case Display::TableFooterGroup:
        return new TableSectionBox(node, style);
    case Display::TableCaption:
        return new TableCaptionBox(node, style);
    default:
        assert(false);
    }
}

Box* Box::createAnonymous(const RefPtr<BoxStyle>& parentStyle, Display display)
{
    auto newBox = create(nullptr, BoxStyle::create(*parentStyle, display));
    newBox->setAnonymous(true);
    return newBox;
}

BlockFlowBox* Box::createAnonymousBlock(const RefPtr<BoxStyle>& parentStyle)
{
    auto newBlock = new BlockFlowBox(nullptr, BoxStyle::create(*parentStyle, Display::Block));
    newBlock->setAnonymous(true);
    return newBlock;
}

BlockBox* Box::containingBlock() const
{
    auto parent = parentBox();
    while(parent && parent->isInline() && !parent->isReplaced())
        parent = parent->parentBox();
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

BoxLayer::BoxLayer(BoxModel* box, BoxLayer* parent)
    : m_box(box), m_parent(parent)
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
{
    setInline(true);
}

BoxModel::BoxModel(Node* node, const RefPtr<BoxStyle>& style)
    : Box(node, style)
{
}

void BoxModel::buildBox(BoxLayer* parent)
{
    if(parent == nullptr || requiresLayer()) {
        m_layer = std::make_unique<BoxLayer>(this, parent);
        parent = m_layer.get();
    }

    Box::buildBox(parent);
}

void BoxModel::addBox(Box* box)
{
    auto children = this->children();
    assert(children != nullptr);
    if(!box->isTableCellBox() && !box->isTableRowBox()
        && !box->isTableCaptionBox() && !box->isTableColumnBox()
        && !box->isTableSectionBox()) {
        children->append(this, box);
        return;
    }

    auto lastChild = children->lastBox();
    if(lastChild && lastChild->isAnonymous() && lastChild->isTableBox()) {
        lastChild->addBox(box);
        return;
    }

    auto newTable = createAnonymous(style(), Display::Table);
    children->append(this, newTable);
    newTable->addBox(box);
}

BoxFrame::BoxFrame(Node* node, const RefPtr<BoxStyle>& style)
    : BoxModel(node, style)
{
}

InlineBox::InlineBox(Node* node, const RefPtr<BoxStyle>& style)
    : BoxModel(node, style)
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

    auto clone = new InlineBox(nullptr, style());
    Box* currentParent = parentBox();
    Box* currentChild = this;
    auto currentClone = clone;
    while(currentParent != preBlock) {
        auto parent = to<InlineBox>(currentParent);
        assert(parent->continuation() == nullptr);
        auto clone = new InlineBox(nullptr, parent->style());
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
    setInline(false);
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
        if(lastChild && lastChild->isAnonymous() && lastChild->isBlockBox()) {
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

BlockFlowBox::BlockFlowBox(Node *node, const RefPtr<BoxStyle>& style)
    : BlockBox(node, style)
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

void TableBox::buildBox(BoxLayer* parent)
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
            assert(child->isTableCaptionBox());
            auto caption = to<TableCaptionBox>(child);
            m_captions.push_back(caption);
        }
    }

    BlockBox::buildBox(parent);
}

void TableBox::addBox(Box* box)
{
    if(box->isTableCaptionBox() || box->isTableColumnBox()
        || box->isTableSectionBox()) {
        m_children.append(this, box);
        return;
    }

    auto lastChild = m_children.lastBox();
    if(lastChild && lastChild->isAnonymous() && lastChild->isTableSectionBox()) {
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
    if(box->isTableRowBox()) {
        m_children.append(this, box);
        return;
    }

    auto lastChild = m_children.lastBox();
    if(lastChild && lastChild->isAnonymous() && lastChild->isTableRowBox()) {
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
    if(box->isTableCellBox()) {
        m_children.append(this, box);
        return;
    }

    auto lastChild = m_children.lastBox();
    if(lastChild && lastChild->isAnonymous() && lastChild->isTableCellBox()) {
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
