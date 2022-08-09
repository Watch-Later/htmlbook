#ifndef RESOURCE_H
#define RESOURCE_H

#include "stb_truetype.h"
#include "stb_image.h"

#include "pointer.h"

#include <memory>
#include <string>
#include <vector>
#include <array>
#include <map>

namespace htmlbook {

class Resource : public RefCounted<Resource> {
public:
    enum class Type {
        Text,
        Image,
        Font
    };

    virtual ~Resource() = default;
    virtual Type type() const = 0;

protected:
    Resource() = default;
};

class TextResource final : public Resource {
public:
    static RefPtr<TextResource> create(std::string_view mimeType, std::string_view textEncoding, std::vector<char> data);
    static std::string decode(const char* data, size_t length, std::string_view mimeType, std::string_view textEncoding);
    const std::string& text() const { return m_text; }
    Type type() const final { return Type::Text; }

private:
    TextResource(std::string text) : m_text(std::move(text)) {}
    std::string m_text;
};

template<>
struct is<TextResource> {
    static bool check(const Resource& value) { return value.type() == Resource::Type::Text; }
};

class Image;

class ImageResource final : public Resource {
public:
    static RefPtr<ImageResource> create(std::string_view mimeType, std::string_view textEncoding, std::vector<char> data);
    const RefPtr<Image>& image() const { return m_image; }
    Type type() const final { return Type::Image; }

private:
    ImageResource(RefPtr<Image> image) : m_image(std::move(image)) {}
    RefPtr<Image> m_image;
};

template<>
struct is<ImageResource> {
    static bool check(const Resource& value) { return value.type() == Resource::Type::Image; }
};

class FontFace;

class FontResource final : public Resource {
public:
    static RefPtr<FontResource> create(std::string_view mimeType, std::string_view textEncoding, std::vector<char> data);
    const RefPtr<FontFace>& face() const { return m_face; }
    Type type() const final { return Type::Font; }

private:
    FontResource(RefPtr<FontFace> face) : m_face(std::move(face)) {}
    RefPtr<FontFace> m_face;
};

template<>
struct is<FontResource> {
    static bool check(const Resource& value) { return value.type() == Resource::Type::Font; }
};

class Image : public RefCounted<Image> {
public:
    static RefPtr<Image> create(const char* data, size_t length);

    int width() const { return m_width; }
    int height() const { return m_height; }
    const uint8_t* data() const { return m_data; }

    ~Image();

private:
    Image(int width, int height, uint8_t* data) : m_width(width), m_height(height), m_data(data) {}
    int m_width;
    int m_height;
    uint8_t* m_data;
};

class Glyph : public RefCounted<Glyph> {
public:
    static RefPtr<Glyph> create(const FontFace* face, uint32_t codepoint);

    const FontFace* face() const { return m_face; }
    const stbtt_vertex* vertexData() const { return m_vertexData; }
    int vertexLength() const { return m_vertexLength; }
    uint32_t codepoint() const { return m_codepoint; }
    int index() const { return m_index; }
    int advanceWidth() const { return m_advanceWidth; }
    int leftSideBearing() const { return m_leftSideBearing; }
    int x1() const { return m_x1; }
    int y1() const { return m_y1; }
    int x2() const { return m_x2; }
    int y2() const { return m_y2; }

    ~Glyph();

private:
    Glyph(const FontFace* face, uint32_t codepoint, int index);
    const FontFace* m_face;
    stbtt_vertex* m_vertexData;
    int m_vertexLength;
    uint32_t m_codepoint;
    int m_index;
    int m_advanceWidth;
    int m_leftSideBearing;
    int m_x1;
    int m_y1;
    int m_x2;
    int m_y2;
};

using GlyphPage = std::array<RefPtr<Glyph>, 256>;

class FontFace : public RefCounted<FontFace> {
public:
    static RefPtr<FontFace> create(std::vector<char> data);

    RefPtr<Glyph> getGlyph(uint32_t codepoint) const;
    RefPtr<Glyph> findGlyph(uint32_t codepoint) const;
    RefPtr<Glyph> findGlyph(const FontFace* face, uint32_t codepoint) const;

    float scale(float size) const;
    int ascent() const { return m_ascent; }
    int descent() const { return m_descent; }
    int lineGap() const { return m_lineGap; }
    int x1() const { return m_x1; }
    int y1() const { return m_y1; }
    int x2() const { return m_x2; }
    int y2() const { return m_y2; }
    const stbtt_fontinfo* info() const { return &m_info; }

private:
    FontFace(const stbtt_fontinfo& info, std::vector<char> data);
    std::vector<char> m_data;
    stbtt_fontinfo m_info;
    mutable std::map<int, std::unique_ptr<GlyphPage>> m_pages;
    mutable uint32_t m_version;
    int m_ascent;
    int m_descent;
    int m_lineGap;
    int m_x1;
    int m_y1;
    int m_x2;
    int m_y2;
};

class FontCache {
public:
    void addFont(const std::string& family, bool italic, bool smallCaps, int weight, RefPtr<FontFace> face);
    RefPtr<FontFace> getFace(const std::string& family, bool italic, bool smallCaps, int weight) const;
    RefPtr<Glyph> findGlyph(const FontFace* face, uint32_t codepoint) const;
    uint32_t version() const { return m_version; }

    friend FontCache* fontCache();

private:
    FontCache() = default;
    using FontDescription = std::tuple<std::string, bool, bool, int>;
    using FontFaceMap = std::map<FontDescription, RefPtr<FontFace>>;
    FontFaceMap m_fontFaceMap;
    uint32_t m_version{0};
};

FontCache* fontCache();

} // namespace htmlbook

#endif // RESOURCE_H
