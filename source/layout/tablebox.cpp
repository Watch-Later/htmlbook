#include "tablebox.h"

namespace htmlbook {

TableBox::TableBox(Node* node, const RefPtr<BoxStyle>& style)
    : BlockBox(node, style)
    , m_sections(style->heap())
    , m_columns(style->heap())
    , m_borderCollapse(style->borderCollapse())
{
    setChildrenInline(false);
}

void TableBox::computePreferredWidths(float& minWidth, float& maxWidth) const
{
    m_tableLayout->computePreferredWidths(minWidth, maxWidth);
}

void TableBox::build(BoxLayer* layer)
{
    auto addColumn = [this](TableColumnBox* column) {
        auto columnSpanCount = column->span();
        while(columnSpanCount--) {
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

    if(m_borderCollapse == BorderCollapse::Separate) {
        m_horizontalBorderSpacing = style()->borderHorizontalSpacing();
        m_verticalBorderSpacing = style()->borderVerticalSpacing();
    }

    TableSectionBox* headerSection = nullptr;
    TableSectionBox* footerSection = nullptr;
    for(auto box = firstBox(); box; box = box->nextBox()) {
        if(auto section = to<TableSectionBox>(box)) {
            switch(section->display()) {
            case Display::TableHeaderGroup:
                if(!headerSection)
                    headerSection = section;
                break;
            case Display::TableFooterGroup:
                if(!footerSection)
                    footerSection = section;
                break;
            case Display::TableRowGroup:
                m_sections.push_back(section);
                break;
            default:
                assert(false);
            }
        }
    }

    if(headerSection)
        m_sections.push_front(headerSection);
    if(footerSection) {
        m_sections.push_back(footerSection);
    }

    BlockBox::build(layer);

    m_tableLayout = TableLayoutAlgorithm::create(this);
    m_tableLayout->build();
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

std::unique_ptr<TableLayoutAlgorithm> TableLayoutAlgorithm::create(TableBox* table)
{
    const auto& tableStyle = table->style();
    if(tableStyle->tableLayout() == TableLayout::Fixed && !tableStyle->width().isAuto())
        return TableLayoutAlgorithmFixed::create(table);
    return TableLayoutAlgorithmAuto::create(table);
}

std::unique_ptr<TableLayoutAlgorithmFixed> TableLayoutAlgorithmFixed::create(TableBox* table)
{
    return std::unique_ptr<TableLayoutAlgorithmFixed>(new (table->heap()) TableLayoutAlgorithmFixed(table));
}

void TableLayoutAlgorithmFixed::computePreferredWidths(float& minWidth, float& maxWidth) const
{
    minWidth = 0;
    maxWidth = 0;
}

void TableLayoutAlgorithmFixed::build()
{
}

void TableLayoutAlgorithmFixed::layout()
{
}

TableLayoutAlgorithmFixed::TableLayoutAlgorithmFixed(TableBox* table)
    : TableLayoutAlgorithm(table)
    , m_widths(table->heap())
{
}

std::unique_ptr<TableLayoutAlgorithmAuto> TableLayoutAlgorithmAuto::create(TableBox* table)
{
    return std::unique_ptr<TableLayoutAlgorithmAuto>(new (table->heap()) TableLayoutAlgorithmAuto(table));
}

void TableLayoutAlgorithmAuto::computePreferredWidths(float& minWidth, float& maxWidth) const
{
    minWidth = 0;
    maxWidth = 0;
}

void TableLayoutAlgorithmAuto::build()
{
}

void TableLayoutAlgorithmAuto::layout()
{
}

TableLayoutAlgorithmAuto::TableLayoutAlgorithmAuto(TableBox* table)
    : TableLayoutAlgorithm(table)
    , m_widths(table->heap())
{
}

TableSectionBox::TableSectionBox(Node* node, const RefPtr<BoxStyle>& style)
    : Box(node, style)
    , m_rows(style->heap())
{
    setHasTransform(style->hasTransform());
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

void TableSectionBox::build(BoxLayer* layer)
{
    for(auto box = firstBox(); box; box = box->nextBox()) {
        assert(box->isTableRowBox());
        auto rowBox = to<TableRowBox>(box);
        rowBox->setRowIndex(m_rows.size());
        m_rows.push_back(rowBox);
    }

    const uint32_t rowCount = m_rows.size();
    for(uint32_t rowIndex = 0; rowIndex < rowCount; ++rowIndex) {
        const auto rowBox = m_rows[rowIndex];

        uint32_t columnIndex = 0;
        for(auto box = rowBox->firstBox(); box; box = box->nextBox()) {
            assert(box->isTableCellBox());
            auto cellBox = to<TableCellBox>(box);

            auto& cells = rowBox->cells();
            while(true) {
                if(!cells.contains(columnIndex))
                    break;
                ++columnIndex;
            }

            cellBox->setColumnIndex(columnIndex);
            if(cellBox->rowSpan() == 0) {
                cellBox->setRowSpan(rowCount - rowIndex);
            } else {
                cellBox->setRowSpan(std::min(rowCount - rowIndex, cellBox->rowSpan()));
            }

            for(uint32_t row = 0; row < cellBox->rowSpan(); ++row) {
                auto& cells = m_rows[row + rowIndex]->cells();
                for(uint32_t col = 0; col < cellBox->colSpan(); ++col) {
                    cells.emplace(col + columnIndex, TableCell(cellBox, row > 0, col > 0));
                }
            }

            columnIndex += cellBox->colSpan();

            auto& columns = table()->columns();
            while(columnIndex > columns.size()) {
                columns.emplace_back(nullptr);
            }
        }
    }
}

TableRowBox::TableRowBox(Node* node, const RefPtr<BoxStyle>& style)
    : Box(node, style)
    , m_cells(style->heap())
{
    setHasTransform(style->hasTransform());
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
    : BlockFlowBox(node, style)
    , m_captionSide(style->captionSide())
{
}

} // namespace htmlbook
