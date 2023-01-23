#ifndef TEXTBOX_H
#define TEXTBOX_H

#include "box.h"

namespace htmlbook {

class TextBox : public Box {
public:
    TextBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isOfType(Type type) const override { return type == Type::Text || Box::isOfType(type); }

    const HeapString& text() const { return m_text; }
    void setText(const HeapString& text) { m_text = text; }

    void addLine(std::unique_ptr<TextLineBox> line);
    void removeLine(LineBox* line);
    const TextLineBoxList& lines() const { return m_lines; }

    const char* name() const override { return "TextBox"; }

private:
    HeapString m_text;
    TextLineBoxList m_lines;
};

template<>
struct is_a<TextBox> {
    static bool check(const Box& box) { return box.isOfType(Box::Type::Text); }
};

} // namespace htmlbook

#endif // TEXTBOX_H
