#ifndef BOX_H
#define BOX_H

#include "geometry.h"
#include "boxstyle.h"
#include "linebox.h"

namespace htmlbook {

class Node;
class BoxList;
class BlockBox;

class Box {
public:
    Box(Node* node, const RefPtr<BoxStyle>& style);

    virtual ~Box();
    virtual bool isTextBox() const { return false; }
    virtual bool isBoxModel() const { return false; }
    virtual bool isBoxFrame() const { return false; }
    virtual bool isInlineBox() const { return false; }
    virtual bool isBlockBox() const { return false; }
    virtual bool isFlexibleBox() const { return false; }
    virtual bool isReplacedBox() const { return false; }
    virtual bool isImageBox() const { return false; }
    virtual bool isListItemBox() const { return false; }
    virtual bool isListMarkerBox() const { return false; }
    virtual bool isTableBox() const { return false; }
    virtual bool isTableCellBox() const { return false; }
    virtual bool isTableColumnBox() const { return false; }
    virtual bool isTableColumnGroupBox() const { return false; }
    virtual bool isTableRowBox() const { return false; }
    virtual bool isTableCaptionBox() const { return false; }
    virtual bool isTableSectionBox() const { return false; }

    virtual BoxList* children() const { return nullptr; }
    virtual LineBoxList* lines() const { return nullptr; }
    virtual void beginBuildingChildern() {}
    virtual void finishBuildingChildern() {}
    virtual void addBox(Box* box);
    virtual void removeBox(Box* box);
    virtual void computePreferredWidths(float& minWidth, float& maxWidth) const;

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

    static Box* create(Node* node, const RefPtr<BoxStyle>& style);
    static Box* createAnonymous(const BoxStyle& parentStyle, Display display);
    static BlockBox* createAnonymousBlock(const BoxStyle& parentStyle);

    BlockBox* containingBlock() const;

    void moveChildrenTo(Box* to, Box* begin, Box* end);
    void moveChildrenTo(Box* to, Box* begin);
    void moveChildrenTo(Box* to);

    bool isAnonymous() const { return m_anonymous; }
    bool isReplaced() const { return m_replaced; }
    bool isInline() const { return m_inline; }
    bool isFloating() const { return m_floating; }
    bool isPositioned() const { return m_positioned; }
    bool isFloatingOrPositioned() const { return m_floating || m_positioned; }
    bool isChildrenInline() const { return m_childrenInline; }

    void setAnonymous(bool value) { m_anonymous = value; }
    void setReplaced(bool value) { m_replaced = value; }
    void setInline(bool value) { m_inline = value; }
    void setFloating(bool value) { m_floating = value; }
    void setPositioned(bool value) { m_positioned = value; }
    void setChildrenInline(bool value) { m_childrenInline = value; }

private:
    Node* m_node;
    RefPtr<BoxStyle> m_style;
    Box* m_parentBox{nullptr};
    Box* m_prevBox{nullptr};
    Box* m_nextBox{nullptr};
    bool m_anonymous{false};
    bool m_replaced{false};
    bool m_inline{true};
    bool m_floating{false};
    bool m_positioned{false};
    bool m_childrenInline{true};
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

class BoxModel;

class BoxLayer {
public:
    static std::unique_ptr<BoxLayer> create(BoxModel* box, BoxLayer* parent);

    int index() const { return m_index; }
    BoxModel* box() const { return m_box; }
    BoxLayer* parent() const { return m_parent; }

private:
    BoxLayer(BoxModel* box, BoxLayer* parent);
    int m_index;
    BoxModel* m_box;
    BoxLayer* m_parent;
    std::list<BoxLayer*> m_children;
};

class TextBox : public Box {
public:
    TextBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isTextBox() const final { return true; }

    LineBoxList* lines() const final { return &m_lines; }
    const std::string& text() const { return m_text; }
    void setText(std::string text) { m_text = std::move(text); }

private:
    std::string m_text;
    mutable LineBoxList m_lines;
};

template<>
struct is<TextBox> {
    static bool check(const Box& box) { return box.isTextBox(); }
};

class BoxModel : public Box {
public:
    BoxModel(Node* node, const RefPtr<BoxStyle>& style);

    bool isBoxModel() const final { return false; }

    BoxLayer* layer() const { return m_layer.get(); }
    void setLayer(std::unique_ptr<BoxLayer> layer) { m_layer = std::move(layer); }

    float marginTop() const { return m_marginTop; }
    float marginBottom() const { return m_marginBottom; }
    float marginLeft() const { return m_marginLeft; }
    float marginRight() const { return m_marginRight; }

    float borderTop() const { return m_borderTop; }
    float borderBottom() const { return m_borderBottom; }
    float borderLeft() const { return m_borderLeft; }
    float borderRight() const { return m_borderRight; }

    float paddingTop() const { return m_paddingTop; }
    float paddingBottom() const { return m_paddingBottom; }
    float paddingLeft() const { return m_paddingLeft; }
    float paddingRight() const { return m_paddingRight; }

    void setMarginTop(float value) { m_marginTop = value; }
    void setMarginBottom(float value) { m_marginBottom = value; }
    void setMarginLeft(float value) { m_marginLeft = value; }
    void setMarginRight(float value) { m_marginRight = value; }

    void setBorderTop(float value) { m_borderTop = value; }
    void setBorderBottom(float value) { m_borderBottom = value; }
    void setBorderLeft(float value) { m_borderLeft = value; }
    void setBorderRight(float value) { m_borderRight = value; }

    void setPaddingTop(float value) { m_paddingTop = value; }
    void setPaddingBottom(float value) { m_paddingBottom = value; }
    void setPaddingLeft(float value) { m_paddingLeft = value; }
    void setPaddingRight(float value) { m_paddingRight = value; }

private:
    std::unique_ptr<BoxLayer> m_layer;

    float m_marginTop{0};
    float m_marginBottom{0};
    float m_marginLeft{0};
    float m_marginRight{0};

    float m_borderTop{0};
    float m_borderBottom{0};
    float m_borderLeft{0};
    float m_borderRight{0};

    float m_paddingTop{0};
    float m_paddingBottom{0};
    float m_paddingLeft{0};
    float m_paddingRight{0};
};

template<>
struct is<BoxModel> {
    static bool check(const Box& box) { return box.isBoxModel(); }
};

class BoxFrame : public BoxModel {
public:
    BoxFrame(Node* node, const RefPtr<BoxStyle>& style);

    bool isBoxFrame() const final { return true; }

    LineBox* line() const { return m_line; }
    void setLine(LineBox* line) { m_line = line; }

    float x() const { return m_x; }
    float y() const { return m_y; }
    float width() const { return m_width; }
    float height() const { return m_height; }

    void setX(float x) { m_x = x; }
    void setY(float y) { m_y = y; }
    void setWidth(float width) { m_width = width; }
    void setHeight(float height) { m_height = height; }

    void move(float dx, float dy) { m_x += dx; m_y += dy; }

private:
    LineBox* m_line{nullptr};

    float m_x{0};
    float m_y{0};
    float m_width{0};
    float m_height{0};
};

template<>
struct is<BoxFrame> {
    static bool check(const Box& box) { return box.isBoxFrame(); }
};

class InlineBox : public BoxModel {
public:
    InlineBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isInlineBox() const final { return true; }

    BoxList* children() const final { return &m_children; }
    LineBoxList* lines() const final { return &m_lines; }
    Box* continuation() const { return m_continuation; }
    void setContinuation(Box* continuation) { m_continuation = continuation; }

    void addBox(Box* box) override;

private:
    mutable BoxList m_children;
    mutable LineBoxList m_lines;
    Box* m_continuation{nullptr};
};

template<>
struct is<InlineBox> {
    static bool check(const Box& box) { return box.isInlineBox(); }
};

class BlockBox : public BoxFrame {
public:
    BlockBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isBlockBox() const final { return true; }

    BoxList* children() const final { return &m_children; }
    LineBoxList* lines() const final { return &m_lines; }
    const RefPtr<BoxStyle>& firstLineStyle() const { return m_firstLineStyle; }
    Box* continuation() const { return m_continuation; }
    void setContinuation(Box* continuation) { m_continuation = continuation; }

    void addBox(Box* box) override;

private:
    mutable BoxList m_children;
    mutable LineBoxList m_lines;
    RefPtr<BoxStyle> m_firstLineStyle;
    Box* m_continuation{nullptr};
};

template<>
struct is<BlockBox> {
    static bool check(const Box& box) { return box.isBlockBox(); }
};

class FlexibleBox : public BlockBox {
public:
    FlexibleBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isFlexibleBox() const final { return true; }
};

template<>
struct is<FlexibleBox> {
    static bool check(const Box& box) { return box.isFlexibleBox(); }
};

class ReplacedBox : public BoxFrame {
public:
    ReplacedBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isReplacedBox() const final { return true; }

    const SizeF& intrinsicSize() const { return m_intrinsicSize; }
    void setIntrinsicSize(const SizeF& size) { m_intrinsicSize = size; }

private:
    SizeF m_intrinsicSize;
};

template<>
struct is<ReplacedBox> {
    static bool check(const Box& box) { return box.isReplacedBox(); }
};

class Image;

class ImageBox : public ReplacedBox {
public:
    ImageBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isImageBox() const final { return true; }

    const RefPtr<Image>& image() const { return m_image; }
    const std::string& alternativeText() const { return m_alternativeText; }

    void setImage(RefPtr<Image> image);
    void setAlternativeText(std::string text) { m_alternativeText = std::move(text); }

private:
    RefPtr<Image> m_image;
    std::string m_alternativeText;
};

template<>
struct is<ImageBox> {
    static bool check(const Box& box) { return box.isImageBox(); }
};

class ListMarkerBox;

class ListItemBox final : public BlockBox {
public:
    ListItemBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isListItemBox() const final { return true; }

    ListMarkerBox* listMarker() const { return m_listMarker; }
    void setListMarker(ListMarkerBox* marker) { m_listMarker = marker; }

private:
    ListMarkerBox* m_listMarker{nullptr};
};

template<>
struct is<ListItemBox> {
    static bool check(const Box& box) { return box.isListItemBox(); }
};

class ListMarkerBox final : public BoxFrame {
public:
    ListMarkerBox(ListItemBox* item, const RefPtr<BoxStyle>& style);
    ~ListMarkerBox() final;

    bool isListMarkerBox() const final { return true; }

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

template<>
struct is<ListMarkerBox> {
    static bool check(const Box& box) { return box.isListMarkerBox(); }
};

class TableCaptionBox;
class TableSectionBox;

class TableBox final : public BlockBox {
public:
    TableBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isTableBox() const final { return true; }

    TableSectionBox* head() const { return m_head; }
    TableSectionBox* foot() const { return m_foot; }
    const std::vector<TableCaptionBox*>& captions() const { return m_captions; }
    const std::vector<TableSectionBox*>& sections() const { return m_sections; }

private:
    TableSectionBox* m_head{nullptr};
    TableSectionBox* m_foot{nullptr};
    std::vector<TableSectionBox*> m_sections;
    std::vector<TableCaptionBox*> m_captions;
};

template<>
struct is<TableBox> {
    static bool check(const Box& box) { return box.isTableBox(); }
};

class TableCellBox final : public BlockBox {
public:
    TableCellBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isTableCellBox() const final { return true; }

    int colSpan() const { return m_colSpan; }
    int rowSpan() const { return m_rowSpan; }

    void setColSpan(int span) { m_colSpan = span; }
    void setRowSpan(int span) { m_rowSpan = span; }

private:
    int m_colSpan{1};
    int m_rowSpan{1};
};

template<>
struct is<TableCellBox> {
    static bool check(const Box& box) { return box.isTableCellBox(); }
};

class TableColumnBox : public Box {
public:
    TableColumnBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isTableColumnBox() const final { return true; }

    int span() const { return m_span; }
    void setSpan(int span) { m_span = span; }

private:
    int m_span{1};
};

template<>
struct is<TableColumnBox> {
    static bool check(const Box& box) { return box.isTableColumnBox(); }
};

class TableColumnGroupBox final : public TableColumnBox {
public:
    TableColumnGroupBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isTableColumnGroupBox() const final { return true; }

    BoxList* children() const final { return &m_children; }

private:
    mutable BoxList m_children;
};

template<>
struct is<TableColumnGroupBox> {
    static bool check(const Box& box) { return box.isTableColumnGroupBox(); }
};

class TableRowBox final : public BoxFrame {
public:
    TableRowBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isTableRowBox() const final { return true; }

    BoxList* children() const final { return &m_children; }

private:
    mutable BoxList m_children;
};

template<>
struct is<TableRowBox> {
    static bool check(const Box& box) { return box.isTableRowBox(); }
};

class TableCaptionBox final : public BlockBox {
public:
    TableCaptionBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isTableCaptionBox() const final { return true; }

    CaptionSide captionSide() const;
};

template<>
struct is<TableCaptionBox> {
    static bool check(const Box& box) { return box.isTableCaptionBox(); }
};

class TableSectionBox final : public BoxFrame {
public:
    TableSectionBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isTableSectionBox() const final { return true; }

    BoxList* children() const final { return &m_children; }

private:
    mutable BoxList m_children;
};

template<>
struct is<TableSectionBox> {
    static bool check(const Box& box) { return box.isTableSectionBox(); }
};

} // namespace htmlbook

#endif // BOX_H
