#include "htmlparser.h"
#include "htmldocument.h"

#include <algorithm>

namespace htmlbook {

inline bool isNumberedHeaderTag(const GlobalString& tagName)
{
    return tagName == h1Tag
        || tagName == h2Tag
        || tagName == h3Tag
        || tagName == h4Tag
        || tagName == h5Tag
        || tagName == h6Tag;
}

inline bool isFormattingTag(const GlobalString& tagName)
{
    return tagName == aTag
        || tagName == bTag
        || tagName == bigTag
        || tagName == codeTag
        || tagName == emTag
        || tagName == fontTag
        || tagName == iTag
        || tagName == nobrTag
        || tagName == sTag
        || tagName == smallTag
        || tagName == strikeTag
        || tagName == strongTag
        || tagName == ttTag
        || tagName == uTag;
}

inline bool isImpliedEndTag(const GlobalString& tagName)
{
    return tagName == ddTag
        || tagName == dtTag
        || tagName == liTag
        || tagName == optionTag
        || tagName == optgroupTag
        || tagName == pTag
        || tagName == rpTag
        || tagName == rtTag;
}

inline bool isFosterParentingTag(const GlobalString& tagName)
{
    return tagName == tableTag
        || tagName == tbodyTag
        || tagName == theadTag
        || tagName == trTag;
}

inline bool isNumberedHeaderElement(const Element* element)
{
    return isNumberedHeaderTag(element->tagName());
}

inline bool isSpecialElement(const Element* element)
{
    auto tagName = element->tagName();
    if(element->namespaceUri() == namespaceuri::svg) {
        return tagName == foreignObjectTag
            || tagName == descTag
            || tagName == titleTag;
    }

    if(element->namespaceUri() == namespaceuri::mathml) {
        return tagName == miTag
            || tagName == moTag
            || tagName == mnTag
            || tagName == msTag
            || tagName == mtextTag
            || tagName == annotation_xmlTag;
    }

    return tagName == addressTag
        || tagName == areaTag
        || tagName == articleTag
        || tagName == asideTag
        || tagName == baseTag
        || tagName == basefontTag
        || tagName == bgsoundTag
        || tagName == blockquoteTag
        || tagName == bodyTag
        || tagName == brTag
        || tagName == buttonTag
        || tagName == captionTag
        || tagName == centerTag
        || tagName == colTag
        || tagName == colgroupTag
        || tagName == commandTag
        || tagName == ddTag
        || tagName == detailsTag
        || tagName == dirTag
        || tagName == divTag
        || tagName == dlTag
        || tagName == dtTag
        || tagName == embedTag
        || tagName == fieldsetTag
        || tagName == figcaptionTag
        || tagName == figureTag
        || tagName == footerTag
        || tagName == formTag
        || tagName == frameTag
        || tagName == framesetTag
        || isNumberedHeaderTag(tagName)
        || tagName == headTag
        || tagName == headerTag
        || tagName == hgroupTag
        || tagName == hrTag
        || tagName == htmlTag
        || tagName == iframeTag
        || tagName == imgTag
        || tagName == inputTag
        || tagName == liTag
        || tagName == linkTag
        || tagName == listingTag
        || tagName == mainTag
        || tagName == marqueeTag
        || tagName == menuTag
        || tagName == metaTag
        || tagName == navTag
        || tagName == noembedTag
        || tagName == noframesTag
        || tagName == noscriptTag
        || tagName == objectTag
        || tagName == olTag
        || tagName == pTag
        || tagName == paramTag
        || tagName == plaintextTag
        || tagName == preTag
        || tagName == scriptTag
        || tagName == sectionTag
        || tagName == selectTag
        || tagName == styleTag
        || tagName == summaryTag
        || tagName == tableTag
        || tagName == tbodyTag
        || tagName == tfootTag
        || tagName == theadTag
        || tagName == tdTag
        || tagName == textareaTag
        || tagName == thTag
        || tagName == titleTag
        || tagName == trTag
        || tagName == ulTag
        || tagName == wbrTag
        || tagName == xmpTag;
}

inline bool isHTMLIntegrationPoint(const Element* element)
{
    if(element->namespaceUri() == namespaceuri::mathml
        && element->tagName() == annotation_xmlTag) {
        auto attribute = element->findAttribute(encodingAttr);
        if(attribute == nullptr)
            return false;
        auto& encoding = attribute->value();
        return equals(encoding, "text/html", false)
            || equals(encoding, "application/xhtml+xml", false);
    }

    if(element->namespaceUri() == namespaceuri::svg) {
        return element->tagName() == foreignObjectTag
            || element->tagName() == descTag
            || element->tagName() == titleTag;
    }

    return false;
}

inline bool isMathMLTextIntegrationPoint(const Element* element)
{
    if(element->namespaceUri() == namespaceuri::mathml) {
        return element->tagName() == miTag
            || element->tagName() == moTag
            || element->tagName() == mnTag
            || element->tagName() == msTag
            || element->tagName() == mtextTag;
    }

    return false;
}

inline bool isScopeMarker(const Element* element)
{
    auto tagName = element->tagName();
    if(element->namespaceUri() == namespaceuri::svg) {
        return tagName == foreignObjectTag
            || tagName == descTag
            || tagName == titleTag;
    }

    if(element->namespaceUri() == namespaceuri::mathml) {
        return tagName == miTag
            || tagName == moTag
            || tagName == mnTag
            || tagName == msTag
            || tagName == mtextTag
            || tagName == annotation_xmlTag;
    }

    return tagName == captionTag
        || tagName == marqueeTag
        || tagName == objectTag
        || tagName == tableTag
        || tagName == tdTag
        || tagName == thTag
        || tagName == htmlTag;
}

inline bool isListItemScopeMarker(const Element* element)
{
    return isScopeMarker(element)
        || element->tagName() == olTag
        || element->tagName() == ulTag;
}

inline bool isTableScopeMarker(const Element* element)
{
    return element->tagName() == tableTag
        || element->tagName() == htmlTag;
}

inline bool isTableBodyScopeMarker(const Element* element)
{
    return element->tagName() == tbodyTag
        || element->tagName() == tfootTag
        || element->tagName() == theadTag
        || element->tagName() == htmlTag;
}

inline bool isTableRowScopeMarker(const Element* element)
{
    return element->tagName() == trTag
        || element->tagName() == htmlTag;
}

inline bool isForeignContentScopeMarker(const Element* element)
{
    return isMathMLTextIntegrationPoint(element)
        || isHTMLIntegrationPoint(element)
        || element->namespaceUri() == namespaceuri::xhtml;
}

inline bool isButtonScopeMarker(const Element* element)
{
    return isScopeMarker(element)
        || element->tagName() == buttonTag;
}

inline bool isSelectScopeMarker(const Element* element)
{
    return element->tagName() != optgroupTag
        && element->tagName() !=optionTag;
}

void HTMLElementStack::push(Element* element)
{
    assert(element->tagName() != htmlTag);
    assert(element->tagName() != headTag);
    assert(element->tagName() != bodyTag);
    m_elements.push_back(element);
}

void HTMLElementStack::pushHTMLHtmlElement(Element* element)
{
    assert(element->tagName() == htmlTag);
    assert(m_htmlElement == nullptr);
    assert(m_elements.empty());
    m_htmlElement = element;
    m_elements.push_back(element);
}

void HTMLElementStack::pushHTMLHeadElement(Element* element)
{
    assert(element->tagName() == headTag);
    assert(m_headElement == nullptr);
    m_headElement = element;
    m_elements.push_back(element);
}

void HTMLElementStack::pushHTMLBodyElement(Element* element)
{
    assert(element->tagName() == bodyTag);
    assert(m_bodyElement == nullptr);
    m_bodyElement = element;
    m_elements.push_back(element);
}

void HTMLElementStack::pop()
{
    auto element = m_elements.back();
    assert(element->tagName() != htmlTag);
    assert(element->tagName() != headTag);
    assert(element->tagName() != bodyTag);
    element->finishParsingChildren();
    m_elements.pop_back();
}

void HTMLElementStack::popHTMLHeadElement()
{
    auto element = m_elements.back();
    assert(element == m_headElement);
    element->finishParsingChildren();
    m_headElement = nullptr;
    m_elements.pop_back();
}

void HTMLElementStack::popHTMLBodyElement()
{
    auto element = m_elements.back();
    assert(element == m_bodyElement);
    element->finishParsingChildren();
    m_bodyElement = nullptr;
    m_elements.pop_back();
}

void HTMLElementStack::popUntil(const GlobalString& tagName)
{
    while(tagName != top()->tagName())
        pop();
}

void HTMLElementStack::popUntil(Element* element)
{
    while(element != top())
        pop();
}

void HTMLElementStack::popUntilNumberedHeaderElement()
{
    while(!isNumberedHeaderElement(top()))
        pop();
}

void HTMLElementStack::popUntilTableScopeMarker()
{
    while(!isTableScopeMarker(top()))
        pop();
}

void HTMLElementStack::popUntilTableBodyScopeMarker()
{
    while(!isTableBodyScopeMarker(top()))
        pop();
}

void HTMLElementStack::popUntilTableRowScopeMarker()
{
    while(!isTableRowScopeMarker(top()))
        pop();
}

void HTMLElementStack::popUntilForeignContentScopeMarker()
{
    while(!isForeignContentScopeMarker(top()))
        pop();
}

void HTMLElementStack::popUntilPopped(const GlobalString& tagName)
{
    popUntil(tagName);
    pop();
}

void HTMLElementStack::popUntilPopped(Element* element)
{
    popUntil(element);
    pop();
}

void HTMLElementStack::popUntilNumberedHeaderElementPopped()
{
    popUntilNumberedHeaderElement();
    pop();
}

void HTMLElementStack::popAll()
{
    m_htmlElement = nullptr;
    m_headElement = nullptr;
    m_bodyElement = nullptr;
    while(!m_elements.empty()) {
        auto element = m_elements.back();
        element->finishParsingChildren();
        m_elements.pop_back();
    }
}

void HTMLElementStack::generateImpliedEndTags()
{
    while(isImpliedEndTag(top()->tagName()))
        pop();
}

void HTMLElementStack::generateImpliedEndTagsExcept(const GlobalString& tagName)
{
    while(top()->tagName() != tagName && isImpliedEndTag(top()->tagName()))
        pop();
}

void HTMLElementStack::remove(Element* element)
{
    if(element == top())
        return pop();
    assert(element->tagName() != htmlTag);
    assert(element->tagName() != headTag);
    assert(element->tagName() != bodyTag);
    auto it = std::find(m_elements.begin(), m_elements.end(), element);
    assert(it != m_elements.end());
    element->finishParsingChildren();
    m_elements.erase(it);
}

void HTMLElementStack::removeHTMLHeadElement(Element* element)
{
    if(element == top())
        return popHTMLHeadElement();
    assert(m_headElement == element);
    auto it = std::find(m_elements.begin(), m_elements.end(), element);
    assert(it != m_elements.end());
    element->finishParsingChildren();
    m_headElement = nullptr;
    m_elements.erase(it);
}

void HTMLElementStack::removeHTMLBodyElement()
{
    assert(m_htmlElement != nullptr);
    assert(m_bodyElement != nullptr);
    auto element = m_bodyElement;
    m_htmlElement->removeChild(m_bodyElement);
    popUntil(m_bodyElement);
    popHTMLBodyElement();
    assert(m_htmlElement == top());
    delete element;
}

Element* HTMLElementStack::find(const Element* element) const
{
    auto it = std::find(m_elements.begin(), m_elements.end(), element);
    if(it == m_elements.end())
        return nullptr;
    return *it;
}

Element* HTMLElementStack::furthestBlockForFormattingElement(const Element* formattingElement) const
{
    Element* furthestBlock = nullptr;
    auto it = m_elements.rbegin();
    auto end = m_elements.rend();
    for(; it != end; ++it) {
        if(formattingElement == *it)
            return furthestBlock;
        if(!isSpecialElement(*it))
            continue;
        furthestBlock = *it;
    }

    assert(false);
}

Element* HTMLElementStack::topmost(const GlobalString& tagName) const
{
    auto it = m_elements.rbegin();
    auto end = m_elements.rend();
    for(; it != end; ++it) {
        auto element = *it;
        if(tagName == element->tagName()) {
            return element;
        }
    }

    return nullptr;
}

Element* HTMLElementStack::previous(const Element* element) const
{
    Element* lastElement = nullptr;
    auto it = m_elements.begin();
    auto end = m_elements.end();
    for(; it != end; ++it) {
        if(element == *it)
            return lastElement;
        lastElement = *it;
    }

    assert(false);
}

template<bool isMarker(const Element*)>
bool HTMLElementStack::inScopeTemplate(const GlobalString& tagName) const
{
    auto it = m_elements.rbegin();
    auto end = m_elements.rend();
    for(; it != end; ++it) {
        auto element = *it;
        if(element->tagName() == tagName)
            return true;
        if(isMarker(element))
            return false;
    }

    assert(false);
}

bool HTMLElementStack::inScope(const Element* element) const
{
    auto it = m_elements.rbegin();
    auto end = m_elements.rend();
    for(; it != end; ++it) {
        if(element == *it)
            return true;
        if(isScopeMarker(*it))
            return false;
    }

    assert(false);
}

bool HTMLElementStack::inScope(const GlobalString& tagName) const
{
    return inScopeTemplate<isScopeMarker>(tagName);
}

bool HTMLElementStack::inButtonScope(const GlobalString& tagName) const
{
    return inScopeTemplate<isButtonScopeMarker>(tagName);
}

bool HTMLElementStack::inListItemScope(const GlobalString& tagName) const
{
    return inScopeTemplate<isListItemScopeMarker>(tagName);
}

bool HTMLElementStack::inTableScope(const GlobalString& tagName) const
{
    return inScopeTemplate<isTableScopeMarker>(tagName);
}

bool HTMLElementStack::inSelectScope(const GlobalString& tagName) const
{
    return inScopeTemplate<isSelectScopeMarker>(tagName);
}

bool HTMLElementStack::isNumberedHeaderElementInScope() const
{
    auto it = m_elements.rbegin();
    auto end = m_elements.rend();
    for(; it != end; ++it) {
        if(isNumberedHeaderElement(*it))
            return true;
        if(isScopeMarker(*it))
            return false;
    }

    assert(false);
}

void HTMLElementStack::replace(const Element* element, Element* item)
{
    auto it = m_elements.rbegin();
    auto end = m_elements.rend();
    for(; it != end; ++it) {
        if(element == *it) {
            *it = item;
            return;
        }
    }

    assert(false);
}

void HTMLElementStack::replace(int index, Element* item)
{
    assert(index < m_elements.size());
    m_elements[index] = item;
}

void HTMLElementStack::insertAfter(const Element* element, Element* item)
{
    auto it = std::find(m_elements.begin(), m_elements.end(), element);
    assert(it != m_elements.end());
    m_elements.insert(std::next(it), item);
}

void HTMLElementStack::insert(int index, Element* item)
{
    auto it = std::next(m_elements.begin(), index);
    assert(it != m_elements.end());
    m_elements.insert(it, item);
}

int HTMLElementStack::index(const Element* element) const
{
    auto it = std::find(m_elements.begin(), m_elements.end(), element);
    assert(it != m_elements.end());
    return it - m_elements.begin();
}

bool HTMLElementStack::contains(const Element* element) const
{
    auto it = std::find(m_elements.rbegin(), m_elements.rend(), element);
    return it != m_elements.rend();
}

void HTMLFormattingElementList::append(Element* element)
{
    assert(element != nullptr);
    auto it = m_elements.rbegin();
    auto end = m_elements.rend();
    int count = 0;
    for(; it != end; ++it) {
        auto item = *it;
        if(item == nullptr)
            break;
        if(element->tagName() == item->tagName()
            && element->namespaceUri() == item->namespaceUri()
            && element->attributes() == item->attributes()) {
            count += 1;
        }

        if(count == 3) {
            remove(*it);
            break;
        }
    }

    m_elements.push_back(element);
}

void HTMLFormattingElementList::appendMarker()
{
    m_elements.push_back(nullptr);
}

void HTMLFormattingElementList::clearToLastMarker()
{
    while(!m_elements.empty()) {
        auto element = m_elements.back();
        m_elements.pop_back();
        if(element == nullptr)
            break;
    }
}

void HTMLFormattingElementList::remove(Element* element)
{
    auto it = std::find(m_elements.begin(), m_elements.end(), element);
    if(it == m_elements.end())
        return;
    m_elements.erase(it);
}

void HTMLFormattingElementList::replace(const Element* element, Element* item)
{
    auto it = m_elements.rbegin();
    auto end = m_elements.rend();
    for(; it != end; ++it) {
        if(element == *it) {
            *it = item;
            return;
        }
    }

    assert(false);
}

void HTMLFormattingElementList::replace(int index, Element* item)
{
    assert(index < m_elements.size());
    m_elements[index] = item;
}

void HTMLFormattingElementList::insert(int index, Element* element)
{
    m_elements.insert(m_elements.begin() + index, element);
}

int HTMLFormattingElementList::index(const Element* element) const
{
    auto it = std::find(m_elements.begin(), m_elements.end(), element);
    assert(it != m_elements.end());
    return it - m_elements.begin();
}

bool HTMLFormattingElementList::contains(const Element* element) const
{
    auto it = std::find(m_elements.begin(), m_elements.end(), element);
    return it != m_elements.end();
}

Element* HTMLFormattingElementList::closestElementInScope(const GlobalString& tagName)
{
    auto it = m_elements.rbegin();
    auto end = m_elements.rend();
    for(; it != end; ++it) {
        auto element = *it;
        if(element == nullptr)
            break;
        if(element->tagName() == tagName)
            return element;
    }

    return nullptr;
}

Element* HTMLParser::createHTMLElement(HTMLTokenView& token) const
{
    return createElement(token, namespaceuri::xhtml);
}

Element* HTMLParser::createElement(HTMLTokenView& token, const GlobalString& namespaceUri) const
{
    auto element = m_document->createElement(token.tagName(), namespaceUri);
    for(auto attribute : token.attributes())
        element->setAttribute(attribute);
    return element;
}

Element* HTMLParser::cloneElement(const Element* element) const
{
    auto newElement = m_document->createElement(element->tagName(), element->namespaceUri());
    newElement->setAttributeList(element->attributes());
    return newElement;
}

void HTMLParser::insert(const InsertionLocation& location)
{
    if(location.nextChild == nullptr)
        location.parent->appendChild(location.child);
    else
        location.parent->insertChild(location.child, location.nextChild);

    if(!location.child->isContainerNode())
        return;

    auto child = to<ContainerNode>(location.child);
    child->beginParsingChildren();
    if(location.selfClosing)
        child->finishParsingChildren();
}

void HTMLParser::append(ContainerNode* parent, Node* child, bool selfClosing)
{
    if(shouldFosterParent()) {
        fosterParent(child);
        return;
    }

    InsertionLocation location;
    location.parent = parent;
    location.child = child;
    location.selfClosing = selfClosing;
    insert(location);
}

void HTMLParser::append(Node* child, bool selfClosing)
{
    append(currentElement(), child, selfClosing);
}

bool HTMLParser::shouldFosterParent() const
{
    return m_fosterParenting && isFosterParentingTag(currentElement()->tagName());
}

void HTMLParser::findFosterLocation(InsertionLocation& location) const
{
    auto lastTable = m_openElements.topmost(tableTag);
    assert(lastTable != nullptr);
    auto parent = lastTable->parentNode();
    if(parent == nullptr) {
        location.parent = m_openElements.previous(lastTable);
        return;
    }

    location.parent = parent;
    location.nextChild = lastTable;
}

void HTMLParser::fosterParent(Node* child)
{
    InsertionLocation location;
    location.child = child;
    findFosterLocation(location);
    insert(location);
}

void HTMLParser::adoptionAgencyAlgorithm(HTMLTokenView& token)
{
    static const int outerIterationLimit = 8;
    static const int innerIterationLimit = 3;
    for(int i = 0; i < outerIterationLimit; ++i) {
        auto formattingElement = m_activeFormattingElements.closestElementInScope(token.tagName());
        if(formattingElement == nullptr) {
            defaultForInBodyEndTagToken(token);
            return;
        }

        if(!m_openElements.contains(formattingElement)) {
            handleErrorToken(token);
            m_activeFormattingElements.remove(formattingElement);
            return;
        }

        if(!m_openElements.inScope(formattingElement)) {
            handleErrorToken(token);
            return;
        }

        if(formattingElement != m_openElements.top())
            handleErrorToken(token);

        auto furthestBlock = m_openElements.furthestBlockForFormattingElement(formattingElement);
        if(furthestBlock == nullptr) {
            m_openElements.popUntilPopped(formattingElement);
            m_activeFormattingElements.remove(formattingElement);
            return;
        }

        auto commonAncestor = m_openElements.previous(formattingElement);
        auto bookmark = m_activeFormattingElements.index(formattingElement);

        auto furthestBlockIndex = m_openElements.index(furthestBlock);
        auto lastNode = furthestBlock;
        for(int i = 0; i < innerIterationLimit; ++i) {
            furthestBlockIndex -= 1;
            auto node = m_openElements.at(furthestBlockIndex);
            if(!m_activeFormattingElements.contains(node)) {
                m_openElements.remove(node);
                continue;
            }

            if(node == formattingElement)
                break;

            if(lastNode == furthestBlock) {
                bookmark = m_activeFormattingElements.index(node);
                bookmark += 1;
            }

            auto newNode = cloneElement(node);
            m_activeFormattingElements.replace(node, newNode);
            m_openElements.replace(furthestBlockIndex, newNode);

            lastNode->reparent(newNode);
            lastNode = newNode;
        }

        if(lastNode->parentNode())
            lastNode->parentNode()->removeChild(lastNode);

        if(isFosterParentingTag(commonAncestor->tagName())) {
            fosterParent(lastNode);
        } else {
            commonAncestor->appendChild(lastNode);
        }

        auto newNode = cloneElement(formattingElement);
        furthestBlock->reparentChildren(newNode);
        furthestBlock->appendChild(newNode);

        m_activeFormattingElements.remove(formattingElement);
        m_activeFormattingElements.insert(bookmark, newNode);

        m_openElements.remove(formattingElement);
        m_openElements.insertAfter(furthestBlock, newNode);
    }
}

void HTMLParser::reconstructActiveFormattingElements()
{
    if(m_activeFormattingElements.empty())
        return;

    auto index = m_activeFormattingElements.size();
    do {
        index -= 1;
        auto element = m_activeFormattingElements.at(index);
        if(element == nullptr || m_openElements.contains(element)) {
            index += 1;
            break;
        }
    } while(index > 0);

    for(; index < m_activeFormattingElements.size(); ++index) {
        auto element = m_activeFormattingElements.at(index);
        auto newElement = cloneElement(element);
        append(newElement, false);
        m_openElements.push(newElement);
        m_activeFormattingElements.replace(index, newElement);
    }
}

void HTMLParser::closeCell()
{
    assert(m_insertionMode == InsertionMode::InCell);
    if(m_openElements.inTableScope(tdTag)) {
        assert(!m_openElements.inTableScope(thTag));
        handleFakeEndTagToken(tdTag);
        return;
    }

    assert(m_openElements.inTableScope(thTag));
    handleFakeEndTagToken(thTag);
    assert(m_insertionMode == InsertionMode::InRow);
}

void HTMLParser::adjustSVGTagNames(HTMLTokenView& token)
{
    static const GlobalString altglyph("altglyph");
    static const GlobalString altglyphdef("altglyphdef");
    static const GlobalString altglyphitem("altglyphitem");
    static const GlobalString animatecolor("animatecolor");
    static const GlobalString animatemotion("animatemotion");
    static const GlobalString animatetransform("animatetransform");
    static const GlobalString clippath("clippath");
    static const GlobalString feblend("feblend");
    static const GlobalString fecolormatrix("fecolormatrix");
    static const GlobalString fecomponenttransfer("fecomponenttransfer");
    static const GlobalString fecomposite("fecomposite");
    static const GlobalString feconvolvematrix("feconvolvematrix");
    static const GlobalString fediffuselighting("fediffuselighting");
    static const GlobalString fedisplacementmap("fedisplacementmap");
    static const GlobalString fedistantlight("fedistantlight");
    static const GlobalString fedropshadow("fedropshadow");
    static const GlobalString feflood("feflood");
    static const GlobalString fefunca("fefunca");
    static const GlobalString fefuncb("fefuncb");
    static const GlobalString fefuncg("fefuncg");
    static const GlobalString fefuncr("fefuncr");
    static const GlobalString fegaussianblur("fegaussianblur");
    static const GlobalString feimage("feimage");
    static const GlobalString femerge("femerge");
    static const GlobalString femergenode("femergenode");
    static const GlobalString femorphology("femorphology");
    static const GlobalString feoffset("feoffset");
    static const GlobalString fepointlight("fepointlight");
    static const GlobalString fespecularlighting("fespecularlighting");
    static const GlobalString fespotlight("fespotlight");
    static const GlobalString glyphref("glyphref");
    static const GlobalString lineargradient("lineargradient");
    static const GlobalString radialgradient("radialgradient");
    static const GlobalString textpath("textpath");

    token.adjustTagName(altglyph, altGlyphTag);
    token.adjustTagName(altglyphdef, altGlyphDefTag);
    token.adjustTagName(altglyphitem, altGlyphItemTag);
    token.adjustTagName(animatecolor, animateColorTag);
    token.adjustTagName(animatemotion, animateMotionTag);
    token.adjustTagName(animatetransform, animateTransformTag);
    token.adjustTagName(clippath, clipPathTag);
    token.adjustTagName(feblend, feBlendTag);
    token.adjustTagName(fecolormatrix, feColorMatrixTag);
    token.adjustTagName(fecomponenttransfer, feComponentTransferTag);
    token.adjustTagName(fecomposite, feCompositeTag);
    token.adjustTagName(feconvolvematrix, feConvolveMatrixTag);
    token.adjustTagName(fediffuselighting, feDiffuseLightingTag);
    token.adjustTagName(fedisplacementmap, feDisplacementMapTag);
    token.adjustTagName(fedistantlight, feDistantLightTag);
    token.adjustTagName(fedropshadow, feDropShadowTag);
    token.adjustTagName(feflood, feFloodTag);
    token.adjustTagName(fefunca, feFuncATag);
    token.adjustTagName(fefuncb, feFuncBTag);
    token.adjustTagName(fefuncg, feFuncGTag);
    token.adjustTagName(fefuncr, feFuncRTag);
    token.adjustTagName(fegaussianblur, feGaussianBlurTag);
    token.adjustTagName(feimage, feImageTag);
    token.adjustTagName(femerge, feMergeTag);
    token.adjustTagName(femergenode, feMergeNodeTag);
    token.adjustTagName(femorphology, feMorphologyTag);
    token.adjustTagName(feoffset, feOffsetTag);
    token.adjustTagName(fepointlight, fePointLightTag);
    token.adjustTagName(fespecularlighting, feSpecularLightingTag);
    token.adjustTagName(fespotlight, feSpotLightTag);
    token.adjustTagName(glyphref, glyphRefTag);
    token.adjustTagName(lineargradient, linearGradientTag);
    token.adjustTagName(radialgradient, radialGradientTag);
    token.adjustTagName(textpath, textPathTag);
}

void HTMLParser::adjustSVGAttributes(HTMLTokenView& token)
{
    static const GlobalString attributename("attributename");
    static const GlobalString attributetype("attributetype");
    static const GlobalString basefrequency("basefrequency");
    static const GlobalString baseprofile("baseprofile");
    static const GlobalString calcmode("calcmode");
    static const GlobalString clippathunits("clippathunits");
    static const GlobalString diffuseconstant("diffuseconstant");
    static const GlobalString edgemode("edgemode");
    static const GlobalString filterunits("filterunits");
    static const GlobalString glyphref("glyphref");
    static const GlobalString gradienttransform("gradienttransform");
    static const GlobalString gradientunits("gradientunits");
    static const GlobalString kernelmatrix("kernelmatrix");
    static const GlobalString kernelunitlength("kernelunitlength");
    static const GlobalString keypoints("keypoints");
    static const GlobalString keysplines("keysplines");
    static const GlobalString keytimes("keytimes");
    static const GlobalString lengthadjust("lengthadjust");
    static const GlobalString limitingconeangle("limitingconeangle");
    static const GlobalString markerheight("markerheight");
    static const GlobalString markerunits("markerunits");
    static const GlobalString markerwidth("markerwidth");
    static const GlobalString maskcontentunits("maskcontentunits");
    static const GlobalString maskunits("maskunits");
    static const GlobalString numoctaves("numoctaves");
    static const GlobalString pathlength("pathlength");
    static const GlobalString patterncontentunits("patterncontentunits");
    static const GlobalString patterntransform("patterntransform");
    static const GlobalString patternunits("patternunits");
    static const GlobalString pointsatx("pointsatx");
    static const GlobalString pointsaty("pointsaty");
    static const GlobalString pointsatz("pointsatz");
    static const GlobalString preservealpha("preservealpha");
    static const GlobalString preserveaspectratio("preserveaspectratio");
    static const GlobalString primitiveunits("primitiveunits");
    static const GlobalString refx("refx");
    static const GlobalString refy("refy");
    static const GlobalString repeatcount("repeatcount");
    static const GlobalString repeatdur("repeatdur");
    static const GlobalString requiredextensions("requiredextensions");
    static const GlobalString requiredfeatures("requiredfeatures");
    static const GlobalString specularconstant("specularconstant");
    static const GlobalString specularexponent("specularexponent");
    static const GlobalString spreadmethod("spreadmethod");
    static const GlobalString startoffset("startoffset");
    static const GlobalString stddeviation("stddeviation");
    static const GlobalString stitchtiles("stitchtiles");
    static const GlobalString surfacescale("surfacescale");
    static const GlobalString systemlanguage("systemlanguage");
    static const GlobalString tablevalues("tablevalues");
    static const GlobalString targetx("targetx");
    static const GlobalString targety("targety");
    static const GlobalString textlength("textlength");
    static const GlobalString viewbox("viewbox");
    static const GlobalString viewtarget("viewtarget");
    static const GlobalString xchannelselector("xchannelselector");
    static const GlobalString ychannelselector("ychannelselector");
    static const GlobalString zoomandpan("zoomandpan");

    token.adjustAttributeName(attributename, attributeNameAttr);
    token.adjustAttributeName(attributetype, attributeTypeAttr);
    token.adjustAttributeName(basefrequency, baseFrequencyAttr);
    token.adjustAttributeName(baseprofile, baseProfileAttr);
    token.adjustAttributeName(calcmode, calcModeAttr);
    token.adjustAttributeName(clippathunits, clipPathUnitsAttr);
    token.adjustAttributeName(diffuseconstant, diffuseConstantAttr);
    token.adjustAttributeName(edgemode, edgeModeAttr);
    token.adjustAttributeName(filterunits, filterUnitsAttr);
    token.adjustAttributeName(glyphref, glyphRefAttr);
    token.adjustAttributeName(gradienttransform, gradientTransformAttr);
    token.adjustAttributeName(gradientunits, gradientUnitsAttr);
    token.adjustAttributeName(kernelmatrix, kernelMatrixAttr);
    token.adjustAttributeName(kernelunitlength, kernelUnitLengthAttr);
    token.adjustAttributeName(keypoints, keyPointsAttr);
    token.adjustAttributeName(keysplines, keySplinesAttr);
    token.adjustAttributeName(keytimes, keyTimesAttr);
    token.adjustAttributeName(lengthadjust, lengthAdjustAttr);
    token.adjustAttributeName(limitingconeangle, limitingConeAngleAttr);
    token.adjustAttributeName(markerheight, markerHeightAttr);
    token.adjustAttributeName(markerunits, markerUnitsAttr);
    token.adjustAttributeName(markerwidth, markerWidthAttr);
    token.adjustAttributeName(maskcontentunits, maskContentUnitsAttr);
    token.adjustAttributeName(maskunits, maskUnitsAttr);
    token.adjustAttributeName(numoctaves, numOctavesAttr);
    token.adjustAttributeName(pathlength, pathLengthAttr);
    token.adjustAttributeName(patterncontentunits, patternContentUnitsAttr);
    token.adjustAttributeName(patterntransform, patternTransformAttr);
    token.adjustAttributeName(patternunits, patternUnitsAttr);
    token.adjustAttributeName(pointsatx, pointsAtXAttr);
    token.adjustAttributeName(pointsaty, pointsAtYAttr);
    token.adjustAttributeName(pointsatz, pointsAtZAttr);
    token.adjustAttributeName(preservealpha, preserveAlphaAttr);
    token.adjustAttributeName(preserveaspectratio, preserveAspectRatioAttr);
    token.adjustAttributeName(primitiveunits, primitiveUnitsAttr);
    token.adjustAttributeName(refx, refXAttr);
    token.adjustAttributeName(refy, refYAttr);
    token.adjustAttributeName(repeatcount, repeatCountAttr);
    token.adjustAttributeName(repeatdur, repeatDurAttr);
    token.adjustAttributeName(requiredextensions, requiredExtensionsAttr);
    token.adjustAttributeName(requiredfeatures, requiredFeaturesAttr);
    token.adjustAttributeName(specularconstant, specularConstantAttr);
    token.adjustAttributeName(specularexponent, specularExponentAttr);
    token.adjustAttributeName(spreadmethod, spreadMethodAttr);
    token.adjustAttributeName(startoffset, startOffsetAttr);
    token.adjustAttributeName(stddeviation, stdDeviationAttr);
    token.adjustAttributeName(stitchtiles, stitchTilesAttr);
    token.adjustAttributeName(surfacescale, surfaceScaleAttr);
    token.adjustAttributeName(systemlanguage, systemLanguageAttr);
    token.adjustAttributeName(tablevalues, tableValuesAttr);
    token.adjustAttributeName(targetx, targetXAttr);
    token.adjustAttributeName(targety, targetYAttr);
    token.adjustAttributeName(textlength, textLengthAttr);
    token.adjustAttributeName(viewbox, viewBoxAttr);
    token.adjustAttributeName(viewtarget, viewTargetAttr);
    token.adjustAttributeName(xchannelselector, xChannelSelectorAttr);
    token.adjustAttributeName(ychannelselector, yChannelSelectorAttr);
    token.adjustAttributeName(zoomandpan, zoomAndPanAttr);
}

void HTMLParser::adjustMathMLAttributes(HTMLTokenView& token)
{
    static const GlobalString definitionurl("definitionurl");
    token.adjustAttributeName(definitionurl, definitionUrlTag);
}

void HTMLParser::insertDoctype(HTMLTokenView& token)
{
}

void HTMLParser::insertComment(HTMLTokenView& token, ContainerNode* parent)
{
}

void HTMLParser::insertHTMLHtmlElement(HTMLTokenView& token)
{
    auto element = createHTMLElement(token);
    append(m_document, element, false);
    m_openElements.pushHTMLHtmlElement(element);
}

void HTMLParser::insertHeadElement(HTMLTokenView& token)
{
    auto element = createHTMLElement(token);
    append(element, false);
    m_openElements.pushHTMLHeadElement(element);
    m_head = element;
}

void HTMLParser::insertHTMLBodyElement(HTMLTokenView& token)
{
    auto element = createHTMLElement(token);
    append(element, false);
    m_openElements.pushHTMLBodyElement(element);
}

void HTMLParser::insertHTMLFormElement(HTMLTokenView& token)
{
    auto element = createHTMLElement(token);
    append(element, false);
    m_openElements.push(element);
    m_form = element;
}

void HTMLParser::insertSelfClosingHTMLElement(HTMLTokenView& token)
{
    auto element = createHTMLElement(token);
    append(element, true);
}

void HTMLParser::insertHTMLElement(HTMLTokenView& token)
{
    auto element = createHTMLElement(token);
    append(element, false);
    m_openElements.push(element);
}

void HTMLParser::insertHTMLFormattingElement(HTMLTokenView& token)
{
    auto element = createHTMLElement(token);
    append(element, false);
    m_openElements.push(element);
    m_activeFormattingElements.append(element);
}

void HTMLParser::insertForeignElement(HTMLTokenView& token, const GlobalString& namespaceUri)
{
    auto element = createElement(token, namespaceUri);
    append(element, token.selfClosing());
    if(!token.selfClosing())
        m_openElements.push(element);
}

void HTMLParser::insertTextNode(const std::string_view& data)
{
    InsertionLocation location;
    location.parent = m_openElements.top();
    location.child = m_document->createText(data);
    if(shouldFosterParent())
        findFosterLocation(location);
    insert(location);
}

void HTMLParser::resetInsertionMode()
{
    for(int i = m_openElements.size() - 1; i >= 0; --i) {
        auto element = m_openElements.at(i);
        if(element->tagName() == selectTag) {
            for(int j = i; j > 0; --j) {
                auto ancestor = m_openElements.at(j - 1);
                if(ancestor->tagName() == tableTag) {
                    m_insertionMode = InsertionMode::InSelectInTable;
                    return;
                }
            }

            m_insertionMode = InsertionMode::InSelect;
            return;
        }

        if(element->tagName() == tdTag
            || element->tagName() == thTag) {
            m_insertionMode = InsertionMode::InCell;
            return;
        }

        if(element->tagName() == trTag) {
            m_insertionMode = InsertionMode::InRow;
            return;
        }

        if(element->tagName() == tbodyTag
            || element->tagName() == theadTag
            || element->tagName() == tfootTag) {
            m_insertionMode = InsertionMode::InTableBody;
            return;
        }

        if(element->tagName() == captionTag) {
            m_insertionMode = InsertionMode::InCaption;
            return;
        }

        if(element->tagName() == colgroupTag) {
            m_insertionMode = InsertionMode::InColumnGroup;
            return;
        }

        if(element->tagName() == tableTag) {
            m_insertionMode = InsertionMode::InTable;
            return;
        }

        if(element->tagName() == headTag
            || element->tagName() == bodyTag) {
            m_insertionMode = InsertionMode::InBody;
            return;
        }

        if(element->tagName() == framesetTag) {
            m_insertionMode = InsertionMode::InFrameset;
            return;
        }

        if(element->tagName() == htmlTag) {
            assert(m_head != nullptr);
            m_insertionMode = InsertionMode::AfterHead;
            return;
        }
    }
}

HTMLParser::InsertionMode HTMLParser::currentInsertionMode(HTMLTokenView& token) const
{
    if(m_openElements.empty())
        return m_insertionMode;

    auto element = m_openElements.top();
    if(element->namespaceUri() == namespaceuri::xhtml)
        return m_insertionMode;

    if(isMathMLTextIntegrationPoint(element)) {
        if(token.type() == HTMLToken::Type::StartTag
            && token.tagName() != mglyphTag
            && token.tagName() != malignmarkTag)
            return m_insertionMode;
        if(token.type() == HTMLToken::Type::Character
            || token.type() == HTMLToken::Type::SpaceCharacter)
            return m_insertionMode;
    }

    if(element->namespaceUri() == namespaceuri::mathml
        && element->tagName() == annotation_xmlTag
        && token.type() == HTMLToken::Type::StartTag
        && token.tagName() == svgTag) {
        return m_insertionMode;
    }

    if(isHTMLIntegrationPoint(element)) {
        if(token.type() == HTMLToken::Type::StartTag)
            return m_insertionMode;
        if(token.type() == HTMLToken::Type::Character
            || token.type() == HTMLToken::Type::SpaceCharacter)
            return m_insertionMode;
    }

    if(token.type() == HTMLToken::Type::EndOfFile)
        return m_insertionMode;

    return InsertionMode::InForeignContent;
}

void HTMLParser::handleInitialMode(HTMLTokenView& token)
{
    if(token.type() == HTMLToken::Type::SpaceCharacter) {
        return;
    }

    handleErrorToken(token);
    m_inQuirksMode = true;
    m_insertionMode = InsertionMode::BeforeHTML;
    handleBeforeHTMLMode(token);
}

void HTMLParser::handleBeforeHTMLMode(HTMLTokenView& token)
{
    if(token.type() == HTMLToken::Type::StartTag) {
        if(token.tagName() == htmlTag) {
            insertHTMLHtmlElement(token);
            m_insertionMode = InsertionMode::BeforeHead;
            return;
        }
    } else if(token.type() == HTMLToken::Type::EndTag) {
        if(token.tagName() != headTag
            || token.tagName() != bodyTag
            || token.tagName() != htmlTag
            || token.tagName() != brTag) {
            handleErrorToken(token);
            return;
        }
    } else if(token.type() == HTMLToken::Type::SpaceCharacter) {
        return;
    }

    handleFakeStartTagToken(htmlTag);
    handleBeforeHeadMode(token);
}

void HTMLParser::handleBeforeHeadMode(HTMLTokenView& token)
{
    if(token.type() == HTMLToken::Type::StartTag) {
        if(token.tagName() == htmlTag) {
            handleInBodyMode(token);
            return;
        }

        if(token.tagName() == headTag) {
            insertHeadElement(token);
            m_insertionMode = InsertionMode::InHead;
            return;
        }
    } else if(token.type() == HTMLToken::Type::EndTag) {
        if(token.tagName() != headTag
            || token.tagName() != bodyTag
            || token.tagName() != htmlTag
            || token.tagName() != brTag) {
            handleErrorToken(token);
            return;
        }
    } else if(token.type() == HTMLToken::Type::SpaceCharacter) {
        return;
    }

    handleFakeStartTagToken(headTag);
    handleInHeadMode(token);
}

void HTMLParser::handleInHeadMode(HTMLTokenView& token)
{
    if(token.type() == HTMLToken::Type::StartTag) {
        if(token.tagName() == htmlTag) {
            handleInBodyMode(token);
            return;
        }

        if(token.tagName() == baseTag
            || token.tagName() == basefontTag
            || token.tagName() == bgsoundTag
            || token.tagName() == commandTag
            || token.tagName() == linkTag
            || token.tagName() == metaTag) {
            insertSelfClosingHTMLElement(token);
            return;
        }

        if(token.tagName() == titleTag) {
            handleRCDataToken(token);
            return;
        }

        if(token.tagName() == noscriptTag) {
            insertHTMLElement(token);
            m_insertionMode = InsertionMode::InHeadNoscript;
            return;
        }

        if(token.tagName() == noframesTag
            || token.tagName() == styleTag) {
            handleRawTextToken(token);
            return;
        }

        if(token.tagName() == scriptTag) {
            handleScriptDataToken(token);
            return;
        }

        if(token.tagName() == headTag) {
            handleErrorToken(token);
            return;
        }
    } else if(token.type() == HTMLToken::Type::EndTag) {
        if(token.tagName() == headTag) {
            m_openElements.popHTMLHeadElement();
            m_insertionMode = InsertionMode::AfterHead;
            return;
        }

        if(token.tagName() != bodyTag
            || token.tagName() != htmlTag
            || token.tagName() != brTag) {
            handleErrorToken(token);
            return;
        }
    } else if(token.type() == HTMLToken::Type::SpaceCharacter) {
        insertTextNode(token.data());
        return;
    }

    handleFakeEndTagToken(headTag);
    handleAfterHeadMode(token);
}

void HTMLParser::handleInHeadNoscriptMode(HTMLTokenView& token)
{
    if(token.type() == HTMLToken::Type::StartTag) {
        if(token.tagName() == htmlTag) {
            handleInBodyMode(token);
            return;
        }

        if(token.tagName() == baseTag
            || token.tagName() == basefontTag
            || token.tagName() == bgsoundTag
            || token.tagName() == commandTag
            || token.tagName() == linkTag
            || token.tagName() == metaTag) {
            insertSelfClosingHTMLElement(token);
            return;
        }

        if(token.tagName() == headTag
            || token.tagName() == noscriptTag) {
            handleErrorToken(token);
            return;
        }
    } else if(token.type() == HTMLToken::Type::EndTag) {
        if(token.tagName() == noscriptTag) {
            assert(currentElement()->tagName() == noscriptTag);
            m_openElements.pop();
            assert(currentElement()->tagName() == headTag);
            m_insertionMode = InsertionMode::InHead;
            return;
        }

        if(token.tagName() != brTag) {
            handleErrorToken(token);
            return;
        }
    } else if(token.type() == HTMLToken::Type::SpaceCharacter) {
        handleInHeadMode(token);
        return;
    }

    handleErrorToken(token);
    handleFakeEndTagToken(noscriptTag);
    handleInHeadMode(token);
}

void HTMLParser::handleAfterHeadMode(HTMLTokenView& token)
{
    if(token.type() == HTMLToken::Type::StartTag) {
        if(token.tagName() == htmlTag) {
            handleInBodyMode(token);
            return;
        }

        if(token.tagName() == bodyTag) {
            m_framesetOk = false;
            insertHTMLBodyElement(token);
            m_insertionMode = InsertionMode::InBody;
            return;
        }

        if(token.tagName() == framesetTag) {
            insertHTMLElement(token);
            m_insertionMode = InsertionMode::InFrameset;
            return;
        }

        if(token.tagName() == baseTag
            || token.tagName() == basefontTag
            || token.tagName() == bgsoundTag
            || token.tagName() == linkTag
            || token.tagName() == metaTag
            || token.tagName() == noframesTag
            || token.tagName() == scriptTag
            || token.tagName() == styleTag
            || token.tagName() == titleTag) {
            handleErrorToken(token);
            m_openElements.pushHTMLHeadElement(m_head);
            handleInHeadMode(token);
            m_openElements.removeHTMLHeadElement(m_head);
            return;
        }

        if(token.tagName() == headTag) {
            handleErrorToken(token);
            return;
        }
    } else if(token.type() == HTMLToken::Type::EndTag) {
        if(token.tagName() != bodyTag
            || token.tagName() != htmlTag
            || token.tagName() != brTag) {
            handleErrorToken(token);
            return;
        }
    } else if(token.type() == HTMLToken::Type::SpaceCharacter) {
        insertTextNode(token.data());
        return;
    }

    handleFakeStartTagToken(bodyTag);
    m_framesetOk = true;
    handleInBodyMode(token);
}

void HTMLParser::handleInBodyMode(HTMLTokenView& token)
{
    if(token.type() == HTMLToken::Type::StartTag) {
        if(token.tagName() == htmlTag) {
            handleErrorToken(token);
            auto element = m_openElements.htmlElement();
            for(auto attribute : token.attributes()) {
                if(element->hasAttribute(attribute.name()))
                    continue;
                element->setAttribute(attribute);
            }

            return;
        }

        if(token.tagName() == baseTag
            || token.tagName() == basefontTag
            || token.tagName() == bgsoundTag
            || token.tagName() == commandTag
            || token.tagName() == linkTag
            || token.tagName() == metaTag
            || token.tagName() == noframesTag
            || token.tagName() == scriptTag
            || token.tagName() == styleTag
            || token.tagName() == titleTag) {
            handleInHeadMode(token);
            return;
        }

        if(token.tagName() == bodyTag) {
            handleErrorToken(token);
            m_framesetOk = false;
            auto element = m_openElements.bodyElement();
            for(auto attribute : token.attributes()) {
                if(element->hasAttribute(attribute.name()))
                    continue;
                element->setAttribute(attribute);
            }

            return;
        }

        if(token.tagName() == framesetTag) {
            handleErrorToken(token);
            if(!m_framesetOk)
                return;
            m_openElements.removeHTMLBodyElement();
            insertHTMLElement(token);
            m_insertionMode = InsertionMode::InFrameset;
            return;
        }

        if(token.tagName() == addressTag
            || token.tagName() == articleTag
            || token.tagName() == asideTag
            || token.tagName() == blockquoteTag
            || token.tagName() == centerTag
            || token.tagName() == detailsTag
            || token.tagName() == dirTag
            || token.tagName() == divTag
            || token.tagName() == dlTag
            || token.tagName() == fieldsetTag
            || token.tagName() == figcaptionTag
            || token.tagName() == figureTag
            || token.tagName() == footerTag
            || token.tagName() == headerTag
            || token.tagName() == hgroupTag
            || token.tagName() == mainTag
            || token.tagName() == menuTag
            || token.tagName() == navTag
            || token.tagName() == olTag
            || token.tagName() == pTag
            || token.tagName() == sectionTag
            || token.tagName() == summaryTag
            || token.tagName() == ulTag) {
            if(m_openElements.inButtonScope(pTag))
                handleFakeEndTagToken(pTag);
            insertHTMLElement(token);
            return;
        }

        if(isNumberedHeaderTag(token.tagName())) {
            if(m_openElements.inButtonScope(pTag))
                handleFakeEndTagToken(pTag);
            if(isNumberedHeaderElement(currentElement())) {
                handleErrorToken(token);
                m_openElements.pop();
            }

            insertHTMLElement(token);
            return;
        }

        if(token.tagName() == preTag
            || token.tagName() == listingTag) {
            if(m_openElements.inButtonScope(pTag))
                handleFakeEndTagToken(pTag);
            insertHTMLElement(token);
            m_skipLeadingNewline = true;
            m_framesetOk = false;
            return;
        }

        if(token.tagName() == formTag) {
            if(m_form != nullptr) {
                handleErrorToken(token);
                return;
            }

            if(m_openElements.inButtonScope(pTag))
                handleFakeEndTagToken(pTag);
            insertHTMLFormElement(token);
            return;
        }

        if(token.tagName() == liTag) {
            m_framesetOk = false;
            for(int i = m_openElements.size() - 1; i >= 0; --i) {
                auto element = m_openElements.at(i);
                if(element->tagName() == liTag) {
                    handleFakeEndTagToken(liTag);
                    break;
                }

                if(isSpecialElement(element)
                    && element->tagName() != addressTag
                    && element->tagName() != divTag
                    && element->tagName() != pTag) {
                    break;
                }
            }

            if(m_openElements.inButtonScope(pTag))
                handleFakeEndTagToken(pTag);
            insertHTMLElement(token);
            return;
        }

        if(token.tagName() == ddTag
            || token.tagName() == dtTag) {
            m_framesetOk = false;
            for(int i = m_openElements.size() - 1; i >= 0; --i) {
                auto element = m_openElements.at(i);
                if(element->tagName() == ddTag
                    || element->tagName() == dtTag) {
                    handleFakeEndTagToken(element->tagName());
                    break;
                }

                if(isSpecialElement(element)
                    && element->tagName() != addressTag
                    && element->tagName() != divTag
                    && element->tagName() != pTag) {
                    break;
                }
            }

            if(m_openElements.inButtonScope(pTag))
                handleFakeEndTagToken(pTag);
            insertHTMLElement(token);
            return;
        }

        if(token.tagName() == plaintextTag) {
            m_tokenizer.setState(HTMLTokenizer::State::PLAINTEXT);
            if(m_openElements.inButtonScope(pTag))
                handleFakeEndTagToken(pTag);
            insertHTMLElement(token);
            return;
        }

        if(token.tagName() == buttonTag) {
            if(m_openElements.inScope(buttonTag)) {
                handleErrorToken(token);
                handleFakeEndTagToken(buttonTag);
                return;
            }

            reconstructActiveFormattingElements();
            insertHTMLElement(token);
            m_framesetOk = false;
            return;
        }

        if(token.tagName() == aTag) {
            auto aElement = m_activeFormattingElements.closestElementInScope(aTag);
            if(aElement != nullptr) {
                handleErrorToken(token);
                handleFakeEndTagToken(aTag);
                m_activeFormattingElements.remove(aElement);
                if(m_openElements.contains(aElement)) {
                    m_openElements.remove(aElement);
                }
            }

            reconstructActiveFormattingElements();
            insertHTMLFormattingElement(token);
            return;
        }

        if(token.tagName() == bTag
            || token.tagName() == bigTag
            || token.tagName() == codeTag
            || token.tagName() == emTag
            || token.tagName() == fontTag
            || token.tagName() == iTag
            || token.tagName() == sTag
            || token.tagName() == smallTag
            || token.tagName() == strikeTag
            || token.tagName() == strongTag
            || token.tagName() == ttTag
            || token.tagName() == uTag) {
            reconstructActiveFormattingElements();
            insertHTMLFormattingElement(token);
            return;
        }

        if(token.tagName() == nobrTag) {
            reconstructActiveFormattingElements();
            if(m_openElements.inScope(nobrTag)) {
                handleErrorToken(token);
                handleFakeEndTagToken(nobrTag);
                reconstructActiveFormattingElements();
            }

            insertHTMLFormattingElement(token);
            return;
        }

        if(token.tagName() == marqueeTag
            || token.tagName() == objectTag) {
            reconstructActiveFormattingElements();
            insertHTMLElement(token);
            m_activeFormattingElements.appendMarker();
            m_framesetOk = false;
            return;
        }

        if(token.tagName() == tableTag) {
            if(!m_inQuirksMode && m_openElements.inButtonScope(pTag))
                handleFakeEndTagToken(pTag);
            insertHTMLElement(token);
            m_framesetOk = false;
            m_insertionMode = InsertionMode::InTable;
            return;
        }

        if(token.tagName() == areaTag
            || token.tagName() == brTag
            || token.tagName() == embedTag
            || token.tagName() == imgTag
            || token.tagName() == imageTag
            || token.tagName() == keygenTag
            || token.tagName() == wbrTag) {
            if(token.tagName() == imageTag) {
                handleErrorToken(token);
                token.adjustTagName(imageTag, imgTag);
            }

            reconstructActiveFormattingElements();
            insertSelfClosingHTMLElement(token);
            m_framesetOk = false;
            return;
        }

        if(token.tagName() == inputTag) {
            auto typeAttribute = token.findAttribute(typeAttr);
            reconstructActiveFormattingElements();
            insertSelfClosingHTMLElement(token);
            if(typeAttribute == nullptr || equals(typeAttribute->value(), "hidden", false))
                m_framesetOk = false;
            return;
        }

        if(token.tagName() == paramTag
            || token.tagName() == sourceTag
            || token.tagName() == trackTag) {
            insertSelfClosingHTMLElement(token);
            return;
        }

        if(token.tagName() == hrTag) {
            if(m_openElements.inButtonScope(pTag))
                handleFakeEndTagToken(pTag);
            insertSelfClosingHTMLElement(token);
            m_framesetOk = false;
            return;
        }

        if(token.tagName() == textareaTag) {
            insertHTMLElement(token);
            m_tokenizer.setState(HTMLTokenizer::State::RCDATA);
            m_originalInsertionMode = m_insertionMode;
            m_skipLeadingNewline = true;
            m_framesetOk = false;
            m_insertionMode = InsertionMode::Text;
            return;
        }

        if(token.tagName() == xmpTag) {
            if(m_openElements.inButtonScope(pTag))
                handleFakeEndTagToken(pTag);
            reconstructActiveFormattingElements();
            m_framesetOk = false;
            handleRawTextToken(token);
            return;
        }

        if(token.tagName() == iframeTag) {
            m_framesetOk = false;
            handleRawTextToken(token);
            return;
        }

        if(token.tagName() == noembedTag
            || token.tagName() == noscriptTag) {
            handleRawTextToken(token);
            return;
        }

        if(token.tagName() == selectTag) {
            reconstructActiveFormattingElements();
            insertHTMLElement(token);
            m_framesetOk = false;
            if(m_insertionMode == InsertionMode::InTable
                || m_insertionMode == InsertionMode::InCaption
                || m_insertionMode == InsertionMode::InColumnGroup
                || m_insertionMode == InsertionMode::InTableBody
                || m_insertionMode == InsertionMode::InRow
                || m_insertionMode == InsertionMode::InCell) {
                m_insertionMode = InsertionMode::InSelectInTable;
            } else {
                m_insertionMode = InsertionMode::InSelect;
            }

            return;
        }

        if(token.tagName() == optgroupTag
            || token.tagName() == optionTag) {
            if(currentElement()->tagName() == optionTag) {
                handleFakeEndTagToken(optionTag);
            }

            reconstructActiveFormattingElements();
            insertHTMLElement(token);
            return;
        }

        if(token.tagName() == rtTag
            || token.tagName() == rpTag) {
            if(m_openElements.inScope(rubyTag)) {
                m_openElements.generateImpliedEndTags();
                if(currentElement()->tagName() != rubyTag) {
                    handleErrorToken(token);
                }
            }

            insertHTMLElement(token);
            return;
        }

        if(token.tagName() == mathTag) {
            reconstructActiveFormattingElements();
            adjustMathMLAttributes(token);
            insertForeignElement(token, namespaceuri::mathml);
            return;
        }

        if(token.tagName() == svgTag) {
            reconstructActiveFormattingElements();
            adjustSVGTagNames(token);
            adjustSVGAttributes(token);
            insertForeignElement(token, namespaceuri::svg);
            return;
        }

        if(token.tagName() == captionTag
            || token.tagName() == colTag
            || token.tagName() == colgroupTag
            || token.tagName() == frameTag
            || token.tagName() == headTag
            || token.tagName() == tbodyTag
            || token.tagName() == tfootTag
            || token.tagName() == theadTag
            || token.tagName() == thTag
            || token.tagName() == tdTag
            || token.tagName() == trTag) {
            handleErrorToken(token);
            return;
        }

        reconstructActiveFormattingElements();
        insertHTMLElement(token);
        return;
    }

    if(token.type() == HTMLToken::Type::EndTag) {
        if(token.tagName() == bodyTag) {
            if(!m_openElements.inScope(bodyTag)) {
                handleErrorToken(token);
                return;
            }

            m_insertionMode = InsertionMode::AfterBody;
            return;
        }

        if(token.tagName() == htmlTag) {
            if(!m_openElements.inScope(bodyTag))
                return;
            handleFakeEndTagToken(bodyTag);
            handleAfterBodyMode(token);
            return;
        }

        if(token.tagName() == addressTag
            || token.tagName() == articleTag
            || token.tagName() == asideTag
            || token.tagName() == blockquoteTag
            || token.tagName() == buttonTag
            || token.tagName() == centerTag
            || token.tagName() == detailsTag
            || token.tagName() == dirTag
            || token.tagName() == divTag
            || token.tagName() == dlTag
            || token.tagName() == fieldsetTag
            || token.tagName() == figcaptionTag
            || token.tagName() == figureTag
            || token.tagName() == footerTag
            || token.tagName() == headerTag
            || token.tagName() == hgroupTag
            || token.tagName() == listingTag
            || token.tagName() == mainTag
            || token.tagName() == menuTag
            || token.tagName() == navTag
            || token.tagName() == olTag
            || token.tagName() == preTag
            || token.tagName() == sectionTag
            || token.tagName() == summaryTag
            || token.tagName() == ulTag) {
            if(!m_openElements.inScope(token.tagName())) {
                handleErrorToken(token);
                return;
            }

            m_openElements.generateImpliedEndTags();
            if(currentElement()->tagName() != token.tagName())
                handleErrorToken(token);
            m_openElements.popUntilPopped(token.tagName());
            return;
        }

        if(token.tagName() == formTag) {
            auto node = m_form;
            m_form = nullptr;
            if(node == nullptr || !m_openElements.inScope(node)) {
                handleErrorToken(token);
                return;
            }

            m_openElements.generateImpliedEndTags();
            if(currentElement() != node)
                handleErrorToken(token);
            m_openElements.remove(node);
            return;
        }

        if(token.tagName() == pTag) {
            if(!m_openElements.inButtonScope(pTag)) {
                handleErrorToken(token);
                handleFakeStartTagToken(pTag);
                assert(m_openElements.inScope(pTag));
                handleInBodyMode(token);
                return;
            }

            m_openElements.generateImpliedEndTagsExcept(pTag);
            if(currentElement()->tagName() != pTag)
                handleErrorToken(token);
            m_openElements.popUntilPopped(pTag);
            return;
        }

        if(token.tagName() == liTag) {
            if(!m_openElements.inListItemScope(liTag)) {
                handleErrorToken(token);
                return;
            }

            m_openElements.generateImpliedEndTagsExcept(liTag);
            if(currentElement()->tagName() != liTag)
                handleErrorToken(token);
            m_openElements.popUntilPopped(liTag);
            return;
        }

        if(token.tagName() == ddTag
            || token.tagName() == dtTag) {
            if(!m_openElements.inScope(token.tagName())) {
                handleErrorToken(token);
                return;
            }

            m_openElements.generateImpliedEndTagsExcept(token.tagName());
            if(currentElement()->tagName() != token.tagName())
                handleErrorToken(token);
            m_openElements.popUntilPopped(token.tagName());
            return;
        }

        if(isNumberedHeaderTag(token.tagName())) {
            if(!m_openElements.isNumberedHeaderElementInScope()) {
                handleErrorToken(token);
                return;
            }

            m_openElements.generateImpliedEndTags();
            if(currentElement()->tagName() != token.tagName())
                handleErrorToken(token);
            m_openElements.popUntilNumberedHeaderElementPopped();
            return;
        }

        if(isFormattingTag(token.tagName())) {
            adoptionAgencyAlgorithm(token);
            return;
        }

        if(token.tagName() == marqueeTag
            || token.tagName() == objectTag) {
            if(!m_openElements.inScope(token.tagName())) {
                handleErrorToken(token);
                return;
            }

            m_openElements.generateImpliedEndTags();
            if(currentElement()->tagName() != token.tagName())
                handleErrorToken(token);
            m_openElements.popUntilPopped(token.tagName());
            m_activeFormattingElements.clearToLastMarker();
            return;
        }

        if(token.tagName() == brTag) {
            handleErrorToken(token);
            handleFakeStartTagToken(brTag);
            return;
        }

        defaultForInBodyEndTagToken(token);
        return;
    }

    if(token.type() == HTMLToken::Type::Character
        || token.type() == HTMLToken::Type::SpaceCharacter) {
        reconstructActiveFormattingElements();
        insertTextNode(token.data());
        if(token.type() == HTMLToken::Type::Character)
            m_framesetOk = false;
        return;
    }

    if(token.type() == HTMLToken::Type::EndOfFile) {
        for(int i = m_openElements.size() - 1; i >= 0; --i) {
            auto element = m_openElements.at(i);
            if(element->tagName() != ddTag
                || element->tagName() != dtTag
                || element->tagName() != liTag
                || element->tagName() != pTag
                || element->tagName() != tbodyTag
                || element->tagName() != tdTag
                || element->tagName() != tfootTag
                || element->tagName() != thTag
                || element->tagName() != theadTag
                || element->tagName() != trTag
                || element->tagName() != bodyTag
                || element->tagName() != htmlTag) {
                handleErrorToken(token);
                return;
            }
        }
    }
}

void HTMLParser::handleTextMode(HTMLTokenView& token)
{
    if(token.type() == HTMLToken::Type::Character
        || token.type() == HTMLToken::Type::SpaceCharacter) {
        insertTextNode(token.data());
        return;
    }

    if(token.type() == HTMLToken::Type::StartTag) {
        assert(false);
    }

    if(token.type() == HTMLToken::Type::EndTag) {
        if(token.tagName() == scriptTag) {
            assert(currentElement()->tagName() == scriptTag);
            m_tokenizer.setState(HTMLTokenizer::State::Data);
            m_openElements.pop();
            m_insertionMode = m_originalInsertionMode;
            return;
        }

        m_openElements.pop();
        m_insertionMode = m_originalInsertionMode;
        return;
    }

    if(token.type() == HTMLToken::Type::EndOfFile) {
        handleErrorToken(token);
        m_openElements.pop();
        m_insertionMode = m_originalInsertionMode;
        handleToken(token, m_insertionMode);
        return;
    }
}

void HTMLParser::handleInTableMode(HTMLTokenView& token)
{
    if(token.type() == HTMLToken::Type::StartTag) {
        if(token.tagName() == captionTag) {
            m_openElements.popUntilTableScopeMarker();
            m_activeFormattingElements.appendMarker();
            insertHTMLElement(token);
            m_insertionMode = InsertionMode::InCaption;
            return;
        }

        if(token.tagName() == colgroupTag) {
            m_openElements.popUntilTableScopeMarker();
            insertHTMLElement(token);
            m_insertionMode = InsertionMode::InColumnGroup;
            return;
        }

        if(token.tagName() == colTag) {
            handleFakeStartTagToken(colgroupTag);
            handleInCaptionMode(token);
            return;
        }

        if(token.tagName() == tbodyTag
            || token.tagName() == tfootTag
            || token.tagName() == theadTag) {
            m_openElements.popUntilTableScopeMarker();
            insertHTMLElement(token);
            m_insertionMode = InsertionMode::InTableBody;
            return;
        }

        if(token.tagName() == thTag
            || token.tagName() == tdTag
            || token.tagName() == trTag) {
            handleFakeStartTagToken(tbodyTag);
            handleInTableBodyMode(token);
            return;
        }

        if(token.tagName() == tableTag) {
            handleErrorToken(token);
            handleFakeEndTagToken(tableTag);
            handleToken(token, m_insertionMode);
            return;
        }

        if(token.tagName() == styleTag
            || token.tagName() == scriptTag) {
            handleInHeadMode(token);
            return;
        }

        if(token.tagName() == inputTag) {
            auto typeAttribute = token.findAttribute(typeAttr);
            if(typeAttribute && equals(typeAttribute->value(), "hidden", false)) {
                handleErrorToken(token);
                insertSelfClosingHTMLElement(token);
                return;
            }

            m_fosterParenting = true;
            handleInBodyMode(token);
            m_fosterParenting = false;
            return;
        }

        if(token.tagName() == formTag) {
            handleErrorToken(token);
            if(m_form != nullptr)
                return;
            insertHTMLFormElement(token);
            m_openElements.pop();
            return;
        }
    } else if(token.type() == HTMLToken::Type::EndTag) {
        if(token.tagName() == tableTag) {
            assert(m_openElements.inTableScope(tableTag));
            m_openElements.popUntilPopped(tableTag);
            resetInsertionMode();
            return;
        }

        if(token.tagName() == bodyTag
            || token.tagName() == captionTag
            || token.tagName() == colTag
            || token.tagName() == colgroupTag
            || token.tagName() == htmlTag
            || token.tagName() == tbodyTag
            || token.tagName() == tfootTag
            || token.tagName() == theadTag
            || token.tagName() == thTag
            || token.tagName() == tdTag
            || token.tagName() == trTag) {
            handleErrorToken(token);
            return;
        }
    } else if(token.type() == HTMLToken::Type::Character
        || token.type() == HTMLToken::Type::SpaceCharacter) {
        m_pendingTableCharacters.clear();
        m_originalInsertionMode = m_insertionMode;
        m_insertionMode = InsertionMode::InTableText;
        handleInTableTextMode(token);
        return;
    }

    if(token.type() == HTMLToken::Type::EndOfFile) {
        assert(currentElement()->tagName() != htmlTag);
        handleErrorToken(token);
        return;
    }

    m_fosterParenting = true;
    handleInBodyMode(token);
    m_fosterParenting = false;
}

void HTMLParser::handleInTableTextMode(HTMLTokenView& token)
{
    if(token.type() == HTMLToken::Type::Character
        || token.type() == HTMLToken::Type::SpaceCharacter) {
        m_pendingTableCharacters += token.data();
        return;
    }

    flushPendingTableCharacters();
    handleToken(token, m_insertionMode);
}

void HTMLParser::handleInCaptionMode(HTMLTokenView& token)
{
    if(token.type() == HTMLToken::Type::StartTag) {
        if(token.tagName() == captionTag
            || token.tagName() == colTag
            || token.tagName() == colgroupTag
            || token.tagName() == tbodyTag
            || token.tagName() == tfootTag
            || token.tagName() == theadTag
            || token.tagName() == thTag
            || token.tagName() == tdTag
            || token.tagName() == trTag) {
            handleErrorToken(token);
            handleFakeEndTagToken(captionTag);
            return;
        }
    } else if(token.type() == HTMLToken::Type::EndTag) {
        if(token.tagName() == captionTag) {
            assert(m_openElements.inTableScope(captionTag));
            m_openElements.generateImpliedEndTags();
            m_openElements.popUntilPopped(captionTag);
            m_activeFormattingElements.clearToLastMarker();
            m_insertionMode = InsertionMode::InTable;
            return;
        }

        if(token.tagName() == tableTag) {
            handleErrorToken(token);
            handleFakeEndTagToken(captionTag);
            handleInTableMode(token);
            return;
        }

        if(token.tagName() == bodyTag
            || token.tagName() == colTag
            || token.tagName() == colgroupTag
            || token.tagName() == htmlTag
            || token.tagName() == tbodyTag
            || token.tagName() == tfootTag
            || token.tagName() == theadTag
            || token.tagName() == thTag
            || token.tagName() == tdTag
            || token.tagName() == trTag) {
            handleErrorToken(token);
            return;
        }
    }

    handleInBodyMode(token);
}

void HTMLParser::handleInColumnGroupMode(HTMLTokenView& token)
{
    if(token.type() == HTMLToken::Type::StartTag) {
        if(token.tagName() == htmlTag) {
            handleInBodyMode(token);
            return;
        }

        if(token.tagName() == colTag) {
            insertSelfClosingHTMLElement(token);
            return;
        }
    } else if(token.type() == HTMLToken::Type::EndTag) {
        if(token.tagName() == colgroupTag) {
            m_openElements.pop();
            m_insertionMode = InsertionMode::InTable;
            return;
        }

        if(token.tagName() == colTag) {
            handleErrorToken(token);
            return;
        }
    } else if(token.type() == HTMLToken::Type::SpaceCharacter) {
        insertTextNode(token.data());
        return;
    }

    if(token.type() == HTMLToken::Type::EndOfFile) {
        assert(currentElement()->tagName() != htmlTag);
    }

    handleFakeEndTagToken(colgroupTag);
    handleInTableMode(token);
}

void HTMLParser::handleInTableBodyMode(HTMLTokenView& token)
{
    if(token.type() == HTMLToken::Type::StartTag) {
        if(token.tagName() == trTag) {
            m_openElements.popUntilTableBodyScopeMarker();
            insertHTMLElement(token);
            m_insertionMode = InsertionMode::InRow;
            return;
        }

        if(token.tagName() == tdTag
            || token.tagName() == thTag) {
            handleErrorToken(token);
            handleFakeStartTagToken(trTag);
            handleInRowMode(token);
            return;
        }

        if(token.tagName() == captionTag
            || token.tagName() == colTag
            || token.tagName() == colgroupTag
            || token.tagName() == tbodyTag
            || token.tagName() == tfootTag
            || token.tagName() == theadTag) {
            assert(m_openElements.inTableScope(tbodyTag) || m_openElements.inTableScope(theadTag) || m_openElements.inTableScope(tfootTag));
            m_openElements.popUntilTableBodyScopeMarker();
            handleFakeEndTagToken(currentElement()->tagName());
            handleInTableMode(token);
            return;
        }
    } else if(token.type() == HTMLToken::Type::EndTag) {
        if(token.tagName() == tbodyTag
            || token.tagName() == tfootTag
            || token.tagName() == theadTag) {
            if(!m_openElements.inTableScope(token.tagName())) {
                handleErrorToken(token);
                return;
            }

            m_openElements.popUntilTableBodyScopeMarker();
            m_openElements.pop();
            m_insertionMode = InsertionMode::InTable;
            return;
        }

        if(token.tagName() == tableTag) {
            assert(m_openElements.inTableScope(tbodyTag) || m_openElements.inTableScope(theadTag) || m_openElements.inTableScope(tfootTag));
            m_openElements.popUntilTableBodyScopeMarker();
            handleFakeEndTagToken(currentElement()->tagName());
            handleInTableMode(token);
            return;
        }

        if(token.tagName() == bodyTag
            || token.tagName() == captionTag
            || token.tagName() == colTag
            || token.tagName() == colgroupTag
            || token.tagName() == htmlTag
            || token.tagName() == thTag
            || token.tagName() == tdTag
            || token.tagName() == trTag) {
            handleErrorToken(token);
            return;
        }
    }

    handleInTableMode(token);
}

void HTMLParser::handleInRowMode(HTMLTokenView& token)
{
    if(token.type() == HTMLToken::Type::StartTag) {
        if(token.tagName() == tdTag
            || token.tagName() == thTag) {
            m_openElements.popUntilTableRowScopeMarker();
            insertHTMLElement(token);
            m_insertionMode = InsertionMode::InCell;
            m_activeFormattingElements.appendMarker();
            return;
        }

        if(token.tagName() == captionTag
            || token.tagName() == colTag
            || token.tagName() == colgroupTag
            || token.tagName() == tbodyTag
            || token.tagName() == tfootTag
            || token.tagName() == theadTag
            || token.tagName() == trTag) {
            handleFakeEndTagToken(trTag);
            handleInTableBodyMode(token);
            return;
        }
    } else if(token.type() == HTMLToken::Type::EndTag) {
        if(token.tagName() == trTag) {
            assert(m_openElements.inTableScope(trTag));
            m_openElements.popUntilTableRowScopeMarker();
            m_openElements.pop();
            m_insertionMode = InsertionMode::InTableBody;
            return;
        }

        if(token.tagName() == tableTag) {
            handleFakeEndTagToken(trTag);
            handleInTableBodyMode(token);
            return;
        }

        if(token.tagName() == tbodyTag
            || token.tagName() == tfootTag
            || token.tagName() == theadTag) {
            if(!m_openElements.inTableScope(token.tagName())) {
                handleErrorToken(token);
                return;
            }

            handleFakeEndTagToken(trTag);
            handleInTableBodyMode(token);
            return;
        }

        if(token.tagName() == bodyTag
            || token.tagName() == captionTag
            || token.tagName() == colTag
            || token.tagName() == colgroupTag
            || token.tagName() == htmlTag
            || token.tagName() == thTag
            || token.tagName() == tdTag) {
            handleErrorToken(token);
            return;
        }
    }

    handleInTableMode(token);
}

void HTMLParser::handleInCellMode(HTMLTokenView& token)
{
    if(token.type() == HTMLToken::Type::StartTag) {
        if(token.tagName() == captionTag
            || token.tagName() == colTag
            || token.tagName() == colgroupTag
            || token.tagName() == tbodyTag
            || token.tagName() == tfootTag
            || token.tagName() == theadTag
            || token.tagName() == thTag
            || token.tagName() == tdTag
            || token.tagName() == trTag) {
            assert(m_openElements.inTableScope(tdTag) || m_openElements.inTableScope(thTag));
            closeCell();
            handleToken(token, m_insertionMode);
            return;
        }
    } else if(token.type() == HTMLToken::Type::EndTag) {
        if(token.tagName() == tdTag
            || token.tagName() == thTag) {
            if(!m_openElements.inTableScope(token.tagName())) {
                handleErrorToken(token);
                return;
            }

            m_openElements.generateImpliedEndTags();
            if(currentElement()->tagName() != token.tagName())
                handleErrorToken(token);
            m_openElements.popUntilPopped(token.tagName());
            m_activeFormattingElements.clearToLastMarker();
            m_insertionMode = InsertionMode::InRow;
            return;
        }

        if(token.tagName() == bodyTag
            || token.tagName() == captionTag
            || token.tagName() == colTag
            || token.tagName() == colgroupTag
            || token.tagName() == htmlTag) {
            handleErrorToken(token);
            return;
        }

        if(token.tagName() == tableTag
            ||token.tagName() == tbodyTag
            || token.tagName() == tfootTag
            || token.tagName() == theadTag
            || token.tagName() == trTag) {
            if(!m_openElements.inTableScope(token.tagName())) {
                handleErrorToken(token);
                return;
            }

            closeCell();
            handleToken(token, m_insertionMode);
            return;
        }
    }

    handleInBodyMode(token);
}

void HTMLParser::handleInSelectMode(HTMLTokenView& token)
{
    if(token.type() == HTMLToken::Type::StartTag) {
        if(token.tagName() == htmlTag) {
            handleInBodyMode(token);
            return;
        }

        if(token.tagName() == optionTag) {
            if(currentElement()->tagName() == optionTag)
                m_openElements.pop();

            insertHTMLElement(token);
            return;
        }

        if(token.tagName() == optgroupTag) {
            if(currentElement()->tagName() == optionTag)
                m_openElements.pop();
            if(currentElement()->tagName() == optgroupTag)
                m_openElements.pop();

            insertHTMLElement(token);
            return;
        }

        if(token.tagName() == selectTag) {
            handleErrorToken(token);
            handleFakeEndTagToken(selectTag);
            return;
        }

        if(token.tagName() == inputTag
            || token.tagName() == keygenTag
            || token.tagName() == textareaTag) {
            handleErrorToken(token);
            assert(m_openElements.inSelectScope(selectTag));
            handleFakeEndTagToken(selectTag);
            handleToken(token, m_insertionMode);
            return;
        }

        if(token.tagName() == scriptTag) {
            handleInHeadMode(token);
            return;
        }
    } else if(token.type() == HTMLToken::Type::EndTag) {
        if(token.tagName() == optgroupTag) {
            if(currentElement()->tagName() == optionTag) {
                auto element = m_openElements.at(m_openElements.size() - 2);
                if(element->tagName() == optgroupTag)
                    m_openElements.pop();
            }

            if(currentElement()->tagName() != optgroupTag) {
                handleErrorToken(token);
                return;
            }

            m_openElements.pop();
            return;
        }

        if(token.tagName() == optionTag) {
            if(currentElement()->tagName() != optionTag) {
                handleErrorToken(token);
                return;
            }

            m_openElements.pop();
            return;
        }

        if(token.tagName() == selectTag) {
            assert(m_openElements.inSelectScope(token.tagName()));
            m_openElements.popUntilPopped(selectTag);
            resetInsertionMode();
            return;
        }
    } else if(token.type() == HTMLToken::Type::Character
        || token.type() == HTMLToken::Type::SpaceCharacter) {
        insertTextNode(token.data());
        return;
    }

    if(token.type() == HTMLToken::Type::EndOfFile) {
        assert(currentElement()->tagName() != htmlTag);
        handleErrorToken(token);
        return;
    }

    handleErrorToken(token);
}

void HTMLParser::handleInSelectInTableMode(HTMLTokenView& token)
{
    if(token.type() == HTMLToken::Type::StartTag) {
        if(token.tagName() == captionTag
            || token.tagName() == tableTag
            || token.tagName() == tbodyTag
            || token.tagName() == tfootTag
            || token.tagName() == theadTag
            || token.tagName() == trTag
            || token.tagName() == tdTag
            || token.tagName() == thTag) {
            handleErrorToken(token);
            handleFakeEndTagToken(selectTag);
            handleToken(token, m_insertionMode);
            return;
        }
    } else if(token.type() == HTMLToken::Type::EndTag) {
        if(token.tagName() == captionTag
            || token.tagName() == tableTag
            || token.tagName() == tbodyTag
            || token.tagName() == tfootTag
            || token.tagName() == theadTag
            || token.tagName() == trTag
            || token.tagName() == tdTag
            || token.tagName() == thTag) {
            handleErrorToken(token);
            if(!m_openElements.inTableScope(token.tagName())) {
                return;
            }

            handleFakeEndTagToken(selectTag);
            handleToken(token, m_insertionMode);
            return;
        }
    }

    handleInSelectMode(token);
}

void HTMLParser::handleInForeignContentMode(HTMLTokenView& token)
{
    if(token.type() == HTMLToken::Type::StartTag) {
        if(token.tagName() == bTag
            || token.tagName() == bigTag
            || token.tagName() == blockquoteTag
            || token.tagName() == bodyTag
            || token.tagName() == brTag
            || token.tagName() == centerTag
            || token.tagName() == codeTag
            || token.tagName() == ddTag
            || token.tagName() == divTag
            || token.tagName() == dlTag
            || token.tagName() == dtTag
            || token.tagName() == emTag
            || token.tagName() == embedTag
            || isNumberedHeaderTag(token.tagName())
            || token.tagName() == headTag
            || token.tagName() == hrTag
            || token.tagName() == iTag
            || token.tagName() == imgTag
            || token.tagName() == liTag
            || token.tagName() == listingTag
            || token.tagName() == menuTag
            || token.tagName() == metaTag
            || token.tagName() == nobrTag
            || token.tagName() == olTag
            || token.tagName() == pTag
            || token.tagName() == preTag
            || token.tagName() == rubyTag
            || token.tagName() == sTag
            || token.tagName() == smallTag
            || token.tagName() == spanTag
            || token.tagName() == strongTag
            || token.tagName() == strikeTag
            || token.tagName() == subTag
            || token.tagName() == supTag
            || token.tagName() == tableTag
            || token.tagName() == ttTag
            || token.tagName() == uTag
            || token.tagName() == ulTag
            || token.tagName() == varTag
            || (token.tagName() == fontTag && (token.findAttribute(colorAttr) || token.findAttribute(faceAttr) || token.findAttribute(sizeAttr)))) {
            handleErrorToken(token);
            m_openElements.popUntilForeignContentScopeMarker();
            handleToken(token, m_insertionMode);
            return;
        }

        auto uri = currentElement()->namespaceUri();
        if(uri == namespaceuri::mathml) {
            adjustMathMLAttributes(token);
        } else if(uri == namespaceuri::svg) {
            adjustSVGTagNames(token);
            adjustSVGAttributes(token);
        }

        insertForeignElement(token, uri);
        return;
    }

    if(token.type() == HTMLToken::Type::EndTag) {
        auto index = m_openElements.size() - 1;
        auto node = m_openElements.at(index);
        if(node->tagName() != token.tagName())
            handleErrorToken(token);
        do {
            if(node->tagName() == token.tagName()) {
                m_openElements.popUntilPopped(node);
                return;
            }

            index -= 1;
            node = m_openElements.at(index);
            if(node->namespaceUri() == namespaceuri::xhtml)
                break;
        } while(true);

        handleToken(token, m_insertionMode);
        return;
    }

    if(token.type() == HTMLToken::Type::Character
        || token.type() == HTMLToken::Type::SpaceCharacter) {
        insertTextNode(token.data());
        if(token.type() == HTMLToken::Type::Character)
            m_framesetOk = false;
        return;
    }
}

void HTMLParser::handleAfterBodyMode(HTMLTokenView& token)
{
    if(token.type() == HTMLToken::Type::StartTag) {
        if(token.tagName() == htmlTag) {
            handleInBodyMode(token);
            return;
        }
    } else if(token.type() == HTMLToken::Type::EndTag) {
        if(token.tagName() == htmlTag) {
            m_insertionMode = InsertionMode::AfterAfterBody;
            return;
        }
    } else if(token.type() == HTMLToken::Type::SpaceCharacter) {
        handleInBodyMode(token);
        return;
    }

    if(token.type() == HTMLToken::Type::EndOfFile) {
        return;
    }

    handleErrorToken(token);
    m_insertionMode = InsertionMode::InBody;
    handleInBodyMode(token);
}

void HTMLParser::handleInFramesetMode(HTMLTokenView& token)
{
    if(token.type() == HTMLToken::Type::StartTag) {
        if(token.tagName() == htmlTag) {
            handleInBodyMode(token);
            return;
        }

        if(token.tagName() == framesetTag) {
            insertHTMLElement(token);
            return;
        }

        if(token.tagName() == frameTag) {
            insertSelfClosingHTMLElement(token);
            return;
        }

        if(token.tagName() == noframesTag) {
            handleInHeadMode(token);
            return;
        }
    } else if(token.type() == HTMLToken::Type::EndTag) {
        if(token.tagName() == framesetTag) {
            assert(currentElement()->tagName() != htmlTag);
            m_openElements.pop();
            if(currentElement()->tagName() != framesetTag)
                m_insertionMode = InsertionMode::AfterFrameset;
            return;
        }
    } else if(token.type() == HTMLToken::Type::SpaceCharacter) {
        insertTextNode(token.data());
        return;
    }

    if(token.type() == HTMLToken::Type::EndOfFile) {
        assert(currentElement()->tagName() != htmlTag);
        handleErrorToken(token);
        return;
    }

    handleErrorToken(token);
}

void HTMLParser::handleAfterFramesetMode(HTMLTokenView& token)
{
    if(token.type() == HTMLToken::Type::StartTag) {
        if(token.tagName() == htmlTag) {
            handleInBodyMode(token);
            return;
        }

        if(token.tagName() == noframesTag) {
            handleInHeadMode(token);
            return;
        }
    } else if(token.type() == HTMLToken::Type::EndTag) {
        if(token.tagName() == htmlTag) {
            m_insertionMode = InsertionMode::AfterAfterFrameset;
            return;
        }
    } else if(token.type() == HTMLToken::Type::SpaceCharacter) {
        insertTextNode(token.data());
        return;
    }

    if(token.type() == HTMLToken::Type::EndOfFile) {
        return;
    }

    handleErrorToken(token);
}

void HTMLParser::handleAfterAfterBodyMode(HTMLTokenView& token)
{
    if(token.type() == HTMLToken::Type::StartTag) {
        if(token.tagName() == htmlTag) {
            handleInBodyMode(token);
            return;
        }
    } else if(token.type() == HTMLToken::Type::SpaceCharacter
        || token.type() == HTMLToken::Type::DOCTYPE) {
        handleInBodyMode(token);
        return;
    }

    if(token.type() == HTMLToken::Type::EndOfFile) {
        return;
    }

    handleErrorToken(token);
    m_insertionMode = InsertionMode::InBody;
    handleInBodyMode(token);
}

void HTMLParser::handleAfterAfterFramesetMode(HTMLTokenView& token)
{
    if(token.type() == HTMLToken::Type::StartTag) {
        if(token.tagName() == htmlTag) {
            handleInBodyMode(token);
            return;
        }

        if(token.tagName() == noframesTag) {
            handleInHeadMode(token);
            return;
        }
    } else if(token.type() == HTMLToken::Type::SpaceCharacter
        || token.type() == HTMLToken::Type::DOCTYPE) {
        handleInBodyMode(token);
        return;
    }

    if(token.type() == HTMLToken::Type::EndOfFile) {
        return;
    }

    handleErrorToken(token);
}

void HTMLParser::handleFakeStartTagToken(const GlobalString& tagName)
{
    HTMLTokenView token(HTMLToken::Type::StartTag, tagName);
    handleToken(token, m_insertionMode);
}

void HTMLParser::handleFakeEndTagToken(const GlobalString& tagName)
{
    HTMLTokenView token(HTMLToken::Type::EndTag, tagName);
    handleToken(token, m_insertionMode);
}

void HTMLParser::defaultForInBodyEndTagToken(HTMLTokenView& token)
{
    for(int i = m_openElements.size() - 1; i >= 0; --i) {
        auto element = m_openElements.at(i);
        if(element->tagName() == token.tagName()) {
            m_openElements.generateImpliedEndTagsExcept(token.tagName());
            if(currentElement()->tagName() != token.tagName())
                handleErrorToken(token);
            m_openElements.popUntilPopped(element);
            break;
        }

        if(isSpecialElement(element)) {
            handleErrorToken(token);
            break;
        }
    }
}

void HTMLParser::flushPendingTableCharacters()
{
    for(auto cc : m_pendingTableCharacters) {
        if(isspace(cc))
            continue;

        reconstructActiveFormattingElements();
        m_fosterParenting = true;
        insertTextNode(m_pendingTableCharacters);
        m_fosterParenting = false;
        m_framesetOk = false;
        m_insertionMode = m_originalInsertionMode;
        return;
    }

    insertTextNode(m_pendingTableCharacters);
    m_insertionMode = m_originalInsertionMode;
}

void HTMLParser::handleErrorToken(HTMLTokenView& token)
{
}

void HTMLParser::handleRCDataToken(HTMLTokenView& token)
{
    insertHTMLElement(token);
    m_tokenizer.setState(HTMLTokenizer::State::RCDATA);
    m_originalInsertionMode = m_insertionMode;
    m_insertionMode = InsertionMode::Text;
}

void HTMLParser::handleRawTextToken(HTMLTokenView& token)
{
    insertHTMLElement(token);
    m_tokenizer.setState(HTMLTokenizer::State::RAWTEXT);
    m_originalInsertionMode = m_insertionMode;
    m_insertionMode = InsertionMode::Text;
}

void HTMLParser::handleScriptDataToken(HTMLTokenView& token)
{
    insertHTMLElement(token);
    m_tokenizer.setState(HTMLTokenizer::State::ScriptData);
    m_originalInsertionMode = m_insertionMode;
    m_insertionMode = InsertionMode::Text;
}

void HTMLParser::handleDoctypeToken(HTMLTokenView& token)
{
    if(m_insertionMode == InsertionMode::Initial) {
        insertDoctype(token);
        m_insertionMode = InsertionMode::BeforeHTML;
        return;
    }

    if(m_insertionMode == InsertionMode::InTableText) {
        flushPendingTableCharacters();
        handleDoctypeToken(token);
        return;
    }

    handleErrorToken(token);
}

void HTMLParser::handleCommentToken(HTMLTokenView& token)
{
    if(m_insertionMode == InsertionMode::Initial
        || m_insertionMode == InsertionMode::BeforeHTML
        || m_insertionMode == InsertionMode::AfterAfterBody
        || m_insertionMode == InsertionMode::AfterAfterFrameset) {
        insertComment(token, m_document);
        return;
    }

    if(m_insertionMode == InsertionMode::AfterBody) {
        insertComment(token, m_openElements.htmlElement());
        return;
    }

    if(m_insertionMode == InsertionMode::InTableText) {
        flushPendingTableCharacters();
        handleCommentToken(token);
        return;
    }

    insertComment(token, m_openElements.top());
}

void HTMLParser::handleToken(HTMLTokenView& token, InsertionMode mode)
{
    switch(mode) {
    case InsertionMode::Initial:
        return handleInitialMode(token);
    case InsertionMode::BeforeHTML:
        return handleBeforeHTMLMode(token);
    case InsertionMode::BeforeHead:
        return handleBeforeHeadMode(token);
    case InsertionMode::InHead:
        return handleInHeadMode(token);
    case InsertionMode::InHeadNoscript:
        return handleInHeadNoscriptMode(token);
    case InsertionMode::AfterHead:
        return handleAfterHeadMode(token);
    case InsertionMode::InBody:
        return handleInBodyMode(token);
    case InsertionMode::Text:
        return handleTextMode(token);
    case InsertionMode::InTable:
        return handleInTableMode(token);
    case InsertionMode::InTableText:
        return handleInTableTextMode(token);
    case InsertionMode::InCaption:
        return handleInCaptionMode(token);
    case InsertionMode::InColumnGroup:
        return handleInColumnGroupMode(token);
    case InsertionMode::InTableBody:
        return handleInTableBodyMode(token);
    case InsertionMode::InRow:
        return handleInRowMode(token);
    case InsertionMode::InCell:
        return handleInCellMode(token);
    case InsertionMode::InSelect:
        return handleInSelectMode(token);
    case InsertionMode::InSelectInTable:
        return handleInSelectInTableMode(token);
    case InsertionMode::InForeignContent:
        return handleInForeignContentMode(token);
    case InsertionMode::AfterBody:
        return handleAfterBodyMode(token);
    case InsertionMode::InFrameset:
        return handleInFramesetMode(token);
    case InsertionMode::AfterFrameset:
        return handleAfterFramesetMode(token);
    case InsertionMode::AfterAfterBody:
        return handleAfterAfterBodyMode(token);
    case InsertionMode::AfterAfterFrameset:
        return handleAfterAfterFramesetMode(token);
    }
}

HTMLParser::HTMLParser(HTMLDocument* document, const std::string_view& content)
    : m_document(document), m_tokenizer(content, document->heap())
{
}

bool HTMLParser::parse()
{
    m_document->beginParsingChildren();
    while(!m_tokenizer.atEOF()) {
        auto token = m_tokenizer.nextToken();
        if(token.type() == HTMLToken::Type::DOCTYPE) {
            handleDoctypeToken(token);
            continue;
        }

        if(token.type() == HTMLToken::Type::Comment) {
            handleCommentToken(token);
            continue;
        }

        if(m_skipLeadingNewline
            && token.type() == HTMLToken::Type::SpaceCharacter) {
            token.skipLeadingNewLine();
        }

        m_skipLeadingNewline = false;
        handleToken(token, currentInsertionMode(token));
    }

    assert(!m_openElements.empty());
    m_openElements.popAll();
    m_document->finishParsingChildren();
    return true;
}

} // namespace htmlbook
