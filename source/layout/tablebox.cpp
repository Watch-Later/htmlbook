#include "tablebox.h"

#include <span>

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
    updateWidth();

    m_tableLayout->layout();

    setHeight(0.f);
    auto layoutCaption = [this](TableCaptionBox* caption) {
        caption->layout();
        caption->setX(caption->marginLeft());
        caption->setY(height() + caption->collapsedMarginTop());
        setHeight(caption->y() + caption->height() + caption->collapsedMarginBottom());
    };

    for(auto caption : m_captions) {
        if(caption->captionSide() == CaptionSide::Top) {
            layoutCaption(caption);
        }
    }

    setHeight(height() + borderAndPaddingTop());
    for(auto section : m_sections) {
        section->layout();
        section->setY(height());
        setHeight(section->y() + section->height());
    }

    setHeight(height() + verticalBorderSpacing() + borderAndPaddingBottom());
    for(auto caption : m_captions) {
        if(caption->captionSide() == CaptionSide::Bottom) {
            layoutCaption(caption);
        }
    }

    updateHeight();
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

void FixedTableLayoutAlgorithm::computePreferredWidths(float& minWidth, float& maxWidth)
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

    const TableRowBox* firstRowBox = nullptr;
    for(auto section : m_table->sections()) {
        const auto& rows = section->rows();
        if(!rows.empty()) {
            firstRowBox = rows.front().box();
            break;
        }
    }

    if(firstRowBox == nullptr)
        return;
    for(auto& [columnIndex, cell] : firstRowBox->cells()) {
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

    size_t autoColumnCount = 0;

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
            autoColumnCount++;
        }
    }

    auto totalWidth = totalFixedWidth + totalPercentWidth;
    if(autoColumnCount == 0 || totalWidth > availableWidth) {
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
                column.setWidth(remainingWidth / autoColumnCount);
                remainingWidth -= column.width();
                autoColumnCount--;
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

static std::vector<float> distributeWidthToColumns(float availableWidth, std::span<TableColumnWidth> columns)
{
    size_t fixedColumnCount = 0;
    size_t percentColumnCount = 0;
    size_t autoColumnCount = 0;

    float totalPercent = 0.f;
    float totalFixedMaxWidth = 0.f;
    float totalAutoMaxWidth = 0.f;

    enum { MinGuess, PercentageGuess, SpecifiedGuess, MaxGuess, AboveMax };

    float guessSizes[AboveMax] = {0.f, 0.f, 0.f, 0.f};
    float guessIncreases[AboveMax] = {0.f, 0.f, 0.f, 0.f};

    for(const auto& column : columns) {
        if(column.width.isPercent()) {
            auto percentWidth = std::max(column.minWidth, column.width.calc(availableWidth));

            guessSizes[MinGuess] += column.minWidth;
            guessSizes[PercentageGuess] += percentWidth;
            guessSizes[SpecifiedGuess] += percentWidth;
            guessSizes[MaxGuess] += percentWidth;
            guessIncreases[PercentageGuess] += percentWidth - column.minWidth;

            totalPercent += column.width.value();
            percentColumnCount++;
        } else if(column.width.isFixed()) {
            guessSizes[MinGuess] += column.minWidth;
            guessSizes[PercentageGuess] += column.minWidth;
            guessSizes[SpecifiedGuess] += column.maxWidth;
            guessSizes[MaxGuess] += column.maxWidth;
            guessIncreases[SpecifiedGuess] += column.maxWidth - column.minWidth;

            totalFixedMaxWidth += column.maxWidth;
            fixedColumnCount++;
        } else {
            guessSizes[MinGuess] += column.minWidth;
            guessSizes[PercentageGuess] += column.minWidth;
            guessSizes[SpecifiedGuess] += column.minWidth;
            guessSizes[MaxGuess] += column.maxWidth;
            guessIncreases[MaxGuess] += column.maxWidth - column.minWidth;

            totalAutoMaxWidth += column.maxWidth;
            autoColumnCount++;
        }
    }

    availableWidth = std::max(availableWidth, guessSizes[MinGuess]);

    auto startingGuess = AboveMax;

    if(guessSizes[MinGuess] >= availableWidth) { startingGuess = MinGuess; }
    else if(guessSizes[PercentageGuess] >= availableWidth) { startingGuess = PercentageGuess; }
    else if(guessSizes[SpecifiedGuess] >= availableWidth) { startingGuess = SpecifiedGuess; }
    else if(guessSizes[MaxGuess] >= availableWidth) { startingGuess = MaxGuess; }

    std::vector<float> widths(columns.size());
    if(startingGuess == MinGuess) {
        for(size_t columnIndex = 0; columnIndex < columns.size(); ++columnIndex) {
            widths[columnIndex] = columns[columnIndex].minWidth;
        }
    } else if(startingGuess == PercentageGuess) {
        auto percentWidthIncrease = guessIncreases[PercentageGuess];
        auto distributableWidth = availableWidth - guessSizes[MinGuess];
        auto remainingDeficit = distributableWidth;

        size_t lastPercentIndex = 0;
        for(size_t columnIndex = 0; columnIndex < columns.size(); ++columnIndex) {
            const auto& column = columns[columnIndex];
            if(column.width.isPercent()) {
                auto percentWidth = std::max(column.minWidth, column.width.calc(availableWidth));
                auto columnWidthIncrease = percentWidth - column.minWidth;

                float delta = 0.f;
                if(percentWidthIncrease > 0.f) {
                    delta = distributableWidth * columnWidthIncrease / percentWidthIncrease;
                } else {
                    delta = distributableWidth / percentColumnCount;
                }

                widths[columnIndex] = column.minWidth + delta;
                remainingDeficit -= delta;
                lastPercentIndex = columnIndex;
            } else {
                widths[columnIndex] = column.minWidth;
            }
        }

        widths[lastPercentIndex] += remainingDeficit;
    } else if(startingGuess == SpecifiedGuess) {
        auto fixedWidthIncrease = guessIncreases[SpecifiedGuess];
        auto distributableWidth = availableWidth - guessSizes[PercentageGuess];
        auto remainingDeficit = distributableWidth;

        size_t lastFixedIndex = 0;
        for(size_t columnIndex = 0; columnIndex < columns.size(); ++columnIndex) {
            const auto& column = columns[columnIndex];
            if(column.width.isPercent()) {
                widths[columnIndex] = std::max(column.minWidth, column.width.calc(availableWidth));
            } else if(column.width.isFixed()) {
                auto columnWidthIncrease = column.maxWidth - column.minWidth;

                float delta = 0.f;
                if(fixedWidthIncrease > 0.f) {
                    delta = distributableWidth * columnWidthIncrease / fixedWidthIncrease;
                } else {
                    delta = distributableWidth / fixedColumnCount;
                }

                widths[columnIndex] = column.minWidth + delta;
                remainingDeficit -= delta;
                lastFixedIndex = columnIndex;
            } else {
                widths[columnIndex] = column.minWidth;
            }
        }

        widths[lastFixedIndex] += remainingDeficit;
    } else if(startingGuess == MaxGuess) {
        auto autoWidthIncrease = guessIncreases[MaxGuess];
        auto distributableWidth = availableWidth - guessSizes[SpecifiedGuess];
        auto remainingDeficit = distributableWidth;

        size_t lastAutoIndex = 0;
        for(size_t columnIndex = 0; columnIndex < columns.size(); ++columnIndex) {
            const auto& column = columns[columnIndex];
            if(column.width.isPercent()) {
                widths[columnIndex] = std::max(column.minWidth, column.width.calc(availableWidth));
            } else if(column.width.isFixed()) {
                widths[columnIndex] = column.maxWidth;
            } else {
                auto columnWidthIncrease = column.maxWidth - column.minWidth;

                float delta = 0.f;
                if(autoWidthIncrease > 0.f) {
                    delta = distributableWidth * columnWidthIncrease / autoWidthIncrease;
                } else {
                    delta = distributableWidth / autoColumnCount;
                }

                widths[columnIndex] = column.minWidth + delta;
                remainingDeficit -= delta;
                lastAutoIndex = columnIndex;
            }
        }

        widths[lastAutoIndex] += remainingDeficit;
    } else if(startingGuess == AboveMax) {
        auto distributableWidth = availableWidth - guessSizes[MaxGuess];
        auto remainingDeficit = distributableWidth;
        if(autoColumnCount > 0) {
            size_t lastAutoIndex = 0;
            for(size_t columnIndex = 0; columnIndex < columns.size(); ++columnIndex) {
                const auto& column = columns[columnIndex];
                if(column.width.isPercent()) {
                    widths[columnIndex] = std::max(column.minWidth, column.width.calc(availableWidth));
                } else if(column.width.isFixed()) {
                    widths[columnIndex] = column.maxWidth;
                } else {
                    float delta = 0.f;
                    if(totalAutoMaxWidth > 0.f) {
                        delta = distributableWidth * column.maxWidth / totalAutoMaxWidth;
                    } else {
                        delta = distributableWidth / autoColumnCount;
                    }

                    widths[columnIndex] = column.maxWidth + delta;
                    remainingDeficit -= delta;
                    lastAutoIndex = columnIndex;
                }
            }

            widths[lastAutoIndex] += remainingDeficit;
        } else if(fixedColumnCount > 0) {
            size_t lastFixedIndex = 0;
            for(size_t columnIndex = 0; columnIndex < columns.size(); ++columnIndex) {
                const auto& column = columns[columnIndex];
                if(column.width.isPercent()) {
                    widths[columnIndex] = std::max(column.minWidth, column.width.calc(availableWidth));
                } else if(column.width.isFixed()) {
                    float delta = 0.f;
                    if(totalFixedMaxWidth > 0.f) {
                        delta = distributableWidth * column.maxWidth / totalFixedMaxWidth;
                    } else {
                        delta = distributableWidth / fixedColumnCount;
                    }

                    widths[columnIndex] = column.maxWidth + delta;
                    remainingDeficit -= delta;
                    lastFixedIndex = columnIndex;
                }
            }

            widths[lastFixedIndex] += remainingDeficit;
        } else if(percentColumnCount > 0) {
            size_t lastPercentIndex = 0;
            for(size_t columnIndex = 0; columnIndex < columns.size(); ++columnIndex) {
                const auto& column = columns[columnIndex];
                if(column.width.isPercent()) {
                    auto percentWidth = std::max(column.minWidth, column.width.calc(availableWidth));

                    float delta = 0.f;
                    if(totalPercent > 0.f) {
                        delta = distributableWidth * column.maxWidth / totalPercent;
                    } else {
                        delta = distributableWidth / percentColumnCount;
                    }

                    widths[columnIndex] = percentWidth + delta;
                    remainingDeficit -= delta;
                    lastPercentIndex = columnIndex;
                }
            }

            widths[lastPercentIndex] += remainingDeficit;
        }
    }

    return widths;
}

static void distributeSpanCellToColumns(const TableCellBox* cellBox, std::span<TableColumnWidth> allColumns, float borderSpacing)
{
    auto columns = allColumns.subspan(cellBox->columnIndex(), cellBox->colSpan());

    auto cellStyle = cellBox->style();
    auto cellStyleWidth = cellStyle->width();
    if(cellStyleWidth.isPercent()) {
        float totalPercent = 0.f;
        float totalNonPercentMaxWidth = 0.f;

        size_t percentColumnCount = 0;
        size_t nonPercentColumnCount = 0;
        for(const auto& column : columns) {
            if(column.width.isPercent()) {
                totalPercent += column.width.value();
                percentColumnCount++;
            } else {
                totalNonPercentMaxWidth += column.maxWidth;
                nonPercentColumnCount++;
            }
        }

        auto surplusPercent = cellStyleWidth.value() - totalPercent;
        if(surplusPercent > 0.f && nonPercentColumnCount > 0) {
            for(auto& column : columns) {
                if(column.width.isPercent())
                    continue;

                float delta = 0.f;
                if(totalNonPercentMaxWidth > 0.f) {
                    delta = surplusPercent * column.maxWidth / totalNonPercentMaxWidth;
                } else {
                    delta = surplusPercent / nonPercentColumnCount;
                }

                column.width = Length(Length::Type::Percent, delta);
            }
        }
    }

    auto cellMinWidth = std::max(0.f, cellBox->minPreferredWidth() - borderSpacing * (cellBox->colSpan() - 1));
    auto cellMaxWidth = std::max(0.f, cellBox->maxPreferredWidth() - borderSpacing * (cellBox->colSpan() - 1));

    const auto minWidths = distributeWidthToColumns(cellMinWidth, columns);
    for(size_t columnIndex = 0; columnIndex < columns.size(); ++columnIndex) {
        columns[columnIndex].minWidth = std::max(columns[columnIndex].minWidth, minWidths[columnIndex]);
    }

    const auto maxWidths = distributeWidthToColumns(cellMaxWidth, columns);
    for(size_t columnIndex = 0; columnIndex < columns.size(); ++columnIndex) {
        columns[columnIndex].maxWidth = std::max(columns[columnIndex].maxWidth, maxWidths[columnIndex]);
    }
}

void AutoTableLayoutAlgorithm::computePreferredWidths(float& minWidth, float& maxWidth)
{
    for(auto& columnWidth : m_columnWidths) {
        columnWidth.width = Length::Auto;
        columnWidth.minWidth = 0.f;
        columnWidth.maxWidth = 0.f;
    }

    for(auto section : m_table->sections()) {
        for(auto& row : section->rows()) {
            for(auto& [columnIndex, cell] : row.cells()) {
                auto cellBox = cell.box();
                if(!cell.inRowSpan() && !cell.inColSpan() && cellBox->colSpan() == 1) {
                    auto& columnWidth = m_columnWidths[columnIndex];
                    if(columnWidth.maxFixedWidth >= 0.f)
                        columnWidth.width = Length(Length::Type::Fixed, columnWidth.maxFixedWidth);
                    if(columnWidth.maxPercentWidth > 0.f)
                        columnWidth.width = Length(Length::Type::Percent, columnWidth.maxPercentWidth);

                    columnWidth.minWidth = std::max(columnWidth.minWidth, cellBox->minPreferredWidth());
                    columnWidth.maxWidth = std::max(columnWidth.maxWidth, std::max(columnWidth.maxFixedWidth, cellBox->maxPreferredWidth()));
                }
            }
        }
    }

    for(auto cellBox : m_spanningCells) {
        distributeSpanCellToColumns(cellBox, m_columnWidths, m_table->horizontalBorderSpacing());
    }

    for(auto& columnWidth : m_columnWidths) {
        minWidth += columnWidth.minWidth + m_table->horizontalBorderSpacing();
        maxWidth += columnWidth.maxWidth + m_table->horizontalBorderSpacing();
    }
}

void AutoTableLayoutAlgorithm::build()
{
    const auto& columns = m_table->columns();

    m_columnWidths.resize(columns.size());
    for(size_t columnIndex = 0; columnIndex < columns.size(); ++columnIndex) {
        auto columnBox = columns[columnIndex].box();
        if(columnBox == nullptr)
            continue;
        auto columnStyle = columnBox->style();
        auto columnStyleWidth = columnStyle->width();

        auto& columnWidth = m_columnWidths[columnIndex];
        if(columnStyleWidth.isFixed()) {
            columnWidth.maxFixedWidth = columnStyleWidth.value();
        } else if(columnStyleWidth.isPercent()) {
            columnWidth.maxPercentWidth = columnStyleWidth.value();
        }
    }

    for(auto section : m_table->sections()) {
        for(auto& row : section->rows()) {
            for(auto& [columnIndex, cell] : row.cells()) {
                if(cell.inRowSpan() || cell.inColSpan())
                    continue;
                auto cellBox = cell.box();
                if(cellBox->colSpan() > 1) {
                    m_spanningCells.push_back(cellBox);
                    continue;
                }

                auto cellStyle = cellBox->style();
                auto cellStyleWidth = cellStyle->width();

                auto& columnWidth = m_columnWidths[columnIndex];
                if(cellStyleWidth.isFixed()) {
                    columnWidth.maxFixedWidth = std::max(columnWidth.maxFixedWidth, cellStyleWidth.value());
                } else if(cellStyleWidth.isPercent()) {
                    columnWidth.maxPercentWidth = std::max(columnWidth.maxPercentWidth, cellStyleWidth.value());
                }
            }
        }
    }

    std::sort(m_spanningCells.begin(), m_spanningCells.end(), [](auto a, auto b) { return a->colSpan() < b->colSpan(); });
}

void AutoTableLayoutAlgorithm::layout()
{
    m_table->updatePreferredWidths();

    const auto availableWidth = m_table->availableWidth();

    auto& columns = m_table->columns();

    const auto widths = distributeWidthToColumns(availableWidth, m_columnWidths);

    auto position = m_table->horizontalBorderSpacing();
    for(size_t columnIndex = 0; columnIndex < columns.size(); ++columnIndex) {
        auto& column = columns[columnIndex];
        column.setX(position);
        column.setWidth(widths[columnIndex]);
        position += column.width() + m_table->horizontalBorderSpacing();
    }
}

AutoTableLayoutAlgorithm::AutoTableLayoutAlgorithm(TableBox* table)
    : TableLayoutAlgorithm(table)
    , m_columnWidths(table->heap())
    , m_spanningCells(table->heap())
{
}

TableSectionBox::TableSectionBox(Node* node, const RefPtr<BoxStyle>& style)
    : Box(node, style)
    , m_rows(style->heap())
    , m_spanningCells(style->heap())
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
        m_rows.emplace_back(rowBox, rowBox->style()->height());
    }

    const uint32_t rowCount = m_rows.size();
    for(uint32_t rowIndex = 0; rowIndex < rowCount; ++rowIndex) {
        const auto rowBox = m_rows[rowIndex].box();

        uint32_t columnIndex = 0;
        for(auto box = rowBox->firstBox(); box; box = box->nextBox()) {
            assert(box->isTableCellBox());
            auto cellBox = to<TableCellBox>(box);

            const auto& cells = rowBox->cells();
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

            if(cellBox->rowSpan() > 1) {
                m_spanningCells.push_back(cellBox);
            } else {
                auto& row = m_rows[rowIndex];
                auto& rowHeight = row.height();

                auto cellStyle = cellBox->style();
                auto cellStyleHeight = cellStyle->height();
                if(cellStyleHeight.isPercent() && !cellStyleHeight.isZero()
                    && (!rowHeight.isPercent() || rowHeight.value() < cellStyleHeight.value())) {
                    row.setHeight(cellStyleHeight);
                } else if(cellStyleHeight.isFixed() && !cellStyleHeight.isZero()
                    && !rowHeight.isPercent() && rowHeight.value() < cellStyleHeight.value()) {
                    row.setHeight(cellStyleHeight);
                }
            }

            for(uint32_t row = 0; row < cellBox->rowSpan(); ++row) {
                auto& cells = m_rows[row + rowIndex].cells();
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

    std::sort(m_spanningCells.begin(), m_spanningCells.end(), [](auto a, auto b) { return a->rowSpan() < b->rowSpan(); });

    Box::build(layer);
}

void TableSectionBox::layout()
{
    const auto& columns = table()->columns();

    const auto horizontalSpacing = table()->horizontalBorderSpacing();
    const auto verticalSpacing = table()->verticalBorderSpacing();

    for(const auto& row : m_rows) {
        auto rowBox = row.box();
        auto rowHeight = row.height();
        if(rowHeight.isFixed()) {
            rowBox->setHeight(rowHeight.value());
        } else {
            rowBox->setHeight(0.f);
        }

        for(auto& [columnIndex, cell] : row.cells()) {
            auto cellBox = cell.box();
            if(cell.inRowSpan() || cell.inColSpan())
                continue;

            auto width = -horizontalSpacing;
            for(size_t col = 0; col < cellBox->colSpan(); ++col) {
                const auto& column = columns[col + columnIndex];
                width += horizontalSpacing + column.width();
            }

            cellBox->clearOverrideSize();
            cellBox->setOverrideWidth(width);
            cellBox->layout();
        }
    }

    for(size_t rowIndex = 0; rowIndex < m_rows.size(); ++rowIndex) {
        auto rowBox = m_rows[rowIndex].box();
        for(auto& [columnIndex, cell] : rowBox->cells()) {
            auto cellBox = cell.box();
            if(!cell.inRowSpan() && !cell.inColSpan() && cellBox->rowSpan() == 1) {
                rowBox->setHeight(std::max(rowBox->height(), cellBox->height()));
            }
        }
    }

    for(size_t rowIndex = 0; rowIndex < m_rows.size(); ++rowIndex) {
        for(auto& [columnIndex, cell] : m_rows[rowIndex].cells()) {
            auto cellBox = cell.box();
            if(cell.inRowSpan() || cell.inColSpan())
                continue;

            auto height = -verticalSpacing;
            for(size_t row = 0; row < cellBox->rowSpan(); ++row) {
                auto rowBox = m_rows[row + rowIndex].box();
                height += verticalSpacing + rowBox->height();
            }

            cellBox->setOverrideHeight(height);
            if(height != cellBox->height()) {
                cellBox->layout();
            }
        }
    }

    auto position = verticalSpacing;
    for(const auto& row : m_rows) {
        auto rowBox = row.box();
        rowBox->setY(position);
        position += verticalSpacing + rowBox->height();
    }

    m_height = position - verticalSpacing;
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
