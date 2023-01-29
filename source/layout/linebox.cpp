#include "linebox.h"
#include "textbox.h"
#include "blockbox.h"

#include <cassert>

namespace htmlbook {

LineBox::LineBox(Box* box)
    : m_box(box)
{
}

LineBox::~LineBox()
{
    if(m_parentLine) {
        m_parentLine->removeLine(this);
    }
}

std::unique_ptr<TextLineBox> TextLineBox::create(TextBox* box, std::string text)
{
    return std::unique_ptr<TextLineBox>(new (box->heap()) TextLineBox(box, std::move(text)));
}

TextLineBox::TextLineBox(TextBox* box, std::string text)
    : LineBox(box), m_text(std::move(text))
{
}

std::unique_ptr<ReplacedLineBox> ReplacedLineBox::create(BoxFrame* box)
{
    return std::unique_ptr<ReplacedLineBox>(new (box->heap()) ReplacedLineBox(box));
}

ReplacedLineBox::ReplacedLineBox(BoxFrame* box)
    : LineBox(box)
{
}

FlowLineBox::FlowLineBox(BoxModel* box)
    : LineBox(box)
{
}

std::unique_ptr<FlowLineBox> FlowLineBox::create(BoxModel* box)
{
    return std::unique_ptr<FlowLineBox>(new (box->heap()) FlowLineBox(box));
}

void FlowLineBox::addLine(LineBox* line)
{
    assert(line->parentLine() == nullptr);
    assert(line->prevLine() == nullptr);
    assert(line->nextLine() == nullptr);
    line->setParentLine(this);
    if(m_firstLine == nullptr) {
        m_firstLine = m_lastLine = line;
        return;
    }

    line->setPrevLine(m_lastLine);
    m_lastLine->setNextLine(line);
    m_lastLine = line;
}

void FlowLineBox::removeLine(LineBox *line)
{
    assert(line->parentLine() == this);
    auto nextLine = line->nextLine();
    auto prevLine = line->prevLine();
    if(nextLine)
        nextLine->setPrevLine(prevLine);
    if(prevLine)
        prevLine->setNextLine(nextLine);

    if(m_firstLine == line)
        m_firstLine = nextLine;
    if(m_lastLine == line)
        m_lastLine = prevLine;

    line->setParentLine(nullptr);
    line->setPrevLine(nullptr);
    line->setNextLine(nullptr);
}

FlowLineBox::~FlowLineBox()
{
    auto line = m_firstLine;
    while(line) {
        line->setParentLine(nullptr);
        line->setPrevLine(nullptr);
        line->setNextLine(nullptr);
        line = line->nextLine();
    }
}

std::unique_ptr<RootLineBox> RootLineBox::create(BlockFlowBox* box)
{
    return std::unique_ptr<RootLineBox>(new (box->heap()) RootLineBox(box));
}

RootLineBox::RootLineBox(BlockFlowBox* box)
    : FlowLineBox(box)
{
}

std::unique_ptr<LineLayout> LineLayout::create(BlockFlowBox* box)
{
    return std::unique_ptr<LineLayout>(new (box->heap()) LineLayout(box));
}

void LineLayout::computeIntrinsicWidths(float& minWidth, float& maxWidth) const
{
    minWidth = 0;
    maxWidth = 0;
}

void LineLayout::build()
{
}

void LineLayout::layout()
{
}

LineLayout::LineLayout(BlockFlowBox* box)
    : m_box(box)
    , m_lines(box->heap())
{
}

} // namespace htmlbook
