#ifndef COUNTERS_H
#define COUNTERS_H

#include "boxstyle.h"

namespace htmlbook {

using Counter = std::pmr::map<HeapString, int>;

class Box;

class Counters {
public:
    explicit Counters(Document* document)
        : m_document(document)
    {}

    void push() { m_counters.push_back(nullptr); }
    void pop() { m_counters.pop_back(); }

    void update(const Box* box);
    void update(const Box* box, CSSPropertyID id);

    void increaseQuoteDepth() { ++m_quoteDepth; }
    void decreaseQuoteDepth() { --m_quoteDepth; }
    size_t quoteDepth() const { return m_quoteDepth; }

    HeapString format(const HeapString& name, ListStyleType listStyle, const HeapString& separator) const;

    int value(const HeapString& name) const;
    std::vector<int> values(const HeapString& name) const;

    void reset(const HeapString& name, int value);
    void set(const HeapString& name, int value);
    void increment(const HeapString& name, int value);

private:
    Counter* find(const HeapString& name) const;
    Document* m_document;
    std::pmr::vector<std::unique_ptr<Counter>> m_counters;
    size_t m_quoteDepth{0};
};

} // namespace htmlbook

#endif // COUNTERS_H
