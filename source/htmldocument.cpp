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

HTMLImageElement::HTMLImageElement(Document* document)
    : HTMLElement(document, htmlnames::imgTag)
{
}

void HTMLImageElement::parseAttribute(const GlobalString& name, const std::string_view& value)
{
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
