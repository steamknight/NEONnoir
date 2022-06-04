#pragma once
#include <stdint.h>

namespace NEONnoir
{
#pragma pack(push, 2)
    struct bmp_header
    {
        uint16_t format;                // Must be 'BM'
        uint32_t bmp_size;
        uint32_t reserved;
        uint32_t image_offset;
    };

    struct bmp_info_header
    {
        uint32_t header_size;           // Must be '40'
        uint32_t width;
        uint32_t height;
        uint16_t color_planes;          // Must be '1'
        uint16_t bits_per_pixel;
        uint32_t compression_method;
        uint32_t bmp_size;              // 0 if uncompressed
        uint32_t horizontal_res;
        uint32_t vertical_res;
        uint32_t palette_color_count;   // 0 means 2^bits_per_pixel
        uint32_t important_colors;      // generally ignored
    };

    struct rgb_color
    {
        uint8_t blue;
        uint8_t green;
        uint8_t red;
        uint8_t alpha;
    };
#pragma pack(pop)
}
