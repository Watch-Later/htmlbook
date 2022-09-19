#ifndef COUNTER_H
#define COUNTER_H

#include "boxstyle.h"

namespace htmlbook {

using Counter = std::map<GlobalString, int>;

class Counters {
public:
    Counters() = default;

    void push() { m_counters.push_back(nullptr); }
    void pop() { m_counters.pop_back(); }

    void update(const BoxStyle& style);

    void increaseQuoteDepth() { ++m_quoteDepth; }
    void decreaseQuoteDepth() { --m_quoteDepth; }
    size_t quoteDepth() const { return m_quoteDepth; }

    std::string format(const GlobalString& name, ListStyleType listStyle, std::string_view seperator) const;

    int value(const GlobalString& name) const;
    std::vector<int> values(const GlobalString& name) const;

    void reset(const GlobalString& name, int value);
    void set(const GlobalString& name, int value);
    void increment(const GlobalString& name, int value);

private:
    Counter* find(const GlobalString& name) const;
    std::vector<std::unique_ptr<Counter>> m_counters;
    size_t m_quoteDepth{0};
};

} // namespace htmlbook

#endif // COUNTER_H
