#ifndef HTMLTOKENIZER_H
#define HTMLTOKENIZER_H

#include "parserstring.h"
#include "document.h"

#include <string>

namespace htmlbook {

class HTMLToken {
public:
    enum class Type {
        Unknown,
        DOCTYPE,
        StartTag,
        EndTag,
        Comment,
        Character,
        SpaceCharacter,
        EndOfFile
    };

    HTMLToken() = default;
    HTMLToken(Type type, const GlobalString& tagName)
        : m_type(type), m_tagName(tagName)
    {}

    Type type() const { return m_type; }
    bool selfClosing() const { return m_selfClosing; }
    bool hasPublicIdentifier() const { return m_hasPublicIdentifier; }
    bool hasSystemIdentifier() const { return m_hasSystemIdentifier; }
    bool forceQuirks() const { return m_forceQuirks; }
    const std::string& publicIdentifier() const { return m_publicIdentifier; }
    const std::string& systemIdentifier() const { return m_systemIdentifier; }
    const std::string& data() const { return m_data; }
    const AttributeList& attributes() const { return m_attributes; }

    const GlobalString& tagName() {
        assert(m_type == Type::StartTag || m_type == Type::EndTag);
        if(!m_tagName.empty())
            return m_tagName;
        m_tagName = GlobalString(m_data);
        return m_tagName;
    }

    Attribute* findAttribute(const GlobalString& name) {
        assert(m_type == Type::StartTag || m_type == Type::EndTag);
        for(auto& attribute : m_attributes) {
            if(name == attribute.name())
                return &attribute;
        }

        return nullptr;
    }

    void adjustTagName(const std::string_view& oldName, const std::string_view& newName) {
        assert(m_type == Type::StartTag || m_type == Type::EndTag);
        if(oldName == m_tagName)
            m_tagName = newName;
    }

    void adjustAttributeName(const std::string_view& oldName, const std::string_view& newName) {
        assert(m_type == Type::StartTag || m_type == Type::EndTag);
        for(auto& attribute : m_attributes) {
            if(oldName == attribute.name())
                attribute.setName(newName);
        }
    }

    void skipLeadingNewLine() {
        assert(m_type == Type::SpaceCharacter);
        if(m_data.front() == '\n')
            m_data.erase(0, 1);
    }

    void beginStartTag() {
        assert(m_type == Type::Unknown);
        m_type = Type::StartTag;
        m_selfClosing = false;
        m_tagName = emptyGlo;
        m_attributes.clear();
        m_data.clear();
    }

    void beginEndTag() {
        assert(m_type == Type::Unknown);
        m_type = Type::EndTag;
        m_selfClosing = false;
        m_tagName = emptyGlo;
        m_attributes.clear();
        m_data.clear();
    }

    void setSelfClosing() {
        assert(m_type == Type::StartTag || m_type == Type::EndTag);
        m_selfClosing = true;
    }

    void addToTagName(char cc) {
        assert(m_type == Type::StartTag || m_type == Type::EndTag);
        m_data += cc;
    }

    void beginAttribute() {
        assert(m_type == Type::StartTag || m_type == Type::EndTag);
        m_attributeName.clear();
        m_attributeValue.clear();
    }

    void addToAttributeName(char cc) {
        assert(m_type == Type::StartTag || m_type == Type::EndTag);
        m_attributeName += cc;
    }

    void addToAttributeValue(char cc) {
        assert(m_type == Type::StartTag || m_type == Type::EndTag);
        m_attributeValue += cc;
    }

    void addToAttributeValue(const std::string& data) {
        assert(m_type == Type::StartTag || m_type == Type::EndTag);
        m_attributeValue += data;
    }

    void endAttribute() {
        assert(m_type == Type::StartTag || m_type == Type::EndTag);
        GlobalString name(m_attributeName);
        m_attributes.emplace_back(name, m_attributeValue);
    }

    void beginComment() {
        assert(m_type == Type::Unknown);
        m_type = Type::Comment;
        m_data.clear();
    }

    void addToComment(char cc) {
        assert(m_type == Type::Comment);
        m_data += cc;
    }

    void beginCharacter() {
        assert(m_type == Type::Unknown);
        m_type = Type::Character;
        m_data.clear();
    }

    void addToCharacter(char cc) {
        assert(m_type == Type::Character);
        m_data += cc;
    }

    void addToCharacter(const std::string& data) {
        assert(m_type == Type::Character);
        m_data += data;
    }

    void beginSpaceCharacter() {
        assert(m_type == Type::Unknown);
        m_type = Type::SpaceCharacter;
        m_data.clear();
    }

    void addToSpaceCharacter(char cc) {
        assert(m_type == Type::SpaceCharacter);
        m_data += cc;
    }

    void beginDOCTYPE() {
        assert(m_type == Type::Unknown);
        m_type = Type::DOCTYPE;
        m_forceQuirks = false;
        m_hasPublicIdentifier = false;
        m_hasSystemIdentifier = false;
        m_publicIdentifier.clear();
        m_systemIdentifier.clear();
        m_data.clear();
    }

    void setForceQuirks() {
        assert(m_type == Type::DOCTYPE);
        m_forceQuirks = true;
    }

    void addToDOCTYPEName(char cc) {
        assert(m_type == Type::DOCTYPE);
        m_data += cc;
    }

    void setPublicIdentifier() {
        assert(m_type == Type::DOCTYPE);
        m_hasPublicIdentifier = true;
        m_publicIdentifier.clear();
    }

    void setSystemIdentifier() {
        assert(m_type == Type::DOCTYPE);
        m_hasSystemIdentifier = true;
        m_systemIdentifier.clear();
    }

    void addToPublicIdentifier(char cc) {
        assert(m_type == Type::DOCTYPE);
        m_publicIdentifier += cc;
    }

    void addToSystemIdentifier(char cc) {
        assert(m_type == Type::DOCTYPE);
        m_systemIdentifier += cc;
    }

    void setEndOfFile() {
        m_type = Type::EndOfFile;
        m_data.clear();
    }

    void reset() {
        m_type = Type::Unknown;
        m_data.clear();
    }

private:
    Type m_type{Type::Unknown};
    bool m_selfClosing{false};
    bool m_hasPublicIdentifier{false};
    bool m_hasSystemIdentifier{false};
    bool m_forceQuirks{false};
    std::string m_publicIdentifier;
    std::string m_systemIdentifier;
    std::string m_attributeName;
    std::string m_attributeValue;
    std::string m_data;
    GlobalString m_tagName;
    AttributeList m_attributes;
};

class HTMLTokenizer {
public:
    enum class State {
        Data,
        CharacterReferenceInData,
        RCDATA,
        CharacterReferenceInRCDATA,
        RAWTEXT,
        ScriptData,
        PLAINTEXT,
        TagOpen,
        EndTagOpen,
        TagName,
        RCDATALessThanSign,
        RCDATAEndTagOpen,
        RCDATAEndTagName,
        RAWTEXTLessThanSign,
        RAWTEXTEndTagOpen,
        RAWTEXTEndTagName,
        ScriptDataLessThanSign,
        ScriptDataEndTagOpen,
        ScriptDataEndTagName,
        ScriptDataEscapeStart,
        ScriptDataEscapeStartDash,
        ScriptDataEscaped,
        ScriptDataEscapedDash,
        ScriptDataEscapedDashDash,
        ScriptDataEscapedLessThanSign,
        ScriptDataEscapedEndTagOpen,
        ScriptDataEscapedEndTagName,
        ScriptDataDoubleEscapeStart,
        ScriptDataDoubleEscaped,
        ScriptDataDoubleEscapedDash,
        ScriptDataDoubleEscapedDashDash,
        ScriptDataDoubleEscapedLessThanSign,
        ScriptDataDoubleEscapeEnd,
        BeforeAttributeName,
        AttributeName,
        AfterAttributeName,
        BeforeAttributeValue,
        AttributeValueDoubleQuoted,
        AttributeValueSingleQuoted,
        AttributeValueUnquoted,
        CharacterReferenceInAttributeValue,
        AfterAttributeValueQuoted,
        SelfClosingStartTag,
        BogusComment,
        MarkupDeclarationOpen,
        CommentStart,
        CommentStartDash,
        Comment,
        CommentEndDash,
        CommentEnd,
        CommentEndBang,
        DOCTYPE,
        BeforeDOCTYPEName,
        DOCTYPEName,
        AfterDOCTYPEName,
        AfterDOCTYPEPublicKeyword,
        BeforeDOCTYPEPublicIdentifier,
        DOCTYPEPublicIdentifierDoubleQuoted,
        DOCTYPEPublicIdentifierSingleQuoted,
        AfterDOCTYPEPublicIdentifier,
        BetweenDOCTYPEPublicAndSystemIdentifiers,
        AfterDOCTYPESystemKeyword,
        BeforeDOCTYPESystemIdentifier,
        DOCTYPESystemIdentifierDoubleQuoted,
        DOCTYPESystemIdentifierSingleQuoted,
        AfterDOCTYPESystemIdentifier,
        BogusDOCTYPE,
        CDATASection,
        CDATASectionRightSquareBracketState, //
        CDATASectionDoubleRightSquareBracketState //
    };

    HTMLTokenizer(const std::string_view& content)
        : m_input(content)
    {}

    HTMLToken& nextToken();

    State state() const { return m_state; }
    void setState(State state) { m_state = state; }
    bool atEOF() const { return m_currentToken.type() == HTMLToken::Type::EndOfFile; }

private:
    bool handleState(char cc);
    bool handleDataState(char cc);
    bool handleCharacterReferenceInDataState(char cc);
    bool handleRCDATAState(char cc);
    bool handleCharacterReferenceInRCDATAState(char cc);
    bool handleRAWTEXTState(char cc);
    bool handleScriptDataState(char cc);
    bool handlePLAINTEXTState(char cc);
    bool handleTagOpenState(char cc);
    bool handleEndTagOpenState(char cc);
    bool handleTagNameState(char cc);
    bool handleRCDATALessThanSignState(char cc);
    bool handleRCDATAEndTagOpenState(char cc);
    bool handleRCDATAEndTagNameState(char cc);
    bool handleRAWTEXTLessThanSignState(char cc);
    bool handleRAWTEXTEndTagOpenState(char cc);
    bool handleRAWTEXTEndTagNameState(char cc);
    bool handleScriptDataLessThanSignState(char cc);
    bool handleScriptDataEndTagOpenState(char cc);
    bool handleScriptDataEndTagNameState(char cc);
    bool handleScriptDataEscapeStartState(char cc);
    bool handleScriptDataEscapeStartDashState(char cc);
    bool handleScriptDataEscapedState(char cc);
    bool handleScriptDataEscapedDashState(char cc);
    bool handleScriptDataEscapedDashDashState(char cc);
    bool handleScriptDataEscapedLessThanSignState(char cc);
    bool handleScriptDataEscapedEndTagOpenState(char cc);
    bool handleScriptDataEscapedEndTagNameState(char cc);
    bool handleScriptDataDoubleEscapeStartState(char cc);
    bool handleScriptDataDoubleEscapedState(char cc);
    bool handleScriptDataDoubleEscapedDashState(char cc);
    bool handleScriptDataDoubleEscapedDashDashState(char cc);
    bool handleScriptDataDoubleEscapedLessThanSignState(char cc);
    bool handleScriptDataDoubleEscapeEndState(char cc);
    bool handleBeforeAttributeNameState(char cc);
    bool handleAttributeNameState(char cc);
    bool handleAfterAttributeNameState(char cc);
    bool handleBeforeAttributeValueState(char cc);
    bool handleAttributeValueDoubleQuotedState(char cc);
    bool handleAttributeValueSingleQuotedState(char cc);
    bool handleAttributeValueUnquotedState(char cc);
    bool handleCharacterReferenceInAttributeValueState(char cc);
    bool handleAfterAttributeValueQuotedState(char cc);
    bool handleSelfClosingStartTagState(char cc);
    bool handleBogusCommentState(char cc);
    bool handleMarkupDeclarationOpenState(char cc);
    bool handleCommentStartState(char cc);
    bool handleCommentStartDashState(char cc);
    bool handleCommentState(char cc);
    bool handleCommentEndDashState(char cc);
    bool handleCommentEndState(char cc);
    bool handleCommentEndBangState(char cc);
    bool handleDOCTYPEState(char cc);
    bool handleBeforeDOCTYPENameState(char cc);
    bool handleDOCTYPENameState(char cc);
    bool handleAfterDOCTYPENameState(char cc);
    bool handleAfterDOCTYPEPublicKeywordState(char cc);
    bool handleBeforeDOCTYPEPublicIdentifierState(char cc);
    bool handleDOCTYPEPublicIdentifierDoubleQuotedState(char cc);
    bool handleDOCTYPEPublicIdentifierSingleQuotedState(char cc);
    bool handleAfterDOCTYPEPublicIdentifierState(char cc);
    bool handleBetweenDOCTYPEPublicAndSystemIdentifiersState(char cc);
    bool handleAfterDOCTYPESystemKeywordState(char cc);
    bool handleBeforeDOCTYPESystemIdentifierState(char cc);
    bool handleDOCTYPESystemIdentifierDoubleQuotedState(char cc);
    bool handleDOCTYPESystemIdentifierSingleQuotedState(char cc);
    bool handleAfterDOCTYPESystemIdentifierState(char cc);
    bool handleBogusDOCTYPEState(char cc);
    bool handleCDATASectionState(char cc);
    bool handleCDATASectionRightSquareBracketState(char cc);
    bool handleCDATASectionDoubleRightSquareBracketState(char cc);

    bool advanceTo(State state);
    bool switchTo(State state);

    bool emitCurrentToken();
    bool emitEOFToken();
    bool emitEndTagToken();

    bool flushCharacterBuffer();
    bool flushEndTagNameBuffer();
    bool flushTemporaryBuffer();

    bool isAppropriateEndTag() const { return m_appropriateEndTagName == m_endTagNameBuffer; }
    bool temporaryBufferIs(const std::string_view& value) const { return m_temporaryBuffer == value; }

    char nextInputCharacter();
    char handleInputCharacter(char inputCharacter);

    bool consumeCharacterReference(std::string& output, bool inAttributeValue);
    bool consumeString(const std::string_view& value, bool caseSensitive);

private:
    ParserString m_input;
    HTMLToken m_currentToken;
    std::string m_entityBuffer;
    std::string m_characterBuffer;
    std::string m_temporaryBuffer;
    std::string m_endTagNameBuffer;
    GlobalString m_appropriateEndTagName;
    State m_state{State::Data};
    bool m_reconsumeCurrentCharacter{true};
    char m_additionalAllowedCharacter{0};
};

inline bool HTMLTokenizer::advanceTo(State state)
{
    m_state = state;
    m_reconsumeCurrentCharacter = false;
    return true;
}

inline bool HTMLTokenizer::switchTo(State state)
{
    m_state = state;
    m_reconsumeCurrentCharacter = true;
    return true;
}

inline char HTMLTokenizer::nextInputCharacter()
{
    if(m_reconsumeCurrentCharacter)
        return handleInputCharacter(m_input.get());
    return handleInputCharacter(m_input.advance());
}

inline char HTMLTokenizer::handleInputCharacter(char inputCharacter)
{
    if(inputCharacter != '\r')
        return inputCharacter;
    if(m_input.peek(1) == '\n')
        m_input.advance();
    return '\n';
}

} // namespace htmlbook

#endif // HTMLTOKENIZER_H
