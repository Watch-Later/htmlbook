#ifndef BLOCKBOX_H
#define BLOCKBOX_H

#include "box.h"

namespace htmlbook {

using PositionedBoxList = std::pmr::set<BoxFrame*>;

class BlockBox : public BoxFrame {
public:
    BlockBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const override { return type == Type::Block || BoxFrame::isOfType(type); }

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

struct FloatingBox {
    BoxFrame* box{nullptr};
    Float type{Float::None};

    bool isHidden{false};
    bool isIntruding{false};
    bool isPlaced{false};

    float x{0};
    float y{0};
    float width{0};
    float height{0};
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

    bool containsFloats() const { return m_floatingBoxes && !m_floatingBoxes->empty(); }
    bool containsFloats(Box* box) const;

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
