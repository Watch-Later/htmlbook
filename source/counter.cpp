#include "counter.h"

#include <cassert>

namespace htmlbook {

void Counters::update(const Box* box)
{
    update(box, CSSPropertyID::CounterReset);
    update(box, CSSPropertyID::CounterSet);
    update(box, CSSPropertyID::CounterIncrement);
}

void Counters::update(const Box* box, CSSPropertyID id)
{
    auto value = box->style()->get(id);
    if(value == nullptr) {
        static const GlobalString listItem("list-item");
        if(id == CSSPropertyID::CounterIncrement && box->isListItemBox())
            increment(listItem, 1);
        return;
    }

    if(!value->isListValue())
        return;
    for(auto& counter : to<CSSListValue>(*value)->values()) {
        auto pair = to<CSSPairValue>(*counter);
        auto name = to<CSSCustomIdentValue>(*pair->first());
        auto value = to<CSSIntegerValue>(*pair->second());
        switch(id) {
        case CSSPropertyID::CounterReset:
            reset(name->value(), value->value());
            break;
        case CSSPropertyID::CounterSet:
            set(name->value(), value->value());
            break;
        case CSSPropertyID::CounterIncrement:
            increment(name->value(), value->value());
            break;
        default:
            assert(false);
        }
    }
}

std::string Counters::format(const GlobalString& name, ListStyleType listStyle, std::string_view separator) const
{
    std::string value;
    return value;
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

void Counters::reset(const GlobalString& name, int value)
{
    auto& counter = m_counters.back();
    if(counter == nullptr)
        counter = std::make_unique<Counter>();
    counter->operator[](name) = value;
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
