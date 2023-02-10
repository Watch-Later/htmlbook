#ifndef FLEXIBLEBOX_H
#define FLEXIBLEBOX_H

#include "blockbox.h"

#include <span>

namespace htmlbook {

class FlexLine;
class FlexibleBox;

class FlexItem {
public:
    FlexItem(BlockBox* box, int order);

    BlockBox* box() const { return m_box; }
    int order() const { return m_order; }

    float flexBaseSize() const { return m_flexBaseSize; }
    float hypotheticalMainSize() const { return m_hypotheticalMainSize; }
    float flexFactor() const { return m_flexFactor; }

    void setFlexBaseSize(float value) { m_flexBaseSize = value; }
    void setHypotheticalMainSize(float value) { m_hypotheticalMainSize = value; }
    void setFlexFactor(float value) { m_flexFactor = value; }

    size_t lineIndex() const { return m_lineIndex; }
    void setLineIndex(size_t index) { m_lineIndex = index; }

    void setMinViolation(bool value) { m_minViolation = value; }
    void setMaxViolation(bool value) { m_maxViolation = value; }

    bool minViolation() const { return m_minViolation; }
    bool maxViolation() const { return m_maxViolation; }

private:
    BlockBox* m_box;

    int m_order;

    float m_flexBaseSize = 0;
    float m_hypotheticalMainSize = 0;
    float m_flexFactor = 0;

    size_t m_lineIndex = 0;

    bool m_minViolation = false;
    bool m_maxViolation = false;
};

using FlexItemList = std::pmr::vector<FlexItem>;
using FlexItemSpan = std::span<FlexItem>;

enum class FlexSign {
    Positive,
    Negative
};

class FlexibleBox;

class FlexLine {
public:
    FlexLine(FlexibleBox* flexBox, const FlexItemSpan& items, float mainSize, float containerMainSize);

    FlexibleBox* flexBox() const { return m_flexBox; }
    const FlexItemSpan& items() const { return m_items; }
    float mainSize() const { return m_mainSize; }
    float containerMainSize() const { return m_containerMainSize; }
    FlexSign flexSign() const;

private:
    FlexibleBox* m_flexBox;
    FlexItemSpan m_items;
    float m_mainSize;
    float m_containerMainSize;
};

using FlexLineList = std::pmr::vector<FlexLine>;

class FlexibleBox final : public BlockBox {
public:
    FlexibleBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const final { return type == Type::Flexible || BlockBox::isOfType(type); }

    void computePreferredWidths(float& minWidth, float& maxWidth) const final;

    void addBox(Box* box) final;
    void build(BoxLayer* layer);

    float computeFlexBaseSize(const BlockBox* child) const;
    float availableMainSize() const;

    void layout() final;

    FlexDirection flexDirection() const { return m_flexDirection; }
    FlexWrap flexWrap() const { return m_flexWrap; }

    bool isHorizontalFlow() const { return m_flexDirection == FlexDirection::Row || m_flexDirection == FlexDirection::RowReverse; }
    bool isVerticalFlow() const { return m_flexDirection == FlexDirection::Column || m_flexDirection == FlexDirection::ColumnReverse; }
    bool isMultiLine() const { return m_flexWrap == FlexWrap::Wrap || m_flexWrap == FlexWrap::WrapReverse; }

    const FlexLineList& lines() const { return m_lines; }
    FlexLineList& lines() { return m_lines; }

    const char* name() const final { return "FlexibleBox"; }

private:
    FlexDirection m_flexDirection;
    FlexWrap m_flexWrap;
    FlexItemList m_items;
    FlexLineList m_lines;
};

template<>
struct is_a<FlexibleBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::Flexible); }
};

} // namespace htmlbook

#endif // FLEXIBLEBOX_H
