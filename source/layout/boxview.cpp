#include "boxview.h"
#include "document.h"

namespace htmlbook {

BoxView::BoxView(Document* document, const RefPtr<BoxStyle>& style)
    : BlockBox(document, style)
{
}

void BoxView::computePreferredWidths(float& minWidth, float& maxWidth) const
{
    assert(false);
}

void BoxView::computeWidth(float& x, float& width, float& marginLeft, float& marginRight) const
{
    assert(false);
}

void BoxView::computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const
{
    assert(false);
}

void BoxView::layout()
{
    setWidth(style()->viewportWidth());
    setHeight(style()->viewportHeight());

    auto child = firstBox();
    while(child) {
        child->layout();
        child = child->nextBox();
    }

    layoutPositionedBoxes();
}

} // namespace htmlbook
