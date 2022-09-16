#ifndef COUNTER_H
#define COUNTER_H

#include "globalstring.h"

#include <map>
#include <memory>

namespace htmlbook {

class BoxStyle;

using Counter = std::map<GlobalString, int>;

class Counters {
public:
    Counters() = default;

    void push() { m_counters.push_back(nullptr); }
    void pop() { m_counters.pop_back(); }

    void reset(const GlobalString& name, int value);
    void set(const GlobalString& name, int value);
    void increment(const GlobalString& name, int value);

    void update(const BoxStyle& style);

    void increaseQuoteDepth() { ++m_quoteDepth; }
    void decreaseQuoteDepth() { --m_quoteDepth; }

    int value(const GlobalString& name) const;
    std::vector<int> values(const GlobalString& name) const;
    size_t quoteDepth() const { return m_quoteDepth; }

private:
    Counter* find(const GlobalString& name) const;
    std::vector<std::unique_ptr<Counter>> m_counters;
    size_t m_quoteDepth{0};
};

} // namespace htmlbook

#endif // COUNTER_H
