#ifndef BOX_H
#define BOX_H

#include "pointer.h"
#include "geometry.h"

#include <string>

namespace htmlbook {

class Node;
class LineBox;
class LineBoxList;
class BoxList;
class BoxStyle;

class Box {
public:
    Box(Node* node, RefPtr<BoxStyle> style);

    virtual ~Box() = default;
    virtual BoxList* children() const { return nullptr; }
    virtual LineBoxList* lines() const { return nullptr; }

    Node* node() const { return m_node; }
    const RefPtr<BoxStyle>& style() const { return m_style; }
    Box* parentBox() const { return m_parentBox; }
    Box* prevBox() const { return m_prevBox; }
    Box* nextBox() const { return m_nextBox; }

    void setParentBox(Box* box) { m_parentBox = box; }
    void setPrevBox(Box* box) { m_prevBox = box; }
    void setNextBox(Box* box) { m_nextBox = box; }

private:
    Node* m_node;
    RefPtr<BoxStyle> m_style;
    Box* m_parentBox{nullptr};
    Box* m_prevBox{nullptr};
    Box* m_nextBox{nullptr};
};

class BoxList {
public:
    BoxList() = default;
    ~BoxList();

    Box* front() const { return m_front; }
    Box* back() const { return m_back; }

    void add(Box* box);
    void remove(Box* box);
    void clear();
    bool empty() const { return !m_front; }

private:
    Box* m_front{nullptr};
    Box* m_back{nullptr};
};

class LineBoxList {
public:
    LineBoxList() = default;
    ~LineBoxList();

    LineBox* front() const { return m_front; }
    LineBox* back() const { return m_back; }

    void add(LineBox* line);
    void remove(LineBox* line);
    void clear();
    bool empty() const { return !m_front; }

private:
    LineBox* m_front{nullptr};
    LineBox* m_back{nullptr};
};

class TextBox : public Box {
public:
    TextBox(Node* node, RefPtr<BoxStyle> style);

    LineBoxList* lines() const final { return &m_lines; }
    const std::string& text() const { return m_text; }
    void setText(std::string text);

private:
    std::string m_text;
    mutable LineBoxList m_lines;
};

class BoxLayer;

class BoxModel : public Box {
public:
    BoxModel(Node* node, RefPtr<BoxStyle> style);
    ~BoxModel() override;

    BoxLayer* layer() const { return m_layer; }
    void setLayer(BoxLayer* layer) { m_layer = layer; }

private:
    BoxLayer* m_layer;
};

class BoxFrame : public BoxModel {
public:
    BoxFrame(Node* node, RefPtr<BoxStyle> style);
    ~BoxFrame() override;

    LineBox* line() const { return m_line; }
    void setLine(LineBox* line) { m_line = line; }

private:
    LineBox* m_line{nullptr};
};

class InlineBox : public BoxModel {
public:
    InlineBox(Node* node, RefPtr<BoxStyle> style);

    BoxList* children() const final { return &m_children; }
    LineBoxList* lines() const final { return &m_lines; }

private:
    mutable BoxList m_children;
    mutable LineBoxList m_lines;
};

class BlockBox : public BoxFrame {
public:
    BlockBox(Node* node, RefPtr<BoxStyle> style);

    BoxList* children() const final { return &m_children; }
    LineBoxList* lines() const final { return &m_lines; }
    const RefPtr<BoxStyle>& firstLineStyle() const { return m_firstLineStyle; }
    void setFirstLineStyle(RefPtr<BoxStyle> style);

private:
    mutable BoxList m_children;
    mutable LineBoxList m_lines;
    RefPtr<BoxStyle> m_firstLineStyle;
};

class FlexibleBox : public BlockBox {
public:
    FlexibleBox(Node* node, RefPtr<BoxStyle> style);
};

class ReplacedBox : public BoxFrame {
public:
    ReplacedBox(Node* node, RefPtr<BoxStyle> style);

    const SizeF& intrinsicSize() const { return m_intrinsicSize; }
    void setIntrinsicSize(const SizeF& size) { m_intrinsicSize = size; }

private:
    SizeF m_intrinsicSize;
};

class Image;

class ImageBox : public ReplacedBox {
public:
    ImageBox(Node* node, RefPtr<BoxStyle> style);

    void setImage(RefPtr<Image> image);
    void setAlternativeText(std::string text) { m_alternativeText = std::move(text); }

private:
    RefPtr<Image> m_image;
    std::string m_alternativeText;
};

class ListMarkerBox;

class ListItemBox final : public BlockBox {
public:
    ListItemBox(Node* node, RefPtr<BoxStyle> style);
    ~ListItemBox();

    void setListMarker(ListMarkerBox* listMarker) { m_listMarker = listMarker; }
    ListMarkerBox* listMarker() const { return m_listMarker; }

private:
    ListMarkerBox* m_listMarker{nullptr};
};

class ListMarkerBox : public BoxFrame {
public:
    ListMarkerBox(ListItemBox* item, RefPtr<BoxStyle> style);

    ListItemBox* listItem() const { return m_listItem; }
    const RefPtr<Image>& image() const { return m_image; }
    const std::string& text() const { return m_text; }

private:
    ListItemBox* m_listItem;
    RefPtr<Image> m_image;
    std::string m_text;
};

class TableSectionBox;

class TableBox : public BlockBox {
public:
    TableBox(Node* node, RefPtr<BoxStyle> style);

    BlockBox* caption() const { return m_caption; }
    TableSectionBox* head() const { return m_head; }
    TableSectionBox* body() const { return m_body; }
    TableSectionBox* foot() const { return m_foot; }

private:
    BlockBox* m_caption;
    TableSectionBox* m_head;
    TableSectionBox* m_body;
    TableSectionBox* m_foot;
};

class TableCellBox : public BlockBox {
public:
    TableCellBox(Node* node, RefPtr<BoxStyle> style);

    int colSpan() const { return m_colSpan; }
    int rowSpan() const { return m_rowSpan; }

    void setColSpan(int span) { m_colSpan = span; }
    void setRowSpan(int span) { m_colSpan = span; }

private:
    int m_colSpan{1};
    int m_rowSpan{1};
};

class TableColumnBox : public Box {
public:
    TableColumnBox(Node* node, RefPtr<BoxStyle> style);

    int span() const { return m_span; }
    void setSpan(int span) { m_span = span; }

private:
    int m_span{1};
};

class TableColumnGroupBox : public TableColumnBox {
public:
    TableColumnGroupBox(Node* node, RefPtr<BoxStyle> style);

    BoxList* children() const { return &m_children; }

private:
    mutable BoxList m_children;
};

class TableRowBox : public BoxFrame {
public:
    TableRowBox(Node* node, RefPtr<BoxStyle> style);

    BoxList* children() const { return &m_children; }

private:
    mutable BoxList m_children;
};

class TableSectionBox : public BoxFrame {
public:
    TableSectionBox(Node* node, RefPtr<BoxStyle> style);

    BoxList* children() const { return &m_children; }

private:
    mutable BoxList m_children;
};

} // namespace htmlbook

#endif // BOX_H
