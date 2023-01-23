#include "boxview.h"
#include "document.h"

namespace htmlbook {

BoxView::BoxView(Document* document, const RefPtr<BoxStyle>& style)
    : BlockBox(document, style)
{
}

void BoxView::computeWidth(float& x, float& width, float& marginLeft, float& marginRight) const
{
    width = style()->viewportWidth();
}

void BoxView::computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const
{
    height = style()->viewportHeight();
}

void BoxView::layout()
{
    updateWidth();
    updateHeight();
    auto child = firstBox();
    while(child) {
        child->layout();
        child = child->nextBox();
    }

    layoutPositionedBoxes();
}

} // namespace htmlbook
