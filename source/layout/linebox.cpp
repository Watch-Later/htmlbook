#include "linebox.h"
#include "textbox.h"
#include "blockbox.h"

#include <cassert>

namespace htmlbook {

LineBox::LineBox(Box* box)
    : m_box(box)
{
}

LineBox::~LineBox() = default;

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
    , m_children(box->heap())
{
}

std::unique_ptr<FlowLineBox> FlowLineBox::create(BoxModel* box)
{
    return std::unique_ptr<FlowLineBox>(new (box->heap()) FlowLineBox(box));
}

void FlowLineBox::addLine(LineBox* line)
{
    m_children.push_back(line);
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
