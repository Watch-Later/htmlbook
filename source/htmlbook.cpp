#include "htmlbook.h"
#include "pdfdocument.h"

#include <cstring>
#include <iostream>

namespace htmlbook {

const PageSize PageSize::A5(148, 210, PageUnit::Millimeters);
const PageSize PageSize::A4(210, 297, PageUnit::Millimeters);
const PageSize PageSize::A3(297, 420, PageUnit::Millimeters);
const PageSize PageSize::B5(176, 250, PageUnit::Millimeters);
const PageSize PageSize::B4(250, 353, PageUnit::Millimeters);
const PageSize PageSize::Letter(8.5, 11, PageUnit::Inches);
const PageSize PageSize::Legal(8.5, 14, PageUnit::Inches);
const PageSize PageSize::Ledger(11, 17, PageUnit::Inches);

std::shared_ptr<ResourceData> ResourceData::create(const uint8_t* data, size_t length, const std::string_view& mimeType, const std::string_view& textEncoding)
{
    uint8_t* newdata;
    auto resource = createUninitialized(newdata, length, mimeType, textEncoding);
    std::memcpy(newdata, data, length);
    return resource;
}

std::shared_ptr<ResourceData> ResourceData::createUninitialized(uint8_t*& data, size_t length, const std::string_view& mimeType, const std::string_view& textEncoding)
{
    auto newdata = new uint8_t[length + sizeof(ResourceData)];
    data = newdata + sizeof(ResourceData);
    return std::shared_ptr<ResourceData>(new (newdata) ResourceData(data, length, mimeType, textEncoding));
}

std::shared_ptr<ResourceData> ResourceData::createStatic(const uint8_t* data, size_t length, const std::string_view& mimeType, const std::string_view& textEncoding)
{
    return std::shared_ptr<ResourceData>(new ResourceData(data, length, mimeType, textEncoding));
}

ResourceData::ResourceData(const uint8_t* data, size_t length, const std::string_view& mimeType, const std::string_view& textEncoding)
    : m_data(data), m_length(length), m_mimeType(mimeType), m_textEncoding(textEncoding)
{
}

Book::Book(const PageSize& pageSize, PageMode pageMode)
    : m_document(new PdfDocument(pageSize, pageMode))
{
}

Book::~Book() = default;

void Book::setPageSize(const PageSize& pageSize)
{
    m_document->setPageSize(pageSize);
}

const PageSize& Book::pageSize() const
{
    return m_document->pageSize();
}

void Book::setClient(BookClient* client)
{
    m_document->setClient(client);
}

BookClient* Book::client() const
{
    return m_document->client();
}

void Book::setTitle(const std::string_view& title)
{
    m_document->setTitle(title);
}

const std::string& Book::title() const
{
    return m_document->title();
}

void Book::setSubject(const std::string_view& subject)
{
    m_document->setSubject(subject);
}

const std::string& Book::subject() const
{
    return m_document->subject();
}

void Book::setAuthor(const std::string_view& author)
{
    m_document->setAuthor(author);
}

const std::string& Book::author() const
{
    return m_document->author();
}

void Book::setCreator(const std::string_view& creator)
{
    m_document->setCreator(creator);
}

const std::string& Book::creator() const
{
    return m_document->creator();
}

void Book::setCreationDate(const std::string_view& creationDate)
{
    m_document->setCreationDate(creationDate);
}

const std::string& Book::creationDate() const
{
    return m_document->creationDate();
}

void Book::setModificationDate(const std::string_view& modificationDate)
{
    m_document->setModificationDate(modificationDate);
}

const std::string& Book::modificationDate() const
{
    return m_document->modificationDate();
}

void Book::setBaseUrl(const std::string_view& baseUrl)
{
    m_document->setBaseUrl(baseUrl);
}

const std::string& Book::baseUrl() const
{
    return m_document->baseUrl().value();
}

void Book::loadUrl(const std::string_view& url)
{
}

void Book::loadData(const uint8_t* data, size_t length, const std::string_view& textEncoding)
{
}

void Book::load(const std::string_view& content)
{
    m_document->load(content);
}

void Book::setUserStyleSheet(const std::string_view& content)
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

PdfDocument* Book::document() const
{
    return m_document.get();
}

} // namespace htmlbook
