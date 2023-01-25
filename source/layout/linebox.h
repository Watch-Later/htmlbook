#ifndef LINEBOX_H
#define LINEBOX_H

#include "pointer.h"

#include <string>
#include <list>
#include <memory>

namespace htmlbook {

class Box;
class FlowLineBox;
class RootLineBox;

class LineBox : public HeapMember {
public:
    LineBox(Box* box);

    virtual ~LineBox();
    virtual bool isTextLineBox() const { return false; }
    virtual bool isReplacedLineBox() const { return false; }
    virtual bool isFlowLineBox() const { return false; }
    virtual bool isRootLineBox() const { return false; }

    Box* box() const { return m_box; }
    FlowLineBox* parentLine() const { return m_parentLine; }
    LineBox* nextOnLine() const { return m_nextOnLine; }
    LineBox* prevOnLine() const { return m_prevOnLine; }

    void setParentLine(FlowLineBox* line) { m_parentLine = line; }
    void setNextOnLine(LineBox* line) { m_nextOnLine = line; }
    void setPrevOnLine(LineBox* line) { m_prevOnLine = line; }

    RootLineBox* rootLine() const;

    float x() const { return m_x; }
    float y() const { return m_y; }
    float width() const { return m_width; }
    float height() const { return m_height; }

    void setX(float x) { m_x = x; }
    void setY(float y) { m_y = y; }
    void setWidth(float width) { m_width = width; }
    void setHeight(float height) { m_height = height; }

private:
    Box* m_box;
    FlowLineBox* m_parentLine{nullptr};
    LineBox* m_nextOnLine{nullptr};
    LineBox* m_prevOnLine{nullptr};

    float m_x{0};
    float m_y{0};
    float m_width{0};
    float m_height{0};
};

using LineBoxList = std::pmr::vector<std::unique_ptr<LineBox>>;

class TextBox;

class TextLineBox final : public LineBox {
public:
    static std::unique_ptr<TextLineBox> create(TextBox* box, std::string text);

    bool isTextLineBox() const final { return true; }

    const std::string& text() const { return m_text; }

private:
    TextLineBox(TextBox* box, std::string text);
    std::string m_text;
};

template<>
struct is_a<TextLineBox> {
    static bool check(const LineBox& line) { return line.isTextLineBox(); }
};

using TextLineBoxList = std::pmr::vector<std::unique_ptr<TextLineBox>>;

class BoxFrame;

class ReplacedLineBox final : public LineBox {
public:
    static std::unique_ptr<ReplacedLineBox> create(BoxFrame* box);

    bool isReplacedLineBox() const final { return true; }

private:
    ReplacedLineBox(BoxFrame* box);
};

template<>
struct is_a<ReplacedLineBox> {
    static bool check(const LineBox& line) { return line.isReplacedLineBox(); }
};

using ReplacedLineBoxList = std::pmr::vector<std::unique_ptr<ReplacedLineBox>>;

class BoxModel;

class FlowLineBox : public LineBox {
public:
    static std::unique_ptr<FlowLineBox> create(BoxModel* box);

    bool isFlowLineBox() const final { return true; }

    LineBox* firstLine() const { return m_firstLine; }
    LineBox* lastLine() const { return m_lastLine; }

    void addLine(LineBox* line);
    void removeLine(LineBox* line);

    float borderTop() const { return 0; }
    float borderBottom() const { return 0; }
    float borderLeft() const { return 0; }
    float borderRight() const { return 0; }

protected:
    FlowLineBox(BoxModel* box);
    LineBox* m_firstLine{nullptr};
    LineBox* m_lastLine{nullptr};
};

template<>
struct is_a<FlowLineBox> {
    static bool check(const LineBox& line) { return line.isFlowLineBox(); }
};

using FlowLineBoxList = std::pmr::vector<std::unique_ptr<FlowLineBox>>;

class BlockFlowBox;

class RootLineBox final : public FlowLineBox {
public:
    static std::unique_ptr<RootLineBox> create(BlockFlowBox* box);

    bool isRootLineBox() const final { return true; }

private:
    RootLineBox(BlockFlowBox* box);
};

template<>
struct is_a<RootLineBox> {
    static bool check(const LineBox& line) { return line.isRootLineBox(); }
};

using RootLineBoxList = std::pmr::vector<std::unique_ptr<RootLineBox>>;

class LineLayout : public HeapMember {
public:
    static std::unique_ptr<LineLayout> create(BlockFlowBox* box);

    BlockFlowBox* box() const { return m_box; }
    const RootLineBoxList& lines() const { return m_lines; }
    bool empty() const { return m_lines.empty(); }

    void computeIntrinsicWidths(float& minWidth, float& maxWidth) const;

    void build();
    void layout();

private:
    LineLayout(BlockFlowBox* box);
    BlockFlowBox* m_box;
    RootLineBoxList m_lines;
};

} // htmlbook

#endif // LINEBOX_H
