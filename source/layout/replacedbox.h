#ifndef REPLACEDBOX_H
#define REPLACEDBOX_H

#include "box.h"

namespace htmlbook {

class ReplacedBox : public BoxFrame {
public:
    ReplacedBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const override { return type == Type::Replaced || BoxFrame::isOfType(type); }

    float intrinsicWidth() const { return m_intrinsicWidth; }
    float intrinsicHeight() const { return m_intrinsicHeight; }

    void setIntrinsicWidth(float width) { m_intrinsicWidth = width; }
    void setIntrinsicHeight(float height) { m_intrinsicHeight = height; }

private:
    float m_intrinsicWidth{0};
    float m_intrinsicHeight{0};
};

template<>
struct is_a<ReplacedBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::Replaced); }
};

class Image;

class ImageBox final : public ReplacedBox {
public:
    ImageBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const final { return type == Type::Image || ReplacedBox::isOfType(type); }

    const RefPtr<Image>& image() const { return m_image; }
    const HeapString& alternativeText() const { return m_alternativeText; }

    void setImage(RefPtr<Image> image);
    void setAlternativeText(const HeapString& text) { m_alternativeText = text; }

private:
    RefPtr<Image> m_image;
    HeapString m_alternativeText;
};

template<>
struct is_a<ImageBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::Image); }
};

} // namespace htmlbook

#endif // REPLACEDBOX_H
