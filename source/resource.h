#ifndef RESOURCE_H
#define RESOURCE_H

#include <memory>
#include <string>

namespace htmlbook {

class ResourceData;

class Resource {
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
    static std::shared_ptr<TextResource> create(std::shared_ptr<ResourceData> data);
    static std::string decode(const uint8_t* data, size_t length, std::string_view mimeType, std::string_view textEncoding);
    const std::string& text() const { return m_text; }
    Type type() const final { return Resource::Type::Text; }

private:
    TextResource(std::string text) : m_text(std::move(text)) {}
    std::string m_text;
};

class ImageData;

class ImageResource final : public Resource {
public:
    static std::shared_ptr<ImageResource> create(std::shared_ptr<ResourceData> data);
    std::shared_ptr<ImageData> image() const { return m_image; }
    Type type() const final { return Resource::Type::Image; }

private:
    ImageResource(std::shared_ptr<ImageData> image) : m_image(std::move(image)) {}
    std::shared_ptr<ImageData> m_image;
};

class FontData;

class FontResource final : public Resource {
public:
    static std::shared_ptr<FontResource> create(std::shared_ptr<ResourceData> data);
    std::shared_ptr<FontData> font() const { return m_font; }
    Type type() const final { return Resource::Type::Font; }

private:
    FontResource(std::shared_ptr<FontData> font) : m_font(std::move(font)) {}
    std::shared_ptr<FontData> m_font;
};

} // namespace htmlbook

#endif // RESOURCE_H
