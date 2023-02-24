#include "tablebox.h"

namespace htmlbook {

TableBox::TableBox(Node* node, const RefPtr<BoxStyle>& style)
    : BlockBox(node, style)
    , m_rows(style->heap())
    , m_columns(style->heap())
{
    setChildrenInline(false);
}

void TableBox::computePreferredWidths(float& minWidth, float& maxWidth) const
{
    minWidth = 0;
    maxWidth = 0;
}

void TableBox::build(BoxLayer* layer)
{
    auto addColumn = [this](TableColumnBox* column) {
        auto columnSpanCount = column->span();
        assert(columnSpanCount > 0);
        while(--columnSpanCount) {
            m_columns.emplace_back(column);
        }
    };

    for(auto child = firstBox(); child; child = child->nextBox()) {
        if(auto column = to<TableColumnBox>(child)) {
            if(column->display() == Display::TableColumn) {
                addColumn(column);
            } else {
                if(auto child = column->firstBox()) {
                    do {
                        if(auto column = to<TableColumnBox>(child))
                            addColumn(column);
                        child = child->nextBox();
                    } while(child);
                } else {
                    addColumn(column);
                }
            }
        }
    }

    auto addSection = [this](TableSectionBox* section) {
        for(auto box = section->firstBox(); box; box = box->nextBox()) {
            assert(box->isTableRowBox());
            auto rowBox = to<TableRowBox>(box);
            m_rows.emplace_back(rowBox);
        }
    };

    for(auto child = firstBox(); child; child = child->nextBox()) {
        if(auto section = to<TableSectionBox>(child)) {
            if(child->display() == Display::TableHeaderGroup) {
                addSection(section);
                break;
            }
        }
    }

    for(auto child = firstBox(); child; child = child->nextBox()) {
        if(auto section = to<TableSectionBox>(child)) {
            if(child->display() == Display::TableRowGroup) {
                addSection(section);
            }
        }
    }

    for(auto child = firstBox(); child; child = child->nextBox()) {
        if(auto section = to<TableSectionBox>(child)) {
            if(child->display() == Display::TableFooterGroup) {
                addSection(section);
                break;
            }
        }
    }

    auto rowCount = m_rows.size();
    for(size_t rowIndex = 0; rowIndex < rowCount; ++rowIndex) {
        auto rowBox = m_rows[rowIndex].box();

        size_t columnIndex = 0;
        for(auto box = rowBox->firstBox(); box; box = box->nextBox()) {
            assert(box->isTableCellBox());
            auto cellBox = to<TableCellBox>(box);

            auto& cells = rowBox->cells();
            while(true) {
                if(!cells.contains(columnIndex))
                    break;
                ++columnIndex;
            }

            if(cellBox->rowSpan() == 0) {
                cellBox->setRowSpan(rowCount - rowIndex);
            } else {
                cellBox->setRowSpan(std::min(rowCount - rowIndex, cellBox->rowSpan()));
            }

            for(size_t row = 0; row < cellBox->rowSpan(); ++row) {
                auto& cells = m_rows[row + rowIndex].cells();
                for(size_t col = 0; col < cellBox->colSpan(); ++col) {
                    cells.emplace(col + columnIndex, TableCell(cellBox, col > 0, row > 0));
                }
            }

            columnIndex += cellBox->colSpan();
            while(columnIndex > m_columns.size()) {
                m_columns.emplace_back(nullptr);
            }
        }
    }

    BlockBox::build(layer);
}

void TableBox::layout()
{
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
    , m_cells(style->heap())
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

TableCellBox::TableCellBox(Node* node, const RefPtr<BoxStyle>& style)
    : BlockFlowBox(node, style)
{
}

TableCaptionBox::TableCaptionBox(Node* node, const RefPtr<BoxStyle>& style)
    : BlockFlowBox(node, style), m_captionSide(style->captionSide())
{
}

} // namespace htmlbook
