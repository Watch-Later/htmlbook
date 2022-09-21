#include "htmldocument.h"
#include "htmlparser.h"
#include "resource.h"
#include "box.h"
#include "counter.h"

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
    if(pseudoType == PseudoType::Marker && !parent->isListItemBox())
        return;
    auto style = document()->pseudoStyleForElement(this, *parent->style(), pseudoType);
    if(style == nullptr || style->display() == Display::None)
        return;

    auto box = Box::create(nullptr, style);
    parent->addBox(box);
    if(pseudoType == PseudoType::Before || pseudoType == PseudoType::After) {
        counters.update(*style);
        buildPseudoBox(counters, box, PseudoType::Marker);
    }

    auto addText = [&](const auto& text) {
        if(text.empty())
            return;
        auto lastBox = box->lastBox();
        if(lastBox && lastBox->isTextBox()) {
            auto textBox = to<TextBox>(lastBox);
            textBox->appendText(text);
            return;
        }

        auto textBox = new TextBox(nullptr, style);
        textBox->setText(text);
        box->addBox(textBox);
    };

    auto addImage = [&](const auto& image) {
        if(image == nullptr)
            return;
        auto imageBox = new ImageBox(nullptr, style);
        imageBox->setImage(image);
        box->addBox(imageBox);
    };

    auto content = style->get(CSSPropertyID::Content);
    if(content == nullptr || !content->isListValue()) {
        if(pseudoType != PseudoType::Marker)
            return;
        if(auto image = style->listStyleImage()) {
            addImage(image);
            return;
        }

        static const GlobalString listItem("list-item");
        addText(counters.format(listItem, style->listStyleType(), emptyString));
        return;
    }

    for(auto& value : to<CSSListValue>(*content)->values()) {
        if(auto string = to<CSSStringValue>(*value)) {
            addText(string->value());
        } else if(auto image = to<CSSImageValue>(*value)) {
            addImage(image->fetch(document()));
        } else if(auto counter = to<CSSCounterValue>(*value)) {
            addText(counters.format(counter->identifier(), counter->listStyle(), counter->separator()));
        } else if(auto ident = to<CSSIdentValue>(*value)) {
            auto usequote = (ident->value() == CSSValueID::OpenQuote || ident->value() == CSSValueID::CloseQuote);
            auto openquote = (ident->value() == CSSValueID::OpenQuote || ident->value() == CSSValueID::NoOpenQuote);
            if(counters.quoteDepth() > 0 && !openquote)
                counters.decreaseQuoteDepth();
            if(usequote)
                addText(style->getQuote(openquote, counters.quoteDepth()));
            if(openquote)
                counters.increaseQuoteDepth();
        } else {
            auto function = to<CSSFunctionValue>(*value);
            auto name = to<CSSCustomIdentValue>(*function->front());
            auto attribute = findAttribute(name->value());
            if(attribute == nullptr)
                continue;
            addText(attribute->value());
        }
    }
}

void HTMLElement::buildBox(Counters& counters, Box* parent)
{
    auto style = document()->styleForElement(this, *parent->style());
    if(style == nullptr || style->display() == Display::None)
        return;
    auto box = createBox(style);
    if(box == nullptr)
        return;
    parent->addBox(box);
    counters.push();
    counters.update(*style);
    buildPseudoBox(counters, box, PseudoType::Marker);
    buildPseudoBox(counters, box, PseudoType::Before);
    ContainerNode::buildBox(counters, box);
    buildPseudoBox(counters, box, PseudoType::After);
    counters.pop();
}

HTMLBodyElement::HTMLBodyElement(Document* document)
    : HTMLElement(document, bodyTag)
{
}

void HTMLBodyElement::collectAttributeStyle(const GlobalString& name, const std::string& value, std::string& output) const
{
    if(name == textAttr) {
        addAttributeStyle("color", value, output);
    } else if(name == bgcolorAttr) {
        addAttributeStyle("background-color", value, output);
    } else if(name == backgroundAttr) {
        addAttributeStyle("background-image", "url(" + value + ")", output);
    } else {
        HTMLElement::collectAttributeStyle(name, value, output);
    }
}

HTMLImageElement::HTMLImageElement(Document* document)
    : HTMLElement(document, imgTag)
{
}

void HTMLImageElement::collectAttributeStyle(const GlobalString& name, const std::string& value, std::string& output) const
{
    if(name == widthAttr) {
        addAttributeStyle(name, value, output);
    } else if(name == heightAttr) {
        addAttributeStyle(name, value, output);
    } else if(name == valignAttr) {
        addAttributeStyle("vertical-align", value, output);
    } else if(name == borderAttr) {
        addAttributeStyle("border-width", value, output);
    } else if(name == hspaceAttr) {
        addAttributeStyle("margin-left", value, output);
        addAttributeStyle("margin-right", value, output);
    } else if(name == vspaceAttr) {
        addAttributeStyle("margin-top", value, output);
        addAttributeStyle("margin-bottom", value, output);
    } else {
        HTMLElement::collectAttributeStyle(name, value, output);
    }
}

const std::string& HTMLImageElement::src() const
{
    return getAttribute(srcAttr);
}

const std::string& HTMLImageElement::altText() const
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
    auto box = new ImageBox(this, style);
    box->setImage(image());
    box->setAlternativeText(altText());
    return box;
}

HTMLFontElement::HTMLFontElement(Document* document)
    : HTMLElement(document, fontTag)
{
}

void HTMLFontElement::collectAttributeStyle(const GlobalString& name, const std::string& value, std::string& output) const
{
    if(name == colorAttr) {
        addAttributeStyle(name, value, output);
    } else if(name == faceAttr) {
        addAttributeStyle("font-family", value, output);
    } else {
        HTMLElement::collectAttributeStyle(name, value, output);
    }
}

HTMLHRElement::HTMLHRElement(Document* document)
    : HTMLElement(document, hrTag)
{
}

void HTMLHRElement::collectAttributeStyle(const GlobalString& name, const std::string& value, std::string& output) const
{
    if(name == widthAttr) {
        addAttributeStyle(name, value, output);
    } else if(name == colorAttr) {
        addAttributeStyle("border-color", value, output);
    } else {
        HTMLElement::collectAttributeStyle(name, value, output);
    }
}

HTMLLIElement::HTMLLIElement(Document* document)
    : HTMLElement(document, liTag)
{
}

void HTMLLIElement::collectAttributeStyle(const GlobalString& name, const std::string& value, std::string& output) const
{
    if(name == valueAttr) {
        addAttributeStyle("counter-reset", "list-item " + value, output);
    } else {
        HTMLElement::collectAttributeStyle(name, value, output);
    }
}

HTMLOLElement::HTMLOLElement(Document* document)
    : HTMLElement(document, olTag)
{
}

void HTMLOLElement::collectAttributeStyle(const GlobalString& name, const std::string& value, std::string& output) const
{
    if(name == startAttr) {
        addAttributeStyle("counter-reset", "list-item " + value, output);
    } else {
        HTMLElement::collectAttributeStyle(name, value, output);
    }
}

HTMLTableElement::HTMLTableElement(Document* document)
    : HTMLElement(document, tableTag)
{
}

void HTMLTableElement::collectAttributeStyle(const GlobalString& name, const std::string& value, std::string& output) const
{
    if(name == widthAttr) {
        addAttributeStyle(name, value, output);
    } else if(name == heightAttr) {
        addAttributeStyle(name, value, output);
    } else if(name == valignAttr) {
        addAttributeStyle("vertical-align", value, output);
    } else if(name == cellspacingAttr) {
        addAttributeStyle("border-spacing", value, output);
    } else if(name == borderAttr) {
        addAttributeStyle("border-width", value, output);
    } else if(name == bordercolorAttr) {
        addAttributeStyle("border-color", value, output);
    } else if(name == bgcolorAttr) {
        addAttributeStyle("background-color", value, output);
    } else if(name == backgroundAttr) {
        addAttributeStyle("background-image", "url(" + value + ")", output);
    } else {
        HTMLElement::collectAttributeStyle(name, value, output);
    }
}

HTMLTableSectionElement::HTMLTableSectionElement(Document* document, const GlobalString& tagName)
    : HTMLElement(document, tagName)
{
}

void HTMLTableSectionElement::collectAttributeStyle(const GlobalString& name, const std::string& value, std::string& output) const
{
    if(name == heightAttr) {
        addAttributeStyle(name, value, output);
    } else if(name == bgcolorAttr) {
        addAttributeStyle("background-color", value, output);
    } else if(name == backgroundAttr) {
        addAttributeStyle("background-image", "url(" + value + ")", output);
    } else {
        HTMLElement::collectAttributeStyle(name, value, output);
    }
}

HTMLTableCaptionElement::HTMLTableCaptionElement(Document* document)
    : HTMLElement(document, captionTag)
{
}

void HTMLTableCaptionElement::collectAttributeStyle(const GlobalString& name, const std::string& value, std::string& output) const
{
    if(name == widthAttr) {
        addAttributeStyle(name, value, output);
    } else {
        HTMLElement::collectAttributeStyle(name, value, output);
    }
}

HTMLTableRowElement::HTMLTableRowElement(Document* document)
    : HTMLElement(document, trTag)
{
}

void HTMLTableRowElement::collectAttributeStyle(const GlobalString& name, const std::string& value, std::string& output) const
{
    if(name == heightAttr) {
        addAttributeStyle(name, value, output);
    } else if(name == bgcolorAttr) {
        addAttributeStyle("background-color", value, output);
    } else if(name == backgroundAttr) {
        addAttributeStyle("background-image", "url(" + value + ")", output);
    } else {
        HTMLElement::collectAttributeStyle(name, value, output);
    }
}

HTMLTableColElement::HTMLTableColElement(Document* document, const GlobalString& tagName)
    : HTMLElement(document, tagName)
{
}

void HTMLTableColElement::collectAttributeStyle(const GlobalString& name, const std::string& value, std::string& output) const
{
    if(name == widthAttr) {
        addAttributeStyle(name, value, output);
    } else if(name == heightAttr) {
        addAttributeStyle(name, value, output);
    } else if(name == bgcolorAttr) {
        addAttributeStyle("background-color", value, output);
    } else if(name == backgroundAttr) {
        addAttributeStyle("background-image", "url(" + value + ")", output);
    } else {
        HTMLElement::collectAttributeStyle(name, value, output);
    }
}

int HTMLTableColElement::span() const
{
    return 0;
}

Box* HTMLTableColElement::createBox(const RefPtr<BoxStyle> &style)
{
    auto box = HTMLElement::createBox(style);
    if(box->isTableColumnBox()) {
        auto column = to<TableColumnBox>(*box);
        column->setSpan(span());
    }

    return box;
}

HTMLTableCellElement::HTMLTableCellElement(Document* document, const GlobalString& tagName)
    : HTMLElement(document, tagName)
{
}

void HTMLTableCellElement::collectAttributeStyle(const GlobalString& name, const std::string& value, std::string& output) const
{
    if(name == widthAttr) {
        addAttributeStyle(name, value, output);
    } else if(name == heightAttr) {
        addAttributeStyle(name, value, output);
    } else if(name == bgcolorAttr) {
        addAttributeStyle("background-color", value, output);
    } else if(name == backgroundAttr) {
        addAttributeStyle("background-image", "url(" + value + ")", output);
    } else {
        HTMLElement::collectAttributeStyle(name, value, output);
    }
}

int HTMLTableCellElement::colSpan() const
{
    return 0;
}

int HTMLTableCellElement::rowSpan() const
{
    return 0;
}

Box* HTMLTableCellElement::createBox(const RefPtr<BoxStyle>& style)
{
    auto box = HTMLElement::createBox(style);
    if(box->isTableCellBox()) {
        auto cell = to<TableCellBox>(*box);
        cell->setColSpan(colSpan());
        cell->setRowSpan(rowSpan());
    }

    return box;
}

HTMLDocument::HTMLDocument(const PageSize& pageSize)
    : Document(pageSize)
{
}

bool HTMLDocument::load(const std::string_view& content)
{
    return HTMLParser(this, content).parse();
}

Box* HTMLDocument::createBox(const RefPtr<BoxStyle>& style)
{
    return new BlockBox(this, style);
}

} // namespace htmlbook
