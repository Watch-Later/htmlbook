#ifndef BLOCKBOX_H
#define BLOCKBOX_H

#include "box.h"

namespace htmlbook {

class BlockBox : public BoxFrame {
public:
    BlockBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const override { return type == Type::Block || BoxFrame::isOfType(type); }

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
    ~BlockFlowBox() override;
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

} // namespace htmlbook

#endif // BLOCKBOX_H
