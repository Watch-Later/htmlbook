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
    Box(Node* node, const RefPtr<BoxStyle>& style);

    virtual ~Box();
    virtual BoxList* children() const { return nullptr; }
    virtual LineBoxList* lines() const { return nullptr; }
    virtual void beginBuildingChildern() {}
    virtual void finishBuildingChildern() {}
    virtual void addBox(Box* box, Box* nextBox = nullptr);
    virtual void removeBox(Box* box);

    void addLine(LineBox* line);
    void removeLine(LineBox* line);

    Box* firstBox() const;
    Box* lastBox() const;

    LineBox* firstLine() const;
    LineBox* lastLine() const;

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

    Box* firstBox() const { return m_firstBox; }
    Box* lastBox() const { return m_lastBox; }

    void insert(Box* parent, Box* box, Box* nextBox);
    void append(Box* parent, Box* box);
    void remove(Box* parent, Box* box);
    bool empty() const { return !m_firstBox; }

private:
    Box* m_firstBox{nullptr};
    Box* m_lastBox{nullptr};
};

class FlowLineBox;
class RootLineBox;

class LineBox {
public:
    LineBox(Box* box);
    virtual ~LineBox();

    Box* box() const { return m_box; }
    FlowLineBox* parentLine() const { return m_parentLine; }
    LineBox* nextOnLine() const { return m_nextOnLine; }
    LineBox* prevOnLine() const { return m_prevOnLine; }
    LineBox* nextOnBox() const { return m_nextOnBox; }
    LineBox* prevOnBox() const { return m_prevOnBox; }

    void setParentLine(FlowLineBox* line) { m_parentLine = line; }
    void setNextOnLine(LineBox* line) { m_nextOnLine = line; }
    void setPrevOnLine(LineBox* line) { m_prevOnLine = line; }
    void setNextOnBox(LineBox* line) { m_nextOnBox = line; }
    void setPrevOnBox(LineBox* line) { m_prevOnBox = line; }

    RootLineBox* rootLine() const;

private:
    Box* m_box;
    FlowLineBox* m_parentLine{nullptr};
    LineBox* m_nextOnLine{nullptr};
    LineBox* m_prevOnLine{nullptr};
    LineBox* m_nextOnBox{nullptr};
    LineBox* m_prevOnBox{nullptr};
};

class TextLineBox : public LineBox {
public:
    TextLineBox(Box* box, int begin, int end);

    int begin() const { return m_begin; }
    int end() const { return m_end; }

private:
    int m_begin;
    int m_end;
};

class BoxFrame;

class PlaceHolderLineBox : public LineBox {
public:
    PlaceHolderLineBox(Box* box, BoxFrame* placeHolderBox);
    ~PlaceHolderLineBox() override;

    BoxFrame* placeHolderBox() const { return m_placeHolderBox; }

private:
    BoxFrame* m_placeHolderBox;
};

class FlowLineBox : public LineBox {
public:
    FlowLineBox(Box* box);
    virtual ~FlowLineBox();

    LineBox* firstLine() const { return m_firstLine; }
    LineBox* lastLine() const { return m_lastLine; }

    void addLine(LineBox* line);
    void removeLine(LineBox* line);

private:
    LineBox* m_firstLine{nullptr};
    LineBox* m_lastLine{nullptr};
};

class LineBoxList {
public:
    LineBoxList() = default;
    ~LineBoxList();

    LineBox* firstLine() const { return m_firstLine; }
    LineBox* lastLine() const { return m_lastLine; }

    void add(Box* box, LineBox* line);
    void remove(Box* box, LineBox* line);
    bool empty() const { return !m_firstLine; }

private:
    LineBox* m_firstLine{nullptr};
    LineBox* m_lastLine{nullptr};
};

class InlineTextBox : public Box {
public:
    InlineTextBox(Node* node, const RefPtr<BoxStyle>& style);

    LineBoxList* lines() const final { return &m_lines; }
    const std::string& text() const { return m_text; }
    void setText(std::string text) { m_text = std::move(text); }

private:
    std::string m_text;
    mutable LineBoxList m_lines;
};

class BoxLayer;

class BoxModel : public Box {
public:
    BoxModel(Node* node, const RefPtr<BoxStyle>& style);
    ~BoxModel() override;

    BoxLayer* layer() const { return m_layer; }
    void setLayer(BoxLayer* layer) { m_layer = layer; }

private:
    BoxLayer* m_layer;
};

class BoxFrame : public BoxModel {
public:
    BoxFrame(Node* node, const RefPtr<BoxStyle>& style);
    ~BoxFrame() override;

    LineBox* line() const { return m_line; }
    void setLine(LineBox* line) { m_line = line; }

private:
    LineBox* m_line{nullptr};
};

class InlineBox : public BoxModel {
public:
    InlineBox(Node* node, const RefPtr<BoxStyle>& style);

    BoxList* children() const final { return &m_children; }
    LineBoxList* lines() const final { return &m_lines; }

private:
    mutable BoxList m_children;
    mutable LineBoxList m_lines;
};

class BlockBox : public BoxFrame {
public:
    BlockBox(Node* node, const RefPtr<BoxStyle>& style);

    BoxList* children() const final { return &m_children; }
    LineBoxList* lines() const final { return &m_lines; }
    const RefPtr<BoxStyle>& firstLineStyle() const { return m_firstLineStyle; }

private:
    mutable BoxList m_children;
    mutable LineBoxList m_lines;
    RefPtr<BoxStyle> m_firstLineStyle;
};

class FlexibleBox : public BlockBox {
public:
    FlexibleBox(Node* node, const RefPtr<BoxStyle>& style);
};

class ReplacedBox : public BoxFrame {
public:
    ReplacedBox(Node* node, const RefPtr<BoxStyle>& style);

    const SizeF& intrinsicSize() const { return m_intrinsicSize; }
    void setIntrinsicSize(const SizeF& size) { m_intrinsicSize = size; }

private:
    SizeF m_intrinsicSize;
};

class Image;

class ImageBox : public ReplacedBox {
public:
    ImageBox(Node* node, const RefPtr<BoxStyle>& style);

    const RefPtr<Image>& image() const { return m_image; }
    const std::string& alternativeText() const { return m_alternativeText; }

    void setImage(RefPtr<Image> image);
    void setAlternativeText(std::string text) { m_alternativeText = std::move(text); }

private:
    RefPtr<Image> m_image;
    std::string m_alternativeText;
};

class ListMarkerBox;

class ListItemBox final : public BlockBox {
public:
    ListItemBox(Node* node, const RefPtr<BoxStyle>& style);
    ~ListItemBox();

    ListMarkerBox* listMarker() const { return m_listMarker; }
    void setListMarker(ListMarkerBox* marker) { m_listMarker = marker; }

private:
    ListMarkerBox* m_listMarker{nullptr};
};

class ListMarkerBox final : public BoxFrame {
public:
    ListMarkerBox(ListItemBox* item, const RefPtr<BoxStyle>& style);

    ListItemBox* listItem() const { return m_listItem; }
    const RefPtr<Image>& image() const { return m_image; }
    const std::string& text() const { return m_text; }

    void setImage(RefPtr<Image> image);
    void setText(std::string text) { m_text = std::move(text); }

private:
    ListItemBox* m_listItem;
    RefPtr<Image> m_image;
    std::string m_text;
};

class TableSectionBox;

class TableBox final : public BlockBox {
public:
    TableBox(Node* node, const RefPtr<BoxStyle>& style);

    BlockBox* caption() const { return m_caption; }
    TableSectionBox* head() const { return m_head; }
    TableSectionBox* body() const { return m_body; }
    TableSectionBox* foot() const { return m_foot; }

private:
    BlockBox* m_caption{nullptr};
    TableSectionBox* m_head{nullptr};
    TableSectionBox* m_body{nullptr};
    TableSectionBox* m_foot{nullptr};
};

class TableCellBox final : public BlockBox {
public:
    TableCellBox(Node* node, const RefPtr<BoxStyle>& style);

    int colSpan() const { return m_colSpan; }
    int rowSpan() const { return m_rowSpan; }

    void setColSpan(int span) { m_colSpan = span; }
    void setRowSpan(int span) { m_rowSpan = span; }

private:
    int m_colSpan{1};
    int m_rowSpan{1};
};

class TableColumnBox : public Box {
public:
    TableColumnBox(Node* node, const RefPtr<BoxStyle>& style);

    int span() const { return m_span; }
    void setSpan(int span) { m_span = span; }

private:
    int m_span{1};
};

class TableColumnGroupBox final : public TableColumnBox {
public:
    TableColumnGroupBox(Node* node, const RefPtr<BoxStyle>& style);

    BoxList* children() const final { return &m_children; }

private:
    mutable BoxList m_children;
};

class TableRowBox final : public BoxFrame {
public:
    TableRowBox(Node* node, const RefPtr<BoxStyle>& style);

    BoxList* children() const final { return &m_children; }

private:
    mutable BoxList m_children;
};

class TableSectionBox final : public BoxFrame {
public:
    TableSectionBox(Node* node, const RefPtr<BoxStyle>& style);

    BoxList* children() const final { return &m_children; }

private:
    mutable BoxList m_children;
};

} // namespace htmlbook

#endif // BOX_H
