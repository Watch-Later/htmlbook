#define STB_TRUETYPE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "resource.h"

namespace htmlbook {

RefPtr<TextResource> TextResource::create(std::string_view mimeType, std::string_view textEncoding, std::vector<char> data)
{
    auto text = decode(data.data(), data.size(), mimeType, textEncoding);
    if(text.empty())
        return nullptr;
    return adoptPtr(new TextResource(std::move(text)));
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
    return adoptPtr(new ImageResource(std::move(image)));
}

RefPtr<FontResource> FontResource::create(std::string_view mimeType, std::string_view textEncoding, std::vector<char> data)
{
    auto face = FontFace::create(std::move(data));
    if(face == nullptr)
        return nullptr;
    return adoptPtr(new FontResource(std::move(face)));
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
    if(m_version != fontCache()->version()) {
        m_version = fontCache()->version();
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
    return fontCache()->findGlyph(this, codepoint);
}

RefPtr<Glyph> FontFace::findGlyph(const FontFace* face, uint32_t codepoint) const
{
    auto macstyle = [](auto& info) { return ttUSHORT(info.data + info.head + 44); };
    if(face == this || macstyle(m_info) != macstyle(*face->info()))
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

float FontFace::scale(float size) const
{
    return stbtt_ScaleForMappingEmToPixels(&m_info, size);
}

FontFace::FontFace(const stbtt_fontinfo& info, std::vector<char> data)
{
    m_version = fontCache()->version();
    m_info = info;
    m_data = std::move(data);
    stbtt_GetFontVMetrics(&info, &m_ascent, &m_descent, &m_lineGap);
    stbtt_GetFontBoundingBox(&info, &m_x1, &m_y1, &m_x2, &m_y2);
}

void FontCache::addFace(const std::string& family, bool italic, bool smallCaps, int weight, RefPtr<FontFace> face)
{
    auto description = std::tie(family, italic, smallCaps, weight);
    m_fontFaceMap.emplace(description, std::move(face));
    m_version += 1;
}

RefPtr<FontFace> FontCache::getFace(const std::string& family, bool italic, bool smallCaps, int weight) const
{
    auto description = std::tie(family, italic, smallCaps, weight);
    auto it = m_fontFaceMap.find(description);
    if(it == m_fontFaceMap.end())
        return nullptr;
    return it->second;
}

RefPtr<Glyph> FontCache::findGlyph(const FontFace* face, uint32_t codepoint) const
{
    for(auto& [name, value] : m_fontFaceMap) {
        if(auto glyph = value->findGlyph(face, codepoint)) {
            if(glyph->index() == 0)
                break;
            return glyph;
        }
    }

    return nullptr;
}

FontCache* fontCache()
{
    static FontCache cache;
    return &cache;
}

} // namespace htmlbook
