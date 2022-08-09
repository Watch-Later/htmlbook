#include "htmlbook.h"
#include "document.h"
#include "resource.h"

namespace htmlbook {

const PageSize PageSize::A5(148, 210, PageUnit::Millimeters);
const PageSize PageSize::A4(210, 297, PageUnit::Millimeters);
const PageSize PageSize::A3(297, 420, PageUnit::Millimeters);
const PageSize PageSize::B5(176, 250, PageUnit::Millimeters);
const PageSize PageSize::B4(250, 353, PageUnit::Millimeters);
const PageSize PageSize::Letter(8.5, 11, PageUnit::Inches);
const PageSize PageSize::Legal(8.5, 14, PageUnit::Inches);
const PageSize PageSize::Ledger(11, 17, PageUnit::Inches);

Book::Book(const PageSize& pageSize)
    : m_document(new Document(pageSize))
{
}

Book::~Book() = default;

void Book::setBaseUrl(std::string_view baseUrl)
{
    m_document->setBaseUrl(baseUrl);
}

std::string_view Book::baseUrl() const
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

void Book::setUserStyleSheet(std::string_view content)
{
    m_document->setUserStyleSheet(content);
}

void Book::clearUserStyleSheet()
{
    m_document->clearUserStyleSheet();
}

void Book::save(const std::string& filename)
{
}

void Book::serialize(std::ostream& o) const
{
    m_document->serialize(o);
}

Document* Book::document() const
{
    return m_document.get();
}

bool Book::addFontData(const std::string& family, bool italic, bool smallCaps, int weight, std::vector<char> data)
{
    auto face = FontFace::create(std::move(data));
    if(face == nullptr)
        return false;
    fontCache()->addFont(family, italic, smallCaps, weight, std::move(face));
    return true;
}

} // namespace htmlbook
