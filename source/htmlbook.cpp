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

std::shared_ptr<ResourceData> ResourceData::create(const uint8_t* data, size_t length, std::string_view mimeType, std::string_view textEncoding)
{
    uint8_t* newdata;
    auto resource = createUninitialized(newdata, length, mimeType, textEncoding);
    std::memcpy(newdata, data, length);
    return resource;
}

std::shared_ptr<ResourceData> ResourceData::createUninitialized(uint8_t*& data, size_t length, std::string_view mimeType, std::string_view textEncoding)
{
    auto newdata = new uint8_t[length + sizeof(ResourceData)];
    data = newdata + sizeof(ResourceData);
    return std::shared_ptr<ResourceData>(new (newdata) ResourceData(data, length, mimeType, textEncoding));
}

std::shared_ptr<ResourceData> ResourceData::createStatic(const uint8_t* data, size_t length, std::string_view mimeType, std::string_view textEncoding)
{
    return std::shared_ptr<ResourceData>(new ResourceData(data, length, mimeType, textEncoding));
}

ResourceData::ResourceData(const uint8_t* data, size_t length, std::string_view mimeType, std::string_view textEncoding)
    : m_data(data), m_length(length), m_mimeType(mimeType), m_textEncoding(textEncoding)
{
}

Book::Book(const PageSize& pageSize, PageMode pageMode)
    : m_pageSize(pageSize), m_pageMode(pageMode)
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
    if(m_document == nullptr)
        return;
    m_document->setBaseUrl(baseUrl);
}

std::string_view Book::baseUrl() const
{
    if(m_document == nullptr)
        return emptyString;
    return m_document->baseUrl();
}

void Book::loadUrl(std::string_view url)
{
    if(m_client == nullptr)
        return;
    auto resourceData = m_client->loadUrl(url);
    if(resourceData == nullptr)
        return;
    load(resourceData->data(), resourceData->length(), resourceData->mimeType(), resourceData->textEncoding(), url);
}

void Book::loadHtml(const uint8_t* data, size_t length, std::string_view textEncoding, std::string_view baseUrl)
{
    load(data, length, "text/html", textEncoding, baseUrl);
}

void Book::loadHtml(std::string_view content, std::string_view baseUrl)
{
    load(content, "text/html", baseUrl);
}

void Book::load(const uint8_t* data, size_t length, std::string_view mimeType, std::string_view textEncoding, std::string_view baseUrl)
{
    m_document = Document::create(this, mimeType, baseUrl);
    m_document->loadData(data, length, textEncoding);
}

void Book::load(std::string_view content, std::string_view mimeType, std::string_view baseUrl)
{
    m_document = Document::create(this, mimeType, baseUrl);
    m_document->load(content);
}

void Book::setUserStyleSheet(std::string_view content)
{
    if(m_document == nullptr)
        return;
    m_document->setUserStyleSheet(content);
}

void Book::clearUserStyleSheet()
{
    if(m_document == nullptr)
        return;
    m_document->clearUserStyleSheet();
}

void Book::save(const std::string& filename)
{
}

void Book::serialize(std::ostream& o) const
{
    if(m_document == nullptr)
        return;
    m_document->serialize(o);
}

Document* Book::document() const
{
    return m_document.get();
}

} // namespace htmlbook
