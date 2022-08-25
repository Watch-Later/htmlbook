#include "box.h"
#include "document.h"
#include "boxstyle.h"
#include "resource.h"

namespace htmlbook {

Box::~Box()
{
    if(m_parentBox)
        m_parentBox->removeBox(this);
    if(m_node)
        m_node->setBox(nullptr);
}

void Box::addBox(Box* box, Box* nextBox)
{
    if(auto children = this->children())
        children->insert(this, box, nextBox);
}

void Box::removeBox(Box* box)
{
    if(auto children = this->children())
        children->remove(this, box);
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

LineBox::LineBox(Box* box)
    : m_box(box)
{
    box->addLine(this);
}

LineBox::~LineBox()
{
    m_box->removeLine(this);
}

PlaceHolderLineBox::PlaceHolderLineBox(Box* box, BoxFrame* placeHolderBox)
    : LineBox(box), m_placeHolderBox(placeHolderBox)
{
    placeHolderBox->setLine(this);
}

PlaceHolderLineBox::~PlaceHolderLineBox()
{
    m_placeHolderBox->setLine(nullptr);
}

FlowLineBox::~FlowLineBox()
{
    auto line = m_firstLine;
    while(line) {
        auto nextLine = line->nextOnLine();
        line->setParentLine(nullptr);
        line->setPrevOnLine(nullptr);
        line->setNextOnLine(nullptr);
        delete line;
        line = nextLine;
    }
}

void FlowLineBox::addLine(LineBox* line)
{
    assert(line->parentLine() == nullptr);
    assert(line->prevOnLine() == nullptr);
    assert(line->nextOnLine() == nullptr);
    line->setParentLine(this);
    if(m_firstLine == nullptr) {
        m_firstLine = m_lastLine = line;
        return;
    }

    line->setPrevOnLine(m_lastLine);
    m_lastLine->setNextOnLine(line);
    m_lastLine = line;
}

void FlowLineBox::removeLine(LineBox *line)
{
    assert(line->parentLine() == this);
    auto nextLine = line->nextOnLine();
    auto prevLine = line->prevOnLine();
    if(nextLine)
        nextLine->setPrevOnLine(prevLine);
    if(prevLine)
        prevLine->setNextOnLine(nextLine);

    if(m_firstLine == line)
        m_firstLine = nextLine;
    if(m_lastLine == line)
        m_lastLine = prevLine;

    line->setParentLine(nullptr);
    line->setPrevOnLine(nullptr);
    line->setNextOnLine(nullptr);
}

LineBoxList::~LineBoxList()
{
    auto line = m_firstLine;
    while(line) {
        auto nextLine = line->nextOnBox();
        line->setPrevOnBox(nullptr);
        line->setNextOnBox(nullptr);
        delete line;
        line = nextLine;
    }
}

void LineBoxList::add(Box* box, LineBox* line)
{
    assert(line->box() == box);
    assert(line->prevOnBox() == nullptr);
    assert(line->nextOnBox() == nullptr);
    if(m_firstLine == nullptr) {
        m_firstLine = m_lastLine = line;
        return;
    }

    line->setPrevOnBox(m_lastLine);
    m_lastLine->setNextOnBox(line);
    m_lastLine = line;
}

void LineBoxList::remove(Box* box, LineBox* line)
{
    assert(line->box() == box);
    auto nextLine = line->nextOnBox();
    auto prevLine = line->prevOnBox();
    if(nextLine)
        nextLine->setPrevOnBox(prevLine);
    if(prevLine)
        prevLine->setNextOnBox(nextLine);

    if(m_firstLine == line)
        m_firstLine = nextLine;
    if(m_lastLine == line)
        m_lastLine = prevLine;

    line->setPrevOnBox(nullptr);
    line->setNextOnBox(nullptr);
}

std::unique_ptr<BoxLayer> BoxLayer::create(BoxModel *box, BoxLayer *parent)
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

} // namespace htmlbook
