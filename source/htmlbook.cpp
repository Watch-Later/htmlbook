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
    : m_pageSize(size)
    , m_pageOrientation(orientation)
    , m_pageMargins(margins)
{
}

Book::~Book() = default;

void Book::loadUrl(const std::string_view& url, const std::string_view& userStyle)
{
    std::string mimeType;
    std::string textEncoding;
    std::vector<char> data;
    if(!resourceLoader()->loadUrl(url, mimeType, textEncoding, data))
        return;
    load(TextResource::decode(data.data(), data.size(), mimeType, textEncoding), url, userStyle);
}

void Book::loadData(const char* data, size_t length, const std::string_view& textEncoding, const std::string_view& baseUrl, const std::string_view& userStyle)
{
    load(TextResource::decode(data, length, "text/html", textEncoding), baseUrl, userStyle);
}

void Book::load(const std::string_view& content, const std::string_view& baseUrl, const std::string_view& userStyle)
{
    m_document.reset();
    m_heap = std::make_unique<Heap>(1024 * 25);

    m_document = HTMLDocument::create(this);
    m_document->setBaseUrl(baseUrl);
    m_document->load(content);
    m_document->addStyleSheet(userStyle);
}

void Book::clear()
{
    m_document.reset();
    m_heap.reset();
}

bool Book::empty()
{
    return !!m_document;
}

void Book::save(const std::string& filename)
{
}

void Book::serialize(std::ostream& o) const
{
    if(m_document)
        m_document->serialize(o);
}

} // namespace htmlbook
