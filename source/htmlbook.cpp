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

Book::Book(std::pmr::memory_resource* upstream)
    : Book(PageSize::A4, PageOrientation::Portrait, PageMargins(), upstream)
{
}

Book::Book(const PageSize& size, PageOrientation orientation, const PageMargins& margins, std::pmr::memory_resource* upstream)
    : m_pageSize(size)
    , m_pageOrientation(orientation)
    , m_pageMargins(margins)
    , m_heap(1024* 5, upstream)
{
}

Book::~Book()
{
    delete m_document;
}

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
    delete m_document;
    m_heap.release();

    m_document = new (&m_heap) HTMLDocument(&m_heap, this);
    m_document->setBaseUrl(baseUrl);
    m_document->load(content);
    m_document->addStyleSheet(userStyle);
}

void Book::save(const std::string& filename)
{
}

void Book::serialize(std::ostream& o) const
{
    m_document->serialize(o);
}

} // namespace htmlbook
