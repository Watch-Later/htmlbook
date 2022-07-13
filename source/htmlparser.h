#ifndef HTMLPARSER_H
#define HTMLPARSER_H

#include "htmltokenizer.h"

namespace htmlbook {

class HTMLElementStack {
public:
    HTMLElementStack() = default;

    void push(Element* element);
    void pushHTMLHtmlElement(Element* element);
    void pushHTMLHeadElement(Element* element);
    void pushHTMLBodyElement(Element* element);

    void pop();
    void popHTMLHeadElement();
    void popHTMLBodyElement();
    void popUntil(const GlobalString& tagName);
    void popUntil(Element* element);
    void popUntilNumberedHeaderElement();
    void popUntilTableScopeMarker();
    void popUntilTableBodyScopeMarker();
    void popUntilTableRowScopeMarker();
    void popUntilForeignContentScopeMarker();
    void popUntilPopped(const GlobalString& tagName);
    void popUntilPopped(Element* element);
    void popUntilNumberedHeaderElementPopped();
    void popAll();

    void generateImpliedEndTags();
    void generateImpliedEndTagsExcept(const GlobalString& tagName);

    void remove(Element* element);
    void removeHTMLHeadElement(Element* element);
    void removeHTMLBodyElement();

    Element* find(const Element* element) const;
    Element* furthestBlockForFormattingElement(const Element* formattingElement) const;
    Element* topmost(const GlobalString& tagName) const;
    Element* previous(const Element* element) const;

    Element* htmlElement() const { return m_htmlElement; }
    Element* headElement() const { return m_headElement; }
    Element* bodyElement() const { return m_bodyElement; }

    bool inScope(const Element* element) const;
    bool inScope(const GlobalString& tagName) const;
    bool inButtonScope(const GlobalString& tagName) const;
    bool inListItemScope(const GlobalString& tagName) const;
    bool inTableScope(const GlobalString& tagName) const;
    bool inSelectScope(const GlobalString& tagName) const;
    bool isNumberedHeaderElementInScope() const;

    void replace(const Element* element, Element* item);
    void replace(int index, Element* item);
    void insertAfter(const Element* element, Element* item);
    void insert(int index, Element* item);

    int index(const Element* element) const;
    bool contains(const Element* element) const;
    Element* at(int index) const { return m_elements.at(index); }
    Element* top() const { return m_elements.back(); }

    bool empty() const { return m_elements.empty(); }
    int size() const { return m_elements.size(); }

private:
    std::vector<Element*> m_elements;

    Element* m_htmlElement{nullptr};
    Element* m_headElement{nullptr};
    Element* m_bodyElement{nullptr};
};

class HTMLFormattingElementList {
public:
    HTMLFormattingElementList() = default;

    void append(Element* element);
    void appendMarker();
    void clearToLastMarker();

    void remove(Element* element);
    void replace(const Element* element, Element* item);
    void replace(int index, Element* item);
    void insert(int index, Element* element);

    int index(const Element* element) const;
    bool contains(const Element* element) const;
    Element* at(int index) const { return m_elements.at(index); }
    Element* closestElementInScope(const GlobalString& tagName);

    bool empty() const { return m_elements.empty(); }
    int size() const { return m_elements.size(); }

private:
    std::vector<Element*> m_elements;
};

class HTMLParser {
public:
    HTMLParser(Document* document, const std::string_view& content)
        : m_document(document), m_tokenizer(content)
    {}

    void parse();

private:
    Element* createHTMLElement(HTMLToken& token) const;
    Element* createElement(HTMLToken& token, const GlobalString& namespaceUri) const;
    Element* cloneElement(const Element* element) const;
    Element* currentElement() const { return m_openElements.top(); }

    struct InsertionLocation {
        ContainerNode* parent{nullptr};
        Node* child{nullptr};
        Node* nextChild{nullptr};
        bool selfClosing{false};
    };

    static void insert(const InsertionLocation& location);
    void append(ContainerNode* parent, Node* child, bool selfClosing);
    void append(Node* child, bool selfClosing);

    bool shouldFosterParent() const;
    void fosterParent(Node* child);
    void findFosterLocation(InsertionLocation& location);

    void adoptionAgencyAlgorithm(HTMLToken& token);
    void reconstructActiveFormattingElements();
    void closeCell();

    static void adjustSVGTagNames(HTMLToken& token);
    static void adjustSVGAttributes(HTMLToken& token);
    static void adjustMathMLAttributes(HTMLToken& token);

    void insertDoctype(HTMLToken& token);
    void insertComment(HTMLToken& token, ContainerNode* parent);
    void insertHTMLHtmlElement(HTMLToken& token);
    void insertHeadElement(HTMLToken& token);
    void insertHTMLBodyElement(HTMLToken& token);
    void insertHTMLFormElement(HTMLToken& token);
    void insertSelfClosingHTMLElement(HTMLToken& token);
    void insertHTMLElement(HTMLToken& token);
    void insertHTMLFormattingElement(HTMLToken& token);
    void insertForeignElement(HTMLToken& token, const GlobalString& namespaceUri);
    void insertTextNode(const std::string& data);

    enum class InsertionMode {
        Initial,
        BeforeHTML,
        BeforeHead,
        InHead,
        InHeadNoscript,
        AfterHead,
        InBody,
        Text,
        InTable,
        InTableText,
        InCaption,
        InColumnGroup,
        InTableBody,
        InRow,
        InCell,
        InSelect,
        InSelectInTable,
        inForeignContent,
        AfterBody,
        InFrameset,
        AfterFrameset,
        AfterAfterBody,
        AfterAfterFrameset
    };

    void resetInsertionMode();
    void setInsertionMode(InsertionMode mode) { m_insertionMode = mode; }
    InsertionMode insertionMode() const { return m_insertionMode; }
    InsertionMode currentInsertionMode(HTMLToken& token) const;

    void handleInitialMode(HTMLToken& token);
    void handleBeforeHTMLMode(HTMLToken& token);
    void handleBeforeHeadMode(HTMLToken& token);
    void handleInHeadMode(HTMLToken& token);
    void handleInHeadNoscriptMode(HTMLToken& token);
    void handleAfterHeadMode(HTMLToken& token);
    void handleInBodyMode(HTMLToken& token);
    void handleTextMode(HTMLToken& token);
    void handleInTableMode(HTMLToken& token);
    void handleInTableTextMode(HTMLToken& token);
    void handleInCaptionMode(HTMLToken& token);
    void handleInColumnGroupMode(HTMLToken& token);
    void handleInTableBodyMode(HTMLToken& token);
    void handleInRowMode(HTMLToken& token);
    void handleInCellMode(HTMLToken& token);
    void handleInSelectMode(HTMLToken& token);
    void handleInSelectInTableMode(HTMLToken& token);
    void handleinForeignContentMode(HTMLToken& token);
    void handleAfterBodyMode(HTMLToken& token);
    void handleInFramesetMode(HTMLToken& token);
    void handleAfterFramesetMode(HTMLToken& token);
    void handleAfterAfterBodyMode(HTMLToken& token);
    void handleAfterAfterFramesetMode(HTMLToken& token);

    void handleFakeStartTagToken(const GlobalString& tagName);
    void handleFakeEndTagToken(const GlobalString& tagName);

    void defaultForInBodyEndTagToken(HTMLToken& token);
    void flushPendingTableCharacters();

    void handleErrorToken(HTMLToken& token);
    void handleRCDataToken(HTMLToken& token);
    void handleRawTextToken(HTMLToken& token);
    void handleScriptDataToken(HTMLToken& token);

    void handleDoctypeToken(HTMLToken& token);
    void handleCommentToken(HTMLToken& token);

    void handleToken(HTMLToken& token, InsertionMode mode);
    void buildTree(HTMLToken& token);
    void finishTree();

private:
    Document* m_document;
    Element* m_form{nullptr};
    Element* m_head{nullptr};

    HTMLTokenizer m_tokenizer;
    HTMLElementStack m_openElements;
    HTMLFormattingElementList m_activeFormattingElements;
    std::string m_pendingTableCharacters;

    InsertionMode m_insertionMode{InsertionMode::Initial};
    InsertionMode m_originalInsertionMode{InsertionMode::Initial};
    bool m_inQuirksMode{false};
    bool m_framesetOk{false};
    bool m_fosterParenting{false};
    bool m_skipLeadingNewline{false};
};

} // namespace htmlbook

#endif // HTMLPARSER_H
