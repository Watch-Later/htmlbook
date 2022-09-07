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
        m_parentBox->removeBox(this);
    if(m_node)
        m_node->setBox(nullptr);
}

void Box::addBox(Box* box)
{
    auto children = this->children();
    if(children == nullptr)
        return;
    auto requirestable = [this](auto box) {
        if(box->isTableCaptionBox() || box->isTableSectionBox())
            return !isTableBox();
        if(box->isTableColumnBox())
            return !isTableBox()  && !isTableColumnGroupBox();
        if(box->isTableRowBox())
            return !isTableSectionBox();
        if(box->isTableCellBox())
            return !isTableRowBox();
        return false;
    };

    if(!requirestable(box)) {
        children->append(this, box);
        return;
    }

    auto lastChild = children->lastBox();
    if(lastChild && lastChild->isAnonymous() && lastChild->isTableBox()) {
        lastChild->addBox(box);
        return;
    }

    auto newTable = createAnonymous(*box->style(), Display::Table);
    children->append(this, newTable);
    newTable->addBox(box);
}

void Box::removeBox(Box* box)
{
    if(auto children = this->children())
        children->remove(this, box);
}

void Box::computePreferredWidths(float& minWidth, float& maxWidth) const
{
    minWidth = 0;
    maxWidth = 0;
}

void Box::addLine(LineBox* line)
{
    if(auto lines = this->lines())
        lines->add(this, line);
}

void Box::removeLine(LineBox* line)
{
    if(auto lines = this->lines())
        lines->remove(this, line);
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

LineBox* Box::firstLine() const
{
    if(auto lines = this->lines())
        return lines->firstLine();
    return nullptr;
}

LineBox* Box::lastLine() const
{
    if(auto lines = this->lines())
        return lines->lastLine();
    return nullptr;
}

Box* Box::create(Node* node, const RefPtr<BoxStyle>& style)
{
    switch(style->display()) {
    case Display::Inline:
        return new InlineBox(node, style);
    case Display::Block:
    case Display::InlineBlock:
        return new BlockBox(node, style);
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

Box* Box::createAnonymous(const BoxStyle& parentStyle, Display display)
{
    auto box = create(nullptr, BoxStyle::create(parentStyle, display));
    box->setAnonymous(true);
    return box;
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
    return std::unique_ptr<BoxLayer>(new BoxLayer(box, parent));
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
}

BoxModel::BoxModel(Node* node, const RefPtr<BoxStyle>& style)
    : Box(node, style)
{
}

BoxFrame::BoxFrame(Node* node, const RefPtr<BoxStyle>& style)
    : BoxModel(node, style)
{
}

InlineBox::InlineBox(Node* node, const RefPtr<BoxStyle>& style)
    : BoxModel(node, style)
{
}

BlockBox::BlockBox(Node* node, const RefPtr<BoxStyle>& style)
    : BoxFrame(node, style)
{
}

void BlockBox::addBox(Box* box)
{
    if(m_childrenInline && !box->isInline() && !box->isFloatingOrPositioned()) {
        m_childrenInline = false;
        auto newBlock = createAnonymous(*style(), Display::Block);
        auto children = newBlock->children();
        if(auto child = m_children.firstBox()) {
            assert(box->isInline() || box->isFloatingOrPositioned());
            m_children.remove(this, child);
            children->append(newBlock, child);
        }

        m_children.append(this, newBlock);
    } else if(!m_childrenInline && (box->isInline() || box->isFloatingOrPositioned())) {
        auto lastChild = m_children.lastBox();
        if(lastChild && lastChild->isAnonymous() && lastChild->isBlockBox()) {
            lastChild->addBox(box);
            return;
        }

        if(box->isInline()) {
            auto newBlock = createAnonymous(*style(), Display::Block);
            m_children.append(this, newBlock);

            auto children = newBlock->children();
            auto child = newBlock->prevBox();
            while(child && child->isFloatingOrPositioned()) {
                auto prevBox = child->prevBox();
                m_children.remove(this, child);
                children->insert(newBlock, child, newBlock->firstBox());
                child = prevBox;
            }

            newBlock->addBox(box);
            return;
        }
    }

    BoxFrame::addBox(box);
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

ListItemBox::ListItemBox(Node* node, const RefPtr<BoxStyle>& style)
    : BlockBox(node, style)
{
}

ListMarkerBox::ListMarkerBox(ListItemBox* item, const RefPtr<BoxStyle>& style)
    : BoxFrame(nullptr, style)
{
    item->setListMarker(this);
}

ListMarkerBox::~ListMarkerBox()
{
    m_listItem->setListMarker(nullptr);
}

TableBox::TableBox(Node* node, const RefPtr<BoxStyle>& style)
    : BlockBox(node, style)
{
}

TableCellBox::TableCellBox(Node* node, const RefPtr<BoxStyle>& style)
    : BlockBox(node, style)
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

TableRowBox::TableRowBox(Node* node, const RefPtr<BoxStyle>& style)
    : BoxFrame(node, style)
{
}

TableCaptionBox::TableCaptionBox(Node* node, const RefPtr<BoxStyle>& style)
    : BlockBox(node, style)
{
}

TableSectionBox::TableSectionBox(Node* node, const RefPtr<BoxStyle>& style)
    : BoxFrame(node, style)
{
}

} // namespace htmlbook
