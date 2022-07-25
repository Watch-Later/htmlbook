#ifndef RESOURCE_H
#define RESOURCE_H

#include "pointer.h"

#include <memory>
#include <string>

namespace htmlbook {

class ResourceData;

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
    static RefPtr<TextResource> create(std::shared_ptr<ResourceData> data);
    static std::string decode(const uint8_t* data, size_t length, std::string_view mimeType, std::string_view textEncoding);
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

class Image : public RefCounted<Image> {
public:
    virtual ~Image() = default;

protected:
    Image(std::shared_ptr<ResourceData> data);
    std::shared_ptr<ResourceData> m_data;
};

class ImageResource final : public Resource {
public:
    static RefPtr<ImageResource> create(std::shared_ptr<ResourceData> data);
    Image* image() const { return m_image.get(); }
    Type type() const final { return Type::Image; }

private:
    ImageResource(RefPtr<Image> image);
    RefPtr<Image> m_image;
};

template<>
struct is<ImageResource> {
    static bool check(const Resource& value) { return value.type() == Resource::Type::Image; }
};

class FontFace : public RefCounted<FontFace> {
public:
    virtual ~FontFace() = default;

protected:
    FontFace(std::shared_ptr<ResourceData> data);
    std::shared_ptr<ResourceData> m_data;
};

class FontResource final : public Resource {
public:
    static RefPtr<FontResource> create(std::shared_ptr<ResourceData> data);
    FontFace* fontFace() const { return m_fontFace.get(); }
    Type type() const final { return Type::Font; }

private:
    FontResource(RefPtr<FontFace> fontFace);
    RefPtr<FontFace> m_fontFace;
};

template<>
struct is<FontResource> {
    static bool check(const Resource& value) { return value.type() == Resource::Type::Font; }
};

} // namespace htmlbook

#endif // RESOURCE_H
