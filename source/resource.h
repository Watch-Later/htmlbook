#ifndef RESOURCE_H
#define RESOURCE_H

#include "pointer.h"

#include <memory>
#include <string>

namespace htmlbook {

class ByteData;

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
    static RefPtr<TextResource> create(std::shared_ptr<ByteData> data, std::string_view mimeType, std::string_view textEncoding);
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
    virtual bool isBitmapImage() const { return false; }
    virtual bool isVectorImage() const { return false; }

protected:
    Image() = default;
};

class ImageResource final : public Resource {
public:
    static RefPtr<ImageResource> create(std::shared_ptr<ByteData> data, std::string_view mimeType, std::string_view textEncoding);
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
    RefPtr<FontFace> create(std::shared_ptr<ByteData> data);

private:
    FontFace(std::shared_ptr<ByteData> data);
    std::shared_ptr<ByteData> m_data;
};

class FontResource final : public Resource {
public:
    static RefPtr<FontResource> create(std::shared_ptr<ByteData> data, std::string_view mimeType, std::string_view textEncoding);
    FontFace* face() const { return m_face.get(); }
    Type type() const final { return Type::Font; }

private:
    FontResource(RefPtr<FontFace> face);
    RefPtr<FontFace> m_face;
};

template<>
struct is<FontResource> {
    static bool check(const Resource& value) { return value.type() == Resource::Type::Font; }
};

} // namespace htmlbook

#endif // RESOURCE_H
