#ifndef BLOCKBOX_H
#define BLOCKBOX_H

#include "box.h"

namespace htmlbook {

using PositionedBoxList = std::pmr::set<BoxFrame*>;

class BlockBox : public BoxFrame {
public:
    BlockBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const override { return type == Type::Block || BoxFrame::isOfType(type); }

    virtual void computeInlinePreferredWidths(float& minWidth, float& maxWidth) const;
    virtual void computeBlockPreferredWidths(float& minWidth, float& maxWidth) const;

    void computePreferredWidths(float& minWidth, float& maxWidth) const override;

    BoxList* children() const final { return &m_children; }
    Box* continuation() const { return m_continuation; }
    void setContinuation(Box* continuation) { m_continuation = continuation; }

    void addBox(Box* box) override;

    void insertPositonedBox(BoxFrame* box);
    void removePositonedBox(BoxFrame* box);

    const PositionedBoxList* positionedBoxes() const { return m_positionedBoxes.get(); }

protected:
    mutable BoxList m_children;
    Box* m_continuation{nullptr};
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
    void setIsHidden(bool value) { m_isHidden = value; }
    void setIsPlaced(bool value) { m_isPlaced = value; }

    bool isIntruding() const { return m_isIntruding; }
    bool isHidden() const { return m_isHidden; }
    bool isPlaced() const { return m_isPlaced; }

    void setX(float x) { m_x = x; }
    void setY(float y) { m_y = y; }
    void setWidth(float width) { m_width = width; }
    void setHeight(float height) { m_height = height; }

    float x() const { return m_x; }
    float y() const { return m_y; }
    float width() const { return m_width; }
    float height() const { return m_height; }

private:
    BoxFrame* m_box;
    Float m_type;

    bool m_isIntruding{false};
    bool m_isHidden{false};
    bool m_isPlaced{false};

    float m_x{0};
    float m_y{0};
    float m_width{0};
    float m_height{0};
};

using FloatingBoxList = std::pmr::list<FloatingBox>;

class BlockFlowBox : public BlockBox {
public:
    ~BlockFlowBox() override;
    BlockFlowBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const override { return type == Type::BlockFlow || BlockBox::isOfType(type); }

    LineBoxList* lines() const final { return &m_lines; }
    const RefPtr<BoxStyle>& firstLineStyle() const { return m_firstLineStyle; }
    void setFirstLineStyle(RefPtr<BoxStyle> firstLineStyle);

    bool containsFloat(Box* box) const;
    bool containsFloats() const { return m_floatingBoxes && !m_floatingBoxes->empty(); }

    void insertFloatingBox(BoxFrame* box);
    void removeFloatingBox(BoxFrame* box);

    const FloatingBoxList* floatingBoxes() const { return m_floatingBoxes.get(); }

private:
    mutable LineBoxList m_lines;
    RefPtr<BoxStyle> m_firstLineStyle;
    std::unique_ptr<FloatingBoxList> m_floatingBoxes;
};

template<>
struct is_a<BlockFlowBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::BlockFlow); }
};

} // namespace htmlbook

#endif // BLOCKBOX_H
