#pragma once
#include <string>
#include "stb_image_write.h"

inline static void SaveToImage(std::string a_Name, int a_W, int a_H, int a_Channels, unsigned char* a_Data)
{
    stbi_flip_vertically_on_write(true);
    const auto result = stbi_write_jpg(a_Name.c_str(), a_W, a_H, a_Channels, a_Data, 100);
    assert(result);
}
