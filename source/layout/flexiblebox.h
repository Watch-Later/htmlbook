#ifndef FLEXIBLEBOX_H
#define FLEXIBLEBOX_H

#include "blockbox.h"

namespace htmlbook {

class FlexibleBox final : public BlockBox {
public:
    FlexibleBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const final { return type == Type::Flexible || BlockBox::isOfType(type); }

    bool avoidsFloats() const final { return true; }

    void computeBlockPreferredWidths(float& minWidth, float& maxWidth) const final;
};

template<>
struct is_a<FlexibleBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::Flexible); }
};

} // namespace htmlbook

#endif // FLEXIBLEBOX_H
