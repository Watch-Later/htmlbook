#include "tablebox.h"

namespace htmlbook {

TableBox::TableBox(Node* node, const RefPtr<BoxStyle>& style)
    : BlockBox(node, style)
    , m_captions(style->heap())
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

    TableSectionBox* headerSection = nullptr;
    TableSectionBox* footerSection = nullptr;
    for(auto child = firstBox(); child; child = child->nextBox()) {
        if(auto section = to<TableSectionBox>(child)) {
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
        } else if(auto column = to<TableColumnBox>(child)) {
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
        } else if(auto caption = to<TableCaptionBox>(child)) {
            m_captions.push_back(caption);
        }
    }

    if(headerSection)
        m_sections.push_front(headerSection);
    if(footerSection) {
        m_sections.push_back(footerSection);
    }

    if(m_borderCollapse == BorderCollapse::Separate) {
        m_horizontalBorderSpacing = style()->borderHorizontalSpacing();
        m_verticalBorderSpacing = style()->borderVerticalSpacing();
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
    if(tableStyle->tableLayout() == TableLayout::Auto || tableStyle->width().isAuto())
        return AutoTableLayoutAlgorithm::create(table);
    return FixedTableLayoutAlgorithm::create(table);
}

std::unique_ptr<FixedTableLayoutAlgorithm> FixedTableLayoutAlgorithm::create(TableBox* table)
{
    return std::unique_ptr<FixedTableLayoutAlgorithm>(new (table->heap()) FixedTableLayoutAlgorithm(table));
}

void FixedTableLayoutAlgorithm::computePreferredWidths(float& minWidth, float& maxWidth) const
{
    for(auto& width : m_widths) {
        if(width.isFixed()) {
            minWidth += width.value() + m_table->horizontalBorderSpacing();;
            maxWidth += width.value() + m_table->horizontalBorderSpacing();;
        }
    }
}

void FixedTableLayoutAlgorithm::build()
{
    const auto& columns = m_table->columns();

    m_widths.reserve(columns.size());
    for(size_t columnIndex = 0; columnIndex < columns.size(); ++columnIndex) {
        auto columnBox = columns[columnIndex].box();
        if(columnBox == nullptr) {
            m_widths.push_back(Length::Auto);
        } else {
            m_widths.push_back(columnBox->style()->width());
        }
    }

    const TableRowBox* firstRow = nullptr;
    for(auto section : m_table->sections()) {
        const auto& rows = section->rows();
        if(!rows.empty()) {
            firstRow = rows.front();
            break;
        }
    }

    if(firstRow == nullptr)
        return;
    for(auto& [columnIndex, cell] : firstRow->cells()) {
        if(!cell.inRowSpan() && !cell.inColSpan() && m_widths[columnIndex].isAuto()) {
            auto cellBox = cell.box();
            auto cellStyle = cellBox->style();
            auto cellStyleWidth = cellStyle->width();
            if(cellStyleWidth.isAuto())
                continue;
            for(size_t col = 0; col < cellBox->colSpan(); ++col) {
                m_widths[col + columnIndex] = cellStyleWidth;
            }
        }
    }
}

void FixedTableLayoutAlgorithm::layout()
{
    auto availableWidth = m_table->availableWidth();

    float totalFixedWidth = 0;
    float totalPercentWidth = 0;

    size_t autoWidthCount = 0;

    auto& columns = m_table->columns();
    for(size_t columnIndex = 0; columnIndex < columns.size(); ++columnIndex) {
        auto& column = columns[columnIndex];

        auto& width = m_widths[columnIndex];
        if(width.isFixed()) {
            column.setWidth(width.value());
            totalFixedWidth += column.width();
        } else if(width.isPercent()) {
            column.setWidth(width.calc(availableWidth));
            totalPercentWidth += column.width();
        } else if(width.isAuto()) {
            column.setWidth(0);
            autoWidthCount += 1;
        }
    }

    auto totalWidth = totalFixedWidth + totalPercentWidth;
    if(autoWidthCount == 0 || totalWidth > availableWidth) {
        if(totalFixedWidth > 0 && totalWidth < availableWidth) {
            auto availableFixedWidth = availableWidth - totalPercentWidth;

            auto totalFixed = totalFixedWidth;

            totalFixedWidth = 0;
            for(size_t columnIndex = 0; columnIndex < columns.size(); ++columnIndex) {
                auto& column = columns[columnIndex];

                auto& width = m_widths[columnIndex];
                if(width.isFixed()) {
                    column.setWidth(width.value() * availableFixedWidth / totalFixed);
                    totalFixedWidth += column.width();
                }
            }
        }

        if(totalPercentWidth > 0 && totalFixedWidth >= availableWidth) {
            totalPercentWidth = 0;
            for(size_t columnIndex = 0; columnIndex < columns.size(); ++columnIndex) {
                auto& column = columns[columnIndex];

                auto& width = m_widths[columnIndex];
                if(width.isPercent()) {
                    column.setWidth(0);
                }
            }
        }

        if(totalPercentWidth > 0) {
            auto availablePercentWidth = availableWidth - totalFixedWidth;

            float totalPercent = 0;
            for(auto& width : m_widths) {
                if(width.isPercent()) {
                    totalPercent += width.value();
                }
            }

            for(size_t columnIndex = 0; columnIndex < columns.size(); ++columnIndex) {
                auto& column = columns[columnIndex];

                auto& width = m_widths[columnIndex];
                if(width.isPercent()) {
                    column.setWidth(width.value() * availablePercentWidth / totalPercent);
                }
            }
        }
    } else {
        auto remainingWidth = availableWidth - totalFixedWidth - totalPercentWidth;
        for(size_t columnIndex = 0; columnIndex < columns.size(); ++columnIndex) {
            auto& column = columns[columnIndex];

            auto& width = m_widths[columnIndex];
            if(width.isAuto()) {
                column.setWidth(remainingWidth / autoWidthCount);
                remainingWidth -= column.width();
                autoWidthCount -= 1;
            }
        }
    }

    auto position = m_table->horizontalBorderSpacing();
    for(auto& column : columns) {
        column.setX(position);
        position += column.width() + m_table->horizontalBorderSpacing();
    }
}

FixedTableLayoutAlgorithm::FixedTableLayoutAlgorithm(TableBox* table)
    : TableLayoutAlgorithm(table)
    , m_widths(table->heap())
{
}

std::unique_ptr<AutoTableLayoutAlgorithm> AutoTableLayoutAlgorithm::create(TableBox* table)
{
    return std::unique_ptr<AutoTableLayoutAlgorithm>(new (table->heap()) AutoTableLayoutAlgorithm(table));
}

void AutoTableLayoutAlgorithm::computePreferredWidths(float& minWidth, float& maxWidth) const
{
    const auto& columns = m_table->columns();

    const auto columnCount = columns.size();

    std::vector<float> minWidths(columnCount, 0.f);
    std::vector<float> maxWidths(columnCount, 0.f);

    for(auto section : m_table->sections()) {
        for(auto row : section->rows()) {
            for(auto& [columnIndex, cell] : row->cells()) {
                auto cellBox = cell.box();
                if(!cell.inRowSpan() && !cell.inColSpan() && cellBox->colSpan() == 1) {
                    minWidths[columnIndex] = std::max(minWidths[columnIndex], cellBox->minPreferredWidth());
                    maxWidths[columnIndex] = std::max(maxWidths[columnIndex], std::max(m_maxFixedWidths[columnIndex], cellBox->maxPreferredWidth()));
                }
            }
        }
    }

    for(auto cellBox : m_spanningCells) {
        auto cellMinWidth = cellBox->minPreferredWidth();
        auto cellMaxWidth = cellBox->maxPreferredWidth();
        for(auto columnIndex = cellBox->columnBegin(); columnIndex < cellBox->columnEnd(); ++columnIndex) {
            cellMinWidth -= minWidths[columnIndex];
            cellMaxWidth -= maxWidths[columnIndex];
        }

        cellMinWidth = std::max(0.f, cellMinWidth / cellBox->colSpan());
        cellMaxWidth = std::max(0.f, cellMaxWidth / cellBox->colSpan());
        for(auto columnIndex = cellBox->columnBegin(); columnIndex < cellBox->columnEnd(); ++columnIndex) {
            minWidths[columnIndex] += cellMinWidth;
            maxWidths[columnIndex] += cellMaxWidth;
        }
    }

    for(size_t columnIndex = 0; columnIndex < columnCount; ++columnIndex) {
        minWidth += minWidths[columnIndex] + m_table->horizontalBorderSpacing();
        maxWidth += maxWidths[columnIndex] + m_table->horizontalBorderSpacing();
    }
}

void AutoTableLayoutAlgorithm::build()
{
    const auto& columns = m_table->columns();

    const auto columnCount = columns.size();

    m_maxFixedWidths.resize(columnCount, -1.f);
    m_maxPercentWidths.resize(columnCount, -1.f);
    for(size_t columnIndex = 0; columnIndex < columnCount; ++columnIndex) {
        auto columnBox = columns[columnIndex].box();
        if(columnBox == nullptr)
            continue;
        auto columnStyle = columnBox->style();
        auto columnStyleWidth = columnStyle->width();
        if(columnStyleWidth.isFixed()) {
            m_maxFixedWidths[columnIndex] = columnStyleWidth.value();
        } else if(columnStyleWidth.isPercent()) {
            m_maxPercentWidths[columnIndex] = columnStyleWidth.value();
        }
    }

    for(auto section : m_table->sections()) {
        for(auto row : section->rows()) {
            for(auto& [columnIndex, cell] : row->cells()) {
                if(cell.inRowSpan() || cell.inColSpan())
                    continue;
                auto cellBox = cell.box();
                if(cellBox->colSpan() > 1) {
                    m_spanningCells.push_back(cellBox);
                    continue;
                }

                auto cellStyle = cellBox->style();
                auto cellStyleWidth = cellStyle->width();
                if(cellStyleWidth.isFixed()) {
                    m_maxFixedWidths[columnIndex] = std::max(m_maxFixedWidths[columnIndex], cellStyleWidth.value());
                } else if(cellStyleWidth.isPercent()) {
                    m_maxPercentWidths[columnIndex] = std::max(m_maxPercentWidths[columnIndex], cellStyleWidth.value());
                }
            }
        }
    }

    std::sort(m_spanningCells.begin(), m_spanningCells.end(), [](auto a, auto b) { return a->colSpan() < b->colSpan(); });
}

void AutoTableLayoutAlgorithm::layout()
{
    auto& columns = m_table->columns();

    const auto columnCount = columns.size();

    std::vector<Length> widths(columnCount, Length::Auto);

    std::vector<float> minWidths(columnCount, 0.f);
    std::vector<float> maxWidths(columnCount, 0.f);

    for(auto section : m_table->sections()) {
        for(auto row : section->rows()) {
            for(auto& [columnIndex, cell] : row->cells()) {
                auto cellBox = cell.box();
                if(!cell.inRowSpan() && !cell.inColSpan() && cellBox->colSpan() == 1) {
                    if(m_maxFixedWidths[columnIndex] >= 0)
                        widths[columnIndex] = Length(Length::Type::Fixed, m_maxFixedWidths[columnIndex]);
                    if(m_maxPercentWidths[columnIndex] >= 0)
                        widths[columnIndex] = Length(Length::Type::Percent, m_maxPercentWidths[columnIndex]);

                    minWidths[columnIndex] = std::max(minWidths[columnIndex], cellBox->minPreferredWidth());
                    maxWidths[columnIndex] = std::max(maxWidths[columnIndex], std::max(m_maxFixedWidths[columnIndex], cellBox->maxPreferredWidth()));
                }
            }
        }
    }
}

AutoTableLayoutAlgorithm::AutoTableLayoutAlgorithm(TableBox* table)
    : TableLayoutAlgorithm(table)
    , m_spanningCells(table->heap())
    , m_maxFixedWidths(table->heap())
    , m_maxPercentWidths(table->heap())
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

    Box::build(layer);
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
