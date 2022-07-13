#ifndef URL_H
#define URL_H

#include <string>
#include <ostream>

namespace htmlbook {

class Url {
public:
    Url() = default;
    Url(const std::string_view& value);

    bool empty() const { return m_value.empty(); }
    const std::string& value() const { return m_value; }

    Url complete(std::string_view relative) const;

private:
    std::string m_value;
    unsigned m_schemeEnd{0};
    unsigned m_userBegin{0};
    unsigned m_userEnd{0};
    unsigned m_passwordEnd{0};
    unsigned m_hostEnd{0};
    unsigned m_portEnd{0};
    unsigned m_pathEnd{0};
    unsigned m_queryEnd{0};
    unsigned m_fragmentEnd{0};
};

inline std::ostream& operator<<(std::ostream& o, const Url& in) { return o << in.value(); }

inline bool operator==(const Url& a, const Url& b) { return a.value() == b.value(); }
inline bool operator!=(const Url& a, const Url& b) { return a.value() != b.value(); }

inline bool operator==(const Url& a, const std::string_view& b) { return a.value() == b; }
inline bool operator!=(const Url& a, const std::string_view& b) { return a.value() != b; }

inline bool operator==(const std::string_view& a, const Url& b) { return a == b.value(); }
inline bool operator!=(const std::string_view& a, const Url& b) { return a != b.value(); }

inline bool operator<(const Url& a, const Url& b) { return a.value() < b.value(); }
inline bool operator>(const Url& a, const Url& b) { return a.value() > b.value(); }

inline bool operator<(const Url& a, const std::string_view& b) { return a.value() < b; }
inline bool operator>(const Url& a, const std::string_view& b) { return a.value() > b; }

inline bool operator<(const std::string_view& a, const Url& b) { return a < b.value(); }
inline bool operator>(const std::string_view& a, const Url& b) { return a > b.value(); }

} // namespace htmlbook

#endif // URL_H
