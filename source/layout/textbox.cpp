#include "textbox.h"

namespace htmlbook {

TextBox::TextBox(Node* node, const RefPtr<BoxStyle>& style)
    : Box(node, style)
    , m_lines(style->heap())
{
    setInline(true);
}

} // namespace htmlbook
