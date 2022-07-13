#ifndef PDFDOCUMENT_H
#define PDFDOCUMENT_H

#include "htmlbook.h"
#include "document.h"

namespace htmlbook {

class PdfDocument final : public Document {
public:
    PdfDocument(const PageSize& pageSize, PageMode pageMode);

    const PageSize& pageSize() const { return m_pageSize; }
    void setPageSize(const PageSize& pageSize) { m_pageSize = pageSize; }

    BookClient* client() const { return m_client; }
    void setClient(BookClient* client) { m_client = client; }

    const std::string& title() const { return m_title; }
    void setTitle(const std::string_view& title) { m_title = title; }

    const std::string& subject() const { return m_subject; }
    void setSubject(const std::string_view& subject) { m_subject = subject; }

    const std::string& author() const { return m_author; }
    void setAuthor(const std::string_view& author) { m_author = author; }

    const std::string& creator() const { return m_creator; }
    void setCreator(const std::string_view& creator) { m_creator = creator; }

    const std::string& creationDate() const { return m_creationDate; }
    void setCreationDate(const std::string_view& creationDate) { m_creationDate = creationDate; }

    const std::string& modificationDate() const { return m_modificationDate; }
    void setModificationDate(const std::string_view& modificationDate) { m_modificationDate = modificationDate; }

    std::shared_ptr<ResourceData> fetchUrl(const std::string_view& url) final;
    std::shared_ptr<ResourceData> fetchFont(const std::string_view& family, bool italic, bool smallCaps, int weight) final;

private:
    PageSize m_pageSize;
    PageMode m_pageMode;
    BookClient* m_client{nullptr};

    std::string m_title;
    std::string m_subject;
    std::string m_author;
    std::string m_creator;
    std::string m_creationDate;
    std::string m_modificationDate;
};

} // namespace htmlbook

#endif // PDFDOCUMENT_H
