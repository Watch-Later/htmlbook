#ifndef HEAPSTRING_H
#define HEAPSTRING_H

#include <string_view>
#include <memory_resource>
#include <ostream>
#include <cstring>

namespace htmlbook {

using Heap = std::pmr::monotonic_buffer_resource;

class HeapString {
public:
    HeapString() = default;

    const char* data() const { return m_value.data(); }
    size_t length() const { return m_value.length(); }

    const char* begin() const { return m_value.data(); }
    const char* end() const { return m_value.data() + m_value.length(); }

    const char& at(size_t index) const { return m_value.at(index); }
    const char& operator[](size_t index) const { return m_value.operator[](index); }

    const char& front() const { return m_value.front(); }
    const char& back() const { return m_value.back(); }

    bool empty() const { return m_value.empty(); }

    HeapString substring(size_t offset) const { return m_value.substr(offset); }
    HeapString substring(size_t offset, size_t count) const { return m_value.substr(offset, count); }

    const std::string_view& value() const { return m_value; }
    operator const std::string_view&() const { return m_value; }

    static HeapString create(Heap* heap, const char* data) { return create(heap, data, std::strlen(data)); }
    static HeapString create(Heap* heap, const std::string_view& value) { return create(heap, value.data(), value.length()); }
    static HeapString create(Heap* heap, const char* data, size_t length);

private:
    HeapString(const std::string_view& value) : m_value(value) {}
    std::string_view m_value;
};

inline HeapString HeapString::create(Heap* heap, const char* data, size_t length)
{
    auto content = static_cast<char*>(heap->allocate(length, alignof(char)));
    std::memcpy(content, data, length);
    return HeapString({content, length});
}

inline std::ostream& operator<<(std::ostream& o, const HeapString& in) { return o << in.value(); }

inline bool operator==(const HeapString& a, const HeapString& b) { return a.value() == b.value(); }
inline bool operator!=(const HeapString& a, const HeapString& b) { return a.value() != b.value(); }

inline bool operator==(const HeapString& a, const std::string_view& b) { return a.value() == b; }
inline bool operator!=(const HeapString& a, const std::string_view& b) { return a.value() != b; }

inline bool operator==(const std::string_view& a, const HeapString& b) { return a == b.value(); }
inline bool operator!=(const std::string_view& a, const HeapString& b) { return a != b.value(); }

inline bool operator<(const HeapString& a, const HeapString& b) { return a.value() < b.value(); }
inline bool operator>(const HeapString& a, const HeapString& b) { return a.value() > b.value(); }

inline bool operator<(const HeapString& a, const std::string_view& b) { return a.value() < b; }
inline bool operator>(const HeapString& a, const std::string_view& b) { return a.value() > b; }

inline bool operator<(const std::string_view& a, const HeapString& b) { return a < b.value(); }
inline bool operator>(const std::string_view& a, const HeapString& b) { return a > b.value(); }

} // namespace htmlbook

#endif // HEAPSTRING_H
