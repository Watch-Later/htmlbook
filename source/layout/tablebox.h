#ifndef TABLEBOX_H
#define TABLEBOX_H

#include "blockbox.h"

namespace htmlbook {

class TableCaptionBox;
class TableSectionBox;
class TableColumnBox;

class TableBox final : public BlockBox {
public:
    TableBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const final { return type == Type::Table || BlockBox::isOfType(type); }

    bool avoidsFloats() const final { return true; }

    TableSectionBox* header() const { return m_header; }
    TableSectionBox* footer() const { return m_footer; }
    const std::vector<TableCaptionBox*>& captions() const { return m_captions; }
    const std::vector<TableSectionBox*>& sections() const { return m_sections; }
    const std::vector<TableColumnBox*>& columns() const { return m_columns; }

    void addBox(Box* box) final;
    void buildBox(BoxLayer* layer) final;

private:
    TableSectionBox* m_header{nullptr};
    TableSectionBox* m_footer{nullptr};
    std::vector<TableCaptionBox*> m_captions;
    std::vector<TableSectionBox*> m_sections;
    std::vector<TableColumnBox*> m_columns;
};

template<>
struct is_a<TableBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::Table); }
};

class TableSectionBox final : public Box {
public:
    TableSectionBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const final { return type == Type::TableSection || Box::isOfType(type); }

    BoxList* children() const final { return &m_children; }

    void addBox(Box* box) final;

private:
    mutable BoxList m_children;
};

template<>
struct is_a<TableSectionBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::TableSection); }
};

class TableRowBox final : public Box {
public:
    TableRowBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const final { return type == Type::TableRow || Box::isOfType(type); }

    BoxList* children() const final { return &m_children; }

    void addBox(Box* box) final;

private:
    mutable BoxList m_children;
};

template<>
struct is_a<TableRowBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::TableRow); }
};

class TableCellBox final : public BlockFlowBox {
public:
    TableCellBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const final { return type == Type::TableCell || BlockFlowBox::isOfType(type); }

    int colSpan() const { return m_colSpan; }
    int rowSpan() const { return m_rowSpan; }

    void setColSpan(int span) { m_colSpan = span; }
    void setRowSpan(int span) { m_rowSpan = span; }

private:
    int m_colSpan{1};
    int m_rowSpan{1};
};

template<>
struct is_a<TableCellBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::TableCell); }
};

class TableColumnBox : public Box {
public:
    TableColumnBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const override { return type == Type::TableColumn || Box::isOfType(type); }

    int span() const { return m_span; }
    void setSpan(int span) { m_span = span; }

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

    BoxList* children() const final { return &m_children; }

    void addBox(Box* box) final;

private:
    mutable BoxList m_children;
};

template<>
struct is_a<TableColumnGroupBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::TableColumnGroup); }
};

class TableCaptionBox final : public BlockFlowBox {
public:
    TableCaptionBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const final { return type == Type::TableCaption || BlockFlowBox::isOfType(type); }

    CaptionSide captionSide() const { return m_captionSide; }

private:
    CaptionSide m_captionSide;
};

template<>
struct is_a<TableCaptionBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::TableCaption); }
};

} // namespace htmlbook

#endif // TABLEBOX_H
