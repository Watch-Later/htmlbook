#ifndef TABLEBOX_H
#define TABLEBOX_H

#include "blockbox.h"

namespace htmlbook {

class TableCellBox;
class TableColumnBox;
class TableRowBox;

class TableCell {
public:
    TableCell(TableCellBox* box, bool inColSpan, bool inRowSpan)
        : m_box(box), m_inColSpan(inColSpan), m_inRowSpan(inRowSpan)
    {}

    TableCellBox* box() const { return m_box; }
    bool inColSpan() const { return m_inColSpan; }
    bool inRowSpan() const { return m_inRowSpan; }

private:
    TableCellBox* m_box;
    bool m_inColSpan;
    bool m_inRowSpan;
};

using TableCellMap = std::pmr::map<size_t, TableCell>;

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

class TableBox final : public BlockBox {
public:
    TableBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const final { return type == Type::Table || BlockBox::isOfType(type); }

    void computePreferredWidths(float& minWidth, float& maxWidth) const final;

    const TableRowList& rows() const { return m_rows; }
    const TableColumnList& columns() const { return m_columns; }

    void addBox(Box* box) final;
    void build(BoxLayer* layer) final;
    void layout() final;

    const char* name() const final { return "TableBox"; }

private:
    TableRowList m_rows;
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

    const char* name() const final { return "TableSectionBox"; }
};

template<>
struct is_a<TableSectionBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::TableSection); }
};

class TableRowBox final : public Box {
public:
    TableRowBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const final { return type == Type::TableRow || Box::isOfType(type); }

    void addBox(Box* box) final;

    const char* name() const final { return "TableRowBox"; }

    const TableCellMap& cells() const { return m_cells; }
    TableCellMap& cells()  { return m_cells; }

private:
    TableCellMap m_cells;
};

template<>
struct is_a<TableRowBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::TableRow); }
};

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

    const char* name() const final { return "TableCellBox"; }

private:
    int m_colSpan{1};
    int m_rowSpan{1};
};

template<>
struct is_a<TableCellBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::TableCell); }
};

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
