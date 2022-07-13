#ifndef HTMLENTITYPARSER_H
#define HTMLENTITYPARSER_H

#include "parserstring.h"

#include <string>

namespace htmlbook {

struct HTMLEntity {
    uint32_t firstValue;
    uint32_t secondValue;
    std::string_view name;
};

class HTMLEntitySearch {
public:
    HTMLEntitySearch() = default;

    bool advance(char cc);
    size_t offset() const { return m_offset; }
    const HTMLEntity* lastMatch() const { return m_lastMatch; }

private:
    size_t m_offset{0};
    const HTMLEntity* m_first{nullptr};
    const HTMLEntity* m_last{nullptr};
    const HTMLEntity* m_lastMatch{nullptr};
};

class HTMLEntityParser {
public:
    HTMLEntityParser(std::string& output, ParserString& input, bool inAttributeValue)
        : m_output(output), m_input(input), m_inAttributeValue(inAttributeValue)
    {}

    bool parse();

private:
    bool handleNamed(char cc);
    bool handleNumber(char cc);
    bool handleDecimal(char cc);
    bool handleMaybeHex(char cc);
    bool handleHex(char cc);
    void append(uint32_t cp);

private:
    std::string& m_output;
    ParserString& m_input;
    bool m_inAttributeValue;
};

} // namespace htmlbook

#endif // HTMLENTITYPARSER_H
