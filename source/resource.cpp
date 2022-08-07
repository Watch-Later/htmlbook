#define STB_TRUETYPE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "resource.h"
#include "parserstring.h"

namespace htmlbook {

RefPtr<TextResource> TextResource::create(std::string_view mimeType, std::string_view textEncoding, std::vector<char> data)
{
    return nullptr;
}

std::string TextResource::decode(const char* data, size_t length, std::string_view mimeType, std::string_view textEncoding)
{
    std::string value;
    return value;
}

RefPtr<ImageResource> ImageResource::create(std::string_view mimeType, std::string_view textEncoding, std::vector<char> data)
{
    auto image = Image::create(data.data(), data.size());
    if(image == nullptr)
        return nullptr;
    return adoptPtr(new ImageResource(image));
}

RefPtr<FontResource> FontResource::create(std::string_view mimeType, std::string_view textEncoding, std::vector<char> data)
{
    auto face = FontFace::create(std::move(data));
    if(face == nullptr)
        return nullptr;
    return adoptPtr(new FontResource(face));
}

RefPtr<Image> Image::create(const char* data, size_t length)
{
    auto buffer = reinterpret_cast<const uint8_t*>(data);
    int width = 0;
    int height = 0;
    auto image = stbi_load_from_memory(buffer, length, &width, &height, nullptr, STBI_rgb_alpha);
    if(image == nullptr)
        return nullptr;
    return adoptPtr(new Image(width, height, image));
}

Image::~Image()
{
    stbi_image_free(m_data);
}

RefPtr<Glyph> Glyph::create(const FontFace* face, uint32_t codepoint)
{
    auto index = stbtt_FindGlyphIndex(face->info(), codepoint);
    if(index == 0)
        return nullptr;
    return adoptPtr(new Glyph(face, codepoint, index));
}

Glyph::~Glyph()
{
    stbtt_FreeShape(m_face->info(), m_vertexData);
}

Glyph::Glyph(const FontFace* face, uint32_t codepoint, int index)
{
    const auto info = face->info();
    m_face = face;
    m_codepoint = codepoint;
    m_index = index;
    m_vertexLength = stbtt_GetGlyphShape(info, index, &m_vertexData);
    stbtt_GetGlyphHMetrics(info, index, &m_advanceWidth, &m_leftSideBearing);
    stbtt_GetGlyphBox(info, index, &m_x1, &m_y1, &m_x2, &m_y2);
}

RefPtr<FontFace> FontFace::create(std::vector<char> data)
{
    auto buffer = reinterpret_cast<const uint8_t*>(data.data());
    stbtt_fontinfo info;
    if(stbtt_InitFont(&info, buffer, 0) == 0)
        return nullptr;
    return adoptPtr(new FontFace(info, std::move(data)));
}

RefPtr<Glyph> FontFace::getGlyph(uint32_t codepoint) const
{
    if(m_version != FontCache::version()) {
        m_version = FontCache::version();
        for(auto& [index, page] : m_pages) {
            if(page == nullptr)
                continue;
            for(int i = 0; i < 256; i++) {
                auto& glyph = page->at(i);
                if(glyph == nullptr || (glyph->index() && this == glyph->face()))
                    continue;
                glyph.clear();
            }
        }
    }

    auto pageIndex = codepoint / 256;
    auto glyphIndex = codepoint % 256;
    auto& page = m_pages[pageIndex];
    if(page == nullptr)
        page.reset(new GlyphPage);
    auto& glyph = page->at(glyphIndex);
    if(glyph == nullptr)
        glyph = findGlyph(codepoint);
    return glyph;
}

RefPtr<Glyph> FontFace::findGlyph(uint32_t codepoint) const
{
    if(auto glyph = Glyph::create(this, codepoint))
        return glyph;
    return FontCache::findGlyph(this, codepoint);
}

RefPtr<Glyph> FontFace::findGlyph(const FontFace* face, uint32_t codepoint) const
{
    if(face == this)
        return nullptr;
    auto pageIndex = codepoint / 256;
    auto glyphIndex = codepoint % 256;
    auto& page = m_pages[pageIndex];
    if(page == nullptr) {
        auto glyph = Glyph::create(this, codepoint);
        if(glyph == nullptr)
            return nullptr;
        page.reset(new GlyphPage);
        page->at(glyphIndex) = glyph;
        return glyph;
    }

    auto& glyph = page->at(glyphIndex);
    if(glyph == nullptr)
        glyph = Glyph::create(this, codepoint);
    return glyph;
}

static std::string ttfname(const stbtt_fontinfo& info, int nameId)
{
    std::string name;
    auto fc = info.data;
    auto nm = stbtt__find_table(fc, info.fontstart, "name");
    if(nm == 0) return name;

    auto count = ttUSHORT(fc+nm+2);
    auto offset = nm + ttUSHORT(fc+nm+4);
    for(int i = 0;i < count; i++) {
        auto loc = nm + 6 + 12 * i;
        if(nameId != ttUSHORT(fc+loc+6))
            continue;
        auto platform = ttUSHORT(fc+loc+0);
        auto encoding = ttUSHORT(fc+loc+2);

        auto data = fc + offset + ttUSHORT(fc+loc+10);
        auto end = data + ttUSHORT(fc+loc+8);
        if(platform == 0 || (platform == 3 && (encoding == 0 || encoding == 1 || encoding == 10))) {
            while(data < end) {
                uint32_t cp = ttUSHORT(data);
                if(cp >= 0xD800 && cp < 0xDC00) {
                    auto ch = ttUSHORT(data+2);
                    cp = ((cp - 0xD800) << 10) + (ch - 0xDC00) + 0x10000;
                    data += 2;
                }

                appendCodepoint(name, cp);
                data += 2;
            }

            break;
        }

        if(platform == 1 && encoding == 0) {
            name.assign(data, end);
            break;
        }
    }

    return name;
}

std::string FontFace::family() const
{
    return ttfname(m_info, 1);
}

inline int macstyle(const stbtt_fontinfo& info)
{
    return ttUSHORT(info.data + info.head + 44);
}

bool FontFace::bold() const
{
    return macstyle(m_info) & 1;
}

bool FontFace::italic() const
{
    return macstyle(m_info) & 2;
}

float FontFace::scale(float size) const
{
    return stbtt_ScaleForMappingEmToPixels(&m_info, size);
}

FontFace::FontFace(const stbtt_fontinfo& info, std::vector<char> data)
{
    m_version = FontCache::version();
    m_info = info;
    m_data = std::move(data);
    stbtt_GetFontVMetrics(&info, &m_ascent, &m_descent, &m_lineGap);
    stbtt_GetFontBoundingBox(&info, &m_x1, &m_y1, &m_x2, &m_y2);
}

void FontCache::addFont(const FontDescription& description, RefPtr<FontFace> face)
{
}

RefPtr<FontFace> FontCache::getFace(const FontDescription& description)
{
    return nullptr;
}

RefPtr<Glyph> FontCache::findGlyph(const FontFace* face, uint32_t codepoint)
{
    return nullptr;
}

uint32_t FontCache::version()
{
    return 0;
}

} // namespace htmlbook
