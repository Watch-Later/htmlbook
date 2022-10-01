#include "cssparser.h"

#include <sstream>
#include <algorithm>
#include <cmath>

namespace htmlbook {

void CSSParser::parseSheet(CSSRuleList& rules, const std::string_view& content)
{
    CSSTokenizer tokenizer(content);
    auto input = tokenizer.tokenize();
    while(!input.empty()) {
        input.consumeWhitespace();
        if(input->type() == CSSToken::Type::CDC
            || input->type() == CSSToken::Type::CDO) {
            input.consume();
            continue;
        }

        auto rule = consumeRule(input);
        if(rule == nullptr)
            continue;
        rules.push_back(std::move(rule));
    }
}

void CSSParser::parseStyle(CSSPropertyList& properties, const std::string_view& content)
{
    CSSTokenizer tokenizer(content);
    auto input = tokenizer.tokenize();
    consumeDeclaractionList(input, properties);
}

std::unique_ptr<CSSRule> CSSParser::consumeRule(CSSTokenStream& input)
{
    if(input->type() == CSSToken::Type::AtKeyword)
        return consumeAtRule(input);
    return consumeStyleRule(input);
}

std::unique_ptr<CSSRule> CSSParser::consumeStyleRule(CSSTokenStream& input)
{
    auto preludeBegin = input.begin();
    while(!input.empty() && input->type() != CSSToken::Type::LeftCurlyBracket) {
        input.consumeComponent();
    }

    CSSTokenStream prelude(preludeBegin, input.begin());
    if(input.empty())
        return nullptr;

    auto block = input.consumeBlock();
    CSSSelectorList selectors;
    if(!consumeSelectorList(prelude, selectors))
        return nullptr;

    CSSPropertyList properties;
    consumeDeclaractionList(block, properties);
    return CSSStyleRule::create(std::move(selectors), std::move(properties));
}

std::unique_ptr<CSSRule> CSSParser::consumeAtRule(CSSTokenStream& input)
{
    assert(input->type() == CSSToken::Type::AtKeyword);
    auto name = input->data();
    input.consume();
    auto preludeBegin = input.begin();
    while(input->type() != CSSToken::Type::EndOfFile
        && input->type() != CSSToken::Type::LeftCurlyBracket
        && input->type() != CSSToken::Type::Semicolon) {
        input.consumeComponent();
    }

    CSSTokenStream prelude(preludeBegin, input.begin());
    if(input->type() == CSSToken::Type::EndOfFile
        || input->type() == CSSToken::Type::Semicolon) {
        if(input->type() == CSSToken::Type::Semicolon)
            input.consume();
        if(equals(name, "import", false))
            return consumeImportRule(prelude);
        return nullptr;
    }

    auto block = input.consumeBlock();
    if(equals(name, "font-face", false))
        return consumeFontFaceRule(prelude, block);
    if(equals(name, "page", false))
        return consumePageRule(prelude, block);
    return nullptr;
}

std::unique_ptr<CSSRule> CSSParser::consumeImportRule(CSSTokenStream& input)
{
    std::string href;
    input.consumeWhitespace();
    switch(input->type()) {
    case CSSToken::Type::Url:
    case CSSToken::Type::String:
        href = input->data();
        input.consumeIncludingWhitespace();
        break;
    case CSSToken::Type::Function: {
        if(!equals(input->data(), "url", false))
            return nullptr;
        auto block = input.consumeBlock();
        block.consumeWhitespace();
        href = block->data();
        block.consumeIncludingWhitespace();
        if(!block.empty())
            return nullptr;
        input.consumeWhitespace();
        break;
    }

    default:
        return nullptr;
    }

    return CSSImportRule::create(std::move(href));
}

std::unique_ptr<CSSRule> CSSParser::consumeFontFaceRule(CSSTokenStream& prelude, CSSTokenStream& block)
{
    prelude.consumeWhitespace();
    if(!prelude.empty())
        return nullptr;

    CSSPropertyList properties;
    consumeDeclaractionList(block, properties);
    return CSSFontFaceRule::create(std::move(properties));
}

std::unique_ptr<CSSRule> CSSParser::consumePageRule(CSSTokenStream& prelude, CSSTokenStream& block)
{
    CSSPageSelectorList selectors;
    if(!consumePageSelectorList(prelude, selectors))
        return nullptr;

    CSSPageMarginRuleList margins;
    CSSPropertyList properties;
    while(!block.empty()) {
        switch(block->type()) {
        case CSSToken::Type::Whitespace:
        case CSSToken::Type::Semicolon:
            block.consume();
            break;
        case CSSToken::Type::Ident:
            consumeDeclaraction(block, properties);
            break;
        case CSSToken::Type::AtKeyword:
            if(auto margin = consumePageMarginRule(block))
                margins.push_back(std::move(margin));
            break;
        default:
            while(!block.empty() && block->type() != CSSToken::Type::Semicolon)
                block.consumeComponent();
            break;
        }
    }

    return CSSPageRule::create(std::move(selectors), std::move(margins), std::move(properties));
}

std::unique_ptr<CSSPageMarginRule> CSSParser::consumePageMarginRule(CSSTokenStream& input)
{
    assert(input->type() == CSSToken::Type::AtKeyword);
    auto name = input->data();
    input.consume();
    auto preludeBegin = input.begin();
    while(!input.empty() && input->type() != CSSToken::Type::LeftCurlyBracket) {
        input.consumeComponent();
    }

    CSSTokenStream prelude(preludeBegin, input.begin());
    if(input.empty())
        return nullptr;

    auto block = input.consumeBlock();
    prelude.consumeWhitespace();
    if(!prelude.empty())
        return nullptr;

    static const struct {
        std::string_view name;
        CSSPageMarginRule::MarginType value;
    } table[] = {
        {"top-left-corner", CSSPageMarginRule::MarginType::TopLeftCorner},
        {"top-left", CSSPageMarginRule::MarginType::TopLeft},
        {"top-center", CSSPageMarginRule::MarginType::TopCenter},
        {"top-right", CSSPageMarginRule::MarginType::TopRight},
        {"top-right-corner", CSSPageMarginRule::MarginType::TopRightCorner},
        {"bottom-left-corner", CSSPageMarginRule::MarginType::BottomLeftCorner},
        {"bottom-left", CSSPageMarginRule::MarginType::BottomLeft},
        {"bottom-center", CSSPageMarginRule::MarginType::BottomCenter},
        {"bottom-right", CSSPageMarginRule::MarginType::BottomRight},
        {"bottom-right-corner", CSSPageMarginRule::MarginType::BottomRightCorner},
        {"left-top", CSSPageMarginRule::MarginType::LeftTop},
        {"left-middle", CSSPageMarginRule::MarginType::LeftMiddle},
        {"left-bottom", CSSPageMarginRule::MarginType::LeftBottom},
        {"right-top", CSSPageMarginRule::MarginType::RightTop},
        {"right-middle", CSSPageMarginRule::MarginType::RightMiddle},
        {"right-bottom", CSSPageMarginRule::MarginType::RightBottom}
    };

    auto it = std::find_if(table, std::end(table), [name](auto& item) { return equals(name, item.name, false); });
    if(it == std::end(table))
        return nullptr;

    CSSPropertyList properties;
    consumeDeclaractionList(block, properties);
    return CSSPageMarginRule::create(it->value, std::move(properties));
}

bool CSSParser::consumePageSelector(CSSTokenStream& input, CSSPageSelector& selector)
{
    if(input->type() != CSSToken::Type::Ident
        && input->type() != CSSToken::Type::Colon) {
        return false;
    }

    if(input->type() == CSSToken::Type::Ident) {
        selector.emplace_back(CSSSimpleSelector::MatchType::Tag, input->data());
        input.consume();
    }

    static const struct {
        std::string_view name;
        CSSSimpleSelector::MatchType value;
    } table[] = {
        {"first", CSSSimpleSelector::MatchType::PseudoPageFirst},
        {"left", CSSSimpleSelector::MatchType::PseudoPageLeft},
        {"right", CSSSimpleSelector::MatchType::PseudoPageRight},
        {"blank", CSSSimpleSelector::MatchType::PseudoPageBlank}
    };

    while(input->type() == CSSToken::Type::Colon) {
        input.consume();
        if(input->type() != CSSToken::Type::Ident)
            return false;
        auto name = input->data();
        input.consume();
        auto it = std::find_if(table, std::end(table), [name](auto& item) { return equals(name, item.name, false); });
        if(it == std::end(table))
            return false;
        selector.emplace_back(it->value);
    }

    return true;
}

bool CSSParser::consumePageSelectorList(CSSTokenStream& input, CSSPageSelectorList& selectors)
{
    CSSPageSelector selector;
    input.consumeWhitespace();
    if(!consumePageSelector(input, selector))
        return false;

    selectors.push_back(std::move(selector));
    input.consumeWhitespace();
    while(input->type() == CSSToken::Type::Comma) {
        input.consumeIncludingWhitespace();
        if(!consumePageSelector(input, selector))
            return false;
        selectors.push_back(std::move(selector));
        input.consumeWhitespace();
    }

    return input.empty();
}

bool CSSParser::consumeSelector(CSSTokenStream& input, CSSSelector& selector)
{
    auto combinator = CSSComplexSelector::Combinator::None;
    do {
        CSSCompoundSelector sel;
        if(!consumeCompoundSelector(input, sel))
            return combinator == CSSComplexSelector::Combinator::Descendant;
        selector.emplace_back(combinator, std::move(sel));
    } while(consumeCombinator(input, combinator));
    return true;
}

bool CSSParser::consumeSelectorList(CSSTokenStream& input, CSSSelectorList& selectors)
{
    CSSSelector selector;
    input.consumeWhitespace();
    if(!consumeSelector(input, selector))
        return false;

    selectors.push_back(std::move(selector));
    while(input->type() == CSSToken::Type::Comma) {
        input.consumeIncludingWhitespace();
        if(!consumeSelector(input, selector))
            return false;
        selectors.push_back(std::move(selector));
    }

    return input.empty();
}

bool CSSParser::consumeCompoundSelector(CSSTokenStream& input, CSSCompoundSelector& selector)
{
    if(!consumeTagSelector(input, selector)
        && !consumeSimpleSelector(input, selector)) {
        return false;
    }

    while(consumeSimpleSelector(input, selector));
    return true;
}

bool CSSParser::consumeCompoundSelectorList(CSSTokenStream& input, CSSCompoundSelectorList& selectors)
{
    CSSCompoundSelector selector;
    if(!consumeCompoundSelector(input, selector))
        return false;

    selectors.push_back(std::move(selector));
    input.consumeWhitespace();
    while(input->type() == CSSToken::Type::Comma) {
        input.consumeIncludingWhitespace();
        if(!consumeCompoundSelector(input, selector))
            return false;
        selectors.push_back(std::move(selector));
        input.consumeWhitespace();
    }

    return true;
}

bool CSSParser::consumeSimpleSelector(CSSTokenStream& input, CSSCompoundSelector& selector)
{
    if(input->type() == CSSToken::Type::Hash)
        return consumeIdSelector(input, selector);
    if(input->type() == CSSToken::Type::Delim && input->delim() == '.')
        return consumeClassSelector(input, selector);
    if(input->type() == CSSToken::Type::LeftSquareBracket)
        return consumeAttributeSelector(input, selector);
    if(input->type() == CSSToken::Type::Colon)
        return consumePseudoSelector(input, selector);
    return false;
}

bool CSSParser::consumeTagSelector(CSSTokenStream& input, CSSCompoundSelector& selector)
{
    if(input->type() == CSSToken::Type::Ident) {
        selector.emplace_back(CSSSimpleSelector::MatchType::Tag, input->data());
        input.consume();
        return true;
    }

    if(input->type() == CSSToken::Type::Delim && input->delim() == '*') {
        selector.emplace_back(CSSSimpleSelector::MatchType::Universal);
        input.consume();
        return true;
    }

    return false;
}

bool CSSParser::consumeIdSelector(CSSTokenStream& input, CSSCompoundSelector& selector)
{
    assert(input->type() == CSSToken::Type::Hash);
    if(input->hashType() == CSSToken::HashType::Identifier) {
        selector.emplace_back(CSSSimpleSelector::MatchType::Id, input->data());
        input.consume();
        return true;
    }

    return false;
}

bool CSSParser::consumeClassSelector(CSSTokenStream& input, CSSCompoundSelector& selector)
{
    assert(input->type() == CSSToken::Type::Delim);
    input.consume();
    if(input->type() == CSSToken::Type::Ident) {
        selector.emplace_back(CSSSimpleSelector::MatchType::Class, input->data());
        input.consume();
        return true;
    }

    return false;
}

bool CSSParser::consumeAttributeSelector(CSSTokenStream& input, CSSCompoundSelector& selector)
{
    assert(input->type() == CSSToken::Type::LeftSquareBracket);
    auto block = input.consumeBlock();
    if(block->type() != CSSToken::Type::Ident)
        return false;

    auto name = block->data();
    block.consumeIncludingWhitespace();
    if(block.empty()) {
        selector.emplace_back(CSSSimpleSelector::MatchType::AttributeHas, name);
        return true;
    }

    if(block->type() != CSSToken::Type::Delim)
        return false;

    CSSSimpleSelector::MatchType matchType;
    switch(block->delim()) {
    case '=':
        matchType = CSSSimpleSelector::MatchType::AttributeEquals;
        break;
    case '~':
        matchType = CSSSimpleSelector::MatchType::AttributeIncludes;
        break;
    case '*':
        matchType = CSSSimpleSelector::MatchType::AttributeContains;
        break;
    case '|':
        matchType = CSSSimpleSelector::MatchType::AttributeDashEquals;
        break;
    case '^':
        matchType = CSSSimpleSelector::MatchType::AttributeStartsWith;
        break;
    case '$':
        matchType = CSSSimpleSelector::MatchType::AttributeEndsWith;
        break;
    default:
        return false;
    }

    if(matchType != CSSSimpleSelector::MatchType::AttributeEquals) {
        block.consume();
        if(block->type() != CSSToken::Type::Delim && block->delim() != '=')
            return false;
    }

    block.consumeIncludingWhitespace();
    if(block->type() != CSSToken::Type::Ident && block->type() != CSSToken::Type::String)
        return false;

    auto value = std::make_unique<std::string>(block->data());
    block.consumeIncludingWhitespace();

    auto caseType = CSSSimpleSelector::AttributeCaseType::Sensitive;
    if(block->type() == CSSToken::Type::Ident && block->data() == "i") {
        caseType = CSSSimpleSelector::AttributeCaseType::InSensitive;
        block.consumeIncludingWhitespace();
    }

    if(block.empty()) {
        selector.emplace_back(matchType, caseType, name, std::move(value));
        return true;
    }

    return false;
}

bool CSSParser::consumePseudoSelector(CSSTokenStream& input, CSSCompoundSelector& selector)
{
    assert(input->type() == CSSToken::Type::Colon);
    input.consume();
    if(input->type() == CSSToken::Type::Colon) {
        input.consume();
        if(input->type() != CSSToken::Type::Ident)
            return false;
        auto name = input->data();
        input.consume();
        static const struct {
            std::string_view name;
            CSSSimpleSelector::MatchType value;
        } table[] = {
            {"before", CSSSimpleSelector::MatchType::PseudoElementBefore},
            {"after", CSSSimpleSelector::MatchType::PseudoElementAfter},
            {"marker", CSSSimpleSelector::MatchType::PseudoElementMarker},
            {"first-letter", CSSSimpleSelector::MatchType::PseudoElementFirstLetter},
            {"first-line", CSSSimpleSelector::MatchType::PseudoElementFirstLine}
        };

        auto it = std::find_if(table, std::end(table), [name](auto& item) { return equals(name, item.name, false); });
        if(it == std::end(table))
            return false;
        selector.emplace_back(it->value);
        return true;
    }

    if(input->type() == CSSToken::Type::Ident) {
        auto name = input->data();
        input.consume();
        static const struct {
            std::string_view name;
            CSSSimpleSelector::MatchType value;
        } table[] = {
            {"link", CSSSimpleSelector::MatchType::PseudoClassLink},
            {"enabled", CSSSimpleSelector::MatchType::PseudoClassEnabled},
            {"disabled", CSSSimpleSelector::MatchType::PseudoClassDisabled},
            {"checked", CSSSimpleSelector::MatchType::PseudoClassChecked},
            {"root", CSSSimpleSelector::MatchType::PseudoClassRoot},
            {"empty", CSSSimpleSelector::MatchType::PseudoClassEmpty},
            {"first-child", CSSSimpleSelector::MatchType::PseudoClassFirstChild},
            {"last-child", CSSSimpleSelector::MatchType::PseudoClassLastChild},
            {"only-child", CSSSimpleSelector::MatchType::PseudoClassOnlyChild},
            {"first-of-type", CSSSimpleSelector::MatchType::PseudoClassFirstOfType},
            {"last-of-type", CSSSimpleSelector::MatchType::PseudoClassLastOfType},
            {"only-of-type", CSSSimpleSelector::MatchType::PseudoClassOnlyOfType}
        };

        auto it = std::find_if(table, std::end(table), [name](auto& item) { return equals(name, item.name, false); });
        if(it == std::end(table))
            return false;
        selector.emplace_back(it->value);
        return true;
    }

    if(input->type() == CSSToken::Type::Function) {
        auto name = input->data();
        auto block = input.consumeBlock();
        block.consumeIncludingWhitespace();
        static const struct {
            std::string_view name;
            CSSSimpleSelector::MatchType value;
        } table[] = {
            {"is", CSSSimpleSelector::MatchType::PseudoClassIs},
            {"not", CSSSimpleSelector::MatchType::PseudoClassNot},
            {"lang", CSSSimpleSelector::MatchType::PseudoClassLang},
            {"nth-child", CSSSimpleSelector::MatchType::PseudoClassNthChild},
            {"nth-last-child", CSSSimpleSelector::MatchType::PseudoClassNthLastChild},
            {"nth-of-type", CSSSimpleSelector::MatchType::PseudoClassNthOfType},
            {"nth-last-of-type", CSSSimpleSelector::MatchType::PseudoClassNthLastOfType}
        };

        auto it = std::find_if(table, std::end(table), [name](auto& item) { return equals(name, item.name, false); });
        if(it == std::end(table))
            return false;
        switch(it->value) {
        case CSSSimpleSelector::MatchType::PseudoClassIs:
        case CSSSimpleSelector::MatchType::PseudoClassNot: {
            auto subSelectors = std::make_unique<CSSCompoundSelectorList>();
            if(!consumeCompoundSelectorList(block, *subSelectors))
                return false;
            selector.emplace_back(it->value, std::move(subSelectors));
            break;
        }

        case CSSSimpleSelector::MatchType::PseudoClassLang: {
            if(block->type() != CSSToken::Type::Ident)
                return false;
            selector.emplace_back(it->value, block->data());
            block.consume();
            break;
        }

        case CSSSimpleSelector::MatchType::PseudoClassNthChild:
        case CSSSimpleSelector::MatchType::PseudoClassNthLastChild:
        case CSSSimpleSelector::MatchType::PseudoClassNthOfType:
        case CSSSimpleSelector::MatchType::PseudoClassNthLastOfType: {
            CSSSimpleSelector::MatchPattern pattern;
            if(!consumeMatchPattern(block, pattern))
                return false;
            selector.emplace_back(it->value, pattern);
            break;
        }

        default:
            assert(false);
        }

        block.consumeWhitespace();
        return block.empty();
    }

    return false;
}

bool CSSParser::consumeCombinator(CSSTokenStream& input, CSSComplexSelector::Combinator& combinator)
{
    combinator = CSSComplexSelector::Combinator::None;
    while(input->type() == CSSToken::Type::Whitespace) {
        combinator = CSSComplexSelector::Combinator::Descendant;
        input.consume();
    }

    if(input->type() == CSSToken::Type::Delim) {
        if(input->delim() == '+') {
            combinator = CSSComplexSelector::Combinator::DirectAdjacent;
            input.consumeIncludingWhitespace();
            return true;
        }

        if(input->delim() == '~') {
            combinator = CSSComplexSelector::Combinator::InDirectAdjacent;
            input.consumeIncludingWhitespace();
            return true;
        }

        if(input->delim() == '>') {
            combinator = CSSComplexSelector::Combinator::Child;
            input.consumeIncludingWhitespace();
            return true;
        }
    }

    return combinator == CSSComplexSelector::Combinator::Descendant;
}

bool CSSParser::consumeMatchPattern(CSSTokenStream& input, CSSSimpleSelector::MatchPattern& pattern)
{
    if(input->type() == CSSToken::Type::Number) {
        if(input->numberType() != CSSToken::NumberType::Integer)
            return false;
        pattern = std::make_pair(0, input->integer());
        input.consume();
        return true;
    }

    if(input->type() == CSSToken::Type::Ident) {
        if(equals(input->data(), "odd", false)) {
            pattern = std::make_pair(2, 1);
            input.consume();
            return true;
        }

        if(equals(input->data(), "even", false)) {
            pattern = std::make_pair(2, 0);
            input.consume();
            return true;
        }
    }

    std::stringstream ss;
    if(input->type() == CSSToken::Type::Delim) {
        if(input->delim() != '+')
            return false;
        input.consume();
        if(input->type() != CSSToken::Type::Ident)
            return false;
        pattern.first = 1;
        ss << input->data();
        input.consume();
    } else if(input->type() == CSSToken::Type::Ident) {
        auto ident = input->data();
        input.consume();
        if(ident.front() == '-') {
            pattern.first = -1;
            ss << ident.substr(1);
        } else {
            pattern.first = 1;
            ss << ident;
        }
    } else if(input->type() == CSSToken::Type::Dimension) {
        if(input->numberType() != CSSToken::NumberType::Integer)
            return false;
        pattern.first = input->integer();
        ss << input->data();
        input.consume();
    }

    constexpr auto eof = std::stringstream::traits_type::eof();
    if(ss.peek() == eof || !equals(ss.get(), 'n', false))
        return false;

    auto sign = CSSToken::NumberSign::None;
    if(ss.peek() != eof) {
        if(ss.get() != '-')
            return false;
        sign = CSSToken::NumberSign::Minus;
        if(ss.peek() != eof) {
            ss >> pattern.second;
            if(ss.fail())
                return false;
            pattern.second = -pattern.second;
            return true;
        }
    }

    input.consumeWhitespace();
    if(sign == CSSToken::NumberSign::None && input->type() == CSSToken::Type::Delim) {
        auto delim = input->delim();
        if(delim == '+')
            sign = CSSToken::NumberSign::Plus;
        else if(delim == '-')
            sign = CSSToken::NumberSign::Minus;
        else
            return false;
        input.consumeIncludingWhitespace();
    }

    if(sign == CSSToken::NumberSign::None && input->type() != CSSToken::Type::Number) {
        pattern.second = 0;
        return true;
    }

    if(input->type() != CSSToken::Type::Number || input->numberType() != CSSToken::NumberType::Integer)
        return false;

    if(sign == CSSToken::NumberSign::None && input->numberSign() == CSSToken::NumberSign::None)
        return false;

    if(sign != CSSToken::NumberSign::None && input->numberSign() != CSSToken::NumberSign::None)
        return false;

    pattern.second = input->integer();
    if(sign == CSSToken::NumberSign::Minus)
        pattern.second = -pattern.second;
    input.consume();
    return true;
}

void CSSParser::consumeDeclaractionList(CSSTokenStream& input, CSSPropertyList& properties)
{
    input.consumeWhitespace();
    consumeDeclaraction(input, properties);
    while(input->type() == CSSToken::Type::Semicolon) {
        input.consumeIncludingWhitespace();
        consumeDeclaraction(input, properties);
    }
}

bool CSSParser::consumeDeclaraction(CSSTokenStream& input, CSSPropertyList& properties)
{
    auto begin = input.begin();
    while(!input.empty() && input->type() != CSSToken::Type::Semicolon) {
        input.consumeComponent();
    }

    CSSTokenStream newInput(begin, input.begin());
    if(newInput->type() != CSSToken::Type::Ident)
        return false;

    auto id = csspropertyid(newInput->data());
    if(id == CSSPropertyID::Unknown)
        return false;

    newInput.consumeIncludingWhitespace();
    if(newInput->type() != CSSToken::Type::Colon)
        return false;

    newInput.consumeIncludingWhitespace();
    auto valueBegin = newInput.begin();
    auto valueEnd = newInput.end();

    auto it = valueEnd - 1;
    while(it->type() == CSSToken::Type::Whitespace) {
        it -= 1;
    }

    bool important = false;
    if(it->type() == CSSToken::Type::Ident && equals(it->data(), "important", false)) {
        do {
            it -= 1;
        } while(it->type() == CSSToken::Type::Whitespace);
        if(it->type() == CSSToken::Type::Delim && it->delim() == '!') {
            important = true;
            valueEnd = it;
        }
    }

    CSSTokenStream value(valueBegin, valueEnd);
    return consumeDeclaractionValue(value, properties, id, important);
}

bool CSSParser::consumeDeclaractionValue(CSSTokenStream& input, CSSPropertyList& properties, CSSPropertyID id, bool important)
{
    if(input->type() == CSSToken::Type::Ident) {
        if(equals(input->data(), "inherit", false)) {
            input.consumeIncludingWhitespace();
            if(!input.empty())
                return false;
            addExpandedProperty(properties, id, important, CSSInheritValue::create());
            return true;
        }

        if(equals(input->data(), "initial", false)) {
            input.consumeIncludingWhitespace();
            if(!input.empty())
                return false;
            addExpandedProperty(properties, id, important, CSSInitialValue::create());
            return true;
        }
    }

    switch(id) {
    case CSSPropertyID::BorderTop:
    case CSSPropertyID::BorderRight:
    case CSSPropertyID::BorderBottom:
    case CSSPropertyID::BorderLeft:
    case CSSPropertyID::FlexFlow:
    case CSSPropertyID::ListStyle:
    case CSSPropertyID::ColumnRule:
    case CSSPropertyID::Outline:
    case CSSPropertyID::TextDecoration:
        return consumeShorthand(input, properties, id, important);
    case CSSPropertyID::Margin:
    case CSSPropertyID::Padding:
    case CSSPropertyID::BorderColor:
    case CSSPropertyID::BorderStyle:
    case CSSPropertyID::BorderWidth:
        return consume4Shorthand(input, properties, id, important);
    case CSSPropertyID::Overflow:
    case CSSPropertyID::BorderSpacing:
        return consume2Shorthand(input, properties, id, important);
    case CSSPropertyID::Background:
        return consumeBackground(input, properties, important);
    case CSSPropertyID::Font:
        return consumeFont(input, properties, important);
    case CSSPropertyID::Border:
        return consumeBorder(input, properties, important);
    case CSSPropertyID::BorderRadius:
        return consumeBorderRadius(input, properties, important);
    case CSSPropertyID::Columns:
        return consumeColumns(input, properties, important);
    case CSSPropertyID::Flex:
        return consumeFlex(input, properties, important);
    default:
        break;
    }

    auto value = consumeLonghand(input, id);
    input.consumeWhitespace();
    if(value && input.empty()) {
        addProperty(properties, id, important, value);
        return true;
    }

    return false;
}

void CSSParser::addProperty(CSSPropertyList& properties, CSSPropertyID id, bool important, RefPtr<CSSValue> value)
{
    if(value == nullptr) {
        switch(id) {
        case CSSPropertyID::FontStyle:
        case CSSPropertyID::FontVariant:
        case CSSPropertyID::FontWeight:
        case CSSPropertyID::LineHeight:
            value = CSSIdentValue::create(CSSValueID::Normal);
            break;
        case CSSPropertyID::ColumnWidth:
        case CSSPropertyID::ColumnCount:
            value = CSSIdentValue::create(CSSValueID::Auto);
            break;
        case CSSPropertyID::FlexGrow:
        case CSSPropertyID::FlexShrink:
            value = CSSNumberValue::create(1.0);
            break;
        case CSSPropertyID::FlexBasis:
            value = CSSLengthValue::create(0.0, CSSLengthValue::Unit::None);
            break;
        default:
            value = CSSInitialValue::create();
            break;
        }
    }

    properties.emplace_back(id, important, value);
}

void CSSParser::addExpandedProperty(CSSPropertyList& properties, CSSPropertyID id, bool important, RefPtr<CSSValue> value)
{
    auto longhand = CSSShorthand::longhand(id);
    if(longhand.empty()) {
        addProperty(properties, id, important, value);
        return;
    }

    size_t index = 0;
    do {
        addProperty(properties, longhand.at(index), important, value);
        index += 1;
    } while(index < longhand.length());
}

struct idententry_t {
    std::string_view name;
    CSSValueID value;
};

template<unsigned int N>
inline CSSValueID matchIdent(const CSSTokenStream& input, const idententry_t(&table)[N])
{
    if(input->type() != CSSToken::Type::Ident)
        return CSSValueID::Unknown;

    auto name = input->data();
    for(auto& entry : table) {
        if(equals(name, entry.name, false))
            return entry.value;
    }

    return CSSValueID::Unknown;
}

template<unsigned int N>
inline RefPtr<CSSValue> consumeIdent(CSSTokenStream& input, const idententry_t(&table)[N])
{
    auto id = matchIdent(input, table);
    if(id == CSSValueID::Unknown)
        return nullptr;
    input.consumeIncludingWhitespace();
    return CSSIdentValue::create(id);
}

RefPtr<CSSValue> CSSParser::consumeNone(CSSTokenStream& input)
{
    if(input->type() == CSSToken::Type::Ident && equals(input->data(), "none", false)) {
        input.consumeIncludingWhitespace();
        return CSSIdentValue::create(CSSValueID::None);
    }

    return nullptr;
}

RefPtr<CSSValue> CSSParser::consumeAuto(CSSTokenStream& input)
{
    if(input->type() == CSSToken::Type::Ident && equals(input->data(), "auto", false)) {
        input.consumeIncludingWhitespace();
        return CSSIdentValue::create(CSSValueID::Auto);
    }

    return nullptr;
}

RefPtr<CSSValue> CSSParser::consumeNormal(CSSTokenStream& input)
{
    if(input->type() == CSSToken::Type::Ident && equals(input->data(), "normal", false)) {
        input.consumeIncludingWhitespace();
        return CSSIdentValue::create(CSSValueID::Normal);
    }

    return nullptr;
}

RefPtr<CSSValue> CSSParser::consumeNoneOrAuto(CSSTokenStream& input)
{
    if(auto value = consumeNone(input))
        return value;
    return consumeAuto(input);
}

RefPtr<CSSValue> CSSParser::consumeNoneOrNormal(CSSTokenStream& input)
{
    if(auto value = consumeNone(input))
        return value;
    return consumeNormal(input);
}

RefPtr<CSSValue> CSSParser::consumeInteger(CSSTokenStream& input, bool negative)
{
    if(input->type() != CSSToken::Type::Number || input->numberType() != CSSToken::NumberType::Integer || (input->integer() < 0 && !negative))
        return nullptr;

    auto value = input->integer();
    input.consumeIncludingWhitespace();
    return CSSIntegerValue::create(value);
}

RefPtr<CSSValue> CSSParser::consumePositiveInteger(CSSTokenStream& input)
{
    if(input->type() != CSSToken::Type::Number || input->numberType() != CSSToken::NumberType::Integer || input->integer() < 1)
        return nullptr;

    auto value = input->integer();
    input.consumeIncludingWhitespace();
    return CSSIntegerValue::create(value);
}

RefPtr<CSSValue> CSSParser::consumePercent(CSSTokenStream& input, bool negative)
{
    if(input->type() != CSSToken::Type::Percentage || (input->number() < 0 && !negative))
        return nullptr;

    auto value = input->number();
    input.consumeIncludingWhitespace();
    return CSSPercentValue::create(value);
}

RefPtr<CSSValue> CSSParser::consumeNumber(CSSTokenStream& input, bool negative)
{
    if(input->type() != CSSToken::Type::Number || (input->number() < 0 && !negative))
        return nullptr;

    auto value = input->number();
    input.consumeIncludingWhitespace();
    return CSSNumberValue::create(value);
}

RefPtr<CSSValue> CSSParser::consumeLength(CSSTokenStream& input, bool negative, bool unitless)
{
    if(input->type() != CSSToken::Type::Dimension && input->type() != CSSToken::Type::Number)
        return nullptr;

    auto value = input->number();
    if(value < 0.0 && !negative || (input->type() == CSSToken::Type::Number && !unitless))
        return nullptr;
    if(input->type() == CSSToken::Type::Number) {
        input.consumeIncludingWhitespace();
        return CSSLengthValue::create(value, CSSLengthValue::Unit::None);
    }

    static const struct {
        std::string_view name;
        CSSLengthValue::Unit value;
    } table[] = {
        {"em", CSSLengthValue::Unit::Ems},
        {"ex", CSSLengthValue::Unit::Exs},
        {"px", CSSLengthValue::Unit::Pixels},
        {"cm", CSSLengthValue::Unit::Centimeters},
        {"mm", CSSLengthValue::Unit::Millimeters},
        {"in", CSSLengthValue::Unit::Inches},
        {"pt", CSSLengthValue::Unit::Points},
        {"pc", CSSLengthValue::Unit::Picas},
        {"vw", CSSLengthValue::Unit::ViewportWidth},
        {"vh", CSSLengthValue::Unit::ViewportHeight},
        {"vmin", CSSLengthValue::Unit::ViewportMin},
        {"vmax", CSSLengthValue::Unit::ViewportMax},
        {"rem", CSSLengthValue::Unit::Rems},
        {"ch", CSSLengthValue::Unit::Chs}
    };

    auto name = input->data();
    auto it = std::find_if(table, std::end(table), [name](auto& item) { return equals(name, item.name, false); });
    if(it == std::end(table))
        return nullptr;
    input.consumeIncludingWhitespace();
    return CSSLengthValue::create(value, it->value);
}

RefPtr<CSSValue> CSSParser::consumeLengthOrAuto(CSSTokenStream& input, bool negative, bool unitless)
{
    if(auto value = consumeAuto(input))
        return value;
    return consumeLength(input, negative, unitless);
}

RefPtr<CSSValue> CSSParser::consumeLengthOrNormal(CSSTokenStream& input, bool negative, bool unitless)
{
    if(auto value = consumeNormal(input))
        return value;
    return consumeLength(input, negative, unitless);
}

RefPtr<CSSValue> CSSParser::consumeLengthOrPercent(CSSTokenStream& input, bool negative, bool unitless)
{
    auto value = consumeLength(input, negative, unitless);
    if(value == nullptr)
        return consumePercent(input, negative);
    return value;
}

RefPtr<CSSValue> CSSParser::consumeNumberOrPercent(CSSTokenStream& input, bool negative)
{
    auto value = consumeNumber(input, negative);
    if(value == nullptr)
        return consumePercent(input, negative);
    return value;
}

RefPtr<CSSValue> CSSParser::consumeIntegerOrAuto(CSSTokenStream& input, bool negative)
{
    if(auto value = consumeAuto(input))
        return value;
    return consumeInteger(input, negative);
}

RefPtr<CSSValue> CSSParser::consumePositiveIntegerOrAuto(CSSTokenStream& input)
{
    if(auto value = consumeAuto(input))
        return value;
    return consumePositiveInteger(input);
}

RefPtr<CSSValue> CSSParser::consumeLengthOrPercentOrAuto(CSSTokenStream& input, bool negative, bool unitless)
{
    if(auto value = consumeAuto(input))
        return value;
    return consumeLengthOrPercent(input, negative, unitless);
}

RefPtr<CSSValue> CSSParser::consumeLengthOrPercentOrNone(CSSTokenStream& input, bool negative, bool unitless)
{
    if(auto value = consumeNone(input))
        return value;
    return consumeLengthOrPercent(input, negative, unitless);
}

RefPtr<CSSValue> CSSParser::consumeLengthOrPercentOrNormal(CSSTokenStream& input, bool negative, bool unitless)
{
    if(auto value = consumeNormal(input))
        return value;
    return consumeLengthOrPercent(input, negative, unitless);
}

RefPtr<CSSValue> CSSParser::consumeString(CSSTokenStream& input)
{
    if(input->type() == CSSToken::Type::String) {
        std::string value(input->data());
        input.consumeIncludingWhitespace();
        return CSSStringValue::create(std::move(value));
    }

    return nullptr;
}

RefPtr<CSSValue> CSSParser::consumeCustomIdent(CSSTokenStream& input)
{
    if(input->type() == CSSToken::Type::Ident) {
        auto value = input->data();
        input.consumeIncludingWhitespace();
        return CSSCustomIdentValue::create(value);
    }

    return nullptr;
}

RefPtr<CSSValue> CSSParser::consumeUrl(CSSTokenStream& input, bool image)
{
    std::string value;
    switch(input->type()) {
    case CSSToken::Type::Url:
    case CSSToken::Type::String:
        value = input->data();
        input.consumeIncludingWhitespace();
        break;
    case CSSToken::Type::Function: {
        if(!equals(input->data(), "url", false))
            return nullptr;
        CSSTokenStreamGuard guard(input);
        auto block = input.consumeBlock();
        block.consumeWhitespace();
        value = block->data();
        block.consumeIncludingWhitespace();
        if(!block.empty())
            return nullptr;
        input.consumeWhitespace();
        guard.release();
        break;
    }

    default:
        return nullptr;
    }

    if(!image)
        return CSSUrlValue::create(std::move(value));
    return CSSImageValue::create(std::move(value));
}

RefPtr<CSSValue> CSSParser::consumeUrlOrNone(CSSTokenStream& input, bool image)
{
    if(auto value = consumeNone(input))
        return value;
    return consumeUrl(input, image);
}

RefPtr<CSSValue> CSSParser::consumeColor(CSSTokenStream& input)
{
    if(input->type() == CSSToken::Type::Hash) {
        int count = 0;
        uint32_t value = 0;
        for(auto cc : input->data()) {
            if(count >= 6 || !isxdigit(cc))
                return nullptr;
            value = value * 16 + xdigit(cc);
            count += 1;
        }

        if(count != 6 && count != 3)
            return nullptr;
        if(count == 3) {
            value = ((value&0xf00) << 8) | ((value&0x0f0) << 4) | (value&0x00f);
            value |= value << 4;
        }

        input.consumeIncludingWhitespace();
        return CSSColorValue::create(value | 0xFF000000);
    }

    if(input->type() == CSSToken::Type::Function) {
        auto name = input->data();
        if(equals(name, "rgb", false) || equals(name, "rgba", false))
            return consumeRgb(input);
        return nullptr;
    }

    if(input->type() == CSSToken::Type::Ident) {
        auto name = input->data();
        if(equals(name, "currentcolor", false)) {
            input.consumeIncludingWhitespace();
            return CSSIdentValue::create(CSSValueID::CurrentColor);
        }

        if(equals(name, "transparent", false)) {
            input.consumeIncludingWhitespace();
            return CSSColorValue::create(0x00000000);
        }

        static const struct {
            std::string_view name;
            uint32_t value;
        } table[] = {
            {"aliceblue", 0xF0F8FF},
            {"antiquewhite", 0xFAEBD7},
            {"aqua", 0x00FFFF},
            {"aquamarine", 0x7FFFD4},
            {"azure", 0xF0FFFF},
            {"beige", 0xF5F5DC},
            {"bisque", 0xFFE4C4},
            {"black", 0x000000},
            {"blanchedalmond", 0xFFEBCD},
            {"blue", 0x0000FF},
            {"blueviolet", 0x8A2BE2},
            {"brown", 0xA52A2A},
            {"burlywood", 0xDEB887},
            {"cadetblue", 0x5F9EA0},
            {"chartreuse", 0x7FFF00},
            {"chocolate", 0xD2691E},
            {"coral", 0xFF7F50},
            {"cornflowerblue", 0x6495ED},
            {"cornsilk", 0xFFF8DC},
            {"crimson", 0xDC143C},
            {"cyan", 0x00FFFF},
            {"darkblue", 0x00008B},
            {"darkcyan", 0x008B8B},
            {"darkgoldenrod", 0xB8860B},
            {"darkgray", 0xA9A9A9},
            {"darkgreen", 0x006400},
            {"darkgrey", 0xA9A9A9},
            {"darkkhaki", 0xBDB76B},
            {"darkmagenta", 0x8B008B},
            {"darkolivegreen", 0x556B2F},
            {"darkorange", 0xFF8C00},
            {"darkorchid", 0x9932CC},
            {"darkred", 0x8B0000},
            {"darksalmon", 0xE9967A},
            {"darkseagreen", 0x8FBC8F},
            {"darkslateblue", 0x483D8B},
            {"darkslategray", 0x2F4F4F},
            {"darkslategrey", 0x2F4F4F},
            {"darkturquoise", 0x00CED1},
            {"darkviolet", 0x9400D3},
            {"deeppink", 0xFF1493},
            {"deepskyblue", 0x00BFFF},
            {"dimgray", 0x696969},
            {"dimgrey", 0x696969},
            {"dodgerblue", 0x1E90FF},
            {"firebrick", 0xB22222},
            {"floralwhite", 0xFFFAF0},
            {"forestgreen", 0x228B22},
            {"fuchsia", 0xFF00FF},
            {"gainsboro", 0xDCDCDC},
            {"ghostwhite", 0xF8F8FF},
            {"gold", 0xFFD700},
            {"goldenrod", 0xDAA520},
            {"gray", 0x808080},
            {"green", 0x008000},
            {"greenyellow", 0xADFF2F},
            {"grey", 0x808080},
            {"honeydew", 0xF0FFF0},
            {"hotpink", 0xFF69B4},
            {"indianred", 0xCD5C5C},
            {"indigo", 0x4B0082},
            {"ivory", 0xFFFFF0},
            {"khaki", 0xF0E68C},
            {"lavender", 0xE6E6FA},
            {"lavenderblush", 0xFFF0F5},
            {"lawngreen", 0x7CFC00},
            {"lemonchiffon", 0xFFFACD},
            {"lightblue", 0xADD8E6},
            {"lightcoral", 0xF08080},
            {"lightcyan", 0xE0FFFF},
            {"lightgoldenrodyellow", 0xFAFAD2},
            {"lightgray", 0xD3D3D3},
            {"lightgreen", 0x90EE90},
            {"lightgrey", 0xD3D3D3},
            {"lightpink", 0xFFB6C1},
            {"lightsalmon", 0xFFA07A},
            {"lightseagreen", 0x20B2AA},
            {"lightskyblue", 0x87CEFA},
            {"lightslategray", 0x778899},
            {"lightslategrey", 0x778899},
            {"lightsteelblue", 0xB0C4DE},
            {"lightyellow", 0xFFFFE0},
            {"lime", 0x00FF00},
            {"limegreen", 0x32CD32},
            {"linen", 0xFAF0E6},
            {"magenta", 0xFF00FF},
            {"maroon", 0x800000},
            {"mediumaquamarine", 0x66CDAA},
            {"mediumblue", 0x0000CD},
            {"mediumorchid", 0xBA55D3},
            {"mediumpurple", 0x9370DB},
            {"mediumseagreen", 0x3CB371},
            {"mediumslateblue", 0x7B68EE},
            {"mediumspringgreen", 0x00FA9A},
            {"mediumturquoise", 0x48D1CC},
            {"mediumvioletred", 0xC71585},
            {"midnightblue", 0x191970},
            {"mintcream", 0xF5FFFA},
            {"mistyrose", 0xFFE4E1},
            {"moccasin", 0xFFE4B5},
            {"navajowhite", 0xFFDEAD},
            {"navy", 0x000080},
            {"oldlace", 0xFDF5E6},
            {"olive", 0x808000},
            {"olivedrab", 0x6B8E23},
            {"orange", 0xFFA500},
            {"orangered", 0xFF4500},
            {"orchid", 0xDA70D6},
            {"palegoldenrod", 0xEEE8AA},
            {"palegreen", 0x98FB98},
            {"paleturquoise", 0xAFEEEE},
            {"palevioletred", 0xDB7093},
            {"papayawhip", 0xFFEFD5},
            {"peachpuff", 0xFFDAB9},
            {"peru", 0xCD853F},
            {"pink", 0xFFC0CB},
            {"plum", 0xDDA0DD},
            {"powderblue", 0xB0E0E6},
            {"purple", 0x800080},
            {"rebeccapurple", 0x663399},
            {"red", 0xFF0000},
            {"rosybrown", 0xBC8F8F},
            {"royalblue", 0x4169E1},
            {"saddlebrown", 0x8B4513},
            {"salmon", 0xFA8072},
            {"sandybrown", 0xF4A460},
            {"seagreen", 0x2E8B57},
            {"seashell", 0xFFF5EE},
            {"sienna", 0xA0522D},
            {"silver", 0xC0C0C0},
            {"skyblue", 0x87CEEB},
            {"slateblue", 0x6A5ACD},
            {"slategray", 0x708090},
            {"slategrey", 0x708090},
            {"snow", 0xFFFAFA},
            {"springgreen", 0x00FF7F},
            {"steelblue", 0x4682B4},
            {"tan", 0xD2B48C},
            {"teal", 0x008080},
            {"thistle", 0xD8BFD8},
            {"tomato", 0xFF6347},
            {"turquoise", 0x40E0D0},
            {"violet", 0xEE82EE},
            {"wheat", 0xF5DEB3},
            {"white", 0xFFFFFF},
            {"whitesmoke", 0xF5F5F5},
            {"yellow", 0xFFFF00},
            {"yellowgreen", 0x9ACD32}
        };

        auto it = std::lower_bound(table, std::end(table), name, [](auto& item, auto& name) { return item.name < name; });
        if(it == std::end(table) || it->name != name)
            return nullptr;
        input.consumeIncludingWhitespace();
        return CSSColorValue::create(it->value | 0xFF000000);
    }

    return nullptr;
}

inline bool consumeRgbComponent(CSSTokenStream& input, int& component)
{
    if(input->type() != CSSToken::Type::Number
        && input->type() != CSSToken::Type::Percentage) {
        return false;
    }

    auto value = input->number();
    if(input->type() == CSSToken::Type::Percentage)
        value *= 2.55;
    value = std::clamp(value, 0.0, 255.0);
    component = static_cast<int>(std::round(value));
    input.consumeIncludingWhitespace();
    return true;
}

RefPtr<CSSValue> CSSParser::consumeRgb(CSSTokenStream& input)
{
    assert(input->type() == CSSToken::Type::Function);
    CSSTokenStreamGuard guard(input);
    auto block = input.consumeBlock();
    block.consumeWhitespace();

    int red = 0;
    if(!consumeRgbComponent(block, red))
        return nullptr;

    if(block->type() != CSSToken::Type::Comma)
        return nullptr;

    int blue = 0;
    block.consumeIncludingWhitespace();
    if(!consumeRgbComponent(block, blue))
        return nullptr;

    if(block->type() != CSSToken::Type::Comma)
        return nullptr;

    int green = 0;
    block.consumeIncludingWhitespace();
    if(!consumeRgbComponent(block, green))
        return nullptr;

    int alpha = 255;
    if(block->type() == CSSToken::Type::Comma) {
        block.consumeIncludingWhitespace();
        if(block->type() != CSSToken::Type::Number
            && block->type() != CSSToken::Type::Percentage) {
            return nullptr;
        }

        auto value = block->number();
        if(block->type() == CSSToken::Type::Percentage)
            value /= 100.0;
        value = std::clamp(value, 0.0, 1.0);
        alpha = static_cast<int>(std::round(value * 255.0));
        block.consumeIncludingWhitespace();
    }

    if(!block.empty())
        return nullptr;
    input.consumeWhitespace();
    guard.release();
    return CSSColorValue::create(red, green, blue, alpha);
}

RefPtr<CSSValue> CSSParser::consumeFillOrStroke(CSSTokenStream& input)
{
    if(auto value = consumeNone(input))
        return value;

    auto first = consumeUrl(input, false);
    if(first == nullptr)
        return consumeColor(input);

    auto second = consumeNone(input);
    if(second == nullptr)
        second = consumeColor(input);
    if(second == nullptr)
        return first;
    return CSSPairValue::create(first, second);
}

RefPtr<CSSValue> CSSParser::consumeQuotes(CSSTokenStream& input)
{
    if(auto value = consumeNoneOrAuto(input))
        return value;

    CSSValueList values;
    while(!input.empty()) {
        auto value = consumeString(input);
        if(value == nullptr)
            return nullptr;
        values.push_back(value);
    }

    if(!values.empty() && values.size() % 2 == 0)
        return CSSListValue::create(std::move(values));
    return nullptr;
}

RefPtr<CSSValue> CSSParser::consumeContent(CSSTokenStream& input)
{
    if(auto value = consumeNoneOrNormal(input))
        return value;

    CSSValueList values;
    while(!input.empty()) {
        auto value = consumeString(input);
        if(value == nullptr)
            value = consumeUrl(input, true);
        if(value == nullptr && input->type() == CSSToken::Type::Ident) {
            static const idententry_t table[] = {
                {"open-quote", CSSValueID::OpenQuote},
                {"close-quote", CSSValueID::CloseQuote},
                {"no-open-quote", CSSValueID::NoOpenQuote},
                {"no-close-quote", CSSValueID::NoCloseQuote}
            };

            value = consumeIdent(input, table);
        }

        if(value == nullptr && input->type() == CSSToken::Type::Function) {
            auto name = input->data();
            auto block = input.consumeBlock();
            block.consumeWhitespace();
            if(equals(name, "attr", false))
                value = consumeContentAttr(block);
            else if(equals(name, "counter", false))
                value = consumeContentCounter(block, false);
            else if(equals(name, "counters", false))
                value = consumeContentCounter(block, true);
            input.consumeWhitespace();
        }

        if(value == nullptr)
            return nullptr;
        values.push_back(value);
    }

    if(values.empty())
        return nullptr;
    return CSSListValue::create(std::move(values));
}

RefPtr<CSSValue> CSSParser::consumeContentAttr(CSSTokenStream& input)
{
    auto value = consumeCustomIdent(input);
    if(value == nullptr || !input.empty())
        return nullptr;
    return CSSFunctionValue::create(CSSValueID::Attr, std::move(value));
}

RefPtr<CSSValue> CSSParser::consumeContentCounter(CSSTokenStream& input, bool counters)
{
    if(input->type() != CSSToken::Type::Ident)
        return nullptr;

    auto identifier = input->data();
    input.consumeIncludingWhitespace();
    std::string separator;
    if(counters) {
        if(input->type() != CSSToken::Type::Comma)
            return nullptr;
        input.consumeIncludingWhitespace();
        if(input->type() != CSSToken::Type::String)
            return nullptr;
        separator = input->data();
        input.consumeIncludingWhitespace();
    }

    auto listStyle = ListStyleType::Decimal;
    if(input->type() == CSSToken::Type::Comma) {
        input.consumeIncludingWhitespace();
        if(input->type() == CSSToken::Type::Ident)
            return nullptr;
        static const struct {
            std::string_view name;
            ListStyleType value;
        } table[] = {
            {"disc", ListStyleType::Disc},
            {"circle", ListStyleType::Circle},
            {"square", ListStyleType::Square},
            {"decimal", ListStyleType::Decimal},
            {"decimal-leading-zero", ListStyleType::DecimalLeadingZero},
            {"lower-alpha", ListStyleType::LowerAlpha},
            {"lower-latin", ListStyleType::LowerLatin},
            {"lower-roman", ListStyleType::LowerRoman},
            {"upper-alpha", ListStyleType::UpperAlpha},
            {"upper-latin", ListStyleType::UpperLatin},
            {"upper-roman", ListStyleType::UpperRoman}
        };

        auto name = input->data();
        auto it = std::find_if(table, std::end(table), [name](auto& item) { return equals(name, item.name, false); });
        if(it == std::end(table))
            return nullptr;
        listStyle = it->value;
        input.consumeIncludingWhitespace();
    }

    if(!input.empty())
        return nullptr;
    return CSSCounterValue::create(identifier, listStyle, std::move(separator));
}

RefPtr<CSSValue> CSSParser::consumeCounter(CSSTokenStream& input, bool increment)
{
    if(auto value = consumeNone(input))
        return value;

    CSSValueList values;
    while(!input.empty()) {
        auto name = consumeCustomIdent(input);
        if(name == nullptr)
            return nullptr;
        auto value = increment ? 1 : 0;
        if(input->type() == CSSToken::Type::Number
            && input->numberType() == CSSToken::NumberType::Integer) {
            value = input->integer();
            input.consumeIncludingWhitespace();
        }

        values.push_back(CSSPairValue::create(name, CSSIntegerValue::create(value)));
    }

    return CSSListValue::create(std::move(values));
}

RefPtr<CSSValue> CSSParser::consumePage(CSSTokenStream& input)
{
    if(auto value = consumeAuto(input))
        return value;
    return consumeCustomIdent(input);
}

RefPtr<CSSValue> CSSParser::consumeSize(CSSTokenStream& input)
{
    if(auto value = consumeAuto(input))
        return value;

    if(auto width = consumeLength(input, false, false)) {
        auto height = consumeLength(input, false, false);
        if(height == nullptr)
            height = width;
        return CSSPairValue::create(width, height);
    }

    RefPtr<CSSValue> size;
    RefPtr<CSSValue> orientation;
    for(int index = 0; index < 2; ++index) {
        if(size == nullptr) {
            static const idententry_t table[] = {
                {"a3", CSSValueID::A3},
                {"a4", CSSValueID::A4},
                {"a5", CSSValueID::A5},
                {"b4", CSSValueID::B4},
                {"b5", CSSValueID::B5},
                {"ledger", CSSValueID::Ledger},
                {"legal", CSSValueID::Legal},
                {"letter", CSSValueID::Letter}
            };

            if(size = consumeIdent(input, table))
                continue;
        }

        if(orientation == nullptr) {
            static const idententry_t table[] = {
                {"portrait", CSSValueID::Portrait},
                {"landscape", CSSValueID::Landscape}
            };

            if(orientation = consumeIdent(input, table))
                continue;
        }

        break;
    }

    if(size == nullptr && orientation == nullptr)
        return nullptr;
    if(size == nullptr)
        return orientation;
    if(orientation == nullptr)
        return size;
    return CSSPairValue::create(size, orientation);
}

RefPtr<CSSValue> CSSParser::consumeFontWeight(CSSTokenStream& input)
{
    static const idententry_t table[] = {
        {"normal", CSSValueID::Normal},
        {"bold", CSSValueID::Bold},
        {"bolder", CSSValueID::Bolder},
        {"lighter", CSSValueID::Lighter}
    };

    if(auto value = consumeIdent(input, table))
        return value;

    if(input->type() != CSSToken::Type::Number || input->numberType() != CSSToken::NumberType::Integer)
        return nullptr;
    auto value = input->integer();
    if(value < 1 || value > 1000)
        return nullptr;
    input.consumeIncludingWhitespace();
    return CSSIntegerValue::create(value);
}

RefPtr<CSSValue> CSSParser::consumeFontSize(CSSTokenStream& input, bool unitless)
{
    static const idententry_t table[] = {
        {"xx-small", CSSValueID::XxSmall},
        {"x-small", CSSValueID::XSmall},
        {"small", CSSValueID::Small},
        {"medium", CSSValueID::Medium},
        {"large", CSSValueID::Large},
        {"x-large", CSSValueID::XLarge},
        {"xx-large", CSSValueID::XxLarge},
        {"xxx-large", CSSValueID::XxxLarge},
        {"smaller", CSSValueID::Smaller},
        {"larger", CSSValueID::Larger}
    };

    if(auto value = consumeIdent(input, table))
        return value;
    return consumeLengthOrPercent(input, false, unitless);
}

RefPtr<CSSValue> CSSParser::consumeFontFamilyValue(CSSTokenStream& input)
{
    if(input->type() == CSSToken::Type::String) {
        std::string value(input->data());
        input.consumeIncludingWhitespace();
        return CSSStringValue::create(std::move(value));
    }

    std::string value;
    while(input->type() == CSSToken::Type::Ident) {
        if(!value.empty())
            value += ' ';
        value += input->data();
        input.consumeIncludingWhitespace();
    }

    if(value.empty())
        return nullptr;
    return CSSStringValue::create(std::move(value));
}

RefPtr<CSSValue> CSSParser::consumeFontFamily(CSSTokenStream& input)
{
    CSSValueList values;
    while(!input.empty()) {
        auto value = consumeFontFamilyValue(input);
        if(value == nullptr)
            return nullptr;
        values.push_back(value);
    }

    return CSSListValue::create(std::move(values));
}

RefPtr<CSSValue> CSSParser::consumeFontFaceSourceValue(CSSTokenStream& input)
{
    CSSValueList values;
    if(input->type() == CSSToken::Type::Function && equals(input->data(), "local", false)) {
        auto block = input.consumeBlock();
        block.consumeWhitespace();
        auto value = consumeFontFamilyValue(block);
        if(value == nullptr || !block.empty())
            return nullptr;
        input.consumeWhitespace();
        values.push_back(CSSFunctionValue::create(CSSValueID::Local, std::move(value)));
    } else {
        auto url = consumeUrl(input, false);
        if(url == nullptr)
            return nullptr;
        values.push_back(std::move(url));
        if(input->type() == CSSToken::Type::Function && equals(input->data(), "format", false)) {
            auto block = input.consumeBlock();
            block.consumeWhitespace();
            auto value = consumeString(block);
            if(value == nullptr || !block.empty())
                return nullptr;
            input.consumeWhitespace();
            values.push_back(CSSFunctionValue::create(CSSValueID::Format, std::move(value)));
        }
    }

    return CSSListValue::create(std::move(values));
}

RefPtr<CSSValue> CSSParser::consumeFontFaceSource(CSSTokenStream& input)
{
    CSSValueList values;
    auto value = consumeFontFaceSourceValue(input);
    if(value == nullptr)
        return nullptr;

    values.push_back(value);
    while(input->type() == CSSToken::Type::Comma) {
        input.consumeIncludingWhitespace();
        auto value = consumeFontFaceSourceValue(input);
        if(value == nullptr)
            return nullptr;
        values.push_back(value);
    }

    return CSSListValue::create(std::move(values));
}

RefPtr<CSSValue> CSSParser::consumeLineWidth(CSSTokenStream& input, bool unitless)
{
    static const idententry_t table[] = {
        {"thin", CSSValueID::Thin},
        {"medium", CSSValueID::Medium},
        {"thick", CSSValueID::Thick}
    };

    if(auto value = consumeIdent(input, table))
        return value;
    return consumeLength(input, false, unitless);
}

RefPtr<CSSValue> CSSParser::consumeBorderRadiusValue(CSSTokenStream& input)
{
    auto first = consumeLengthOrPercent(input, false, false);
    if(first == nullptr)
        return nullptr;
    auto second = consumeLengthOrPercent(input, false, false);
    if(second == nullptr)
        second = first;
    return CSSPairValue::create(first, second);
}

RefPtr<CSSValue> CSSParser::consumeClip(CSSTokenStream& input)
{
    if(auto value = consumeAuto(input))
        return value;

    if(input->type() != CSSToken::Type::Function || !equals(input->data(), "rect", false))
        return nullptr;

    auto block = input.consumeBlock();
    block.consumeWhitespace();

    auto top = consumeLengthOrPercentOrAuto(block, true, true);
    if(top == nullptr)
        return nullptr;

    if(block->type() == CSSToken::Type::Comma)
        block.consumeIncludingWhitespace();

    auto right = consumeLengthOrPercentOrAuto(block, true, true);
    if(right == nullptr)
        return nullptr;

    if(block->type() == CSSToken::Type::Comma)
        block.consumeIncludingWhitespace();

    auto bottom = consumeLengthOrPercentOrAuto(block, true, true);
    if(bottom == nullptr)
        return nullptr;

    if(block->type() == CSSToken::Type::Comma)
        block.consumeIncludingWhitespace();

    auto left = consumeLengthOrPercentOrAuto(block, true, true);
    if(left == nullptr || !block.empty())
        return nullptr;
    return CSSRectValue::create(top, right, bottom, left);
}

RefPtr<CSSValue> CSSParser::consumeDashList(CSSTokenStream& input)
{
    if(auto value = consumeNone(input))
        return value;

    CSSValueList values;
    auto value = consumeLengthOrPercent(input, false, true);
    if(value == nullptr)
        return nullptr;

    values.push_back(value);
    while(input->type() == CSSToken::Type::Comma) {
        input.consumeIncludingWhitespace();
        auto value = consumeLengthOrPercent(input, false, true);
        if(value == nullptr)
            return nullptr;
        values.push_back(value);
    }

    if(!input.empty())
        return nullptr;
    return CSSListValue::create(std::move(values));
}

RefPtr<CSSValue> CSSParser::consumeVerticalAlign(CSSTokenStream& input)
{
    static const idententry_t table[] = {
        {"baseline", CSSValueID::Baseline},
        {"sub", CSSValueID::Sub},
        {"text-top", CSSValueID::TextTop},
        {"middle", CSSValueID::Middle},
        {"bottom", CSSValueID::Bottom},
        {"text-bottom", CSSValueID::TextBottom}
    };

    if(auto value = consumeIdent(input, table))
        return value;
    return consumeLengthOrPercent(input, true, true);
}

RefPtr<CSSValue> CSSParser::consumeTextDecorationLine(CSSTokenStream& input)
{
    if(auto value = consumeNone(input))
        return value;

    static const idententry_t table[] = {
        {"underline", CSSValueID::Underline},
        {"overline", CSSValueID::Overline},
        {"line-through", CSSValueID::LineThrough}
    };

    CSSValueList values;
    while(!input.empty()) {
        auto value = consumeIdent(input, table);
        if(value == nullptr)
            return nullptr;
        values.push_back(value);
    }

    return CSSListValue::create(std::move(values));
}

RefPtr<CSSValue> CSSParser::consumeBackgroundPosition(CSSTokenStream& input)
{
    RefPtr<CSSValue> first;
    RefPtr<CSSValue> second;
    for(int index = 0; index < 2; ++index) {
        if(first == nullptr && (first = consumeLengthOrPercent(input, true, false)))
            continue;
        if(second == nullptr && (second = consumeLengthOrPercent(input, true, false)))
            continue;
        if(first == nullptr) {
            static const idententry_t table[] = {
                {"left", CSSValueID::Left},
                {"right", CSSValueID::Right},
                {"center", CSSValueID::Center}
            };

            if(first = consumeIdent(input, table))
                continue;
        }

        if(second == nullptr) {
            static const idententry_t table[] = {
                {"top", CSSValueID::Left},
                {"bottom", CSSValueID::Bottom},
                {"center", CSSValueID::Center}
            };

            if(second = consumeIdent(input, table))
                continue;
        }

        break;
    }

    if(first == nullptr && second == nullptr)
        return nullptr;
    if(first == nullptr)
        first = CSSIdentValue::create(CSSValueID::Center);
    if(second == nullptr)
        second = CSSIdentValue::create(CSSValueID::Center);
    return CSSPairValue::create(first, second);
}

RefPtr<CSSValue> CSSParser::consumeBackgroundSize(CSSTokenStream& input)
{
    static const idententry_t table[] = {
        {"contain", CSSValueID::Contain},
        {"cover", CSSValueID::Cover}
    };

    if(auto value = consumeIdent(input, table))
        return value;

    auto first = consumeLengthOrPercentOrAuto(input, false, false);
    if(first == nullptr)
        return nullptr;
    auto second = consumeLengthOrPercentOrAuto(input, false, false);
    if(second == nullptr)
        second = first;
    return CSSPairValue::create(first, second);
}

RefPtr<CSSValue> CSSParser::consumeAngle(CSSTokenStream& input)
{
    if(input->type() != CSSToken::Type::Dimension)
        return nullptr;

    static const struct {
        std::string_view name;
        CSSAngleValue::Unit value;
    } table[] = {
        {"deg", CSSAngleValue::Unit::Degrees},
        {"rad", CSSAngleValue::Unit::Radians},
        {"grad", CSSAngleValue::Unit::Gradians},
        {"turn", CSSAngleValue::Unit::Turns}
    };

    auto unit = input->data();
    auto it = std::find_if(table, std::end(table), [unit](auto& item) { return equals(unit, item.name, false); });
    if(it == std::end(table))
        return nullptr;
    auto value = input->number();
    input.consumeIncludingWhitespace();
    return CSSAngleValue::create(value, it->value);
}

RefPtr<CSSValue> CSSParser::consumeTransformValue(CSSTokenStream& input)
{
    if(input->type() != CSSToken::Type::Function)
        return nullptr;

    static const struct {
        std::string_view name;
        CSSValueID value;
    } table[] = {
        {"rotate", CSSValueID::Rotate},
        {"rotatex", CSSValueID::RotateX},
        {"rotatey", CSSValueID::RotateY},
        {"skew", CSSValueID::Skew},
        {"skewx", CSSValueID::SkewX},
        {"skewy", CSSValueID::SkewY},
        {"scale", CSSValueID::Scale},
        {"scalex", CSSValueID::ScaleX},
        {"scaley", CSSValueID::ScaleY},
        {"translate", CSSValueID::Translate},
        {"translatex", CSSValueID::TranslateX},
        {"translatey", CSSValueID::TranslateY},
        {"matrix", CSSValueID::Matrix}
    };

    auto name = input->data();
    auto it = std::find_if(table, std::end(table), [name](auto& item) { return equals(name, item.name, false); });
    if(it == std::end(table))
        return nullptr;

    CSSValueList values;
    auto block = input.consumeBlock();
    block.consumeWhitespace();
    switch(it->value) {
    case CSSValueID::Rotate:
    case CSSValueID::RotateX:
    case CSSValueID::RotateY:
    case CSSValueID::Skew:
    case CSSValueID::SkewX:
    case CSSValueID::SkewY: {
        auto value = consumeAngle(block);
        if(value == nullptr)
            return nullptr;
        values.push_back(value);
        if(it->value == CSSValueID::Skew && block->type() == CSSToken::Type::Comma) {
            block.consumeIncludingWhitespace();
            auto value = consumeAngle(block);
            if(value == nullptr)
                return nullptr;
            values.push_back(value);
        }

        break;
    }

    case CSSValueID::Scale:
    case CSSValueID::ScaleX:
    case CSSValueID::ScaleY: {
        auto value = consumeNumberOrPercent(block, true);
        if(value == nullptr)
            return nullptr;
        values.push_back(value);
        if(it->value == CSSValueID::Scale && block->type() == CSSToken::Type::Comma) {
            block.consumeIncludingWhitespace();
            auto value = consumeNumberOrPercent(block, true);
            if(value == nullptr)
                return nullptr;
            values.push_back(value);
        }

        break;
    }

    case CSSValueID::Translate:
    case CSSValueID::TranslateX:
    case CSSValueID::TranslateY: {
        auto value = consumeLengthOrPercent(block, true, false);
        if(value == nullptr)
            return nullptr;
        values.push_back(value);
        if(it->value == CSSValueID::Translate && block->type() == CSSToken::Type::Comma) {
            block.consumeIncludingWhitespace();
            auto value = consumeLengthOrPercent(block, true, false);
            if(value == nullptr)
                return nullptr;
            values.push_back(value);
        }

        break;
    }

    case CSSValueID::Matrix: {
        int count = 6;
        while(count > 0) {
            auto value = consumeNumber(block, true);
            if(value == nullptr)
                return nullptr;
            count -= 1;
            if(count > 0 && block->type() == CSSToken::Type::Comma)
                block.consumeIncludingWhitespace();
            values.push_back(value);
        }

        break;
    }

    default:
        return nullptr;
    }

    if(!block.empty())
        return nullptr;
    input.consumeWhitespace();
    return CSSFunctionValue::create(it->value, std::move(values));
}

RefPtr<CSSValue> CSSParser::consumeTransform(CSSTokenStream& input)
{
    if(auto value = consumeNone(input))
        return value;

    CSSValueList values;
    while(!input.empty()) {
        auto value = consumeTransformValue(input);
        if(value == nullptr)
            return nullptr;
        values.push_back(value);
    }

    return CSSListValue::create(std::move(values));
}

RefPtr<CSSValue> CSSParser::consumePaintOrder(CSSTokenStream& input)
{
    if(auto value = consumeNormal(input))
        return value;

    static const idententry_t table[] = {
        {"fill", CSSValueID::Fill},
        {"stroke", CSSValueID::Stroke},
        {"markers", CSSValueID::Markers}
    };

    CSSValueList values;
    while(!input.empty()) {
        auto value = consumeIdent(input, table);
        if(value == nullptr)
            return nullptr;
        values.push_back(value);
    }

    return CSSListValue::create(std::move(values));
}

RefPtr<CSSValue> CSSParser::consumeLonghand(CSSTokenStream& input, CSSPropertyID id)
{
    switch(id) {
    case CSSPropertyID::FlexGrow:
    case CSSPropertyID::FlexShrink:
    case CSSPropertyID::StrokeMiterlimit:
        return consumeNumber(input, false);
    case CSSPropertyID::TabSize:
        return consumeLength(input, false, false);
    case CSSPropertyID::OutlineOffset:
        return consumeLength(input, true, false);
    case CSSPropertyID::BorderHorizontalSpacing:
    case CSSPropertyID::BorderVerticalSpacing:
        return consumeLength(input, false, true);
    case CSSPropertyID::Order:
        return consumeInteger(input, true);
    case CSSPropertyID::Widows:
    case CSSPropertyID::Orphans:
        return consumePositiveInteger(input);
    case CSSPropertyID::ColumnCount:
        return consumePositiveIntegerOrAuto(input);
    case CSSPropertyID::ZIndex:
        return consumeIntegerOrAuto(input, true);
    case CSSPropertyID::X:
    case CSSPropertyID::Y:
    case CSSPropertyID::Cx:
    case CSSPropertyID::Cy:
    case CSSPropertyID::StrokeDashoffset:
    case CSSPropertyID::TextIndent:
        return consumeLengthOrPercent(input, true, true);
    case CSSPropertyID::R:
    case CSSPropertyID::Rx:
    case CSSPropertyID::Ry:
    case CSSPropertyID::PaddingTop:
    case CSSPropertyID::PaddingRight:
    case CSSPropertyID::PaddingBottom:
    case CSSPropertyID::PaddingLeft:
    case CSSPropertyID::StrokeWidth:
        return consumeLengthOrPercent(input, false, true);
    case CSSPropertyID::Opacity:
    case CSSPropertyID::FillOpacity:
    case CSSPropertyID::StrokeOpacity:
    case CSSPropertyID::StopOpacity:
        return consumeNumberOrPercent(input, false);
    case CSSPropertyID::Bottom:
    case CSSPropertyID::Left:
    case CSSPropertyID::Right:
    case CSSPropertyID::Top:
    case CSSPropertyID::MarginTop:
    case CSSPropertyID::MarginRight:
    case CSSPropertyID::MarginBottom:
    case CSSPropertyID::MarginLeft:
        return consumeLengthOrPercentOrAuto(input, true, true);
    case CSSPropertyID::MinWidth:
    case CSSPropertyID::MinHeight:
    case CSSPropertyID::Width:
    case CSSPropertyID::Height:
        return consumeLengthOrPercentOrAuto(input, false, true);
    case CSSPropertyID::MaxWidth:
    case CSSPropertyID::MaxHeight:
        return consumeLengthOrPercentOrNone(input, false, true);
    case CSSPropertyID::FlexBasis:
        return consumeLengthOrPercentOrAuto(input, false, false);
    case CSSPropertyID::Fill:
    case CSSPropertyID::Stroke:
        return consumeFillOrStroke(input);
    case CSSPropertyID::BorderBottomWidth:
    case CSSPropertyID::BorderLeftWidth:
    case CSSPropertyID::BorderRightWidth:
    case CSSPropertyID::BorderTopWidth:
        return consumeLineWidth(input, true);
    case CSSPropertyID::ColumnRuleWidth:
    case CSSPropertyID::OutlineWidth:
        return consumeLineWidth(input, false);
    case CSSPropertyID::LineHeight:
    case CSSPropertyID::ColumnGap:
        return consumeLengthOrPercentOrNormal(input, false, false);
    case CSSPropertyID::LetterSpacing:
    case CSSPropertyID::WordSpacing:
        return consumeLengthOrNormal(input, true, true);
    case CSSPropertyID::ColumnWidth:
        return consumeLengthOrAuto(input, false, false);
    case CSSPropertyID::Quotes:
        return consumeQuotes(input);
    case CSSPropertyID::Clip:
        return consumeClip(input);
    case CSSPropertyID::Size:
        return consumeSize(input);
    case CSSPropertyID::Page:
        return consumePage(input);
    case CSSPropertyID::FontWeight:
        return consumeFontWeight(input);
    case CSSPropertyID::FontSize:
        return consumeFontSize(input, true);
    case CSSPropertyID::FontFamily:
        return consumeFontFamily(input);
    case CSSPropertyID::Src:
        return consumeFontFaceSource(input);
    case CSSPropertyID::BorderBottomLeftRadius:
    case CSSPropertyID::BorderBottomRightRadius:
    case CSSPropertyID::BorderTopLeftRadius:
    case CSSPropertyID::BorderTopRightRadius:
        return consumeBorderRadiusValue(input);
    case CSSPropertyID::Color:
    case CSSPropertyID::BackgroundColor:
    case CSSPropertyID::TextDecorationColor:
    case CSSPropertyID::StopColor:
    case CSSPropertyID::OutlineColor:
    case CSSPropertyID::ColumnRuleColor:
    case CSSPropertyID::BorderBottomColor:
    case CSSPropertyID::BorderLeftColor:
    case CSSPropertyID::BorderRightColor:
    case CSSPropertyID::BorderTopColor:
        return consumeColor(input);
    case CSSPropertyID::ClipPath:
    case CSSPropertyID::MarkerEnd:
    case CSSPropertyID::MarkerMid:
    case CSSPropertyID::MarkerStart:
    case CSSPropertyID::Mask:
        return consumeUrlOrNone(input, false);
    case CSSPropertyID::ListStyleImage:
    case CSSPropertyID::BackgroundImage:
        return consumeUrlOrNone(input, true);
    case CSSPropertyID::Content:
        return consumeContent(input);
    case CSSPropertyID::CounterIncrement:
        return consumeCounter(input, true);
    case CSSPropertyID::CounterReset:
    case CSSPropertyID::CounterSet:
        return consumeCounter(input, false);
    case CSSPropertyID::StrokeDasharray:
        return consumeDashList(input);
    case CSSPropertyID::VerticalAlign:
        return consumeVerticalAlign(input);
    case CSSPropertyID::TextDecorationLine:
        return consumeTextDecorationLine(input);
    case CSSPropertyID::BackgroundSize:
        return consumeBackgroundSize(input);
    case CSSPropertyID::BackgroundPosition:
    case CSSPropertyID::TransformOrigin:
        return consumeBackgroundPosition(input);
    case CSSPropertyID::Transform:
        return consumeTransform(input);
    case CSSPropertyID::PaintOrder:
        return consumePaintOrder(input);
    case CSSPropertyID::BackgroundAttachment: {
        static const idententry_t table[] = {
            {"scroll", CSSValueID::Scroll},
            {"fixed", CSSValueID::Fixed},
            {"local", CSSValueID::Local}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::BackgroundClip:
    case CSSPropertyID::BackgroundOrigin: {
        static const idententry_t table[] = {
            {"border-box", CSSValueID::BorderBox},
            {"padding-box", CSSValueID::PaddingBox},
            {"content-box", CSSValueID::ContentBox}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::BackgroundRepeat: {
        static const idententry_t table[] = {
            {"repeat", CSSValueID::Repeat},
            {"repeat-x", CSSValueID::RepeatX},
            {"repeat-y", CSSValueID::RepeatY},
            {"no-repeat", CSSValueID::NoRepeat}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::FontStyle: {
        static const idententry_t table[] = {
            {"normal", CSSValueID::Normal},
            {"italic", CSSValueID::Italic},
            {"oblique", CSSValueID::Oblique}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::FontVariant: {
        static const idententry_t table[] = {
            {"normal", CSSValueID::Normal},
            {"small-caps", CSSValueID::SmallCaps}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::BorderCollapse: {
        static const idententry_t table[] = {
            {"collapse", CSSValueID::Collapse},
            {"separate", CSSValueID::Separate}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::BorderTopStyle:
    case CSSPropertyID::BorderRightStyle:
    case CSSPropertyID::BorderBottomStyle:
    case CSSPropertyID::BorderLeftStyle:
    case CSSPropertyID::ColumnRuleStyle: {
        static const idententry_t table[] = {
            {"none", CSSValueID::None},
            {"inset", CSSValueID::Inset},
            {"groove", CSSValueID::Groove},
            {"ridge", CSSValueID::Ridge},
            {"outset", CSSValueID::Outset},
            {"dotted", CSSValueID::Dotted},
            {"dashed", CSSValueID::Dashed},
            {"solid", CSSValueID::Solid},
            {"double", CSSValueID::Double}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::BoxSizing: {
        static const idententry_t table[] = {
            {"border-box", CSSValueID::BorderBox},
            {"content-box", CSSValueID::ContentBox}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::CaptionSide: {
        static const idententry_t table[] = {
            {"top", CSSValueID::Top},
            {"bottom", CSSValueID::Bottom}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::Clear: {
        static const idententry_t table[] = {
            {"none", CSSValueID::None},
            {"left", CSSValueID::Left},
            {"right", CSSValueID::Right},
            {"both", CSSValueID::Both}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::EmptyCells: {
        static const idententry_t table[] = {
            {"show", CSSValueID::Show},
            {"hide", CSSValueID::Hide}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::FillRule:
    case CSSPropertyID::ClipRule: {
        static const idententry_t table[] = {
            {"nonzero", CSSValueID::Nonzero},
            {"evenodd", CSSValueID::Evenodd}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::Float: {
        static const idententry_t table[] = {
            {"none", CSSValueID::None},
            {"left", CSSValueID::Left},
            {"right", CSSValueID::Right}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::Hyphens: {
        static const idententry_t table[] = {
            {"none", CSSValueID::None},
            {"auto", CSSValueID::Auto},
            {"manual", CSSValueID::Manual}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::ListStyleType:  {
        static const idententry_t table[] = {
            {"none", CSSValueID::None},
            {"disc", CSSValueID::Disc},
            {"circle", CSSValueID::Circle},
            {"square", CSSValueID::Square},
            {"decimal", CSSValueID::Decimal},
            {"decimal-leading-zero", CSSValueID::DecimalLeadingZero},
            {"lower-alpha", CSSValueID::LowerAlpha},
            {"lower-latin", CSSValueID::LowerLatin},
            {"lower-roman", CSSValueID::LowerRoman},
            {"upper-alpha", CSSValueID::UpperAlpha},
            {"upper-latin", CSSValueID::UpperLatin},
            {"upper-roman", CSSValueID::UpperRoman}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::ListStylePosition: {
        static const idententry_t table[] = {
            {"inside", CSSValueID::Inside},
            {"outside", CSSValueID::Outside}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::OutlineStyle: {
        static const idententry_t table[] = {
            {"auto", CSSValueID::Auto},
            {"none", CSSValueID::None},
            {"inset", CSSValueID::Inset},
            {"groove", CSSValueID::Groove},
            {"ridge", CSSValueID::Ridge},
            {"outset", CSSValueID::Outset},
            {"dotted", CSSValueID::Dotted},
            {"dashed", CSSValueID::Dashed},
            {"solid", CSSValueID::Solid},
            {"double", CSSValueID::Double}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::OverflowWrap:
    case CSSPropertyID::WordWrap: {
        static const idententry_t table[] = {
            {"normal", CSSValueID::Normal},
            {"break-word", CSSValueID::BreakWord},
            {"anywhere", CSSValueID::Anywhere}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::OverflowX:
    case CSSPropertyID::OverflowY: {
        static const idententry_t table[] = {
            {"auto", CSSValueID::Auto},
            {"visible", CSSValueID::Visible},
            {"hidden", CSSValueID::Hidden},
            {"scroll", CSSValueID::Scroll},
            {"overlay", CSSValueID::Overlay},
            {"clip", CSSValueID::Clip}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::ColumnBreakAfter:
    case CSSPropertyID::ColumnBreakBefore:
    case CSSPropertyID::PageBreakAfter:
    case CSSPropertyID::PageBreakBefore: {
        static const idententry_t table[] = {
            {"auto", CSSValueID::Auto},
            {"avoid", CSSValueID::Avoid},
            {"always", CSSValueID::Always},
            {"left", CSSValueID::Left},
            {"right", CSSValueID::Right}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::ColumnBreakInside:
    case CSSPropertyID::PageBreakInside: {
        static const idententry_t table[] = {
            {"auto", CSSValueID::Auto},
            {"avoid", CSSValueID::Avoid}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::PageOrientation: {
        static const idententry_t table[] = {
            {"upright", CSSValueID::Upright},
            {"rotate-left", CSSValueID::RotateLeft},
            {"rotate-right", CSSValueID::RotateRight}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::Position: {
        static const idententry_t table[] = {
            {"static", CSSValueID::Static},
            {"relative", CSSValueID::Relative},
            {"absolute", CSSValueID::Absolute},
            {"fixed", CSSValueID::Fixed}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::StrokeLinecap: {
        static const idententry_t table[] = {
            {"butt", CSSValueID::Butt},
            {"round", CSSValueID::Round},
            {"square", CSSValueID::Square}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::StrokeLinejoin: {
        static const idententry_t table[] = {
            {"miter", CSSValueID::Miter},
            {"round", CSSValueID::Round},
            {"bevel", CSSValueID::Bevel}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::TableLayout: {
        static const idententry_t table[] = {
            {"auto", CSSValueID::Auto},
            {"fixed", CSSValueID::Fixed}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::TextAlign: {
        static const idententry_t table[] = {
            {"left", CSSValueID::Left},
            {"right", CSSValueID::Right},
            {"center", CSSValueID::Center},
            {"justify", CSSValueID::Justify}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::TextAnchor: {
        static const idententry_t table[] = {
            {"start", CSSValueID::Start},
            {"middle", CSSValueID::Middle},
            {"end", CSSValueID::End}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::TextDecorationStyle: {
        static const idententry_t table[] = {
            {"solid", CSSValueID::Solid},
            {"double", CSSValueID::Double},
            {"dotted", CSSValueID::Dotted},
            {"dashed", CSSValueID::Dashed},
            {"wavy", CSSValueID::Wavy}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::TextOverflow: {
        static const idententry_t table[] = {
            {"clip", CSSValueID::Clip},
            {"ellipsis", CSSValueID::Ellipsis}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::TextTransform: {
        static const idententry_t table[] = {
            {"none", CSSValueID::None},
            {"capitalize", CSSValueID::Capitalize},
            {"uppercase", CSSValueID::Uppercase},
            {"lowercase", CSSValueID::Lowercase}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::MixBlendMode: {
        static const idententry_t table[] = {
            {"normal", CSSValueID::Normal},
            {"multiply", CSSValueID::Multiply},
            {"screen", CSSValueID::Screen},
            {"overlay", CSSValueID::Overlay},
            {"darken", CSSValueID::Darken},
            {"lighten", CSSValueID::Lighten},
            {"color-dodge", CSSValueID::ColorDodge},
            {"color-burn", CSSValueID::ColorBurn},
            {"hard-light", CSSValueID::HardLight},
            {"soft-light", CSSValueID::SoftLight},
            {"difference", CSSValueID::Difference},
            {"exclusion", CSSValueID::Exclusion},
            {"hue", CSSValueID::Hue},
            {"saturation", CSSValueID::Saturation},
            {"color", CSSValueID::Color},
            {"luminosity", CSSValueID::Luminosity}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::VectorEffect: {
        static const idententry_t table[] = {
            {"none", CSSValueID::None},
            {"non-scaling-stroke", CSSValueID::NonScalingStroke}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::Visibility: {
        static const idententry_t table[] = {
            {"visible", CSSValueID::Visible},
            {"hidden", CSSValueID::Hidden},
            {"collapse", CSSValueID::Collapse}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::Display: {
        static const idententry_t table[] = {
            {"none", CSSValueID::None},
            {"block", CSSValueID::Block},
            {"flex", CSSValueID::Flex},
            {"inline", CSSValueID::Inline},
            {"inline-block", CSSValueID::InlineBlock},
            {"Inline-flex", CSSValueID::InlineFlex},
            {"inline-table", CSSValueID::InlineTable},
            {"list-item", CSSValueID::ListItem},
            {"table", CSSValueID::Table},
            {"table-caption", CSSValueID::TableCaption},
            {"table-cell", CSSValueID::TableCell},
            {"table-column", CSSValueID::TableColumn},
            {"table-column-group", CSSValueID::TableColumnGroup},
            {"table-footer-group", CSSValueID::TableFooterGroup},
            {"table-header-group", CSSValueID::TableHeaderGroup},
            {"table-row", CSSValueID::TableRow},
            {"table-row-group", CSSValueID::TableRowGroup}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::FlexDirection: {
        static const idententry_t table[] = {
            {"row", CSSValueID::Row},
            {"row-reverse", CSSValueID::RowReverse},
            {"column", CSSValueID::Column},
            {"column-reverse", CSSValueID::ColumnReverse}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::FlexWrap: {
        static const idententry_t table[] = {
            {"nowrap", CSSValueID::Nowrap},
            {"wrap", CSSValueID::Wrap},
            {"wrap-reverse", CSSValueID::WrapReverse}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::LineBreak: {
        static const idententry_t table[] = {
            {"auto", CSSValueID::Auto},
            {"normal", CSSValueID::Normal},
            {"loose", CSSValueID::Loose},
            {"strict", CSSValueID::Strict},
            {"anywhere", CSSValueID::Anywhere}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::WhiteSpace: {
        static const idententry_t table[] = {
            {"normal", CSSValueID::Normal},
            {"pre", CSSValueID::Pre},
            {"pre-wrap", CSSValueID::PreWrap},
            {"pre-line", CSSValueID::PreLine},
            {"nowrap", CSSValueID::Nowrap},
            {"break-spaces", CSSValueID::BreakSpaces}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::WordBreak: {
        static const idententry_t table[] = {
            {"normal", CSSValueID::Normal},
            {"break-all", CSSValueID::BreakAll},
            {"keep-all", CSSValueID::KeepAll},
            {"break-word", CSSValueID::BreakWord}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::Direction: {
        static const idententry_t table[] = {
            {"ltr", CSSValueID::Ltr},
            {"rtl", CSSValueID::Rtl}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::UnicodeBidi: {
        static const idententry_t table[] = {
            {"normal", CSSValueID::Normal},
            {"embed", CSSValueID::Embed},
            {"bidi-override", CSSValueID::BidiOverride},
            {"isolate", CSSValueID::Isolate},
            {"isolate-override", CSSValueID::IsolateOverride},
            {"plaintext", CSSValueID::Plaintext}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::ColumnSpan: {
        static const idententry_t table[] = {
            {"none", CSSValueID::None},
            {"all", CSSValueID::All}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::ColumnFill: {
        static const idententry_t table[] = {
            {"auto", CSSValueID::Auto},
            {"balance", CSSValueID::Balance}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::AlignContent:
    case CSSPropertyID::JustifyContent: {
        static const idententry_t table[] = {
            {"flex-start", CSSValueID::FlexStart},
            {"flex-end", CSSValueID::FlexEnd},
            {"center", CSSValueID::Center},
            {"space-between", CSSValueID::SpaceBetween},
            {"space-around", CSSValueID::SpaceAround},
            {"space-evenly", CSSValueID::SpaceEvenly},
            {"stretch", CSSValueID::Stretch}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::AlignItems: {
        static const idententry_t table[] = {
            {"flex-start", CSSValueID::FlexStart},
            {"flex-end", CSSValueID::FlexEnd},
            {"center", CSSValueID::Center},
            {"stretch", CSSValueID::Stretch},
            {"baseline", CSSValueID::Baseline}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::AlignSelf: {
        static const idententry_t table[] = {
            {"auto", CSSValueID::Auto},
            {"flex-start", CSSValueID::FlexStart},
            {"flex-end", CSSValueID::FlexEnd},
            {"center", CSSValueID::Center},
            {"stretch", CSSValueID::Stretch},
            {"baseline", CSSValueID::Baseline}
        };

        return consumeIdent(input, table);
    }

    default:
        return nullptr;
    }
}

bool CSSParser::consumeFlex(CSSTokenStream& input, CSSPropertyList& properties, bool important)
{
    RefPtr<CSSValue> grow;
    RefPtr<CSSValue> shrink;
    RefPtr<CSSValue> basis;
    for(int index = 0; index < 3; ++index) {
        if(input->type() == CSSToken::Type::Ident) {
            if(!equals(input->data(), "none", false))
                return false;
            grow = CSSNumberValue::create(0.0);
            shrink = CSSNumberValue::create(0.0);
            basis = CSSIdentValue::create(CSSValueID::Auto);
            input.consumeIncludingWhitespace();
            break;
        }

        if(input->type() == CSSToken::Type::Number) {
            if(input->number() < 0.0)
                return false;
            if(grow == nullptr)
                grow = CSSNumberValue::create(input->number());
            else if(shrink == nullptr)
                shrink = CSSNumberValue::create(input->number());
            else if(input->number() == 0.0)
                basis = CSSLengthValue::create(0.0, CSSLengthValue::Unit::None);
            else
                return false;
            input.consumeIncludingWhitespace();
            continue;
        }

        if(basis == nullptr && (basis = consumeLengthOrPercentOrAuto(input, false, false))) {
            if(index == 1 && !input.empty())
                return false;
            continue;
        }

        break;
    }

    if(!input.empty())
        return false;

    addProperty(properties, CSSPropertyID::FlexGrow, important, grow);
    addProperty(properties, CSSPropertyID::FlexShrink, important, shrink);
    addProperty(properties, CSSPropertyID::FlexBasis, important, basis);
    return true;
}

bool CSSParser::consumeBackground(CSSTokenStream& input, CSSPropertyList& properties, bool important)
{
    RefPtr<CSSValue> color;
    RefPtr<CSSValue> image;
    RefPtr<CSSValue> repeat;
    RefPtr<CSSValue> attachment;
    RefPtr<CSSValue> origin;
    RefPtr<CSSValue> clip;
    RefPtr<CSSValue> position;
    RefPtr<CSSValue> size;
    while(!input.empty()) {
        if(position == nullptr && (position = consumeBackgroundPosition(input))) {
            if(input->type() == CSSToken::Type::Delim && input->delim() == '/') {
                input.consumeIncludingWhitespace();
                if(size = consumeBackgroundSize(input))
                    continue;
                return false;
            }

            continue;
        }

        if(image == nullptr && (image = consumeUrl(input, true)))
            continue;
        if(repeat == nullptr && (repeat = consumeLonghand(input, CSSPropertyID::BackgroundRepeat)))
            continue;
        if(attachment == nullptr && (attachment = consumeLonghand(input, CSSPropertyID::BackgroundAttachment)))
            continue;
        if(origin == nullptr && (origin = consumeLonghand(input, CSSPropertyID::BackgroundOrigin)))
            continue;
        if(clip == nullptr && (clip = consumeLonghand(input, CSSPropertyID::BackgroundClip)))
            continue;
        if(color == nullptr && (color = consumeColor(input)))
            continue;
        return false;
    }

    if(clip == nullptr)
        clip = origin;

    addProperty(properties, CSSPropertyID::BackgroundColor, important, color);
    addProperty(properties, CSSPropertyID::BackgroundImage, important, image);
    addProperty(properties, CSSPropertyID::BackgroundRepeat, important, repeat);
    addProperty(properties, CSSPropertyID::BackgroundAttachment, important, attachment);
    addProperty(properties, CSSPropertyID::BackgroundOrigin, important, origin);
    addProperty(properties, CSSPropertyID::BackgroundClip, important, clip);
    addProperty(properties, CSSPropertyID::BackgroundPosition, important, position);
    addProperty(properties, CSSPropertyID::BackgroundSize, important, size);
    return true;
}

bool CSSParser::consumeColumns(CSSTokenStream& input, CSSPropertyList& properties, bool important)
{
    RefPtr<CSSValue> width;
    RefPtr<CSSValue> count;
    for(int index = 0; index < 2; ++index) {
        if(input->type() == CSSToken::Type::Ident) {
            if(!equals(input->data(), "auto", false))
                return false;
            input.consumeIncludingWhitespace();
            continue;
        }

        if(width == nullptr && (width = consumeLength(input, false, false)))
            continue;
        if(count == nullptr && (count = consumePositiveInteger(input)))
            continue;
        break;
    }

    if(!input.empty())
        return false;

    addProperty(properties, CSSPropertyID::ColumnWidth, important, width);
    addProperty(properties, CSSPropertyID::ColumnCount, important, count);
    return true;
}

bool CSSParser::consumeFont(CSSTokenStream& input, CSSPropertyList& properties, bool important)
{
    RefPtr<CSSValue> style;
    RefPtr<CSSValue> variant;
    RefPtr<CSSValue> weight;
    RefPtr<CSSValue> lineHeight;
    for(int index = 0; index < 3; ++index) {
        if(input->type() == CSSToken::Type::Ident && equals(input->data(), "normal", false)) {
            input.consumeIncludingWhitespace();
            continue;
        }

        if(weight == nullptr && (weight = consumeFontWeight(input)))
            continue;
        if(style == nullptr && (style = consumeLonghand(input, CSSPropertyID::FontStyle)))
            continue;
        if(variant == nullptr && (variant = consumeLonghand(input, CSSPropertyID::FontVariant)))
            continue;
        break;
    }

    if(input.empty())
        return false;

    addProperty(properties, CSSPropertyID::FontStyle, important, style);
    addProperty(properties, CSSPropertyID::FontVariant, important, variant);
    addProperty(properties, CSSPropertyID::FontWeight, important, weight);

    auto size = consumeFontSize(input, false);
    if(size == nullptr || input.empty())
        return false;

    addProperty(properties, CSSPropertyID::FontSize, important, size);
    if(input->type() == CSSToken::Type::Delim && input->delim() == '/') {
        input.consumeIncludingWhitespace();
        auto value = consumeLengthOrPercentOrNormal(input, false, false);
        if(value == nullptr)
            return false;
        lineHeight = value;
    }

    addProperty(properties, CSSPropertyID::LineHeight, important, lineHeight);
    auto family = consumeFontFamily(input);
    if(family == nullptr || !input.empty())
        return false;

    addProperty(properties, CSSPropertyID::FontFamily, important, family);
    return true;
}

bool CSSParser::consumeBorder(CSSTokenStream& input, CSSPropertyList& properties, bool important)
{
    RefPtr<CSSValue> width;
    RefPtr<CSSValue> style;
    RefPtr<CSSValue> color;
    while(!input.empty()) {
        if(width == nullptr && (width = consumeLineWidth(input, false)))
            continue;
        if(style == nullptr && (style = consumeLonghand(input, CSSPropertyID::BorderTopStyle)))
            continue;
        if(color == nullptr && (color = consumeColor(input)))
            continue;
        return false;
    }

    addExpandedProperty(properties, CSSPropertyID::BorderWidth, important, width);
    addExpandedProperty(properties, CSSPropertyID::BorderStyle, important, style);
    addExpandedProperty(properties, CSSPropertyID::BorderColor, important, color);
    return true;
}

bool CSSParser::consumeBorderRadius(CSSTokenStream& input, CSSPropertyList& properties, bool important)
{
    RefPtr<CSSValue> horizontal[4] = {nullptr, nullptr, nullptr, nullptr};
    RefPtr<CSSValue> vertical[4] = {nullptr, nullptr, nullptr, nullptr};

    auto completesides = [](auto sides[4]) {
        if(sides[1] == nullptr) sides[1] = sides[0];
        if(sides[2] == nullptr) sides[2] = sides[0];
        if(sides[3] == nullptr) sides[3] = sides[1];
    };

    for(auto& side : horizontal) {
        if(input.empty() || input->type() == CSSToken::Type::Delim)
            break;
        auto value = consumeLengthOrPercent(input, false, true);
        if(value == nullptr)
            return false;
        side = value;
    }

    if(horizontal[0] == nullptr)
        return false;
    completesides(horizontal);
    if(input->type() == CSSToken::Type::Delim && input->delim() == '/') {
        input.consumeIncludingWhitespace();
        for(auto& side : vertical) {
            if(input->type() == CSSToken::Type::EndOfFile)
                break;
            auto value = consumeLengthOrPercent(input, false, true);
            if(value == nullptr)
                return false;
            side = value;
        }

        if(vertical[0] == nullptr)
            return false;
        completesides(vertical);
    } else if(input->type() == CSSToken::Type::EndOfFile) {
        vertical[0] = horizontal[0];
        vertical[1] = horizontal[1];
        vertical[2] = horizontal[2];
        vertical[3] = horizontal[3];
    } else {
        return false;
    }

    auto tl = CSSPairValue::create(horizontal[0], vertical[0]);
    auto tr = CSSPairValue::create(horizontal[1], vertical[1]);
    auto br = CSSPairValue::create(horizontal[2], vertical[2]);
    auto bl = CSSPairValue::create(horizontal[3], vertical[3]);

    addProperty(properties, CSSPropertyID::BorderTopLeftRadius, important, tl);
    addProperty(properties, CSSPropertyID::BorderTopRightRadius, important, tr);
    addProperty(properties, CSSPropertyID::BorderBottomRightRadius, important, br);
    addProperty(properties, CSSPropertyID::BorderBottomLeftRadius, important, bl);
    return true;
}

bool CSSParser::consume2Shorthand(CSSTokenStream& input, CSSPropertyList& properties, CSSPropertyID id, bool important)
{
    auto longhand = CSSShorthand::longhand(id);
    assert(longhand.length() == 2);
    auto first = consumeLonghand(input, longhand.at(0));
    if(first == nullptr)
        return false;

    addProperty(properties, longhand.at(0), important, first);
    auto second = consumeLonghand(input, longhand.at(1));
    if(second == nullptr) {
        addProperty(properties, longhand.at(1), important, first);
        return true;
    }

    addProperty(properties, longhand.at(1), important, second);
    return true;
}

bool CSSParser::consume4Shorthand(CSSTokenStream& input, CSSPropertyList& properties, CSSPropertyID id, bool important)
{
    auto longhand = CSSShorthand::longhand(id);
    assert(longhand.length() == 4);
    auto top = consumeLonghand(input, longhand.at(0));
    if(top == nullptr)
        return false;

    addProperty(properties, longhand.at(0), important, top);
    auto right = consumeLonghand(input, longhand.at(1));
    if(right == nullptr) {
        addProperty(properties, longhand.at(1), important, top);
        addProperty(properties, longhand.at(2), important, top);
        addProperty(properties, longhand.at(3), important, top);
        return true;
    }

    addProperty(properties, longhand.at(1), important, right);
    auto bottom = consumeLonghand(input, longhand.at(1));
    if(bottom == nullptr) {
        addProperty(properties, longhand.at(2), important, top);
        addProperty(properties, longhand.at(3), important, right);
        return true;
    }

    addProperty(properties, longhand.at(2), important, bottom);
    auto left = consumeLonghand(input, longhand.at(3));
    if(left == nullptr) {
        addProperty(properties, longhand.at(3), important, right);
        return true;
    }

    addProperty(properties, longhand.at(3), important, left);
    return true;
}

bool CSSParser::consumeShorthand(CSSTokenStream& input, CSSPropertyList& properties, CSSPropertyID id, bool important)
{
    RefPtr<CSSValue> values[6] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
    auto longhand = CSSShorthand::longhand(id);
    while(!input.empty()) {
        bool consumed = false;
        for(size_t i = 0; i < longhand.length(); ++i) {
            if(values[i] == nullptr && (values[i] = consumeLonghand(input, longhand.at(i))))
                consumed = true;
        }

        if(!consumed)
            return false;
    }

    for(size_t i = 0; i < longhand.length(); ++i)
        addProperty(properties, longhand.at(i), important, values[i]);
    return true;
}

} // namespace htmlbook
