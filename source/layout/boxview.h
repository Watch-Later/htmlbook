#ifndef BOXVIEW_H
#define BOXVIEW_H

#include "blockbox.h"

namespace htmlbook {

class BoxView final : public BlockBox {
public:
    BoxView(Document* document, const RefPtr<BoxStyle>& style);

    bool isBoxView() const final { return true; }

    void computeIntrinsicWidths(float& minWidth, float& maxWidth) const final;
    void computeWidth(float& x, float& width, float& marginLeft, float& marginRight) const final;
    void computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const final;
    void layout() final;

    const char* name() const final { return "BoxView"; }
};

template<>
struct is_a<BoxView> {
    static bool check(const Box& box) { return box.isBoxView(); }
};

} // namespace htmlbook

#endif // BOXVIEW_H
