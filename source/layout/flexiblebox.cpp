#include "flexiblebox.h"

namespace htmlbook {

FlexibleBox::FlexibleBox(Node* node, const RefPtr<BoxStyle>& style)
    : BlockBox(node, style)
{
    setChildrenInline(false);
}

void FlexibleBox::computeBlockPreferredWidths(float& minWidth, float& maxWidth) const
{
    minWidth = 0;
    maxWidth = 0;
}

} // namespace htmlbook
