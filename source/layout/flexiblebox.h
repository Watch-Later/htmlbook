#ifndef FLEXIBLEBOX_H
#define FLEXIBLEBOX_H

#include "blockbox.h"

namespace htmlbook {

class FlexLine;

class FlexItem : public HeapMember {
public:
    static std::unique_ptr<FlexItem> create(BoxFrame* box);

    BoxFrame* box() const { return m_box; }
    FlexLine* line() const { return m_line; }
    void setLine(FlexLine* line) { m_line = line; }

    int order() const;

private:
    FlexItem(BoxFrame* box);
    BoxFrame* m_box;
    FlexLine* m_line{nullptr};
};

using FlexItemList = std::pmr::vector<std::unique_ptr<FlexItem>>;
using FlexItemListView = std::pmr::vector<FlexItem*>;

class FlexibleBox;

class FlexLine : public HeapMember {
public:
    static std::unique_ptr<FlexLine> create(FlexibleBox* box);

    FlexibleBox* box() const { return m_box; }
    const FlexItemListView& items() const { return m_items; }

    void addItem(FlexItem* item);

private:
    FlexLine(FlexibleBox* box);
    FlexibleBox* m_box;
    FlexItemListView m_items;
};

using FlexLineList = std::pmr::vector<std::unique_ptr<FlexLine>>;

class FlexibleBox final : public BlockBox {
public:
    FlexibleBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const final { return type == Type::Flexible || BlockBox::isOfType(type); }

    void computeIntrinsicWidths(float& minWidth, float& maxWidth) const final;

    void build(BoxLayer* layer) final;

    const char* name() const final { return "FlexibleBox"; }

private:
    FlexItemList m_items;
    FlexLineList m_lines;
};

template<>
struct is_a<FlexibleBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::Flexible); }
};

} // namespace htmlbook

#endif // FLEXIBLEBOX_H
