#include "listitembox.h"

namespace htmlbook {

ListItemBox::ListItemBox(Node* node, const RefPtr<BoxStyle>& style)
    : BlockFlowBox(node, style)
{
}

InsideListMarkerBox::InsideListMarkerBox(const RefPtr<BoxStyle>& style)
    : InlineBox(nullptr, style)
{
}

OutsideListMarkerBox::OutsideListMarkerBox(const RefPtr<BoxStyle>& style)
    : BlockFlowBox(nullptr, style)
{
}

} // namespace htmlbook
