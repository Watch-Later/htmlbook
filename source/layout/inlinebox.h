#ifndef INLINEBOX_H
#define INLINEBOX_H

#include "box.h"

namespace htmlbook {

class InlineBox : public BoxModel {
public:
    InlineBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const override { return type == Type::Inline || BoxModel::isOfType(type); }

    Box* continuation() const { return m_continuation; }
    void setContinuation(Box* continuation) { m_continuation = continuation; }

    void addBox(Box* box) override;

    void addLine(std::unique_ptr<FlowLineBox> line);
    void removeLine(LineBox* line);
    const FlowLineBoxList& lines() const { return m_lines; }

    const char* name() const override { return "InlineBox"; }

private:
    Box* m_continuation{nullptr};
    FlowLineBoxList m_lines;
};

template<>
struct is_a<InlineBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::Inline); }
};

} // namespace htmlbook

#endif // INLINEBOX_H
