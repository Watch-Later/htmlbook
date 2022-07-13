#ifndef PARSERSTRING_H
#define PARSERSTRING_H

#include <string_view>
#include <cassert>
#include <iostream>

namespace htmlbook {

class ParserString {
public:
    explicit ParserString(const std::string_view& value)
        : ParserString(value.begin(), value.begin(), value.end())
    {}

    ParserString(const char* begin, const char* end)
        : ParserString(begin, begin, end)
    {}

    ParserString(const char* current, const char* begin, const char* end)
        : m_current(current), m_begin(begin), m_end(end)
    {}

    ParserString operator+(size_t count) const {
        auto current = m_current + count;
        assert(m_end >= current);
        return ParserString(current, m_begin, m_end);
    }

    ParserString operator-(size_t count) const {
        auto current = m_current - count;
        assert(current >= m_begin);
        return ParserString(current, m_begin, m_end);
    }

    ParserString& operator+=(size_t count) {
        *this = *this + count;
        return *this;
    }

    ParserString& operator-=(size_t count) {
        *this = *this - count;
        return *this;
    }

    const char& operator*() const {
        assert(m_current < m_end);
        return *m_current;
    }

    char peek(size_t count = 0) const {
        auto current = m_current + count;
        assert(m_end >= current);
        if(current == m_end)
            return 0;
        return *current;
    }

    char advance(size_t count = 1) {
        m_current += count;
        assert(m_end >= m_current);
        if(m_current == m_end)
            return 0;
        return *m_current;
    }

    char get() const {
        assert(m_end >= m_current);
        if(m_current == m_end)
            return 0;
        return *m_current;
    }

    std::string_view string(size_t offset, size_t count) const { return string().substr(offset, count); }
    std::string_view substring(size_t offset, size_t count) const { return substring().substr(offset, count); }

    std::string_view string() const { return std::string_view(m_begin, length()); }
    std::string_view substring() const { return std::string_view(m_current, sublength()); }

    size_t offset() const { return m_current - m_begin; }
    size_t length() const { return m_end - m_begin; }
    size_t sublength() const { return m_end - m_current; }

    const char* current() const { return m_current; }
    const char* begin() const { return m_begin; }
    const char* end() const { return m_end; }

    bool empty() const { return m_current == m_end; }

private:
    const char* m_current;
    const char* m_begin;
    const char* m_end;
};

constexpr bool isspace(int cc) { return (cc == ' ' || cc == '\n' || cc == '\t' || cc == '\r' || cc == '\f'); }
constexpr bool isdigit(int cc) { return (cc >= '0' && cc <= '9'); }
constexpr bool isupper(int cc) { return (cc >= 'A' && cc <= 'Z'); }
constexpr bool islower(int cc) { return (cc >= 'a' && cc <= 'z'); }
constexpr bool isalpha(int cc) { return isupper(cc) || islower(cc); }

constexpr bool isxupper(int cc) { return (cc >= 'A' && cc <= 'F'); }
constexpr bool isxlower(int cc) { return (cc >= 'a' && cc <= 'f'); }
constexpr bool isxdigit(int cc) { return isdigit(cc) || isxupper(cc) || isxlower(cc); }

constexpr int xdigit(int cc) {
    if(isdigit(cc))
        return cc - '0';
    if(isxupper(cc))
        return 10 + cc - 'A';
    if(isxlower(cc))
        return 10 + cc - 'a';
    return 0;
}

constexpr char tolower(int cc) {
    if(isupper(cc))
        return cc + 0x20;
    return cc;
}

constexpr bool equals(int a, int b, bool caseSensitive) {
    if(caseSensitive)
        return a == b;
    return tolower(a) == tolower(b);
}

constexpr bool equals(const char* aData, size_t aLength, const char* bData, size_t bLength, bool caseSensitive) {
    if(aLength != bLength)
        return false;

    auto aEnd = aData + aLength;
    while(aData != aEnd) {
        if(!equals(*aData, *bData, caseSensitive))
            return false;
        ++aData;
        ++bData;
    }

    return true;
}

constexpr bool equals(const std::string_view& a, const std::string_view& b, bool caseSensitive) {
    return equals(a.data(), a.length(), b.data(), b.length(), caseSensitive);
}

constexpr bool contains(const std::string_view& value, const std::string_view& subvalue, bool caseSensitive) {
    auto it = value.begin();
    auto end = value.end();
    while(true) {
        while(it < end && !isspace(*it))
            ++it;
        if(it == end)
            return false;
        size_t count = 0;
        auto begin = it;
        while(it < end && !isspace(*it)) {
            ++count;
            ++it;
        }

        if(equals(begin, count, subvalue.data(), subvalue.length(), caseSensitive))
            return true;
        ++it;
    }

    return false;
}

constexpr bool includes(const std::string_view& value, const std::string_view& subvalue, bool caseSensitive) {
    auto it = value.begin();
    auto end = value.end();
    while(true) {
        while(it < end && !isspace(*it))
            ++it;
        if(it == end)
            return false;
        size_t count = 0;
        auto begin = it;
        while(it < end && !isspace(*it)) {
            ++count;
            ++it;
        }

        if(equals(begin, count, subvalue.data(), subvalue.length(), caseSensitive))
            return true;
        ++it;
    }

    return false;
}

constexpr bool startswith(const std::string_view& value, const std::string_view& prefix, bool caseSensitive) {
    if(value.empty() || prefix.length() > value.length())
        return false;
    auto subvalue = value.substr(0, prefix.size());
    return equals(subvalue, prefix, caseSensitive);
}

constexpr bool endswith(const std::string_view& value, const std::string_view& suffix, bool caseSensitive) {
    if(value.empty() || suffix.length() > value.length())
        return false;
    auto subvalue = value.substr(value.size() - suffix.size(), suffix.size());
    return equals(subvalue, suffix, caseSensitive);
}

constexpr bool dashequals(const std::string_view& value, const std::string_view& subvalue, bool caseSensitive) {
    if(!startswith(value, subvalue, caseSensitive))
        return false;
    if(value.length() == subvalue.length())
        return true;
    return value.at(subvalue.length()) == '-';
}

inline void appendCodepoint(std::string& output, uint32_t cp) {
    char c[5] = {0, 0, 0, 0, 0};
    if(cp < 0x80) {
        c[1] = 0;
        c[0] = cp;
    } else if(cp < 0x800) {
        c[2] = 0;
        c[1] = (cp & 0x3F) | 0x80;
        cp >>= 6;
        c[0] = cp | 0xC0;
    } else if(cp < 0x10000) {
        c[3] = 0;
        c[2] = (cp & 0x3F) | 0x80;
        cp >>= 6;
        c[1] = (cp & 0x3F) | 0x80;
        cp >>= 6;
        c[0] = cp | 0xE0;
    } else if(cp < 0x110000) {
        c[4] = 0;
        c[3] = (cp & 0x3F) | 0x80;
        cp >>= 6;
        c[2] = (cp & 0x3F) | 0x80;
        cp >>= 6;
        c[1] = (cp & 0x3F) | 0x80;
        cp >>= 6;
        c[0] = cp | 0xF0;
    }

    output.append(c);
}

} // namespace htmlbook

#endif // PARSERSTRING_H
