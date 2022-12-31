#ifndef LISTITEMBOX_H
#define LISTITEMBOX_H

#include "blockbox.h"
#include "inlinebox.h"

namespace htmlbook {

class ListItemBox final : public BlockFlowBox {
public:
    ListItemBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const final { return type == Type::ListItem || BlockFlowBox::isOfType(type); }
};

template<>
struct is_a<ListItemBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::ListItem); }
};

class InsideListMarkerBox final : public InlineBox {
public:
    InsideListMarkerBox(const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const final { return type == Type::InsideListMarker || InlineBox::isOfType(type); }
};

template<>
struct is_a<InsideListMarkerBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::InsideListMarker); }
};

class OutsideListMarkerBox final : public BlockFlowBox {
public:
    OutsideListMarkerBox(const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const final { return type == Type::OutsideListMarker || BlockFlowBox::isOfType(type); }
};

template<>
struct is_a<OutsideListMarkerBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::OutsideListMarker); }
};

} // namespace htmlbook

#endif // LISTITEMBOX_H
