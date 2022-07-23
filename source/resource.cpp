#include "resource.h"

namespace htmlbook {

RefPtr<TextResource> TextResource::create(std::shared_ptr<ResourceData> data)
{
    return nullptr;
}

std::string TextResource::decode(const uint8_t* data, size_t length, std::string_view mimeType, std::string_view textEncoding)
{
    std::string value;
    return value;
}

RefPtr<ImageResource> ImageResource::create(std::shared_ptr<ResourceData> data)
{
    return nullptr;
}

RefPtr<FontResource> FontResource::create(std::shared_ptr<ResourceData> data)
{
    return nullptr;
}

} // namespace htmlbook
