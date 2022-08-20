/*
 * Copyright (c) 2022 Nwutobo Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef HTMLBOOK_H
#define HTMLBOOK_H

#include <string>
#include <ostream>
#include <memory>
#include <vector>

#if defined(_MSC_VER) && defined(HTMLBOOK_SHARED)
#ifdef HTMLBOOK_EXPORT
#define HTMLBOOK_API __declspec(dllexport)
#else
#define HTMLBOOK_API __declspec(dllimport)
#endif
#else
#define HTMLBOOK_API
#endif

namespace htmlbook {

/*
 * Contribute : https://github.com/sammycage/htmlbook
 * Donate : https://patreon.com/sammycage
 */

enum class PageUnit {
    Centimeters,
    Millimeters,
    Inches,
    Points,
    Picas,
    Pixels
};

class HTMLBOOK_API PageSize {
public:
    PageSize() = default;
    PageSize(float width, float height, PageUnit unit)
        : width(width), height(height), unit(unit)
    {}

    static const PageSize A5;
    static const PageSize A4;
    static const PageSize A3;
    static const PageSize B5;
    static const PageSize B4;
    static const PageSize Letter;
    static const PageSize Legal;
    static const PageSize Ledger;

public:
    float width{0};
    float height{0};
    PageUnit unit{PageUnit::Pixels};
};

class HTMLBOOK_API ResourceClient {
public:
    /**
     * @brief ResourceClient
     */
    ResourceClient() = default;

    /**
     * @brief ~ResourceClient
     */
    virtual ~ResourceClient() = default;

    /**
     * @brief loadUrl
     * @param url
     * @param mimeType
     * @param textEncoding
     * @param data
     * @return
     */
    virtual bool loadUrl(std::string_view url, std::string& mimeType, std::string& textEncoding, std::vector<char>& data) = 0;

    /**
     * @brief loadFont
     * @param family
     * @param italic
     * @param smallCaps
     * @param weight
     * @param data
     * @return
     */
    virtual bool loadFont(std::string_view family, bool italic, bool smallCaps, int weight, std::vector<char>& data) = 0;
};

class Document;

class HTMLBOOK_API Book {
public:
    /**
     * @brief Book
     * @param pageSize
     */
    Book(const PageSize& pageSize);

    /**
     * @brief ~Book
     */
    ~Book();

    /**
     * @brief setPageSize
     * @param pageSize
     */
    void setPageSize(const PageSize& pageSize);

    /**
     * @brief pageSize
     * @return
     */
    const PageSize& pageSize() const;

    /**
     * @brief setTitle
     * @param title
     */
    void setTitle(std::string_view title);

    /**
     * @brief title
     * @return
     */
    std::string_view title() const;

    /**
     * @brief setSubject
     * @param subject
     */
    void setSubject(std::string_view subject);

    /**
     * @brief subject
     * @return
     */
    std::string_view subject() const;

    /**
     * @brief setAuthor
     * @param author
     */
    void setAuthor(std::string_view author);

    /**
     * @brief author
     * @return
     */
    std::string_view author() const;

    /**
     * @brief setCreator
     * @param creator
     */
    void setCreator(std::string_view creator);

    /**
     * @brief creator
     * @return
     */
    std::string_view creator() const;

    /**
     * @brief setCreationDate
     * @param creationDate
     */
    void setCreationDate(std::string_view creationDate);

    /**
     * @brief creationDate
     * @return
     */
    std::string_view creationDate() const;

    /**
     * @brief setModificationDate
     * @param modificationDate
     */
    void setModificationDate(std::string_view modificationDate);

    /**
     * @brief modificationDate
     * @return
     */
    std::string_view modificationDate() const;

    /**
     * @brief setBaseUrl
     * @param baseUrl
     */
    void setBaseUrl(std::string_view baseUrl);

    /**
     * @brief baseUrl
     * @return
     */
    std::string_view baseUrl() const;

    /**
     * @brief loadUrl
     * @param url
     */
    void loadUrl(std::string_view url);

    /**
     * @brief load
     * @param data
     * @param length
     * @param textEncoding
     */
    void load(const char* data, size_t length, std::string_view textEncoding);

    /**
     * @brief load
     * @param content
     */
    void load(std::string_view content);

    /**
     * @brief addUserStyleSheet
     * @param content
     */
    void addUserStyleSheet(std::string_view content);

    /**
     * @brief clearUserStyleSheet
     */
    void clearUserStyleSheet();

    /**
     * @brief clear
     */
    void clear();

    /**
     * @brief empty
     * @return
     */
    bool empty();

    /**
     * @brief pageCount
     * @return
     */
    size_t pageCount() const;

    /**
     * @brief save
     * @param filename
     */
    void save(const std::string& filename);

    /**
     * @brief save
     * @param output
     */
    void save(std::ostream& output);

    /**
     * @brief serialize
     * @param o
     */
    void serialize(std::ostream& o) const;

    /**
     * @brief document
     * @return
     */
    Document* document() const;

    /**
     * @brief setResourceClient
     * @param client
     */
    static void setResourceClient(ResourceClient* client);

    /**
     * @brief resourceClient
     * @return
     */
    static ResourceClient* resourceClient();

    /**
     * @brief addFontFile
     * @param family
     * @param italic
     * @param smallCaps
     * @param weight
     * @param filename
     * @return
     */
    static bool addFontFile(const std::string& family, bool italic, bool smallCaps, int weight, const std::string& filename);

    /**
     * @brief addFontData
     * @param family
     * @param italic
     * @param smallCaps
     * @param weight
     * @param data
     * @return
     */
    static bool addFontData(const std::string& family, bool italic, bool smallCaps, int weight, std::vector<char> data);

private:
    std::unique_ptr<Document> m_document;
};

inline std::ostream& operator<<(std::ostream& o, const Book& book)
{
    book.serialize(o);
    return o;
}

} // namespace htmlbook

#endif
