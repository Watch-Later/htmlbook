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

class HTMLBOOK_API PageRect {
public:
    PageRect() = default;
    PageRect(int x, int y, int width, int height)
        : x(x), y(y), w(width), h(height)
    {}

public:
    int x{0};
    int y{0};
    int w{0};
    int h{0};
};

class HTMLBOOK_API PageMatrix {
public:
    PageMatrix() = default;
    PageMatrix(float a, float b, float c, float d, float e, float f)
        : a(a), b(b), c(c), d(d), e(e), f(f)
    {}

    PageMatrix operator*(const PageMatrix& matrix) const;
    PageMatrix& operator*=(const PageMatrix& matrix);

    PageMatrix& scale(float x, float y);
    PageMatrix& translate(float x, float y);
    PageMatrix& shear(float x, float y);
    PageMatrix& rotate(float angle);
    PageMatrix& transform(float a, float b, float c, float d, float e, float f);
    PageMatrix& identity();
    PageMatrix& invert();

    static PageMatrix scaled(float x, float y);
    static PageMatrix translated(float x, float y);
    static PageMatrix sheared(float x, float y);
    static PageMatrix rotated(float angle);

public:
    float a{1};
    float b{0};
    float c{0};
    float d{1};
    float e{0};
    float f{0};
};

class HTMLBOOK_API PageBitmap {
public:
    PageBitmap(uint8_t* data, int width, int height, int stride)
        : m_data(data), m_width(width), m_height(height), m_stride(stride)
    {}

    uint8_t* data() const { return m_data; }
    int width() const { return m_width; }
    int height() const { return m_height; }
    int stride() const { return m_stride; }

private:
    uint8_t* m_data;
    int m_width;
    int m_height;
    int m_stride;
};

class HTMLBOOK_API ResourceData {
public:
    static std::shared_ptr<ResourceData> create(const uint8_t* data, size_t length, std::string_view mimeType, std::string_view textEncoding);
    static std::shared_ptr<ResourceData> createUninitialized(uint8_t*& data, size_t length, std::string_view mimeType, std::string_view textEncoding);
    static std::shared_ptr<ResourceData> createStatic(const uint8_t* data, size_t length, std::string_view mimeType, std::string_view textEncoding);

    const uint8_t* data() const { return m_data; }
    size_t length() const { return m_length; }
    const std::string& mimeType() const { return m_mimeType; }
    const std::string& textEncoding() const { return m_textEncoding; }

private:
    ResourceData(const uint8_t* data, size_t length, std::string_view mimeType, std::string_view textEncoding);
    const uint8_t* m_data;
    size_t m_length;
    std::string m_mimeType;
    std::string m_textEncoding;
};

enum class PageMode {
    Single,
    Multiple
};

class HTMLBOOK_API BookClient {
public:
    /**
     * @brief BookClient
     */
    BookClient() = default;

    /**
     * @brief ~BookClient
     */
    virtual ~BookClient() = default;

    /**
     * @brief loadUrl
     * @param url
     * @return
     */
    virtual std::shared_ptr<ResourceData> loadUrl(std::string_view url) = 0;

    /**
     * @brief loadFont
     * @param family
     * @param italic
     * @param smallCaps
     * @param weight
     * @return
     */
    virtual std::shared_ptr<ResourceData> loadFont(std::string_view family, bool italic, bool smallCaps, int weight) = 0;
};

class Document;

class HTMLBOOK_API Book {
public:
    /**
     * @brief Book
     * @param pageSize
     * @param pageMode
     */
    Book(const PageSize& pageSize, PageMode pageMode = PageMode::Multiple);

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
     * @brief setPageMode
     * @param pageMode
     */
    void setPageMode(PageMode pageMode);

    /**
     * @brief pageMode
     * @return
     */
    PageMode pageMode() const;

    /**
     * @brief setClient
     * @param client
     */
    void setClient(BookClient* client);

    /**
     * @brief client
     * @return
     */
    BookClient* client() const;

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
     * @param mimeType
     * @param textEncoding
     */
    void load(const uint8_t* data, size_t length, std::string_view mimeType, std::string_view textEncoding);

    /**
     * @brief load
     * @param content
     */
    void load(std::string_view content);

    /**
     * @brief setUserStyleSheet
     * @param content
     */
    void setUserStyleSheet(std::string_view content);

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
     * @brief width
     * @return
     */
    int width() const;

    /**
     * @brief height
     * @return
     */
    int height() const;

    /**
     * @brief contentWidth
     * @return
     */
    int contentWidth() const;

    /**
     * @brief contentHeight
     * @return
     */
    int contentHeight() const;

    /**
     * @brief documentWidth
     * @return
     */
    int documentWidth() const;

    /**
     * @brief documentHeight
     * @return
     */
    int documentHeight() const;

    /**
     * @brief pageRect
     * @param pageIndex
     * @return
     */
    PageRect pageRect(size_t pageIndex) const;

    /**
     * @brief pageContentRect
     * @param pageIndex
     * @return
     */
    PageRect pageContentRect(size_t pageIndex) const;

    /**
     * @brief renderPage
     * @param bitmap
     * @param pageIndex
     * @param matrix
     */
    void renderPage(PageBitmap& bitmap, size_t pageIndex, const PageMatrix& matrix = PageMatrix());

    /**
     * @brief renderDocument
     * @param bitmap
     * @param contentRect
     * @param matrix
     */
    void renderDocument(PageBitmap& bitmap, const PageRect& contentRect, const PageMatrix& matrix = PageMatrix());

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

private:
    PageSize m_pageSize;
    PageMode m_pageMode;
    BookClient* m_client{nullptr};
    std::unique_ptr<Document> m_document;
};

inline std::ostream& operator<<(std::ostream& o, const Book& book)
{
    book.serialize(o);
    return o;
}

} // namespace htmlbook

#endif
