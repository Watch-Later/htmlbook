#ifndef BOX_H
#define BOX_H

#include "boxstyle.h"
#include "linebox.h"

namespace htmlbook {

class Node;
class BoxList;
class BoxLayer;
class BlockBox;
class BlockFlowBox;

class Box : public HeapMember {
public:
    enum class Type {
        Text,
        Inline,
        Block,
        BlockFlow,
        Flexible,
        Replaced,
        Image,
        ListItem,
        InsideListMarker,
        OutsideListMarker,
        Table,
        TableCell,
        TableColumn,
        TableColumnGroup,
        TableRow,
        TableCaption,
        TableSection
    };

    Box(Node* node, const RefPtr<BoxStyle>& style);

    virtual ~Box();
    virtual bool isBoxModel() const { return false; }
    virtual bool isBoxFrame() const { return false; }
    virtual bool isOfType(Type type) const { return false; }

    virtual void computePreferredWidths(float& minWidth, float& maxWidth) const;

    virtual BoxList* children() const { return nullptr; }
    virtual LineBoxList* lines() const { return nullptr; }

    virtual void addBox(Box* box);
    virtual void buildBox(BoxLayer* layer);

    LineBox* addLine(std::unique_ptr<LineBox> line);
    std::unique_ptr<LineBox> removeLine(LineBox* line);

    void insertChild(Box* box, Box* nextBox);
    void appendChild(Box* box);
    void removeChild(Box* box);

    void moveChildrenTo(Box* to, Box* begin, Box* end);
    void moveChildrenTo(Box* to, Box* begin);
    void moveChildrenTo(Box* to);

    Box* firstBox() const;
    Box* lastBox() const;

    Node* node() const { return m_node; }
    const RefPtr<BoxStyle>& style() const { return m_style; }
    Box* parentBox() const { return m_parentBox; }
    Box* prevBox() const { return m_prevBox; }
    Box* nextBox() const { return m_nextBox; }

    void setParentBox(Box* box) { m_parentBox = box; }
    void setPrevBox(Box* box) { m_prevBox = box; }
    void setNextBox(Box* box) { m_nextBox = box; }

    static Box* create(Node* node, const RefPtr<BoxStyle>& style);
    static Box* createAnonymous(const RefPtr<BoxStyle>& parentStyle, Display display);
    static BlockFlowBox* createAnonymousBlock(const RefPtr<BoxStyle>& parentStyle);

    static Box* containingBox(const Box& box);
    static BlockBox* containingBlock(const Box& box);

    BlockBox* containingBlockFixed() const;
    BlockBox* containingBlockAbsolute() const;

    Box* containingBox() const { return m_containingBox; }
    BlockBox* containingBlock() const { return m_containingBlock; }

    bool isRootBox() const { return !m_parentBox; }
    bool isAnonymous() const { return m_anonymous; }
    bool isReplaced() const { return m_replaced; }
    bool isInline() const { return m_inline; }
    bool isFloating() const { return m_floating; }
    bool isPositioned() const { return m_positioned; }
    bool isFloatingOrPositioned() const { return m_floating || m_positioned; }
    bool isChildrenInline() const { return m_childrenInline; }
    bool hasTransform() const { return m_hasTransform; }

    void setAnonymous(bool value) { m_anonymous = value; }
    void setReplaced(bool value) { m_replaced = value; }
    void setInline(bool value) { m_inline = value; }
    void setFloating(bool value) { m_floating = value; }
    void setPositioned(bool value) { m_positioned = value; }
    void setChildrenInline(bool value) { m_childrenInline = value; }
    void setHasTransform(bool value) { m_hasTransform = value; }

    Heap* heap() const { return m_style->heap(); }
    Document* document() const { return m_style->document(); }
    Display display() const { return m_style->display(); }
    Position position() const { return m_style->position(); }

private:
    Node* m_node;
    RefPtr<BoxStyle> m_style;
    Box* m_parentBox{nullptr};
    Box* m_prevBox{nullptr};
    Box* m_nextBox{nullptr};
    Box* m_containingBox{nullptr};
    BlockBox* m_containingBlock{nullptr};
    bool m_anonymous{false};
    bool m_replaced{false};
    bool m_inline{true};
    bool m_floating{false};
    bool m_positioned{false};
    bool m_childrenInline{true};
    bool m_hasTransform{false};
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

class BoxLayer : public HeapMember {
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
    std::pmr::list<BoxLayer*> m_children;
};

class BoxModel : public Box {
public:
    BoxModel(Node* node, const RefPtr<BoxStyle>& style);

    bool isBoxModel() const final { return true; }

    void addBox(Box* box) override;
    void buildBox(BoxLayer* layer) override;

    virtual bool requiresLayer() const { return false; }
    virtual void computeBorder(float& top, float& bottom, float& left, float& right) const;
    virtual void computePadding(float& top, float& bottom, float& left, float& right) const;

    float borderTop() const;
    float borderBottom() const;
    float borderLeft() const;
    float borderRight() const;

    float paddingTop() const;
    float paddingBottom() const;
    float paddingLeft() const;
    float paddingRight() const;

    float marginTop() const { return m_marginTop; }
    float marginBottom() const { return m_marginBottom; }
    float marginLeft() const { return m_marginLeft; }
    float marginRight() const { return m_marginRight; }

    void setMarginTop(float value) { m_marginTop = value; }
    void setMarginBottom(float value) { m_marginBottom = value; }
    void setMarginLeft(float value) { m_marginLeft = value; }
    void setMarginRight(float value) { m_marginRight = value; }

    BoxLayer* layer() const { return m_layer.get(); }

private:
    std::unique_ptr<BoxLayer> m_layer;

    mutable float m_borderTop{-1};
    mutable float m_borderBottom{-1};
    mutable float m_borderLeft{-1};
    mutable float m_borderRight{-1};

    mutable float m_paddingTop{-1};
    mutable float m_paddingBottom{-1};
    mutable float m_paddingLeft{-1};
    mutable float m_paddingRight{-1};

protected:
    float m_marginTop{0};
    float m_marginBottom{0};
    float m_marginLeft{0};
    float m_marginRight{0};
};

template<>
struct is_a<BoxModel> {
    static bool check(const Box& box) { return box.isBoxModel(); }
};

class BoxFrame : public BoxModel {
public:
    BoxFrame(Node* node, const RefPtr<BoxStyle>& style);

    bool isBoxFrame() const final { return true; }

    LineBox* line() const { return m_line.get(); }
    void setLine(std::unique_ptr<LineBox> line) { m_line = std::move(line); }

    float x() const { return m_x; }
    float y() const { return m_y; }
    float width() const { return m_width; }
    float height() const { return m_height; }

    void setX(float x) { m_x = x; }
    void setY(float y) { m_y = y; }
    void setWidth(float width) { m_width = width; }
    void setHeight(float height) { m_height = height; }

    void setLocation(float x, float y) { m_x = x; m_y = y; }
    void move(float dx, float dy) { m_x += dx; m_y += dy; }

    float minPreferredWidth() const;
    float maxPreferredWidth() const;

    void updateWidth();
    void updateHeight();

    void computePositionedWidthReplaced(float& x, float& width, float& marginLeft, float& marginRight) const;
    void computePositionedWidth(float& x, float& width, float& marginLeft, float& marginRight) const;

    virtual void computeWidth(float& x, float& width, float& marginLeft, float& marginRight) const;
    virtual void computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const;

private:
    std::unique_ptr<LineBox> m_line;

    float m_x{0};
    float m_y{0};
    float m_width{0};
    float m_height{0};

    mutable float m_minPreferredWidth{-1};
    mutable float m_maxPreferredWidth{-1};
};

template<>
struct is_a<BoxFrame> {
    static bool check(const Box& box) { return box.isBoxFrame(); }
};

class TextBox : public Box {
public:
    TextBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const override { return type == Type::Text || Box::isOfType(type); }

    LineBoxList* lines() const final { return &m_lines; }
    const std::string& text() const { return m_text; }
    void setText(const std::string_view& text) { m_text = text; }
    void appendText(const std::string_view& text) { m_text += text; }
    void clearText() { m_text.clear(); }

private:
    std::string m_text;
    mutable LineBoxList m_lines;
};

template<>
struct is_a<TextBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::Text); }
};

class InlineBox : public BoxModel {
public:
    InlineBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const override { return type == Type::Inline || BoxModel::isOfType(type); }

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
struct is_a<InlineBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::Inline); }
};

class BlockBox : public BoxFrame {
public:
    BlockBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const override { return type == Type::Block || BoxFrame::isOfType(type); }

    float availableWidth() const { return 0; }

    BoxList* children() const final { return &m_children; }
    Box* continuation() const { return m_continuation; }
    void setContinuation(Box* continuation) { m_continuation = continuation; }

    void addBox(Box* box) override;

protected:
    mutable BoxList m_children;
    Box* m_continuation{nullptr};
};

template<>
struct is_a<BlockBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::Block); }
};

class BlockFlowBox : public BlockBox {
public:
    BlockFlowBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const override { return type == Type::BlockFlow || BlockBox::isOfType(type); }

    LineBoxList* lines() const final { return &m_lines; }
    const RefPtr<BoxStyle>& firstLineStyle() const { return m_firstLineStyle; }
    void setFirstLineStyle(RefPtr<BoxStyle> firstLineStyle);

private:
    mutable LineBoxList m_lines;
    RefPtr<BoxStyle> m_firstLineStyle;
};

template<>
struct is_a<BlockFlowBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::BlockFlow); }
};

class FlexibleBox : public BlockBox {
public:
    FlexibleBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const final { return type == Type::Flexible || BlockBox::isOfType(type); }
};

template<>
struct is_a<FlexibleBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::Flexible); }
};

class ReplacedBox : public BoxFrame {
public:
    ReplacedBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const override { return type == Type::Replaced || BoxFrame::isOfType(type); }

    float intrinsicWidth() const { return m_intrinsicWidth; }
    float intrinsicHeight() const { return m_intrinsicHeight; }

    void setIntrinsicWidth(float width) { m_intrinsicWidth = width; }
    void setIntrinsicHeight(float height) { m_intrinsicHeight = height; }

private:
    float m_intrinsicWidth{0};
    float m_intrinsicHeight{0};
};

template<>
struct is_a<ReplacedBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::Replaced); }
};

class Image;

class ImageBox : public ReplacedBox {
public:
    ImageBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const override { return type == Type::Image || ReplacedBox::isOfType(type); }

    const RefPtr<Image>& image() const { return m_image; }
    const std::string& alternativeText() const { return m_alternativeText; }

    void setImage(RefPtr<Image> image);
    void setAlternativeText(const std::string_view& text) { m_alternativeText = std::move(text); }

private:
    RefPtr<Image> m_image;
    std::string m_alternativeText;
};

template<>
struct is_a<ImageBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::Image); }
};

class ListItemBox final : public BlockFlowBox {
public:
    ListItemBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const final { return type == Type::ListItem || BlockFlowBox::isOfType(type); }
};

template<>
struct is_a<ListItemBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::ListItem); }
};

class InsideListMarkerBox final : public InlineBox {
public:
    InsideListMarkerBox(const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const final { return type == Type::InsideListMarker || InlineBox::isOfType(type); }
};

template<>
struct is_a<InsideListMarkerBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::InsideListMarker); }
};

class OutsideListMarkerBox final : public BlockFlowBox {
public:
    OutsideListMarkerBox(const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const final { return type == Type::OutsideListMarker || BlockFlowBox::isOfType(type); }
};

template<>
struct is_a<OutsideListMarkerBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::OutsideListMarker); }
};

class TableCaptionBox;
class TableSectionBox;
class TableColumnBox;

class TableBox final : public BlockBox {
public:
    TableBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const final { return type == Type::Table || BlockBox::isOfType(type); }

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

#endif // BOX_H
