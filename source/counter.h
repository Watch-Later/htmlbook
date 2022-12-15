#ifndef COUNTER_H
#define COUNTER_H

#include "box.h"

namespace htmlbook {

using Counter = std::map<HeapString, int>;

class Counters {
public:
    Counters() = default;

    void push() { m_counters.push_back(nullptr); }
    void pop() { m_counters.pop_back(); }

    void update(const Box* box);
    void update(const Box* box, CSSPropertyID id);

    void increaseQuoteDepth() { ++m_quoteDepth; }
    void decreaseQuoteDepth() { --m_quoteDepth; }
    size_t quoteDepth() const { return m_quoteDepth; }

    std::string format(const HeapString& name, ListStyleType listStyle, const std::string_view& separator) const;

    int value(const HeapString& name) const;
    std::vector<int> values(const HeapString& name) const;

    void reset(const HeapString& name, int value);
    void set(const HeapString& name, int value);
    void increment(const HeapString& name, int value);

private:
    Counter* find(const HeapString& name) const;
    std::vector<std::unique_ptr<Counter>> m_counters;
    size_t m_quoteDepth{0};
};

} // namespace htmlbook

#endif // COUNTER_H
