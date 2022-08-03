#define STB_TRUETYPE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "resource.h"

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
    int width    = 0;
    int height   = 0;
    int channels = 0;
    auto image = stbi_load_from_memory(buffer, length, &width, &height, &channels, STBI_rgb_alpha);
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

float FontFace::scale(float pixels) const
{
    return stbtt_ScaleForMappingEmToPixels(&m_info, pixels);
}

FontFace::FontFace(const stbtt_fontinfo& info, std::vector<char> data)
{
    m_info = info;
    m_data = std::move(data);
    stbtt_GetFontVMetrics(&info, &m_ascent, &m_descent, &m_lineGap);
    stbtt_GetFontBoundingBox(&info, &m_x1, &m_y1, &m_x2, &m_y2);
}

} // namespace htmlbook
