/*
 * Copyright (c) 2022 Samuel Ugochukwu <sammycage8051@gmail.com>
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

#include <memory_resource>
#include <string_view>
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

/*
 * Contribute : https://github.com/sammycage/htmlbook
 * Donate : https://patreon.com/sammycage
 */

namespace htmlbook {

class HTMLBOOK_API PageSize {
public:
    PageSize() = default;
    PageSize(float width, float height)
        : m_width(width), m_height(height)
    {}

    void setWidth(float width) { m_width = width; }
    void setHeight(float height) { m_height = height; }

    float width() const { return m_width; }
    float height() const { return m_height; }

    static const PageSize A3;
    static const PageSize A4;
    static const PageSize A5;
    static const PageSize B4;
    static const PageSize B5;
    static const PageSize Letter;
    static const PageSize Legal;
    static const PageSize Ledger;

private:
    float m_width{0};
    float m_height{0};
};

class HTMLBOOK_API PageMargins {
public:
    PageMargins() = default;
    explicit PageMargins(float margin)
        : m_top(margin), m_right(margin), m_bottom(margin), m_left(margin)
    {}

    PageMargins(float vertical, float horizontal)
        : m_top(vertical), m_right(horizontal), m_bottom(vertical), m_left(horizontal)
    {}

    PageMargins(float top, float right, float bottom, float left)
        : m_top(top), m_right(right), m_bottom(bottom), m_left(left)
    {}

    void setTop(float top) { m_top = top; }
    void setRight(float right) { m_right = right; }
    void setBottom(float bottom) { m_bottom = bottom; }
    void setLeft(float left) { m_left = left; }

    float top() const { return m_top; }
    float right() const { return m_right; }
    float bottom() const { return m_bottom; }
    float left() const { return m_left; }

private:
    float m_top{0};
    float m_right{0};
    float m_bottom{0};
    float m_left{0};
};

enum class PageOrientation {
    Portrait,
    Landscape
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
    virtual bool loadUrl(const std::string_view& url, std::string& mimeType, std::string& textEncoding, std::vector<char>& data) = 0;

    /**
     * @brief loadFont
     * @param family
     * @param italic
     * @param smallCaps
     * @param weight
     * @param data
     * @return
     */
    virtual bool loadFont(const std::string_view& family, bool italic, bool smallCaps, int weight, std::vector<char>& data) = 0;
};

using Heap = std::pmr::monotonic_buffer_resource;

class HTMLDocument;

class HTMLBOOK_API Book {
public:
    /**
     * @brief Book
     * @param size
     * @param orientation
     * @param margins
     */
    Book(const PageSize& size, PageOrientation orientation = PageOrientation::Portrait, const PageMargins& margins = PageMargins());

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
    void setTitle(const std::string_view& title);

    /**
     * @brief title
     * @return
     */
    const std::string& title() const;

    /**
     * @brief setSubject
     * @param subject
     */
    void setSubject(const std::string_view& subject);

    /**
     * @brief subject
     * @return
     */
    const std::string& subject() const;

    /**
     * @brief setAuthor
     * @param author
     */
    void setAuthor(const std::string_view& author);

    /**
     * @brief author
     * @return
     */
    const std::string& author() const;

    /**
     * @brief setCreator
     * @param creator
     */
    void setCreator(const std::string_view& creator);

    /**
     * @brief creator
     * @return
     */
    const std::string& creator() const;

    /**
     * @brief setCreationDate
     * @param creationDate
     */
    void setCreationDate(const std::string_view& creationDate);

    /**
     * @brief creationDate
     * @return
     */
    const std::string& creationDate() const;

    /**
     * @brief setModificationDate
     * @param modificationDate
     */
    void setModificationDate(const std::string_view& modificationDate);

    /**
     * @brief modificationDate
     * @return
     */
    const std::string& modificationDate() const;

    /**
     * @brief viewportWidth
     * @return
     */
    float viewportWidth() const;

    /**
     * @brief viewportHeight
     * @return
     */
    float viewportHeight() const;

    /**
     * @brief loadUrl
     * @param url
     * @param userStyle
     */
    void loadUrl(const std::string_view& url, const std::string_view& userStyle = {});

    /**
     * @brief loadData
     * @param data
     * @param length
     * @param textEncoding
     * @param baseUrl
     * @param userStyle
     */
    void loadData(const char* data, size_t length, const std::string_view& textEncoding = {}, const std::string_view& baseUrl = {}, const std::string_view& userStyle = {});

    /**
     * @brief load
     * @param content
     * @param baseUrl
     * @param userStyle
     */
    void load(const std::string_view& content, const std::string_view& baseUrl = {}, const std::string_view& userStyle = {});

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
    HTMLDocument* document() const { return m_document.get(); }

    /**
     * @brief heap
     * @return
     */
    Heap* heap() const { return m_heap.get(); }

private:
    std::unique_ptr<Heap> m_heap;
    std::unique_ptr<HTMLDocument> m_document;
    PageSize m_pageSize;
    PageOrientation m_pageOrientation;
    PageMargins m_pageMargins;
    std::string m_title;
    std::string m_subject;
    std::string m_author;
    std::string m_creator;
    std::string m_creationDate;
    std::string m_modificationDate;
};

inline std::ostream& operator<<(std::ostream& o, const Book& book)
{
    book.serialize(o);
    return o;
}

} // namespace htmlbook

#endif
