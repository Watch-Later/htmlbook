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

    BlockBox* containingBlockFixed() const;
    BlockBox* containingBlockAbsolute() const;

    Box* containingBox() const;
    BlockBox* containingBlock() const;

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

    float staticTop() const { return m_staticTop; }
    float staticLeft() const { return m_staticLeft; }

    void setStaticTop(float top) { m_staticTop = top; }
    void setStaticLeft(float left) { m_staticLeft = left; }

private:
    BoxLayer(BoxModel* box, BoxLayer* parent);
    int m_index;
    BoxModel* m_box;
    BoxLayer* m_parent;
    std::pmr::list<BoxLayer*> m_children;

    float m_staticTop{0};
    float m_staticLeft{0};
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

    virtual float containingBlockWidthForContent() const;

    float borderTop() const;
    float borderBottom() const;
    float borderLeft() const;
    float borderRight() const;

    float borderWidth() const { return borderLeft() + borderRight(); }
    float borderHeight() const { return borderTop() + borderBottom(); }

    float paddingTop() const;
    float paddingBottom() const;
    float paddingLeft() const;
    float paddingRight() const;

    float paddingWidth() const { return paddingLeft() + paddingRight(); }
    float paddingHeight() const { return paddingTop() + paddingBottom(); }

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

    float clientWidth() const { return m_width - borderLeft() - borderRight(); }
    float clientHeight() const { return m_height - borderTop() - borderBottom(); }

    float contentWidth() const { return clientWidth() - paddingLeft() - paddingRight(); }
    float contentHeight() const { return clientHeight() - paddingTop() - paddingBottom(); }

    float minPreferredWidth() const;
    float maxPreferredWidth() const;

    float overrideWidth() const { return m_overrideWidth; }
    float overrideHeight() const { return m_overrideHeight; }

    void setOverrideWidth(float width) { m_overrideWidth = width; }
    void setOverrideHeight(float height) { m_overrideHeight = height; }

    bool hasOverrideWidth() const { return m_overrideWidth >= 0; }
    bool hasOverrideHeight() const { return m_overrideHeight >= 0; }

    float intrinsicWidth() const;
    float intrinsicHeight() const;

    virtual float availableWidth() const;
    virtual float availableHeight() const;
    float availableHeightUsing(const Length& height) const;

    float containingBlockWidthForContent() const override;
    float containingBlockHeightForContent() const;

    BoxModel* containingBoxModel() const { return to<BoxModel>(containingBox()); }
    float containingBlockWidthForPositioned(const BoxModel* containingBox) const;
    float containingBlockHeightForPositioned(const BoxModel* containingBox) const;

    void updateWidth();
    void updateHeight();

    float computeBorderBoxWidth(float width) const;
    float computeBorderBoxHeight(float height) const;
    float computeContentBoxWidth(float width) const;
    float computeContentBoxHeight(float height) const;

    float computeReplacedWidthUsing(const Length& width) const;
    float computeReplacedHeightUsing(const Length& height) const;

    float computePercentageReplacedWidth(const Length& width) const;
    float computePercentageReplacedHeight(const Length& height) const;

    float computeReplacedWidth() const;
    float computeReplacedHeight() const;

    void computeHorizontalMargins(float& marginLeft, float& marginRight, float containerWidth, float childWidth) const;
    void computeVerticalMargins(float& marginTop, float& marginBottom) const;

    float computeWidthUsing(const Length& width, float availableWidth) const;
    std::optional<float> computeHeightUsing(const Length& height) const;
    std::optional<float> computePercentageHeight(const Length& height) const;

    float constrainWidthByMinMax(float width, float availableWidth) const;
    float constrainHeightByMinMax(float height) const;

    void computePositionedWidthUsing(const Length& widthLength, const BoxModel* container, TextDirection containerDirection, float containerWidth,
        const Length& leftLength, const Length& rightLength, const Length& marginLeftLength, const Length& marginRightLength,
        float& x, float& width, float& marginLeft, float& marginRight) const;

    void computePositionedWidthReplaced(float& x, float& width, float& marginLeft, float& marginRight) const;
    void computePositionedWidth(float& x, float& width, float& marginLeft, float& marginRight) const;

    void computePositionedHeightUsing(const Length& heightLength, const BoxModel* container, float containerHeight, float contentHeight,
        const Length& topLength, const Length& bottomLength, const Length& marginTopLength, const Length& marginBottomLength,
        float& y, float& height, float& marginTop, float& marginBottom) const;

    void computePositionedHeightReplaced(float& y, float& height, float& marginTop, float& marginBottom) const;
    void computePositionedHeight(float& y, float& height, float& marginTop, float& marginBottom) const;

    virtual void computeWidth(float& x, float& width, float& marginLeft, float& marginRight) const;
    virtual void computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const;

private:
    std::unique_ptr<LineBox> m_line;

    float m_x{0};
    float m_y{0};
    float m_width{0};
    float m_height{0};

    float m_overrideWidth{-1};
    float m_overrideHeight{-1};

    mutable float m_minPreferredWidth{-1};
    mutable float m_maxPreferredWidth{-1};
};

template<>
struct is_a<BoxFrame> {
    static bool check(const Box& box) { return box.isBoxFrame(); }
};

} // namespace htmlbook

#endif // BOX_H
