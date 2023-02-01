#include "replacedbox.h"
#include "resource.h"

namespace htmlbook {

ReplacedBox::ReplacedBox(Node* node, const RefPtr<BoxStyle>& style)
    : BoxFrame(node, style)
{
    setReplaced(true);
}

void ReplacedBox::updateIntrinsicInformation() const
{
    m_intrinsicWidth = 300;
    m_intrinsicHeight = 150;
    m_intrinsicRatio = m_intrinsicWidth / m_intrinsicHeight;
}

float ReplacedBox::intrinsicWidth() const
{
    if(m_intrinsicWidth < 0)
        updateIntrinsicInformation();
    return m_intrinsicWidth;
}

float ReplacedBox::intrinsicHeight() const
{
    if(m_intrinsicHeight < 0)
        updateIntrinsicInformation();
    return m_intrinsicHeight;
}

float ReplacedBox::intrinsicRatio() const
{
    if(m_intrinsicRatio < 0)
        updateIntrinsicInformation();
    return m_intrinsicRatio;
}

ImageBox::ImageBox(Node* node, const RefPtr<BoxStyle>& style)
    : ReplacedBox(node, style)
{
}

void ImageBox::setImage(RefPtr<Image> image)
{
    m_image = std::move(image);
}

} // namespace htmlbook
