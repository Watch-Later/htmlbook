#ifndef FLEXIBLEBOX_H
#define FLEXIBLEBOX_H

#include "blockbox.h"

#include <span>

namespace htmlbook {

enum class FlexSign {
    Positive,
    Negative
};

class FlexibleBox;
class FlexLine;

class FlexItem {
public:
    FlexItem(BlockBox* box, int order, float flexGrow, float flexShrink, AlignItem alignSelf);

    BlockBox* box() const { return m_box; }
    int order() const { return m_order; }
    float flexGrow() const { return m_flexGrow; }
    float flexShrink() const { return m_flexShrink; }
    float flexFactor(FlexSign sign) const { return sign == FlexSign::Positive ? m_flexGrow : m_flexShrink; }
    AlignItem alignSelf() const { return m_alignSelf; }

    void setMinViolation(bool violation) { m_minViolation = violation; }
    void setMaxViolation(bool violation) { m_maxViolation = violation; }

    bool minViolation() const { return m_minViolation; }
    bool maxViolation() const { return m_maxViolation; }

    float flexBaseSize() const { return m_flexBaseSize; }
    float targetMainSize() const { return m_targetMainSize; }

    void setFlexBaseSize(float value) { m_flexBaseSize = value; }
    void setTargetMainSize(float value) { m_targetMainSize = value; }

    float constrainMainSizeByMinMax(float size) const;

    FlexibleBox* flexBox() const;
    FlexDirection flexDirection() const;
    Direction direction() const;

    bool isHorizontalFlow() const;
    bool isVerticalFlow() const;

    float flexBaseMarginBoxSize() const;
    float flexBaseBorderBoxSize() const;

    float targetMainMarginBoxSize() const;
    float targetMainBorderBoxSize() const;

    float marginBoxMainSize() const;
    float marginBoxCrossSize() const;

    float borderBoxMainSize() const;
    float borderBoxCrossSize() const;

    float marginStart() const;
    float marginEnd() const;
    float marginBefore() const;
    float marginAfter() const;

private:
    BlockBox* m_box;
    int m_order;
    float m_flexGrow;
    float m_flexShrink;
    AlignItem m_alignSelf;

    bool m_minViolation = false;
    bool m_maxViolation = false;

    float m_flexBaseSize = 0;
    float m_targetMainSize = 0;
};

using FlexItemList = std::pmr::vector<FlexItem>;
using FlexItemSpan = std::span<FlexItem>;

class FlexLine {
public:
    explicit FlexLine(const FlexItemSpan& items)
        : m_items(items)
    {}

    const FlexItemSpan& items() const { return m_items; }

    float crossOffset() const { return m_crossOffset; }
    float crossSize() const { return m_crossSize; }

    void setCrossOffset(float offset) { m_crossOffset = offset; }
    void setCrossSize(float size) { m_crossSize = size; }

private:
    FlexItemSpan m_items;

    float m_crossOffset = 0;
    float m_crossSize = 0;
};

using FlexLineList = std::pmr::vector<FlexLine>;

class FlexibleBox final : public BlockBox {
public:
    FlexibleBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const final { return type == Type::Flexible || BlockBox::isOfType(type); }

    void computePreferredWidths(float& minWidth, float& maxWidth) const final;

    void addBox(Box* box) final;
    void build(BoxLayer* layer) final;

    float computeFlexBaseSize(BlockBox* child) const;

    std::optional<float> computeMinMainSize(const BlockBox* child) const;
    std::optional<float> computeMaxMainSize(const BlockBox* child) const;

    float computeMainSize(float hypotheticalMainSize) const;
    float availableCrossSize() const;

    float borderAndPaddingStart() const;
    float borderAndPaddingEnd() const;
    float borderAndPaddingBefore() const;
    float borderAndPaddingAfter() const;

    void layout() final;

    FlexDirection flexDirection() const { return m_flexDirection; }
    FlexWrap flexWrap() const { return m_flexWrap; }
    AlignContent justifyContent() const { return m_justifyContent; }
    AlignContent alignContent() const { return m_alignContent; }

    bool isHorizontalFlow() const { return m_flexDirection == FlexDirection::Row || m_flexDirection == FlexDirection::RowReverse; }
    bool isVerticalFlow() const { return m_flexDirection == FlexDirection::Column || m_flexDirection == FlexDirection::ColumnReverse; }
    bool isMultiLine() const { return m_flexWrap == FlexWrap::Wrap || m_flexWrap == FlexWrap::WrapReverse; }

    const FlexItemList& items() { return m_items; }
    const FlexLineList& lines() const { return m_lines; }

    const char* name() const final { return "FlexibleBox"; }

private:
    FlexDirection m_flexDirection;
    FlexWrap m_flexWrap;
    AlignContent m_justifyContent;
    AlignContent m_alignContent;
    FlexItemList m_items;
    FlexLineList m_lines;
};

template<>
struct is_a<FlexibleBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::Flexible); }
};

inline FlexibleBox* FlexItem::flexBox() const
{
    return static_cast<FlexibleBox*>(m_box->parentBox());
}

inline FlexDirection FlexItem::flexDirection() const
{
    return flexBox()->flexDirection();
}

inline Direction FlexItem::direction() const
{
    return flexBox()->direction();
}

inline bool FlexItem::isHorizontalFlow() const
{
    return flexBox()->isHorizontalFlow();
}

inline bool FlexItem::isVerticalFlow() const
{
    return flexBox()->isVerticalFlow();
}

} // namespace htmlbook

#endif // FLEXIBLEBOX_H
