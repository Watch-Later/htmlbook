#ifndef LINEBOX_H
#define LINEBOX_H

#include "pointer.h"

namespace htmlbook {

class Box;
class FlowLineBox;
class RootLineBox;

class LineBox {
public:
    LineBox(Box* box);
    virtual ~LineBox();

    virtual bool isTextLineBox() const { return false; }
    virtual bool isPlaceHolderLineBox() const { return false; }
    virtual bool isFlowLineBox() const { return false; }
    virtual bool isRootLineBox() const { return false; }

    Box* box() const { return m_box; }
    FlowLineBox* parentLine() const { return m_parentLine; }
    LineBox* nextOnLine() const { return m_nextOnLine; }
    LineBox* prevOnLine() const { return m_prevOnLine; }
    LineBox* nextOnBox() const { return m_nextOnBox; }
    LineBox* prevOnBox() const { return m_prevOnBox; }

    void setParentLine(FlowLineBox* line) { m_parentLine = line; }
    void setNextOnLine(LineBox* line) { m_nextOnLine = line; }
    void setPrevOnLine(LineBox* line) { m_prevOnLine = line; }
    void setNextOnBox(LineBox* line) { m_nextOnBox = line; }
    void setPrevOnBox(LineBox* line) { m_prevOnBox = line; }

    RootLineBox* rootLine() const;

private:
    Box* m_box;
    FlowLineBox* m_parentLine{nullptr};
    LineBox* m_nextOnLine{nullptr};
    LineBox* m_prevOnLine{nullptr};
    LineBox* m_nextOnBox{nullptr};
    LineBox* m_prevOnBox{nullptr};
};

class LineBoxList {
public:
    LineBoxList() = default;
    ~LineBoxList();

    LineBox* firstLine() const { return m_firstLine; }
    LineBox* lastLine() const { return m_lastLine; }

    void add(Box* box, LineBox* line);
    void remove(Box* box, LineBox* line);
    bool empty() const { return !m_firstLine; }

private:
    LineBox* m_firstLine{nullptr};
    LineBox* m_lastLine{nullptr};
};

class TextLineBox final : public LineBox {
public:
    TextLineBox(Box* box, int begin, int end);

    bool isTextLineBox() const final { return true; }

    int begin() const { return m_begin; }
    int end() const { return m_end; }

private:
    int m_begin;
    int m_end;
};

template<>
struct is<TextLineBox> {
    static bool check(const LineBox& line) { return line.isTextLineBox(); }
};

class BoxFrame;

class PlaceHolderLineBox final : public LineBox {
public:
    PlaceHolderLineBox(Box* box, BoxFrame* placeHolderBox);
    ~PlaceHolderLineBox() final;

    bool isPlaceHolderLineBox() const final { return true; }

    BoxFrame* placeHolderBox() const { return m_placeHolderBox; }

private:
    BoxFrame* m_placeHolderBox;
};

template<>
struct is<PlaceHolderLineBox> {
    static bool check(const LineBox& line) { return line.isPlaceHolderLineBox(); }
};

class FlowLineBox : public LineBox {
public:
    FlowLineBox(Box* box);
    ~FlowLineBox() override;

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

class RootLineBox final : public FlowLineBox {
public:
    RootLineBox(Box* box);

    bool isRootLineBox() const final { return true; }
};

template<>
struct is<RootLineBox> {
    static bool check(const LineBox& line) { return line.isRootLineBox(); }
};

} // htmlbook

#endif // LINEBOX_H
