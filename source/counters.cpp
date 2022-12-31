#include "counters.h"
#include "htmldocument.h"
#include "listitembox.h"

#include <cassert>

namespace htmlbook {

void Counters::update(const Box* box)
{
    update(box, CSSPropertyID::CounterReset);
    update(box, CSSPropertyID::CounterSet);
    update(box, CSSPropertyID::CounterIncrement);

    static const GlobalString listItem("list-item");
    auto node = box->node();
    if(is<ListItemBox>(box)) {
        if(node && node->tagName() == liTag) {
            auto element = static_cast<HTMLLIElement*>(node);
            if(auto value = element->value()) {
                reset(listItem, *value);
                return;
            }
        }

        increment(listItem, 1);
        return;
    }

    if(node == nullptr)
        return;
    if(node->tagName() == olTag) {
        auto element = static_cast<HTMLOLElement*>(node);
        reset(listItem, element->start());
        return;
    }

    if(node->tagName() == ulTag
        || node->tagName() == dirTag
        || node->tagName() == menuTag) {
        reset(listItem, 0);
    }
}

void Counters::update(const Box* box, CSSPropertyID id)
{
    auto value = box->style()->get(id);
    if(value == nullptr || !is<CSSListValue>(*value))
        return;
    for(auto& counter : to<CSSListValue>(*value).values()) {
        auto& pair = to<CSSPairValue>(*counter);
        auto& name = to<CSSCustomIdentValue>(*pair.first());
        auto& value = to<CSSIntegerValue>(*pair.second());
        switch(id) {
        case CSSPropertyID::CounterReset:
            reset(name.value(), value.value());
            break;
        case CSSPropertyID::CounterSet:
            set(name.value(), value.value());
            break;
        case CSSPropertyID::CounterIncrement:
            increment(name.value(), value.value());
            break;
        default:
            assert(false);
        }
    }
}

HeapString Counters::format(const HeapString& name, ListStyleType listStyle, const HeapString& separator) const
{
    return emptyGlo;
}

int Counters::value(const HeapString& name) const
{
    auto counter = find(name);
    if(counter == nullptr)
        return 0;
    return counter->at(name);
}

std::vector<int> Counters::values(const HeapString& name) const
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

void Counters::reset(const HeapString& name, int value)
{
    auto& counter = m_counters.back();
    if(counter == nullptr)
        counter = std::make_unique<Counter>(m_document->heap());
    counter->operator[](name) = value;
}

void Counters::set(const HeapString& name, int value)
{
    auto counter = find(name);
    if(counter == nullptr) {
        reset(name, value);
        return;
    }

    counter->at(name) = value;
}

void Counters::increment(const HeapString& name, int value)
{
    auto counter = find(name);
    if(counter == nullptr) {
        reset(name, value);
        return;
    }

    counter->at(name) += value;
}

Counter* Counters::find(const HeapString& name) const
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
