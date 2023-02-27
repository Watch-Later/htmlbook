#ifndef TABLEBOX_H
#define TABLEBOX_H

#include "blockbox.h"

namespace htmlbook {

class TableCellBox;
class TableColumnBox;
class TableRowBox;

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

using TableCellMap = std::pmr::multimap<size_t, TableCell>;

class TableRow {
public:
    explicit TableRow(TableRowBox* box)
        : m_box(box)
    {}

    TableRowBox* box() const { return m_box; }
    TableCellMap& cells() const;

private:
    TableRowBox* m_box;
};

using TableRowList = std::pmr::vector<TableRow>;

class TableColumn {
public:
    explicit TableColumn(const TableColumnBox* box)
        : m_box(box)
    {}

    const TableColumnBox* box() const { return m_box; }

private:
    const TableColumnBox* m_box;
};

using TableColumnList = std::pmr::vector<TableColumn>;

class TableBox final : public BlockBox {
public:
    TableBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const final { return type == Type::Table || BlockBox::isOfType(type); }

    void computePreferredWidths(float& minWidth, float& maxWidth) const final;

    void addBox(Box* box) final;
    void build(BoxLayer* layer) final;
    void layout() final;

    const TableColumnList& columns() const { return m_columns; }
    TableColumnList& columns() { return m_columns; }

    const char* name() const final { return "TableBox"; }

private:
    TableColumnList m_columns;
};

template<>
struct is_a<TableBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::Table); }
};

class TableSectionBox final : public Box {
public:
    TableSectionBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const final { return type == Type::TableSection || Box::isOfType(type); }

    void addBox(Box* box) final;
    void build(BoxLayer* layer) final;

    TableBox* table() const;

    const TableRowList& rows() const { return m_rows; }
    TableRowList& rows() { return m_rows; }

    const char* name() const final { return "TableSectionBox"; }

private:
    TableRowList m_rows;
};

template<>
struct is_a<TableSectionBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::TableSection); }
};

inline TableBox* TableSectionBox::table() const
{
    return static_cast<TableBox*>(parentBox());
}

class TableRowBox final : public Box {
public:
    TableRowBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const final { return type == Type::TableRow || Box::isOfType(type); }

    void addBox(Box* box) final;

    TableSectionBox* section() const;
    TableBox* table() const { return section()->table(); }

    const TableCellMap& cells() const { return m_cells; }
    TableCellMap& cells()  { return m_cells; }

    const char* name() const final { return "TableRowBox"; }

private:
    TableCellMap m_cells;
};

template<>
struct is_a<TableRowBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::TableRow); }
};

inline TableSectionBox* TableRowBox::section() const
{
    return static_cast<TableSectionBox*>(parentBox());
}

inline TableCellMap& TableRow::cells() const
{
    return m_box->cells();
}

class TableColumnBox : public Box {
public:
    TableColumnBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const override { return type == Type::TableColumn || Box::isOfType(type); }

    int span() const { return m_span; }
    void setSpan(int span) { m_span = span; }

    const char* name() const override { return "TableColumnBox"; }

private:
    int m_span{1};
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

    size_t colSpan() const { return m_colSpan; }
    size_t rowSpan() const { return m_rowSpan; }

    void setColSpan(int span) { m_colSpan = span; }
    void setRowSpan(int span) { m_rowSpan = span; }

    TableRowBox* row() const;
    TableSectionBox* section() const { return row()->section(); }
    TableBox* table() const { return section()->table(); }

    const char* name() const final { return "TableCellBox"; }

private:
    int m_colSpan{1};
    int m_rowSpan{1};
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
