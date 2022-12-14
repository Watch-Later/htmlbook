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

class LineBox {
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

using LineBoxList = std::list<std::unique_ptr<LineBox>>;

class TextBox;

class TextLineBox final : public LineBox {
public:
    TextLineBox(TextBox* box, std::string text);

    bool isTextLineBox() const final { return true; }

    const std::string& text() const { return m_text; }

private:
    std::string m_text;
};

template<>
struct is<TextLineBox> {
    static bool check(const LineBox& line) { return line.isTextLineBox(); }
};

class BoxFrame;

class ReplacedLineBox final : public LineBox {
public:
    ReplacedLineBox(BoxFrame* box);

    bool isReplacedLineBox() const final { return true; }
};

template<>
struct is<ReplacedLineBox> {
    static bool check(const LineBox& line) { return line.isReplacedLineBox(); }
};

class BoxModel;

class FlowLineBox : public LineBox {
public:
    FlowLineBox(BoxModel* box);

    bool isFlowLineBox() const final { return true; }

    LineBox* firstLine() const { return m_firstLine; }
    LineBox* lastLine() const { return m_lastLine; }

    void addLine(LineBox* line);
    void removeLine(LineBox* line);

private:
    LineBox* m_firstLine{nullptr};
    LineBox* m_lastLine{nullptr};
};

template<>
struct is<FlowLineBox> {
    static bool check(const LineBox& line) { return line.isFlowLineBox(); }
};

class BlockFlowBox;

class RootLineBox final : public FlowLineBox {
public:
    RootLineBox(BlockFlowBox* box);

    bool isRootLineBox() const final { return true; }
};

template<>
struct is<RootLineBox> {
    static bool check(const LineBox& line) { return line.isRootLineBox(); }
};

} // htmlbook

#endif // LINEBOX_H
