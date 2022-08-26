#include "linebox.h"
#include "box.h"

#include <cassert>

namespace htmlbook {

LineBox::LineBox(Box* box)
    : m_box(box)
{
    box->addLine(this);
}

LineBox::~LineBox()
{
    m_box->removeLine(this);
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

} // namespace htmlbook
