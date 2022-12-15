#include "csstokenizer.h"

#include <cmath>

namespace htmlbook {

constexpr bool isNameStart(char cc) { return isalpha(cc) || cc == '_'; }
constexpr bool isNameChar(char cc) { return isNameStart(cc) || isdigit(cc) || cc == '-'; }
constexpr bool isNewLine(char cc) { return (cc == '\n' || cc == '\r' || cc == '\f'); }
constexpr bool isNonPrintable(char cc) { return (cc >= 0 && cc <= 0x8) || cc == 0xb || (cc >= 0xf && cc <= 0x1f) || cc == 0x7f; }

CSSTokenStream CSSTokenizer::tokenize()
{
    while(true) {
        auto token = nextToken();
        if(token.type() == CSSToken::Type::Comment)
            continue;
        if(token.type() == CSSToken::Type::EndOfFile)
            break;
        m_tokenList.push_back(token);
    }

    auto begin = m_tokenList.data();
    auto end = begin + m_tokenList.size();
    return CSSTokenStream(begin, end);
}

bool CSSTokenizer::isEscapeSequence(char first, char second)
{
    return first == '\\' && !isNewLine(second);
}

bool CSSTokenizer::isIdentSequence(char first, char second, char third)
{
    if(isNameStart(first) || isEscapeSequence(first, second))
        return true;
    if(first == '-')
        return isNameStart(second) || second == '-' || isEscapeSequence(second, third);
    return false;
}

bool CSSTokenizer::isNumberSequence(char first, char second, char third)
{
    if(isdigit(first))
        return true;
    if(first == '-' || first == '+')
        return isdigit(second) || (second == '.' && isdigit(third));
    if(first == '.')
        return isdigit(second);
    return false;
}

bool CSSTokenizer::isEscapeSequence() const
{
    if(m_input.empty())
        return false;
    return isEscapeSequence(*m_input, m_input.peek(1));
}

bool CSSTokenizer::isIdentSequence() const
{
    if(m_input.empty())
        return false;
    auto second = m_input.peek(1);
    if(second == 0)
        return isIdentSequence(*m_input, 0, 0);
    return isIdentSequence(*m_input, second, m_input.peek(2));
}

bool CSSTokenizer::isNumberSequence() const
{
    if(m_input.empty())
        return false;
    auto second = m_input.peek(1);
    if(second == 0)
        return isNumberSequence(*m_input, 0, 0);
    return isNumberSequence(*m_input, second, m_input.peek(2));
}

bool CSSTokenizer::isExponentSequence() const
{
    if(m_input.peek() != 'E' && m_input.peek() != 'e')
        return false;
    if(m_input.peek(1) == '+' || m_input.peek(1) == '-')
        return isdigit(m_input.peek(2));
    return isdigit(m_input.peek(1));
}

HeapString CSSTokenizer::consumeName()
{
    m_characterBuffer.clear();
    while(true) {
        auto cc = m_input.peek();
        if(isNameChar(cc)) {
            m_characterBuffer += cc;
            m_input.advance();
        } else if(isEscapeSequence()) {
            appendCodepoint(m_characterBuffer, consumeEscape());
        } else {
            break;
        }
    }

    return m_heap->createString(m_characterBuffer);
}

uint32_t CSSTokenizer::consumeEscape()
{
    assert(isEscapeSequence());
    auto cc = m_input.advance();
    if(isxdigit(cc)) {
        int count = 0;
        uint32_t cp = 0;
        do {
            cp = cp * 16 + xdigit(cc);
            cc = m_input.advance();
            count += 1;
        } while(count < 6 && isxdigit(cc));

        if(isspace(cc)) {
            if(cc == '\r' && m_input.peek(1) == '\n')
                m_input.advance();
            m_input.advance();
        }

        if(cp == 0 || cp >= 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF))
            return 0xFFFD;
        return cp;
    }

    if(cc == 0)
        return 0xFFFD;
    m_input.advance();
    return cc;
}

CSSToken CSSTokenizer::consumeStringToken()
{
    auto endingCodePoint = m_input.peek();
    assert(endingCodePoint == '\"' || endingCodePoint == '\'');
    m_input.advance();
    m_characterBuffer.clear();
    while(true) {
        auto cc = m_input.peek();
        if(cc == 0)
            break;
        if(cc == endingCodePoint) {
            m_input.advance();
            break;
        }

        if(isNewLine(cc))
            return CSSToken(CSSToken::Type::BadString);

        if(cc == '\\') {
            auto next = m_input.peek(1);
            if(next == 0) {
                m_input.advance();
            } else if(isNewLine(next)) {
                if(next == '\r' && m_input.peek(2) == '\n')
                    m_input.advance();
                m_input.advance(2);
            } else {
                appendCodepoint(m_characterBuffer, consumeEscape());
            }
        } else {
            m_characterBuffer += cc;
            m_input.advance();
        }
    }

    if(m_characterBuffer.empty())
        return CSSToken(CSSToken::Type::String);
    return CSSToken(CSSToken::Type::String, m_heap->createString(m_characterBuffer));
}

CSSToken CSSTokenizer::consumeNumericToken()
{
    assert(isNumberSequence());
    auto numberType = CSSToken::NumberType::Integer;
    auto numberSign = CSSToken::NumberSign::None;
    double fraction = 0;
    double integer = 0;
    int exponent = 0;
    int expsign = 1;

    if(m_input.peek() == '-') {
        numberSign = CSSToken::NumberSign::Minus;
        m_input.advance();
    } else if(m_input.peek() == '+') {
        numberSign = CSSToken::NumberSign::Plus;
        m_input.advance();
    }

    if(isdigit(m_input.peek())) {
        auto cc = m_input.peek();
        do {
            integer = 10.0 * integer + (cc - '0');
            cc = m_input.advance();
        } while(isdigit(cc));
    }

    if(m_input.peek() == '.' && isdigit(m_input.peek(1))) {
        numberType = CSSToken::NumberType::Number;
        auto cc = m_input.advance();
        int count = 0;
        do {
            fraction = 10.0 * fraction + (cc - '0');
            count += 1;
            cc = m_input.advance();
        } while(isdigit(cc));
        fraction *= std::pow(10.0, -count);
    }

    if(isExponentSequence()) {
        numberType = CSSToken::NumberType::Number;
        m_input.advance();
        if(m_input.peek() == '-') {
            expsign = -1;
            m_input.advance();
        } else if(m_input.peek() == '+') {
            m_input.advance();
        }

        auto cc = m_input.peek();
        do {
            exponent = 10 * exponent + (cc - '0');
            cc = m_input.advance();
        } while(isdigit(cc));
    }

    double number = (integer + fraction) * std::pow(10.0, exponent * expsign);
    if(numberSign == CSSToken::NumberSign::Minus)
        number = -number;

    if(m_input.peek() == '%') {
        m_input.advance();
        return CSSToken(CSSToken::Type::Percentage, numberType, numberSign, number);
    }

    if(isIdentSequence())
        return CSSToken(CSSToken::Type::Dimension, numberType, numberSign, number, consumeName());
    return CSSToken(CSSToken::Type::Number, numberType, numberSign, number);
}

CSSToken CSSTokenizer::consumeIdentLikeToken()
{
    auto name = consumeName();
    if(equals(name, "url", false) && m_input.peek() == '(') {
        auto cc = m_input.advance();
        while(isspace(cc) && isspace(m_input.peek(1))) {
            cc = m_input.advance();
        }

        if(isspace(cc))
            cc = m_input.peek(1);

        if(cc == '"' || cc == '\'')
            return CSSToken(CSSToken::Type::Function, name);
        return consumeUrlToken();
    }

    if(m_input.peek() == '(') {
        m_input.advance();
        return CSSToken(CSSToken::Type::Function, name);
    }

    return CSSToken(CSSToken::Type::Ident, name);
}

CSSToken CSSTokenizer::consumeUrlToken()
{
    auto cc = m_input.peek();
    while(isspace(cc)) {
        cc = m_input.advance();
    }

    m_characterBuffer.clear();
    while(true) {
        auto cc = m_input.peek();
        if(cc == 0)
            break;
        if(cc == ')') {
            m_input.advance();
            break;
        }

        if(cc == '\\') {
            if(isEscapeSequence()) {
                appendCodepoint(m_characterBuffer, consumeEscape());
                continue;
            }

            return consumeBadUrlRemnants();
        }

        if(isspace(cc)) {
            do {
                cc = m_input.advance();
            } while(isspace(cc));

            if(cc == 0)
                break;
            if(cc == ')') {
                m_input.advance();
                break;
            }

            return consumeBadUrlRemnants();
        }

        if(cc == '"' || cc == '\'' || cc == '(' || isNonPrintable(cc))
            return consumeBadUrlRemnants();

        m_characterBuffer += cc;
        m_input.advance();
    }

    return CSSToken(CSSToken::Type::Url, m_heap->createString(m_characterBuffer));
}

CSSToken CSSTokenizer::consumeBadUrlRemnants()
{
    while(true) {
        auto cc = m_input.peek();
        if(cc == 0)
            break;
        if(cc == ')') {
            m_input.advance();
            break;
        }

        if(isEscapeSequence()) {
            consumeEscape();
        } else {
            m_input.advance();
        }
    }

    return CSSToken(CSSToken::Type::BadUrl);
}

CSSToken CSSTokenizer::consumeWhitespaceToken()
{
    auto cc = m_input.peek();
    assert(isspace(cc));
    do {
        cc = m_input.advance();
    } while(isspace(cc));

    return CSSToken(CSSToken::Type::Whitespace);
}

CSSToken CSSTokenizer::consumeCommentToken()
{
    while(true) {
        auto cc = m_input.peek();
        if(cc == 0)
            break;
        if(cc == '*' && m_input.peek(1) == '/') {
            m_input.advance(2);
            break;
        }

        m_input.advance();
    }

    return CSSToken(CSSToken::Type::Comment);
}

CSSToken CSSTokenizer::consumeSolidusToken()
{
    auto cc = m_input.advance();
    if(cc == '*') {
        m_input.advance();
        return consumeCommentToken();
    }

    return CSSToken(CSSToken::Type::Delim, '/');
}

CSSToken CSSTokenizer::consumeHashToken()
{
    auto cc = m_input.advance();
    if(isNameChar(cc) || isEscapeSequence()) {
        if(isIdentSequence())
            return CSSToken(CSSToken::Type::Hash, CSSToken::HashType::Identifier, consumeName());
        return CSSToken(CSSToken::Type::Hash, CSSToken::HashType::Unrestricted, consumeName());
    }

    return CSSToken(CSSToken::Type::Delim, '#');
}

CSSToken CSSTokenizer::consumePlusSignToken()
{
    if(isNumberSequence())
        return consumeNumericToken();

    m_input.advance();
    return CSSToken(CSSToken::Type::Delim, '+');
}

CSSToken CSSTokenizer::consumeHyphenMinusToken()
{
    if(isNumberSequence())
        return consumeNumericToken();

    if(m_input.peek(1) == '-' && m_input.peek(2) == '>') {
        m_input.advance(3);
        return CSSToken(CSSToken::Type::CDC);
    }

    if(isIdentSequence())
        return consumeIdentLikeToken();

    m_input.advance();
    return CSSToken(CSSToken::Type::Delim, '-');
}

CSSToken CSSTokenizer::consumeFullStopToken()
{
    if(isNumberSequence())
        return consumeNumericToken();

    m_input.advance();
    return CSSToken(CSSToken::Type::Delim, '.');
}

CSSToken CSSTokenizer::consumeLessThanSignToken()
{
    auto cc = m_input.advance();
    if(cc == '!' && m_input.peek(1) == '-' && m_input.peek(2) == '-') {
        m_input.advance(3);
        return CSSToken(CSSToken::Type::CDO);
    }

    return CSSToken(CSSToken::Type::Delim, '<');
}

CSSToken CSSTokenizer::consumeCommercialAtToken()
{
    m_input.advance();
    if(isIdentSequence())
        return CSSToken(CSSToken::Type::AtKeyword, consumeName());

    return CSSToken(CSSToken::Type::Delim, '@');
}

CSSToken CSSTokenizer::consumeReverseSolidusToken()
{
    if(isEscapeSequence())
        return consumeIdentLikeToken();

    m_input.advance();
    return CSSToken(CSSToken::Type::Delim, '\\');
}

CSSToken CSSTokenizer::nextToken()
{
    auto cc = m_input.peek();
    if(cc == 0)
        return CSSToken(CSSToken::Type::EndOfFile);

    if(isspace(cc))
        return consumeWhitespaceToken();

    if(isdigit(cc))
        return consumeNumericToken();

    if(isNameStart(cc))
        return consumeIdentLikeToken();

    switch(cc) {
    case '/':
        return consumeSolidusToken();
    case '#':
        return consumeHashToken();
    case '+':
        return consumePlusSignToken();
    case '-':
        return consumeHyphenMinusToken();
    case '.':
        return consumeFullStopToken();
    case '<':
        return consumeLessThanSignToken();
    case '@':
        return consumeCommercialAtToken();
    case '\\':
        return consumeReverseSolidusToken();
    case '\"':
        return consumeStringToken();
    case '\'':
        return consumeStringToken();
    default:
        m_input.advance();
    }

    switch(cc) {
    case '(':
        return CSSToken(CSSToken::Type::LeftParenthesis);
    case ')':
        return CSSToken(CSSToken::Type::RightParenthesis);
    case '[':
        return CSSToken(CSSToken::Type::LeftSquareBracket);
    case ']':
        return CSSToken(CSSToken::Type::RightSquareBracket);
    case '{':
        return CSSToken(CSSToken::Type::LeftCurlyBracket);
    case '}':
        return CSSToken(CSSToken::Type::RightCurlyBracket);
    case ',':
        return CSSToken(CSSToken::Type::Comma);
    case ':':
        return CSSToken(CSSToken::Type::Colon);
    case ';':
        return CSSToken(CSSToken::Type::Semicolon);
    default:
        return CSSToken(CSSToken::Type::Delim, cc);
    }
}

} // namespace htmlbook
