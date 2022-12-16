#include "htmlbook.h"
#include "htmldocument.h"
#include "resource.h"

namespace htmlbook {

const PageSize PageSize::A3(842, 1191);
const PageSize PageSize::A4(595, 842);
const PageSize PageSize::A5(420, 595);
const PageSize PageSize::B4(709, 1001);
const PageSize PageSize::B5(499, 709);
const PageSize PageSize::Letter(612, 792);
const PageSize PageSize::Legal(612, 1008);
const PageSize PageSize::Ledger(1224, 792);

Book::Book(const PageSize& size, PageOrientation orientation, const PageMargins& margins)
    : m_document(new HTMLDocument(size, orientation, margins))
{
}

Book::~Book() = default;

void Book::setBaseUrl(std::string_view baseUrl)
{
    m_document->setBaseUrl(baseUrl);
}

const std::string& Book::baseUrl() const
{
    return m_document->baseUrl();
}

void Book::loadUrl(std::string_view url)
{
    auto textResource = m_document->fetchTextResource(url);
    if(textResource == nullptr)
        return;
    load(textResource->text());
}

void Book::load(const char* data, size_t length, std::string_view textEncoding)
{
    load(TextResource::decode(data, length, "text/html", textEncoding));
}

void Book::load(std::string_view content)
{
    m_document->load(content);
}

void Book::addUserStyleSheet(std::string_view content)
{
    m_document->addUserStyleSheet(content);
}

void Book::save(const std::string& filename)
{
}

void Book::serialize(std::ostream& o) const
{
    m_document->serialize(o);
}

HTMLDocument* Book::document() const
{
    return m_document.get();
}

} // namespace htmlbook
