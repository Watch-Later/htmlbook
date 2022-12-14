#include "linebox.h"
#include "box.h"

#include <cassert>

namespace htmlbook {

LineBox::LineBox(Box* box)
    : m_box(box)
{
}

LineBox::~LineBox()
{
    if(m_parentLine)
        m_parentLine->removeLine(this);
}

TextLineBox::TextLineBox(TextBox* box, std::string text)
    : LineBox(box), m_text(std::move(text))
{
}

ReplacedLineBox::ReplacedLineBox(BoxFrame* box)
    : LineBox(box)
{
}

FlowLineBox::FlowLineBox(BoxModel* box)
    : LineBox(box)
{
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

RootLineBox::RootLineBox(BlockFlowBox* box)
    : FlowLineBox(box)
{
}

} // namespace htmlbook
