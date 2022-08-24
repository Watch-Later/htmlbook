#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <vector>

namespace htmlbook {

class Point {
public:
    Point() = default;
    Point(int x, int y) : x(x), y(y) {}

public:
    int x{0};
    int y{0};
};

class Size {
public:
    Size() = default;
    Size(int width, int height) : w(width), h(height) {}

public:
    int w{0};
    int h{0};
};

class Rect {
public:
    Rect() = default;
    Rect(int x, int y, int w, int h) : x(x), y(y), w(w), h(h) {}

public:
    int x{0};
    int y{0};
    int w{0};
    int h{0};
};

class PointF {
public:
    PointF() = default;
    PointF(float x, float y) : x(x), y(y) {}

public:
    float x{0.f};
    float y{0.f};
};

class SizeF {
public:
    SizeF() = default;
    SizeF(float width, float height) : w(width), h(height) {}

public:
    float w{0.f};
    float h{0.f};
};

class RectF {
public:
    RectF() = default;
    RectF(float x, float y, float w, float h) : x(x), y(y), w(w), h(h) {}

public:
    float x{0.f};
    float y{0.f};
    float w{0.f};
    float h{0.f};
};

float deg(float radians);
float rad(float degrees);

class Transform {
public:
    Transform() = default;
    Transform(float a, float b, float c, float d, float e, float f) : a(a), b(b), c(c), d(d), e(e), f(f) {}

    Transform inverted() const;
    Transform operator*(const Transform& transform) const;
    Transform& operator*=(const Transform& transform);

    Transform& premultiply(const Transform& transform);
    Transform& postmultiply(const Transform& transform);
    Transform& rotate(float angle);
    Transform& rotate(float angle, float cx, float cy);
    Transform& scale(float sx, float sy);
    Transform& shear(float shx, float shy);
    Transform& translate(float tx, float ty);
    Transform& transform(float a, float b, float c, float d, float e, float f);
    Transform& identity();
    Transform& invert();

    static Transform rotated(float angle);
    static Transform rotated(float angle, float cx, float cy);
    static Transform scaled(float sx, float sy);
    static Transform sheared(float shx, float shy);
    static Transform translated(float tx, float ty);

public:
    float a{1.f};
    float b{0.f};
    float c{0.f};
    float d{1.f};
    float e{0.f};
    float f{0.f};
};

class Path {
public:
    enum class Command : uint8_t {
        MoveTo,
        LineTo,
        CubicTo,
        Close
    };

    using CommandList = std::vector<Command>;
    using PointList = std::vector<PointF>;

    Path() = default;

    void moveTo(float x, float y);
    void lineTo(float x, float y);
    void quadTo(float x1, float y1, float x2, float y2);
    void cubicTo(float x1, float y1, float x2, float y2);
    void close();

    void translate(float x, float y);
    void scale(float x, float y);
    void transform(const Transform& transform);

    const CommandList& commands() const { return m_commands; }
    const PointList& points() const { return m_points; }

private:
    CommandList m_commands;
    PointList m_points;
};

} // htmlbook

#endif // GEOMETRY_H
