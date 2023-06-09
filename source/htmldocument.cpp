#include "htmldocument.h"
#include "htmlparser.h"
#include "htmlbook.h"
#include "resource.h"
#include "counters.h"
#include "replacedbox.h"
#include "textbox.h"
#include "listitembox.h"
#include "tablebox.h"

namespace htmlbook {

HTMLElement::HTMLElement(Document* document, const GlobalString& tagName)
    : Element(document, tagName, namespaceuri::xhtml)
{
}

Box* HTMLElement::createBox(const RefPtr<BoxStyle>& style)
{
    return Box::create(this, style);
}

void HTMLElement::buildPseudoBox(Counters& counters, Box* parent, PseudoType pseudoType)
{
    if(pseudoType == PseudoType::Marker && !is<ListItemBox>(parent))
        return;
    auto style = document()->pseudoStyleForElement(this, parent->style(), pseudoType);
    if(style == nullptr || style->display() == Display::None)
        return;

    auto box = Box::create(nullptr, style);
    parent->addBox(box);
    if(pseudoType == PseudoType::Before || pseudoType == PseudoType::After) {
        counters.update(box);
        buildPseudoBox(counters, box, PseudoType::Marker);
    }

    auto addText = [&](const auto& text) {
        if(text.empty())
            return;
        auto newBox = new (heap()) TextBox(nullptr, style);
        newBox->setText(text);
        box->addBox(newBox);
    };

    auto addImage = [&](const auto& image) {
        if(image == nullptr)
            return;
        auto newBox = new (heap()) ImageBox(nullptr, style);
        newBox->setImage(image);
        box->addBox(newBox);
    };

    auto content = style->get(CSSPropertyID::Content);
    if(content == nullptr || !is<CSSListValue>(*content)) {
        if(pseudoType != PseudoType::Marker || (content && content->id() == CSSValueID::None))
            return;
        if(auto image = style->listStyleImage()) {
            addImage(image);
            return;
        }

        static const GlobalString listItem("list-item");
        addText(counters.format(listItem, style->listStyleType(), emptyGlo));
        return;
    }

    for(auto& value : to<CSSListValue>(*content).values()) {
        if(auto string = to<CSSStringValue>(value)) {
            addText(string->value());
        } else if(auto image = to<CSSImageValue>(value)) {
            addImage(image->fetch(document()));
        } else if(auto counter = to<CSSCounterValue>(value)) {
            addText(counters.format(counter->identifier(), counter->listStyle(), counter->separator()));
        } else if(auto ident = to<CSSIdentValue>(value)) {
            auto usequote = (ident->value() == CSSValueID::OpenQuote || ident->value() == CSSValueID::CloseQuote);
            auto openquote = (ident->value() == CSSValueID::OpenQuote || ident->value() == CSSValueID::NoOpenQuote);
            if(counters.quoteDepth() > 0 && !openquote)
                counters.decreaseQuoteDepth();
            if(usequote)
                addText(style->getQuote(openquote, counters.quoteDepth()));
            if(openquote) {
                counters.increaseQuoteDepth();
            }
        } else {
            auto& function = to<CSSFunctionValue>(*value);
            auto& name = to<CSSCustomIdentValue>(*function.front());
            for(auto& attribute : attributes()) {
                if(attribute.name() == name.value()) {
                    addText(attribute.value());
                    break;
                }
            }
        }
    }
}

void HTMLElement::buildBox(Counters& counters, Box* parent)
{
    auto style = document()->styleForElement(this, parent->style());
    if(style == nullptr || style->display() == Display::None)
        return;
    auto box = createBox(style);
    if(box == nullptr)
        return;
    parent->addBox(box);
    counters.push();
    counters.update(box);
    buildPseudoBox(counters, box, PseudoType::Marker);
    buildPseudoBox(counters, box, PseudoType::Before);
    ContainerNode::buildBox(counters, box);
    buildPseudoBox(counters, box, PseudoType::After);
    counters.pop();
}

template<typename T>
bool HTMLElement::parseHTMLInteger(const GlobalString& name, T& value) const
{
    std::string_view input = getAttribute(name);
    while(!input.empty() && isspace(input.front())) {
        input.remove_prefix(1);
    }

    constexpr auto isSigned = std::numeric_limits<T>::is_signed;

    bool isNegative = false;
    if(!input.empty() && input.front() == '+')
        input.remove_prefix(1);
    else if(!input.empty() && isSigned && input.front() == '-') {
        input.remove_prefix(1);
        isNegative = true;
    }

    if(input.empty() || !isdigit(input.front()))
        return false;

    T integer = 0;
    do {
        integer = integer * 10 + input.front() - '0';
        input.remove_prefix(1);
    } while(!input.empty() && isdigit(input.front()));

    using SignedType = typename std::make_signed<T>::type;
    if(isNegative)
        value = -static_cast<SignedType>(integer);
    else
        value = integer;
    return true;
}

HTMLBodyElement::HTMLBodyElement(Document* document)
    : HTMLElement(document, bodyTag)
{
}

void HTMLBodyElement::collectAttributeStyle(std::stringstream& output, const GlobalString& name, const HeapString& value) const
{
    if(name == textAttr) {
        output << "color:" << value;
    } else if(name == bgcolorAttr) {
        output << "background-color:" << value;
    } else if(name == backgroundAttr) {
        output << "background-image:" << "url(" << value << ')';
    } else {
        HTMLElement::collectAttributeStyle(output, name, value);
    }
}

HTMLImageElement::HTMLImageElement(Document* document)
    : HTMLElement(document, imgTag)
{
}

void HTMLImageElement::collectAttributeStyle(std::stringstream& output, const GlobalString& name, const HeapString& value) const
{
    if(name == widthAttr) {
        output << "width:" << value;
    } else if(name == heightAttr) {
        output << "height:" << value;
    } else if(name == valignAttr) {
        output << "vertical-align:" << value;
    } else if(name == borderAttr) {
        output << "border-width:" << value;
    } else if(name == hspaceAttr) {
        output << "margin-left:" << value << ';' << "margin-right:" << value;
    } else if(name == vspaceAttr) {
        output << "margin-top:" << value << ';' << "margin-bottom:" << value;
    } else {
        HTMLElement::collectAttributeStyle(output, name, value);
    }
}

const HeapString& HTMLImageElement::src() const
{
    return getAttribute(srcAttr);
}

const HeapString& HTMLImageElement::altText() const
{
    return getAttribute(altAttr);
}

RefPtr<Image> HTMLImageElement::image() const
{
    auto resource = document()->fetchImageResource(src());
    if(resource == nullptr)
        return nullptr;
    return resource->image();
}

Box* HTMLImageElement::createBox(const RefPtr<BoxStyle>& style)
{
    auto box = new (heap()) ImageBox(this, style);
    box->setImage(image());
    box->setAlternativeText(altText());
    return box;
}

HTMLFontElement::HTMLFontElement(Document* document)
    : HTMLElement(document, fontTag)
{
}

void HTMLFontElement::collectAttributeStyle(std::stringstream& output, const GlobalString& name, const HeapString& value) const
{
    if(name == colorAttr) {
        output << "color:" << value;
    } else if(name == faceAttr) {
        output << "font-family:" << value;
    } else {
        HTMLElement::collectAttributeStyle(output, name, value);
    }
}

HTMLHRElement::HTMLHRElement(Document* document)
    : HTMLElement(document, hrTag)
{
}

void HTMLHRElement::collectAttributeStyle(std::stringstream& output, const GlobalString& name, const HeapString& value) const
{
    if(name == widthAttr) {
        output << "width:" << value;
    } else if(name == colorAttr) {
        output << "border-color:" << value;
    } else {
        HTMLElement::collectAttributeStyle(output, name, value);
    }
}

HTMLLIElement::HTMLLIElement(Document* document)
    : HTMLElement(document, liTag)
{
}

std::optional<int> HTMLLIElement::value() const
{
    static const GlobalString valueAttr("value");

    int value = 0;
    if(parseHTMLInteger(valueAttr, value))
        return value;
    return std::nullopt;
}

HTMLOLElement::HTMLOLElement(Document* document)
    : HTMLElement(document, olTag)
{
}

int HTMLOLElement::start() const
{
    static const GlobalString startAttr("start");

    int value = 1;
    if(parseHTMLInteger(startAttr, value))
        return value;
    return 1;
}

HTMLTableElement::HTMLTableElement(Document* document)
    : HTMLElement(document, tableTag)
{
}

void HTMLTableElement::collectAttributeStyle(std::stringstream& output, const GlobalString& name, const HeapString& value) const
{
    if(name == widthAttr) {
        output << "width:" << value;
    } else if(name == heightAttr) {
        output << "height:" << value;
    } else if(name == valignAttr) {
        output << "vertical-align:" << value;
    } else if(name == cellspacingAttr) {
        output << "border-spacing:" << value;;
    } else if(name == borderAttr) {
        output << "border-width:" << value;
    } else if(name == bordercolorAttr) {
        output << "border-color:" << value;
    } else if(name == bgcolorAttr) {
        output << "background-color:" << value;
    } else if(name == backgroundAttr) {
        output << "background-image:" << "url(" << value << ')';
    } else {
        HTMLElement::collectAttributeStyle(output, name, value);
    }
}

HTMLTableSectionElement::HTMLTableSectionElement(Document* document, const GlobalString& tagName)
    : HTMLElement(document, tagName)
{
}

void HTMLTableSectionElement::collectAttributeStyle(std::stringstream& output, const GlobalString& name, const HeapString& value) const
{
    if(name == heightAttr) {
        output << "height:" << value;
    } else if(name == bgcolorAttr) {
        output << "background-color:" << value;
    } else if(name == backgroundAttr) {
        output << "background-image:" << "url(" << value << ')';
    } else {
        HTMLElement::collectAttributeStyle(output, name, value);
    }
}

HTMLTableCaptionElement::HTMLTableCaptionElement(Document* document)
    : HTMLElement(document, captionTag)
{
}

void HTMLTableCaptionElement::collectAttributeStyle(std::stringstream& output, const GlobalString& name, const HeapString& value) const
{
    if(name == widthAttr) {
        output << "width:" << value;
    } else {
        HTMLElement::collectAttributeStyle(output, name, value);
    }
}

HTMLTableRowElement::HTMLTableRowElement(Document* document)
    : HTMLElement(document, trTag)
{
}

void HTMLTableRowElement::collectAttributeStyle(std::stringstream& output, const GlobalString& name, const HeapString& value) const
{
    if(name == heightAttr) {
        output << "height:" << value;
    } else if(name == bgcolorAttr) {
        output << "background-color:" << value;
    } else if(name == backgroundAttr) {
        output << "background-image:" << "url(" << value << ')';
    } else {
        HTMLElement::collectAttributeStyle(output, name, value);
    }
}

HTMLTableColElement::HTMLTableColElement(Document* document, const GlobalString& tagName)
    : HTMLElement(document, tagName)
{
}

void HTMLTableColElement::collectAttributeStyle(std::stringstream& output, const GlobalString& name, const HeapString& value) const
{
    if(name == widthAttr) {
        output << "width:" << value;
    } else if(name == heightAttr) {
        output << "height:" << value;
    } else if(name == bgcolorAttr) {
        output << "background-color:" << value;
    } else if(name == backgroundAttr) {
        output << "background-image:" << "url(" << value << ')';
    } else {
        HTMLElement::collectAttributeStyle(output, name, value);
    }
}

unsigned HTMLTableColElement::span() const
{
    static const GlobalString spanAttr("span");

    unsigned value = 1;
    if(parseHTMLInteger(spanAttr, value))
        return std::max(1u, value);
    return 1;
}

Box* HTMLTableColElement::createBox(const RefPtr<BoxStyle>& style)
{
    auto box = HTMLElement::createBox(style);
    if(auto column = to<TableColumnBox>(box))
        column->setSpan(span());
    return box;
}

HTMLTableCellElement::HTMLTableCellElement(Document* document, const GlobalString& tagName)
    : HTMLElement(document, tagName)
{
}

void HTMLTableCellElement::collectAttributeStyle(std::stringstream& output, const GlobalString& name, const HeapString& value) const
{
    if(name == widthAttr) {
        output << "width:" << value;
    } else if(name == heightAttr) {
        output << "height:" << value;
    } else if(name == bgcolorAttr) {
        output << "background-color:" << value;
    } else if(name == backgroundAttr) {
        output << "background-image:" << "url(" << value << ')';
    } else {
        HTMLElement::collectAttributeStyle(output, name, value);
    }
}

unsigned HTMLTableCellElement::colSpan() const
{
    static const GlobalString colspanAttr("colspan");

    unsigned value = 1;
    if(parseHTMLInteger(colspanAttr, value))
        return std::max(1u, value);
    return 1;
}

unsigned HTMLTableCellElement::rowSpan() const
{
    static const GlobalString rowSpanAttr("rowspan");

    unsigned value = 1;
    if(parseHTMLInteger(rowSpanAttr, value))
        return value;
    return 1;
}

Box* HTMLTableCellElement::createBox(const RefPtr<BoxStyle>& style)
{
    auto box = HTMLElement::createBox(style);
    if(auto cell = to<TableCellBox>(box)) {
        cell->setColSpan(colSpan());
        cell->setRowSpan(rowSpan());
    }

    return box;
}

HTMLStyleElement::HTMLStyleElement(Document* document)
    : HTMLElement(document, styleTag)
{
}

const HeapString& HTMLStyleElement::type() const
{
    return getAttribute(typeAttr);
}

void HTMLStyleElement::finishParsingChildren()
{
    auto& type = getAttribute(typeAttr);
    if(!type.empty() && !equals(type, "text/css", false))
        return;
    std::string content;
    auto child = firstChild();
    while(child) {
        if(auto node = to<TextNode>(child))
            content += node->data();
        child = child->nextSibling();
    }

    document()->addStyleSheet(content);
}

HTMLLinkElement::HTMLLinkElement(Document* document)
    : HTMLElement(document, linkTag)
{
}

const HeapString& HTMLLinkElement::href() const
{
    return getAttribute(hrefAttr);
}

const HeapString& HTMLLinkElement::type() const
{
    return getAttribute(typeAttr);
}

const HeapString& HTMLLinkElement::rel() const
{
    return getAttribute(relAttr);
}

void HTMLLinkElement::finishParsingChildren()
{
    if(!equals(rel(), "stylesheet", false) && !equals(type(), "text/css", false))
        return;
    auto resource = document()->fetchTextResource(href());
    if(resource == nullptr)
        return;
    document()->addStyleSheet(resource->text());
}

std::unique_ptr<HTMLDocument> HTMLDocument::create(Book* book)
{
    return std::unique_ptr<HTMLDocument>(new (book->heap()) HTMLDocument(book));
}

HTMLDocument::HTMLDocument(Book* book)
    : Document(book->heap())
    , m_book(book)
{
}

bool HTMLDocument::load(const std::string_view& content)
{
    return HTMLParser(this, content).parse();
}

float HTMLDocument::viewportWidth() const
{
    return m_book->viewportWidth();
}

float HTMLDocument::viewportHeight() const
{
    return m_book->viewportHeight();
}

} // namespace htmlbook
