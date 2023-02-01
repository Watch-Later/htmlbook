#ifndef REPLACEDBOX_H
#define REPLACEDBOX_H

#include "box.h"

namespace htmlbook {

class ReplacedBox : public BoxFrame {
public:
    ReplacedBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const override { return type == Type::Replaced || BoxFrame::isOfType(type); }

    virtual void updateIntrinsicInformation() const;

    float intrinsicWidth() const;
    float intrinsicHeight() const;
    float intrinsicRatio() const;

    const char* name() const override { return "ReplacedBox"; }

private:
    mutable float m_intrinsicWidth{-1};
    mutable float m_intrinsicHeight{-1};
    mutable float m_intrinsicRatio{-1};
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

    const char* name() const final { return "ImageBox"; }

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
