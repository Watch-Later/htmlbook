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
    enum class Violation : uint8_t {
        None,
        Min,
        Max
    };

    FlexItem(BlockBox* box, int order, float flexGrow, float flexShrink, AlignItem alignSelf);

    BlockBox* box() const { return m_box; }
    int order() const { return m_order; }
    float flexGrow() const { return m_flexGrow; }
    float flexShrink() const { return m_flexShrink; }
    float flexFactor(FlexSign sign) const { return sign == FlexSign::Positive ? m_flexGrow : m_flexShrink; }
    AlignItem alignSelf() const { return m_alignSelf; }

    Violation violation() const { return m_violation; }
    void setViolation(Violation violation) { m_violation = violation; }

    bool minViolation() const { return m_violation == Violation::Min; }
    bool maxViolation() const { return m_violation == Violation::Max; }

    float flexBaseSize() const { return m_flexBaseSize; }
    float contentBaseSize() const { return m_contentBaseSize; }

    void setFlexBaseSize(float value) { m_flexBaseSize = value; }
    void setContentBaseSize(float value) { m_contentBaseSize = value; }

    float constrainMainSizeByMinMax(float size) const;
    float constrainCrossSizeByMinMax(float size) const;

    size_t lineIndex() const { return m_lineIndex; }
    void setLineIndex(size_t index) { m_lineIndex = index; }

    FlexibleBox& flexBox() const;
    FlexLine& flexLine() const;

    bool isHorizontalFlow() const;
    bool isVerticalFlow() const;

    float flexBaseMarginBoxSize() const;
    float flexBaseBorderBoxSize() const;

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

    Violation m_violation = Violation::None;

    float m_flexBaseSize = 0;
    float m_contentBaseSize = 0;

    size_t m_lineIndex = 0;
};

using FlexItemList = std::pmr::vector<FlexItem>;
using FlexItemSpan = std::span<FlexItem>;

class FlexLine {
public:
    FlexLine(const FlexItemSpan& items, float mainSize, float mainOffset, float crossSize, float crossOffset);

    const FlexItemSpan& items() const { return m_items; }
    float mainSize() const { return m_mainSize; }
    float mainOffset() const { return m_mainOffset; }
    float crossSize() const { return m_crossSize; }
    float crossOffset() const { return m_crossOffset; }

private:
    FlexItemSpan m_items;
    float m_mainSize;
    float m_mainOffset;
    float m_crossSize;
    float m_crossOffset;
};

using FlexLineList = std::pmr::vector<FlexLine>;

class FlexibleBox final : public BlockBox {
public:
    FlexibleBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const final { return type == Type::Flexible || BlockBox::isOfType(type); }

    void computePreferredWidths(float& minWidth, float& maxWidth) const final;

    void addBox(Box* box) final;
    void build(BoxLayer* layer);

    float computeFlexBaseSize(BlockBox* child) const;

    std::optional<float> computeMinMainSize(const BlockBox* child) const;
    std::optional<float> computeMaxMainSize(const BlockBox* child) const;

    std::optional<float> computeMinCrossSize(const BlockBox* child) const;
    std::optional<float> computeMaxCrossSize(const BlockBox* child) const;

    float availableMainSize() const;

    float borderStart() const;
    float borderEnd() const;
    float borderBefore() const;
    float borderAfter() const;

    float paddingStart() const;
    float paddingEnd() const;
    float paddingBefore() const;
    float paddingAfter() const;

    void layout() final;

    FlexDirection flexDirection() const { return m_flexDirection; }
    FlexWrap flexWrap() const { return m_flexWrap; }
    AlignContent justifyContent() const { return m_justifyContent; }
    AlignContent alignContent() const { return m_alignContent; }

    bool isHorizontalFlow() const { return m_flexDirection == FlexDirection::Row || m_flexDirection == FlexDirection::RowReverse; }
    bool isVerticalFlow() const { return m_flexDirection == FlexDirection::Column || m_flexDirection == FlexDirection::ColumnReverse; }
    bool isMultiLine() const { return m_flexWrap == FlexWrap::Wrap || m_flexWrap == FlexWrap::WrapReverse; }

    const FlexLineList& lines() const { return m_lines; }
    FlexLineList& lines() { return m_lines; }

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

inline FlexibleBox& FlexItem::flexBox() const
{
    return to<FlexibleBox>(*m_box->parentBox());
}

inline FlexLine& FlexItem::flexLine() const
{
    return flexBox().lines()[m_lineIndex];
}

inline bool FlexItem::isHorizontalFlow() const
{
    return flexBox().isHorizontalFlow();
}

inline bool FlexItem::isVerticalFlow() const
{
    return flexBox().isVerticalFlow();
}

} // namespace htmlbook

#endif // FLEXIBLEBOX_H
