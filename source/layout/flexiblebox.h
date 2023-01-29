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
    FlexItem* nextItem() const { return m_nextItem; }
    FlexItem* prevItem() const { return m_prevItem; }

    void setLine(FlexLine* line) { m_line = line; }
    void setNextItem(FlexItem* item) { m_nextItem = item; }
    void setPrevItem(FlexItem* item) { m_prevItem = item; }

    int order() const;

    ~FlexItem();

private:
    FlexItem(BoxFrame* box);
    BoxFrame* m_box;
    FlexLine* m_line{nullptr};
    FlexItem* m_nextItem{nullptr};
    FlexItem* m_prevItem{nullptr};
};

using FlexItemList = std::pmr::vector<std::unique_ptr<FlexItem>>;

class FlexibleBox;

class FlexLine : public HeapMember {
public:
    static std::unique_ptr<FlexLine> create(FlexibleBox* box);

    FlexibleBox* box() const { return m_box; }
    FlexItem* firstItem() const { return m_firstItem; }
    FlexItem* lastItem() const { return m_firstItem; }
    size_t itemCount() const { return m_itemCount; }

    void addItem(FlexItem* item);
    void removeItem(FlexItem* item);

    ~FlexLine();

private:
    FlexLine(FlexibleBox* box);
    FlexibleBox* m_box;
    FlexItem* m_firstItem{nullptr};
    FlexItem* m_lastItem{nullptr};
    size_t m_itemCount{0};
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
