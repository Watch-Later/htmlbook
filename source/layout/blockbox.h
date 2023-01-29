#ifndef BLOCKBOX_H
#define BLOCKBOX_H

#include "box.h"

namespace htmlbook {

using PositionedBoxList = std::pmr::set<BoxFrame*>;

class BlockBox : public BoxFrame {
public:
    BlockBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const override { return type == Type::Block || BoxFrame::isOfType(type); }

    virtual void computeIntrinsicWidths(float& minWidth, float& maxWidth) const = 0;

    void computePreferredWidths(float& minWidth, float& maxWidth) const override;

    const PositionedBoxList* positionedBoxes() const { return m_positionedBoxes.get(); }
    bool containsPositonedBoxes() const { return m_positionedBoxes && !m_positionedBoxes->empty(); }
    void insertPositonedBox(BoxFrame* box);
    void removePositonedBox(BoxFrame* box);
    void layoutPositionedBoxes();

    void addBox(Box* box) override;

    float leftOffsetForContent() const { return borderLeft() + paddingLeft(); }
    float rightOffsetForContent() const { return leftOffsetForContent() + availableWidth(); }
    float startOffsetForContent() const { return style()->isLeftToRightDirection() ? leftOffsetForContent() : width() - rightOffsetForContent(); }
    float endOffsetForContent() const { return style()->isRightToLeftDirection() ? leftOffsetForContent() : width() - rightOffsetForContent(); }
    float availableWidthForContent() const { return std::max(0.f, rightOffsetForContent() - leftOffsetForContent()); }

    const char* name() const override { return "BlockBox"; }

private:
    std::unique_ptr<PositionedBoxList> m_positionedBoxes;
};

template<>
struct is_a<BlockBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::Block); }
};

class FloatingBox {
public:
    explicit FloatingBox(BoxFrame* box)
        : m_box(box), m_type(box->style()->floating())
    {}

    BoxFrame* box() const { return m_box; }
    Float type() const { return m_type; }

    void setIsIntruding(bool value) { m_isIntruding = value; }
    void setIsPlaced(bool value) { m_isPlaced = value; }

    bool isIntruding() const { return m_isIntruding; }
    bool isPlaced() const { return m_isPlaced; }

    void setX(float x) { m_x = x; }
    void setY(float y) { m_y = y; }
    void setWidth(float width) { m_width = width; }
    void setHeight(float height) { m_height = height; }

    float x() const { return m_x; }
    float y() const { return m_y; }
    float width() const { return m_width; }
    float height() const { return m_height; }

    float right() const { return m_x + m_width; }
    float bottom() const { return m_y + m_height; }

private:
    BoxFrame* m_box;
    Float m_type;

    bool m_isIntruding{false};
    bool m_isPlaced{false};

    float m_x{0};
    float m_y{0};
    float m_width{0};
    float m_height{0};
};

using FloatingBoxList = std::pmr::list<FloatingBox>;

class MarginInfo;

class BlockFlowBox : public BlockBox {
public:
    ~BlockFlowBox() override;
    BlockFlowBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const override { return type == Type::BlockFlow || BlockBox::isOfType(type); }
    bool avoidsFloats() const override;
    bool isSelfCollapsingBlock() const override;

    void computeIntrinsicWidths(float& minWidth, float& maxWidth) const override;

    void adjustPositionedBox(BoxFrame* child, const MarginInfo& marginInfo);
    void adjustFloatingBox(const MarginInfo& marginInfo);
    void handleBottomOfBlock(float top, float bottom, MarginInfo& marginInfo);
    float collapseMargins(BoxFrame* child, MarginInfo& marginInfo);
    void layoutBlockChild(BoxFrame* child, MarginInfo& marginInfo);

    void layoutBlockChildren();
    void layoutInlineChildren();
    void layout() override;

    LineLayout* lineLayout() const { return m_lineLayout.get(); }
    Box* continuation() const { return m_continuation; }
    void setContinuation(Box* continuation) { m_continuation = continuation; }

    const RefPtr<BoxStyle>& firstLineStyle() const { return m_firstLineStyle; }
    void setFirstLineStyle(RefPtr<BoxStyle> firstLineStyle) { m_firstLineStyle = std::move(firstLineStyle); }

    void buildIntrudingFloats();
    void buildOverhangingFloats();
    void addIntrudingFloats(BlockFlowBox* prevBlock, float offsetX, float offsetY);
    void addOverhangingFloats(BlockFlowBox* childBlock);
    void positionNewFloats();
    bool containsFloat(Box* box) const;
    bool containsFloats() const { return m_floatingBoxes && !m_floatingBoxes->empty(); }

    float leftFloatBottom() const;
    float rightFloatBottom() const;
    float floatBottom() const;
    float nextFloatBottom(float y) const;

    float leftOffsetForFloat(float y, float offset, bool indent, float* heightRemaining = nullptr) const;
    float rightOffsetForFloat(float y, float offset, bool indent, float* heightRemaining = nullptr) const;

    float leftOffsetForLine(float y, bool indent) const { return leftOffsetForFloat(y, leftOffsetForContent(), indent); }
    float rightOffsetForLine(float y, bool indent) const { return rightOffsetForFloat(y, rightOffsetForContent(), indent); }
    float startOffsetForLine(float y, bool indent) const { return style()->isLeftToRightDirection() ? leftOffsetForLine(y, indent) : width() - rightOffsetForLine(y, indent); }
    float endOffsetForLine(float y, bool indent) const { return style()->isRightToLeftDirection() ? leftOffsetForLine(y, indent) : width() - rightOffsetForLine(y, indent); }
    float availableWidthForLine(float y, bool indent) const { return std::max(0.f, rightOffsetForLine(y, indent) - leftOffsetForLine(y, indent)); }

    float getClearDelta(BoxFrame* child, float y) const;

    float maxPositiveMarginTop() const { return m_maxPositiveMarginTop; }
    float maxNegativeMarginTop() const { return m_maxNegativeMarginTop; }
    float maxPositiveMarginBottom() const { return m_maxPositiveMarginBottom; }
    float maxNegativeMarginBottom() const { return m_maxNegativeMarginBottom; }
    void updateMaxMargins();

    const FloatingBoxList* floatingBoxes() const { return m_floatingBoxes.get(); }
    void insertFloatingBox(BoxFrame* box);
    void removeFloatingBox(BoxFrame* box);

    void addBox(Box* box) override;
    void build(BoxLayer* layer) override;

    const char* name() const override { return "BlockFlowBox"; }

private:
    Box* m_continuation{nullptr};
    std::unique_ptr<LineLayout> m_lineLayout;
    std::unique_ptr<FloatingBoxList> m_floatingBoxes;
    RefPtr<BoxStyle> m_firstLineStyle;

    float m_maxPositiveMarginTop{0};
    float m_maxNegativeMarginTop{0};
    float m_maxPositiveMarginBottom{0};
    float m_maxNegativeMarginBottom{0};
};

template<>
struct is_a<BlockFlowBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::BlockFlow); }
};

} // namespace htmlbook

#endif // BLOCKBOX_H
