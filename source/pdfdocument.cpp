#include "pdfdocument.h"

namespace htmlbook {

PdfDocument::PdfDocument(const PageSize& pageSize, PageMode pageMode)
    : m_pageSize(pageSize), m_pageMode(pageMode)
{
}

std::shared_ptr<ResourceData> PdfDocument::fetchUrl(const std::string_view& url)
{
    if(m_client == nullptr)
        return nullptr;
    return m_client->loadUrl(url);
}

std::shared_ptr<ResourceData> PdfDocument::fetchFont(const std::string_view& family, bool italic, bool smallCaps, int weight)
{
    if(m_client == nullptr)
        return nullptr;
    return m_client->loadFont(family, italic, smallCaps, weight);
}

} // namespace htmlbook
