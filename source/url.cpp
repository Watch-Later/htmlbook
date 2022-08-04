#include "url.h"
#include "parserstring.h"

#include <cstring>

namespace htmlbook {

constexpr bool isSchemeChar(int c) { return isalpha(c) || isdigit(c) || c == '+' || c == '-' || c == '.'; }
constexpr bool isHostnameChar(int c) { return isalpha(c) || isdigit(c) || c == '.' || c == '-' || c == '_'; }
constexpr bool isIPv6Char(int c) { return isxdigit(c) || c == ':' || c == '%'; }
constexpr bool isPathEndChar(int c) { return c == '#' || c == '?' || c == '/' || c == 0; }
constexpr bool isMark(int c) { return c == '-' || c == '_' || c == '.' || c == '!' || c == '~' || c == '*' || c == '\'' || c == '(' || c == ')'; }
constexpr bool isUnreservedChar(int c) { return isalpha(c) || isdigit(c) ||  isMark(c); }
constexpr bool isUserInfoChar(int c) { return isUnreservedChar(c) || c == '%' || c == ';' || c == ':' || c == '&' || c == '=' || c == '+' || c == '$' || c == ','; }

Url::Url(const std::string_view& value)
{
    if(value.empty() || !isalpha(value.front()))
        return;

    auto peek = [&](auto index) {
        if(index >= value.length())
            return char(0);
        return value[index];
    };

    int schemeEnd = 0;
    while(isSchemeChar(peek(schemeEnd)))
        ++schemeEnd;

    if(peek(schemeEnd) != ':')
        return;

    bool isHttp = false;
    bool isHttps = false;
    bool isFile = false;
    auto valueData = value.data();
    if(equals(valueData, schemeEnd, "http", 4, false))
        isHttp = true;
    else if(equals(valueData, schemeEnd, "https", 5, false))
        isHttps = true;
    else if(equals(valueData, schemeEnd, "file", 4, false))
        isFile = true;

    int userBegin = 0;
    int userEnd = 0;
    int passwordBegin = 0;
    int passwordEnd = 0;
    int hostBegin = 0;
    int hostEnd = 0;
    int portBegin = 0;
    int portEnd = 0;

    bool hierarchical = peek(schemeEnd + 1) == '/';
    if(hierarchical && peek(schemeEnd + 2) == '/') {
        userBegin = schemeEnd + 3;
        userEnd = userBegin;

        int colon = 0;
        while(isUserInfoChar(peek(userEnd))) {
            if(colon == 0 && peek(userEnd) == ':')
                colon = userEnd;
            ++userEnd;
        }

        if(peek(userEnd) == '@') {
            if(colon == 0) {
                passwordBegin = userEnd;
                passwordEnd = passwordBegin;
            } else {
                passwordBegin = colon + 1;
                passwordEnd = userEnd;
                userEnd = colon;
            }

            hostBegin = passwordEnd + 1;
        } else if(peek(userEnd) == '[' || isPathEndChar(peek(userEnd))) {
            hostBegin = userBegin;
            userEnd = hostBegin;
            passwordBegin = userEnd;
            passwordEnd = passwordBegin;
        } else {
            return;
        }

        hostEnd = hostBegin;
        if(peek(hostEnd) == '[') {
            ++hostEnd;
            while(isIPv6Char(peek(hostEnd)))
                ++hostEnd;
            if(peek(hostEnd) == ']')
                ++hostEnd;
            else {
                return;
            }
        } else {
            while(isHostnameChar(peek(hostEnd))) {
                ++hostEnd;
            }
        }

        if(peek(hostEnd) == ':') {
            portBegin = hostEnd + 1;
            portEnd = portBegin;
            while(isdigit(peek(portEnd))) {
                ++portEnd;
            }
        } else {
            portBegin = hostEnd;
            portEnd = portBegin;
        }

        if(!isPathEndChar(peek(portEnd)))
            return;
        if(userBegin == portEnd && !(isHttp || isHttps || isFile)) {
            userBegin = schemeEnd + 3;
            userEnd = userBegin;
            passwordBegin = userEnd;
            passwordEnd = passwordBegin;
            hostBegin = passwordEnd;
            hostEnd = hostBegin;
            portBegin = hostEnd;
            portEnd = portBegin;
        }
    } else {
        userBegin = schemeEnd + 1;
        userEnd = userBegin;
        passwordBegin = userEnd;
        passwordEnd = passwordBegin;
        hostBegin = passwordEnd;
        hostEnd = hostBegin;
        portBegin = hostEnd;
        portEnd = portBegin;
    }

    int pathBegin = portEnd;
    int pathEnd = pathBegin;
    while(pathEnd < value.length() && value[pathEnd] != '?' && value[pathEnd] != '#')
        ++pathEnd;

    int queryBegin = pathEnd;
    int queryEnd = queryBegin;
    if(peek(queryBegin) == '?') {
        do {
            ++queryEnd;
        } while(queryEnd < value.length() && value[queryEnd] != '#');
    }

    int fragmentBegin = queryEnd;
    int fragmentEnd = fragmentBegin;
    if(peek(fragmentBegin) == '#') {
        ++fragmentBegin;
        fragmentEnd = fragmentBegin;
        while(fragmentEnd < value.length()) {
            ++fragmentEnd;
        }
    }

    m_value.reserve(fragmentEnd);
    for(int i = 0; i < schemeEnd; ++i)
        m_value += tolower(value[i]);
    m_schemeEnd = m_value.length();
    m_value += ':';
    if(hostBegin != hostEnd || (isFile && pathBegin != pathEnd) || (userBegin != userEnd || passwordBegin != passwordEnd || hostEnd != portEnd)) {
        m_value += '/';
        m_value += '/';

        m_userBegin = m_value.length();
        m_value += value.substr(userBegin, userEnd - userBegin);
        m_userEnd = m_value.length();

        if(passwordBegin != passwordEnd) {
            m_value += ':';
            m_value += value.substr(passwordBegin, passwordEnd - passwordBegin);
        }

        m_passwordEnd = m_value.length();
        if(m_userBegin != m_value.length())
            m_value += '@';
        for(int i = hostBegin; i < hostEnd; ++i)
            m_value += tolower(value[i]);
        m_hostEnd = m_value.length();
        if(hostEnd != portBegin) {
            m_value += ':';
            m_value += value.substr(portBegin, portEnd - portBegin);
        }

        m_portEnd = m_value.length();
    } else {
        m_userBegin = m_value.length();
        m_userEnd = m_userBegin;
        m_passwordEnd = m_userEnd;
        m_hostEnd = m_passwordEnd;
        m_portEnd = m_hostEnd;
    }

    if(pathBegin == pathEnd && hierarchical && (isHttp || isHttps || isFile))
        m_value += '/';

    auto append = [&](auto begin, auto end) {
        constexpr char hexdigits[] = "0123456789ABCDEF";
        constexpr char unescaped[] = ";,/?@&=+$#-_.!~*'()";
        for(auto i = begin; i < end; ++i) {
            auto cc = value[i];
            if(isalpha(cc) || isdigit(cc) || std::strchr(unescaped, cc)) {
                m_value += cc;
                continue;
            }

            m_value += '%';
            m_value += hexdigits[cc >> 4];
            m_value += hexdigits[cc & 0xF];
        }
    };

    if(!hierarchical) {
        append(pathBegin, pathEnd);
    } else {
        auto begin = m_value.length();
        auto in = begin;
        auto out = begin;
        append(pathBegin, pathEnd);
        auto end = m_value.length();
        auto peek = [&](auto index) {
            index += in;
            if(index >= end)
                return char(0);
            return m_value[index];
        };

        while(in < end) {
            if(peek(0) == '.' && peek(1) == '/')
                in += 2;
            else if(peek(0) == '.' && peek(1) == '.' && peek(2) == '/')
                in += 3;

            if(peek(0) == '/' && peek(1) == '.' && (peek(2) == '/' || peek(2) == 0)) {
                in += 2;
                if(in < end)
                    continue;
                m_value[out++] = '/';
                break;
            }

            if(peek(0) == '/' && peek(1) == '.' && peek(2) == '.' && (peek(3) == '/' || peek(3) == 0)) {
                while(out > begin && m_value[--out] != '/');
                in += 3;
                if(in < end) {
                    if(out == begin && m_value[out] != '/')
                        in += 1;
                    continue;
                }

                if(m_value[out] == '/')
                    out += 1;
                break;
            }

            do {
                m_value[out++] = m_value[in++];
            } while(in < end && m_value[in] != '/');
        }

        m_value.erase(out, end - out);
    }

    m_pathEnd = m_value.length();
    append(queryBegin, queryEnd);
    m_queryEnd = m_value.length();
    if(fragmentBegin != queryEnd) {
        m_value += '#';
        append(fragmentBegin, fragmentEnd);
    }

    m_fragmentEnd = m_value.length();
}

Url Url::complete(std::string_view relative) const
{
    while(!relative.empty() && relative.front() == ' ')
        relative.remove_prefix(1);
    while(!relative.empty() && relative.back() == ' ')
        relative.remove_suffix(1);

    std::string input;
    input.reserve(relative.length());
    for(auto cc : relative) {
        if(cc == '\n' || cc == '\t' || cc == '\r')
            continue;
        input.push_back(cc);
    }

    if(m_value.empty())
        return Url{input};
    assert(m_value[m_schemeEnd] == ':');
    bool hierarchical = m_schemeEnd < m_userBegin && m_value[m_schemeEnd + 1] == '/';
    if(!input.empty() && isalpha(input.front())) {
        auto it = input.begin();
        auto end = input.end();
        do {
            ++it;
        } while(it != end && isSchemeChar(*it));
        if(it != end && *it == ':') {
            auto length = it - input.begin();
            ++it;
            if(it == end || *it == '/' || !hierarchical || !equals(input.data(), length, m_value.data(), m_schemeEnd, false))
                return Url{input};
            input.erase(0, it - input.begin());
        }
    }

    if(!hierarchical) {
        if(!input.empty() && input.front() == '#')
            return Url{input};
        return Url{};
    }

    if(input.empty() || input.front() == '#')
        return Url(m_value.substr(0, m_queryEnd) + input);
    if(input.front() == '?')
        return Url(m_value.substr(0, m_pathEnd) + input);

    if(input.front() == '/') {
        if(input.length() > 1 && input[1] == '/')
            return Url(m_value.substr(0, m_schemeEnd + 1) + input);
        return Url(m_value.substr(0, m_portEnd) + input);
    }

    auto value = m_value.substr(0, m_pathEnd);
    while(m_portEnd < value.length() && value.back() != '/')
        value.pop_back();

    if(m_portEnd == value.length())
        value += '/';
    return Url{value + input};
}

bool Url::protocolIs(const std::string_view& value) const
{
    return equals(m_value.data(), m_schemeEnd, value.data(), value.length(), false);
}

bool Url::decodeData(std::string& mimeType, std::string& textEncoding, std::vector<char>& data) const
{
    return false;
}

} // namespace htmlbook
