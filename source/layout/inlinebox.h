#ifndef INLINEBOX_H
#define INLINEBOX_H

#include "box.h"

namespace htmlbook {

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

} // namespace htmlbook

#endif // INLINEBOX_H
