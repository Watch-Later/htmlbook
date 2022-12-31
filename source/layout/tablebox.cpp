#include "tablebox.h"

namespace htmlbook {

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
