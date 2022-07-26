#include "htmlbook.h"
#include "document.h"
#include "resource.h"

#include <cstring>

namespace htmlbook {

const PageSize PageSize::A5(148, 210, PageUnit::Millimeters);
const PageSize PageSize::A4(210, 297, PageUnit::Millimeters);
const PageSize PageSize::A3(297, 420, PageUnit::Millimeters);
const PageSize PageSize::B5(176, 250, PageUnit::Millimeters);
const PageSize PageSize::B4(250, 353, PageUnit::Millimeters);
const PageSize PageSize::Letter(8.5, 11, PageUnit::Inches);
const PageSize PageSize::Legal(8.5, 14, PageUnit::Inches);
const PageSize PageSize::Ledger(11, 17, PageUnit::Inches);

std::shared_ptr<ByteData> ByteData::create(const uint8_t* data, size_t length)
{
    uint8_t* newdata;
    auto resource = createUninitialized(newdata, length);
    std::memcpy(newdata, data, length);
    return resource;
}

std::shared_ptr<ByteData> ByteData::createUninitialized(uint8_t*& data, size_t length)
{
    auto newdata = new uint8_t[length + sizeof(ByteData)];
    data = newdata + sizeof(ByteData);
    return std::shared_ptr<ByteData>(new (newdata) ByteData(data, length));
}

std::shared_ptr<ByteData> ByteData::createStatic(const uint8_t* data, size_t length)
{
    return std::shared_ptr<ByteData>(new ByteData(data, length));
}

ByteData::ByteData(const uint8_t* data, size_t length)
    : m_data(data), m_length(length)
{
}

Book::Book(const PageSize& pageSize, PageMode pageMode)
    : m_pageSize(pageSize)
    , m_pageMode(pageMode)
    , m_document(new Document(this))
{
}

Book::~Book() = default;

void Book::setPageSize(const PageSize& pageSize)
{
    m_pageSize = pageSize;
}

const PageSize& Book::pageSize() const
{
    return m_pageSize;
}

void Book::setClient(BookClient* client)
{
    m_client = client;
}

BookClient* Book::client() const
{
    return m_client;
}

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
    std::string mimeType;
    std::string textEncoding;
    auto data = m_document->fetchUrl(url, mimeType, textEncoding);
    if(data == nullptr)
        return;
    load(data->data(), data->length(), mimeType, textEncoding);
}

void Book::load(const uint8_t* data, size_t length, std::string_view mimeType, std::string_view textEncoding)
{
    load(TextResource::decode(data, length, mimeType, textEncoding));
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

} // namespace htmlbook
