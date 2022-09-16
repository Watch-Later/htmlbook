#include "counter.h"
#include "boxstyle.h"

namespace htmlbook {

void Counters::reset(const GlobalString& name, int value)
{
    auto& counter = m_counters.back();
    if(counter == nullptr)
        counter = std::make_unique<Counter>();
    counter->at(name) = value;
}

void Counters::set(const GlobalString& name, int value)
{
    auto counter = find(name);
    if(counter == nullptr) {
        reset(name, value);
        return;
    }

    counter->at(name) = value;
}

void Counters::increment(const GlobalString& name, int value)
{
    auto counter = find(name);
    if(counter == nullptr) {
        reset(name, value);
        return;
    }

    counter->at(name) += value;
}

void Counters::update(const BoxStyle& style)
{
    if(auto value = style.get(CSSPropertyID::CounterReset)) {
        for(auto& counter : to<CSSListValue>(*value)->values()) {
            auto pair = to<CSSPairValue>(*counter);
            auto name = to<CSSCustomIdentValue>(*pair->first());
            auto value = to<CSSIntegerValue>(*pair->second());
            reset(name->value(), value->value());
        }
    }

    if(auto value = style.get(CSSPropertyID::CounterSet)) {
        for(auto& counter : to<CSSListValue>(*value)->values()) {
            auto pair = to<CSSPairValue>(*counter);
            auto name = to<CSSCustomIdentValue>(*pair->first());
            auto value = to<CSSIntegerValue>(*pair->second());
            set(name->value(), value->value());
        }
    }

    if(auto value = style.get(CSSPropertyID::CounterIncrement)) {
        for(auto& counter : to<CSSListValue>(*value)->values()) {
            auto pair = to<CSSPairValue>(*counter);
            auto name = to<CSSCustomIdentValue>(*pair->first());
            auto value = to<CSSIntegerValue>(*pair->second());
            increment(name->value(), value->value());
        }
    }
}

int Counters::value(const GlobalString& name) const
{
    auto counter = find(name);
    if(counter == nullptr)
        return 0;
    return counter->at(name);
}

std::vector<int> Counters::values(const GlobalString& name) const
{
    std::vector<int> values;
    for(auto& counter : m_counters) {
        if(counter == nullptr)
            continue;
        auto it = counter->find(name);
        if(it == counter->end())
            continue;
        values.push_back(it->second);
    }

    return values;
}

Counter* Counters::find(const GlobalString& name) const
{
    auto it = m_counters.rbegin();
    auto end = m_counters.rend();
    for(; it != end; ++it) {
        auto& counter = *it;
        if(counter == nullptr)
            continue;
        auto it = counter->find(name);
        if(it == counter->end())
            continue;
        return counter.get();
    }

    return nullptr;
}

} // namespace htmlbook
