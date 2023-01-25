#include "tablebox.h"

namespace htmlbook {

TableBox::TableBox(Node* node, const RefPtr<BoxStyle>& style)
    : BlockBox(node, style)
{
    setChildrenInline(false);
}

void TableBox::computeIntrinsicWidths(float& minWidth, float& maxWidth) const
{
    minWidth = 0;
    maxWidth = 0;
}

void TableBox::buildBox(BoxLayer* layer)
{
    for(auto child = firstBox(); child; child = child->nextBox()) {
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

    BlockBox::buildBox(layer);
}

void TableBox::addBox(Box* box)
{
    if(box->isTableCaptionBox() || box->isTableColumnBox()
        || box->isTableSectionBox()) {
        appendChild(box);
        return;
    }

    auto lastChild = lastBox();
    if(lastChild && lastChild->isAnonymous() && lastChild->isTableSectionBox()) {
        lastChild->addBox(box);
        return;
    }

    auto newSection = createAnonymous(style(), Display::TableRowGroup);
    appendChild(newSection);
    newSection->addBox(box);
}

TableCellBox::TableCellBox(Node* node, const RefPtr<BoxStyle>& style)
    : BlockFlowBox(node, style)
{
}

TableCaptionBox::TableCaptionBox(Node* node, const RefPtr<BoxStyle>& style)
    : BlockFlowBox(node, style), m_captionSide(style->captionSide())
{
}

TableSectionBox::TableSectionBox(Node* node, const RefPtr<BoxStyle>& style)
    : Box(node, style)
{
}

void TableSectionBox::addBox(Box* box)
{
    if(box->isTableRowBox()) {
        appendChild(box);
        return;
    }

    auto lastChild = lastBox();
    if(lastChild && lastChild->isAnonymous() && lastChild->isTableRowBox()) {
        lastChild->addBox(box);
        return;
    }

    auto newRow = createAnonymous(style(), Display::TableRow);
    appendChild(newRow);
    newRow->addBox(box);
}

TableRowBox::TableRowBox(Node* node, const RefPtr<BoxStyle>& style)
    : Box(node, style)
{
}

void TableRowBox::addBox(Box* box)
{
    if(box->isTableCellBox()) {
        appendChild(box);
        return;
    }

    auto lastChild = lastBox();
    if(lastChild && lastChild->isAnonymous() && lastChild->isTableCellBox()) {
        lastChild->addBox(box);
        return;
    }

    auto newCell = createAnonymous(style(), Display::TableCell);
    appendChild(newCell);
    newCell->addBox(box);
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
    appendChild(box);
}

} // namespace htmlbook
