#ifndef TABLEBOX_H
#define TABLEBOX_H

#include "blockbox.h"

namespace htmlbook {

class TableColumnBox;

class TableColumn {
public:
    explicit TableColumn(const TableColumnBox* box)
        : m_box(box)
    {}

    const TableColumnBox* box() const { return m_box; }

    float x() const { return m_x; }
    float width() const { return m_width; }

    void setX(float x) { m_x = x; }
    void setWidth(float width) { m_width = width; }

private:
    const TableColumnBox* m_box;

    float m_x{0};
    float m_width{0};
};

using TableColumnList = std::pmr::vector<TableColumn>;

class TableCaptionBox;
class TableSectionBox;

using TableCaptionBoxList = std::pmr::list<TableCaptionBox*>;
using TableSectionBoxList = std::pmr::list<TableSectionBox*>;

class TableLayoutAlgorithm;

class TableBox final : public BlockBox {
public:
    TableBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const final { return type == Type::Table || BlockBox::isOfType(type); }

    void computePreferredWidths(float& minWidth, float& maxWidth) const final;

    void addBox(Box* box) final;
    void build(BoxLayer* layer) final;
    void layout() final;

    const TableSectionBoxList& sections() const { return m_sections; }
    const TableColumnList& columns() const { return m_columns; }
    TableColumnList& columns() { return m_columns; }

    BorderCollapse borderCollapse() const { return m_borderCollapse; }

    float horizontalBorderSpacing() const { return m_horizontalBorderSpacing; }
    float verticalBorderSpacing() const { return m_verticalBorderSpacing; }

    const char* name() const final { return "TableBox"; }

private:
    TableCaptionBoxList m_captions;
    TableSectionBoxList m_sections;
    TableColumnList m_columns;
    BorderCollapse m_borderCollapse;

    float m_horizontalBorderSpacing{0};
    float m_verticalBorderSpacing{0};

    std::unique_ptr<TableLayoutAlgorithm> m_tableLayout;
};

template<>
struct is_a<TableBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::Table); }
};

class TableLayoutAlgorithm : public HeapMember {
public:
    static std::unique_ptr<TableLayoutAlgorithm> create(TableBox* table);

    virtual ~TableLayoutAlgorithm() = default;
    virtual void computePreferredWidths(float& minWidth, float& maxWidth) const = 0;
    virtual void build() = 0;
    virtual void layout() = 0;

protected:
    TableLayoutAlgorithm(TableBox* table) : m_table(table) {}
    TableBox* m_table;
};

class FixedTableLayoutAlgorithm final : public TableLayoutAlgorithm {
public:
    static std::unique_ptr<FixedTableLayoutAlgorithm> create(TableBox* table);

    void computePreferredWidths(float& minWidth, float& maxWidth) const final;
    void build()  final;
    void layout() final;

private:
    FixedTableLayoutAlgorithm(TableBox* table);
    std::pmr::vector<Length> m_widths;
};

class TableCellBox;

class AutoTableLayoutAlgorithm final : public TableLayoutAlgorithm {
public:
    static std::unique_ptr<AutoTableLayoutAlgorithm> create(TableBox* table);

    void computePreferredWidths(float& minWidth, float& maxWidth) const final;
    void build()  final;
    void layout() final;

private:
    AutoTableLayoutAlgorithm(TableBox* table);
    std::pmr::vector<TableCellBox*> m_spanningCells;
    std::pmr::vector<float> m_maxFixedWidths;
    std::pmr::vector<float> m_maxPercentWidths;
};

class TableRowBox;

using TableRowBoxList = std::pmr::vector<TableRowBox*>;

class TableSectionBox final : public Box {
public:
    TableSectionBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const final { return type == Type::TableSection || Box::isOfType(type); }

    void addBox(Box* box) final;
    void build(BoxLayer* layer) final;

    TableBox* table() const;

    const TableRowBoxList& rows() const { return m_rows; }
    TableRowBoxList& rows() { return m_rows; }

    float x() const { return m_x; }
    float y() const { return m_y; }
    float width() const { return m_width; }
    float height() const { return m_height; }

    void setX(float x) { m_x = x; }
    void setY(float y) { m_y = y; }
    void setWidth(float width) { m_width = width; }
    void setHeight(float height) { m_height = height; }

    const char* name() const final { return "TableSectionBox"; }

private:
    TableRowBoxList m_rows;

    float m_x{0};
    float m_y{0};
    float m_width{0};
    float m_height{0};
};

template<>
struct is_a<TableSectionBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::TableSection); }
};

inline TableBox* TableSectionBox::table() const
{
    return static_cast<TableBox*>(parentBox());
}

class TableCellBox;

class TableCell {
public:
    TableCell(TableCellBox* box, bool inRowSpan, bool inColSpan)
        : m_box(box), m_inRowSpan(inRowSpan), m_inColSpan(inColSpan)
    {}

    TableCellBox* box() const { return m_box; }
    bool inRowSpan() const { return m_inRowSpan; }
    bool inColSpan() const { return m_inColSpan; }

private:
    TableCellBox* m_box;
    bool m_inRowSpan;
    bool m_inColSpan;
};

using TableCellMap = std::pmr::multimap<uint32_t, TableCell>;

class TableRowBox final : public Box {
public:
    TableRowBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const final { return type == Type::TableRow || Box::isOfType(type); }

    void addBox(Box* box) final;

    TableSectionBox* section() const;
    TableBox* table() const { return section()->table(); }

    const TableCellMap& cells() const { return m_cells; }
    TableCellMap& cells()  { return m_cells; }

    uint32_t rowIndex() const { return m_rowIndex; }
    void setRowIndex(uint32_t rowIndex) { m_rowIndex = rowIndex; }

    float y() const { return m_y; }
    float height() const { return m_height; }

    void setY(float y) { m_y = y; }
    void setHeight(float height) { m_height = height; }

    float rowBaseline() const { return m_rowBaseline; }
    void setRowBaseline(float baseline) { m_rowBaseline = baseline; }

    const char* name() const final { return "TableRowBox"; }

private:
    TableCellMap m_cells;
    uint32_t m_rowIndex{0};

    float m_y{0};
    float m_height{0};
    float m_rowBaseline{0};
};

template<>
struct is_a<TableRowBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::TableRow); }
};

inline TableSectionBox* TableRowBox::section() const
{
    return static_cast<TableSectionBox*>(parentBox());
}

class TableColumnBox : public Box {
public:
    TableColumnBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const override { return type == Type::TableColumn || Box::isOfType(type); }

    uint32_t span() const { return m_span; }
    void setSpan(uint32_t span) { m_span = span; }

    const char* name() const override { return "TableColumnBox"; }

private:
    uint32_t m_span{1};
};

template<>
struct is_a<TableColumnBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::TableColumn); }
};

class TableColumnGroupBox final : public TableColumnBox {
public:
    TableColumnGroupBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const final { return type == Type::TableColumnGroup || TableColumnBox::isOfType(type); }

    void addBox(Box* box) final;

    const char* name() const final { return "TableColumnGroupBox"; }
};

template<>
struct is_a<TableColumnGroupBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::TableColumnGroup); }
};

class TableCellBox final : public BlockFlowBox {
public:
    TableCellBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const final { return type == Type::TableCell || BlockFlowBox::isOfType(type); }
    bool avoidsFloats() const final { return true; }

    uint32_t colSpan() const { return m_colSpan; }
    uint32_t rowSpan() const { return m_rowSpan; }

    void setColSpan(uint32_t span) { m_colSpan = span; }
    void setRowSpan(uint32_t span) { m_rowSpan = span; }

    uint32_t columnIndex() const { return m_columnIndex; }
    void setColumnIndex(uint32_t columnIndex) { m_columnIndex = columnIndex; }

    uint32_t columnBegin() const { return m_columnIndex; }
    uint32_t columnEnd() const { return m_columnIndex + m_colSpan; }

    TableRowBox* row() const;
    TableSectionBox* section() const { return row()->section(); }
    TableBox* table() const { return section()->table(); }

    uint32_t rowIndex() const { return row()->rowIndex(); }

    const char* name() const final { return "TableCellBox"; }

private:
    uint32_t m_colSpan{1};
    uint32_t m_rowSpan{1};

    uint32_t m_columnIndex{0};
};

template<>
struct is_a<TableCellBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::TableCell); }
};

inline TableRowBox* TableCellBox::row() const
{
    return static_cast<TableRowBox*>(parentBox());
}

class TableCaptionBox final : public BlockFlowBox {
public:
    TableCaptionBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const final { return type == Type::TableCaption || BlockFlowBox::isOfType(type); }
    bool avoidsFloats() const final { return true; }

    CaptionSide captionSide() const { return m_captionSide; }

    const char* name() const final { return "TableCaptionBox"; }

private:
    CaptionSide m_captionSide;
};

template<>
struct is_a<TableCaptionBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::TableCaption); }
};

} // namespace htmlbook

#endif // TABLEBOX_H
