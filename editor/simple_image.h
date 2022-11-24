#pragma once

#include <vector>
#include <algorithm>
#include <filesystem>

namespace MPG
{
    // Represents a color in the color palette
    struct rgba_color
    {
        uint8_t r{ 0 };
        uint8_t g{ 0 };
        uint8_t b{ 0 };
        uint8_t a{ 255 };
        operator uint32_t() { return a << 24 | r << 16 | g << 8 | static_cast<uint32_t>(b); }
    };

    using color_palette = std::vector<rgba_color>;
    using pixel_data = std::vector<uint8_t>;

    // The simplest image structure.
    struct simple_image
    {
        uint32_t width{ 0 };
        uint32_t height{ 0 };
        uint32_t bit_depth{ 1 };

        color_palette color_palette{};
        pixel_data pixel_data{};
    };

    // Applies the palette to the image and returns a new 32-bit image.
    // Useful to use the image as a texture.
    simple_image depalettize_image(simple_image const& source);

    // Returns a new image that is flipped vertically.
    simple_image flip_vertical(simple_image const& source);

    // Crop the image to the specified size and return it as a new image
    simple_image crop(simple_image const& source, uint32_t x, uint32_t y, uint32_t width, uint32_t height);

    // Trims down the source image's palette. Any color outside the new range is changed to the overflow color
    // and returns it as a new image. This does not do any color quantizantion.
    simple_image crop_palette(simple_image const& source, uint8_t bit_depth, uint8_t overflow_color);

    enum class simple_image_format
    {
        bitmap,
        ilbm,
        unknown
    };

    // Determines what type of image we're trying to read.
    simple_image_format determine_image_format(std::filesystem::path const& image_path);

    // Loads an image provided it's one of the supported formats
    simple_image load_image(std::filesystem::path const& filename);

    simple_image load_simple_bitmap(std::filesystem::path const& filename);
    void save_simple_bitmap(std::filesystem::path const& filename, simple_image const& image);

    // Loads an ILBM/IFF image
    // The only supported chunks are:
    //   * FORM
    //   * BMHD
    //   * CMAP
    //   * CAMG
    //   * BODY
    // 
    // Masks and compression are not supported. If an ILBM has a bit mask, it is ignored.
    simple_image load_simple_ilbm(std::filesystem::path const& filename);

    // Saves an image as an ILBM/IFF image
    // Since the Simple Image doesn't have any extra properties, only the minimum required to
    // make a valid ILBM is supported.
    void save_simple_ilbm(std::filesystem::path const& filename, simple_image const& image);

    // Save a palette only ILBM
    void save_ilbm_palette(std::filesystem::path const& filename, simple_image const& image);

    enum class ilbm_mask_type : uint8_t
    {
        none = 0,
        has_mask,
        has_transparent_color,
        lasso
    };

    enum class ilbm_compression_type : uint8_t
    {
        none = 0,
        byte_run
    };

    pixel_data planar_to_chunky(pixel_data const& scanlines, uint32_t width, uint32_t height, uint8_t bitplanes, ilbm_mask_type mask_type);
    pixel_data chunky_to_planar(simple_image const& image);

    #pragma pack(push, 2)
    // I realize this is named blitz "shapes" but it only defines one shape. That's on purpose
    // as this is the blitz "shapes" image format.
    struct blitz_shapes
    {
        uint16_t width{ 0 };            // Shape width
        uint16_t height{ 0 };           // Shape height
        uint16_t bit_depth{ 0 };        // Number of bitplanes
        uint16_t ebwidth{ 0 };          // Even byte width of shape
        uint16_t blitsize{ 0 };
        uint16_t handle_x{ 0 };         // X-coordinate of the shape's handle
        uint16_t handle_y{ 0 };         // Y-coordinate of the shape's handle

        // You'd think these two would be pointers of offset to the data...
        // You'd be wrong as setting them both to zero has no effects.
        // Perhaps they are used internally by Blitz as to where the data is
        // in memory or something.
        uint32_t data_ptr{ 0 };         // Pointer to graphic data
        uint32_t cookie_ptr{ 0 };       // Pointer to cookiecut for one bitplane

        // More shenanigans here. The -x versions make no sense.
        // I mean, I'm sure there's a reason, but not one I can divine.
        uint16_t onebpmem{ 0 };         // Memory used up by one bitplane
        uint16_t onebpmemx{ 0 };        // Same as above plus a word per vertical pixel
        uint16_t allbpmem{ 0 };         // Memory used by the entire shape
        uint16_t allbpmemx{ 0 };        // Same as above plue a word per vertical pixel

        uint16_t padding;               // Padding
        std::vector<uint8_t> data{ 0 }; // Shape data

        size_t get_size() const noexcept
        {
            return sizeof(blitz_shapes) - sizeof(std::vector<uint8_t>) + data.size();
        }
    };
    #pragma pack(pop)

    // Note that the data in the shape *IS NOT* in big-endian format, that needs to be
    // addressed when outputting it to a file.
    blitz_shapes image_to_blitz_shapes(simple_image const& image);

    // Saves a collection of images as an Amiga BLITZ Basic 2 shapes files to be used
    // with BLITZ's "LoadShapes" function.
    void save_blitz_shapes(std::filesystem::path const& filename, std::vector<simple_image> const& images);
}

//#define SIMPLE_IMAGE_IMPL
#ifdef SIMPLE_IMAGE_IMPL

#include <fstream>
#include <stdexcept>

namespace MPG
{
#pragma pack(push, 2)
    constexpr uint16_t bmp_format = 0x4D42;

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
        uint32_t x_pixels_per_meter;    // Usually 0
        uint32_t y_pixels_per_meter;    // Usually 0
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

    simple_image load_simple_bitmap(std::filesystem::path const& filename)
    {
        auto result = simple_image{};

        auto bitmap_file = std::ifstream{ filename, std::ios::binary};
        if (!bitmap_file)
            throw std::runtime_error("Can't open bitmap file.");

        auto header = bmp_header{};
        bitmap_file.read(reinterpret_cast<char*>(&header), sizeof(bmp_header));

        if (header.format != bmp_format)
            throw std::runtime_error("Not a bitmap file.");

        auto info_header = bmp_info_header{};
        bitmap_file.read(reinterpret_cast<char*>(&info_header), sizeof(bmp_info_header));
        
        if (info_header.bits_per_pixel != 8 && info_header.bits_per_pixel != 24 && info_header.bits_per_pixel != 32)
            throw std::runtime_error("Only 8-, 24-, and 32-bit bitmaps are supported.");

        if (info_header.compression_method != 0)
            throw std::runtime_error("Only uncompressed bitmaps are supported.");

        result.width = info_header.width;
        result.height = info_header.height;
        result.bit_depth = info_header.bits_per_pixel;

        auto const bytes_per_pixel = info_header.bits_per_pixel >> 3;
        if (bytes_per_pixel == 1)
        {
            auto const color_count = info_header.palette_color_count ? info_header.palette_color_count : 256;
            result.color_palette.resize(color_count);

            bitmap_file.read(reinterpret_cast<char*>(&result.color_palette[0]), sizeof(rgba_color) * color_count);
        }

        // Bitmaps store palette colors in BRG format.
        std::for_each(result.color_palette.begin(), result.color_palette.end(), [](rgba_color& color)
            {
                color = rgba_color{ color.b, color.g, color.r, 0xFF };
            });

        // Scanlines are aligned to the next 4-byte boundary
        auto const scanline_size = result.width * bytes_per_pixel;
        auto const remainder = scanline_size % 4;
        auto const scanline_padding = (remainder == 0) ? 0 : 4 - remainder;

        auto scanline = std::vector<uint8_t>(scanline_size, 0);
        auto padding = 0u;

        // We need to read the whole scanline and discard the padding
        auto const pixel_data_size = result.width * result.height * bytes_per_pixel;
        result.pixel_data.resize(pixel_data_size);

        for (int y = result.height - 1; y >= 0; y--)
        {
            bitmap_file.read(reinterpret_cast<char*>(&scanline[0]), scanline_size);

            if (scanline_padding > 0)
            {
                bitmap_file.read(reinterpret_cast<char*>(&padding), scanline_padding);
            }

            // BMPs are stored upside down...
            std::copy(
                scanline.begin(), 
                scanline.end(), 
                result.pixel_data.begin() + (static_cast<size_t>(y) * result.width));

        }

        return result;
    }

    void save_simple_bitmap(std::filesystem::path const& filename, simple_image const& source)
    {
        auto image = flip_vertical(source);

        auto const bmp_size = image.width * image.height * (image.bit_depth >> 3);
        auto const palette_size = static_cast<uint32_t>(image.color_palette.size() * sizeof(rgba_color));

        auto header = bmp_header
        {
            bmp_format,
            bmp_size + palette_size,
            0,
            static_cast<uint32_t>(sizeof(bmp_header) + sizeof(bmp_info_header) + (image.color_palette.size() * sizeof(rgba_color)))
        };

        auto info = bmp_info_header
        {
            sizeof(bmp_info_header),
            image.width,
            image.height,
            1, // Colorplanes must be 1
            static_cast<uint16_t>(image.bit_depth),
            0, // Uncompressed
            bmp_size,
            0,
            0,
            static_cast<uint32_t>(image.color_palette.size()),
            static_cast<uint32_t>(image.color_palette.size())
        };

        auto bitmap_file = std::ofstream{ filename, std::ios::binary | std::ios::trunc };
        if (!bitmap_file)
            throw std::runtime_error("Could not open file for writing.");

        bitmap_file.write(reinterpret_cast<char*>(&header), sizeof(bmp_header));
        bitmap_file.write(reinterpret_cast<char*>(&info), sizeof(bmp_info_header));

        if (image.color_palette.size() > 0)
        {
            char padding = 0;

            for (auto const& color : image.color_palette)
            {
                bitmap_file.write(reinterpret_cast<char const*>(&color.b), 1);
                bitmap_file.write(reinterpret_cast<char const*>(&color.g), 1);
                bitmap_file.write(reinterpret_cast<char const*>(&color.r), 1);
                bitmap_file.write(&padding, 1);
            }

            // Each scanline is aligned to the next 4 byte boundary, this happens naturally for the 24-bit bitmaps since they
            // write four bytes per pixel, but has be to accounted for with indexed images.
            auto const scanline_padding = 4u - (image.width % 4u);

            for (auto y = 0u; y < image.height; y++)
            {
                auto const start_index = y * image.width;
                bitmap_file.write(reinterpret_cast<char const*>(&image.pixel_data[start_index]), image.width);

                for (auto p = 0u; p < scanline_padding; p++)
                {
                    bitmap_file.write(&padding, 1);
                }
            }
        }
        else
        {
            for (size_t pixel = 0; pixel < image.pixel_data.size(); pixel += 4)
            {
                bitmap_file.write(reinterpret_cast<char const*>(&image.pixel_data[pixel + 2]), 1);
                bitmap_file.write(reinterpret_cast<char const*>(&image.pixel_data[pixel + 1]), 1);
                bitmap_file.write(reinterpret_cast<char const*>(&image.pixel_data[pixel]), 1);
                bitmap_file.write(reinterpret_cast<char const*>(&image.pixel_data[pixel + 3]), 1);
            }
        }
    }

    constexpr uint32_t iff_form_name = 0x464F524D;
    constexpr uint32_t iff_ilbm_name = 0x494C424D;
    constexpr uint32_t ilbm_bmhd_name = 0x424D4844;
    constexpr uint32_t ilbm_cmap_name = 0x434D4150;
    constexpr uint32_t ilbm_camg_name = 0x43414D47;
    constexpr uint32_t ilbm_body_name = 0x424F4459;

#pragma pack(push, 1)
    struct iff_chunk
    {
        uint32_t name;
        uint32_t size;
    };

    struct ilbm_bmhd_chunk
    {
        uint16_t width;
        uint16_t height;
        int16_t x;                  // Can be 0
        int16_t y;                  // Can be 0
        uint8_t bitplanes;
        ilbm_mask_type mask_type;
        ilbm_compression_type compression_type;
        uint8_t padding;            // ignore on read, write as 0
        uint16_t transparent_color;
        uint8_t aspect_ratio_x;
        uint8_t aspect_ratio_y;
        uint16_t page_width;
        uint16_t page_height;
    };

    struct ilbm_cmap_color
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };

    struct blitz_shape_header
    {
        uint16_t width;
        uint16_t height;
        uint16_t bitplanes;
        uint16_t ebwidth;       // Even-byte width
        uint16_t blitsize;
        uint16_t xhandle;
        uint16_t yhandle;
        uint32_t data_ptr;      // Pointer to graphics data
        uint32_t cookie_ptr;    // Pointer to a 1-bitplane cookie cut
        uint16_t onebpmem;      // Memory taken by one bitplane
        uint16_t onebpmemx;     // As onebpmem plus and extra word per vertical pixel
        uint16_t allbpmem;      // Memory take by whole shape
        uint16_t allbpmemx;     // As allbpmem plus and extra word per bitplane per vertical pixel
        uint16_t padding;
    };

#pragma pack(pop)

    uint16_t swap16(uint16_t in)
    {
        auto data = reinterpret_cast<char const*>(&in);

        // The last byte needs to be cast to unsigned so that the sign-bit doesn't mess up the operation.
        return data[0] << 8 | static_cast<uint8_t>(data[1]);
    }

    uint32_t swap32(uint32_t in)
    {
        auto data = reinterpret_cast<uint16_t const*>(&in);
        return swap16(data[0]) << 16 | swap16(data[1]);
    }

    uint8_t read_u8(std::ifstream& ifs)
    {
        auto value = uint8_t{};
        ifs.read(reinterpret_cast<char*>(&value), sizeof(uint8_t));
        return value;
    }

    uint16_t read_swap_u16(std::ifstream& ifs)
    {
        auto value = uint16_t{};
        ifs.read(reinterpret_cast<char*>(&value), sizeof(uint16_t));
        return swap16(value);
    }

    uint32_t read_swap_u32(std::ifstream& ifs)
    {
        auto value = uint32_t{};
        ifs.read(reinterpret_cast<char*>(&value), sizeof(uint32_t));
        return swap32(value);
    }

    void write_u8(std::ofstream& ofs, uint8_t value)
    {
        ofs.write(reinterpret_cast<char*>(&value), sizeof(uint8_t));
    }

    void write_swap_u16(std::ofstream& ofs, uint16_t value)
    {
        auto swapped = swap16(value);
        ofs.write(reinterpret_cast<char*>(&swapped), sizeof(uint16_t));
    }

    void write_swap_u32(std::ofstream& ofs, uint32_t value)
    {
        auto swapped = swap32(value);
        ofs.write(reinterpret_cast<char*>(&swapped), sizeof(uint32_t));
    }

    iff_chunk read_chunk_header(std::ifstream& ifs)
    {
        auto value = iff_chunk{};
        value.name = read_swap_u32(ifs);
        value.size = read_swap_u32(ifs);
        return value;
    }

    void write_chunk_header(std::ofstream& ofs, iff_chunk const& chunk)
    {
        write_swap_u32(ofs, chunk.name);
        write_swap_u32(ofs, chunk.size);
    }

    ilbm_bmhd_chunk read_bmhd_chunk(std::ifstream& ifs)
    {
        auto bmhd = ilbm_bmhd_chunk{};

        bmhd.width = read_swap_u16(ifs);
        bmhd.height = read_swap_u16(ifs);
        bmhd.x = read_swap_u16(ifs);
        bmhd.y = read_swap_u16(ifs);

        bmhd.bitplanes = read_u8(ifs);

        bmhd.mask_type = static_cast<ilbm_mask_type>(read_u8(ifs));
        if (bmhd.mask_type == ilbm_mask_type::lasso)
            throw std::runtime_error("Lasso mask is unsupported.");

        bmhd.compression_type = static_cast<ilbm_compression_type>(read_u8(ifs));
        if (bmhd.compression_type != ilbm_compression_type::none)
            throw std::runtime_error("Only uncompressed ILBMs are supported.");

        bmhd.padding = read_u8(ifs);

        bmhd.transparent_color = read_swap_u16(ifs);

        bmhd.aspect_ratio_x = read_u8(ifs);
        bmhd.aspect_ratio_y = read_u8(ifs);

        bmhd.page_width = read_swap_u16(ifs);
        bmhd.page_height = read_swap_u16(ifs);

        return bmhd;
    }

    void write_bmhd_chunk(std::ofstream& ofs, ilbm_bmhd_chunk const& bmhd)
    {
        write_swap_u16(ofs, bmhd.width);
        write_swap_u16(ofs, bmhd.height);
        write_swap_u16(ofs, bmhd.x);
        write_swap_u16(ofs, bmhd.y);

        write_u8(ofs, bmhd.bitplanes);
        write_u8(ofs, static_cast<uint8_t>(bmhd.mask_type));
        write_u8(ofs, static_cast<uint8_t>(bmhd.compression_type));
        write_u8(ofs, bmhd.padding);

        write_swap_u16(ofs, bmhd.transparent_color);

        write_u8(ofs, bmhd.aspect_ratio_x);
        write_u8(ofs, bmhd.aspect_ratio_y);

        write_swap_u16(ofs, bmhd.page_width);
        write_swap_u16(ofs, bmhd.page_height);
    }

    color_palette read_cmap_colors(std::ifstream& ifs, size_t palette_size)
    {
        if (palette_size % 3 != 0)
            throw std::runtime_error("The palette size should be divible by 3 (one byte per color channel).");

        auto color_count = palette_size / 3;
        auto palette = color_palette{};
        palette.reserve(color_count);

        for (auto index = 0; index < color_count; index++)
        {
            auto color = rgba_color{};
            color.r = read_u8(ifs);
            color.g = read_u8(ifs);
            color.b = read_u8(ifs);

            palette.push_back(color);
        }

        return palette;
    }

    void write_cmap_colors(std::ofstream& ofs, color_palette const& palette)
    {
        for (auto const& color : palette)
        {
            write_u8(ofs, color.r);
            write_u8(ofs, color.g);
            write_u8(ofs, color.b);
        }
    }

    void read_camg_flags(std::ifstream& ifs)
    {
        auto const flags = read_swap_u32(ifs);

        if ((flags & 0x80) > 0)
            throw std::runtime_error("Extra Halfbrite images not supported.");

        if ((flags & 0x800) > 0)
            throw std::runtime_error("Hold and Modify images not supported.");
    }

    pixel_data read_body_pixels(std::ifstream& ifs, ilbm_bmhd_chunk const& bmhd, size_t size)
    {
        auto pixels = pixel_data{};
        pixels.reserve(static_cast<size_t>(bmhd.width) * bmhd.height);

        auto scanlines = std::vector<uint8_t>(size);
        ifs.read(reinterpret_cast<char*>(&scanlines[0]), size);

        return planar_to_chunky(scanlines, bmhd.width, bmhd.height, bmhd.bitplanes, bmhd.mask_type);
    }

    pixel_data planar_to_chunky(pixel_data const& scanlines, uint32_t width, uint32_t height, uint8_t bitplanes, ilbm_mask_type mask_type)
    {
        auto pixels = pixel_data{};
        pixels.reserve(static_cast<size_t>(width) * height);

        auto const row_length = ((width + 15) / 16) * 2;
        auto const scanline_length = row_length * bitplanes + ((mask_type == ilbm_mask_type::has_mask) ? row_length : 0);

        for (auto y = 0u; y < height; y++)
        {
            for (auto x = 0u; x < width; x++)
            {
                uint8_t pixel = 0;
                for (auto plane = 0u; plane < bitplanes; plane++)
                {
                    // Figure out which byte and bit corresponds to this x value
                    auto const byte = x / 8u;
                    auto const bit = 7 - (x % 8u);

                    auto value = scanlines[(static_cast<size_t>(y) * scanline_length) + (static_cast<size_t>(row_length) * plane) + byte];
                    //auto value = scanlines[(y * row_length * plane) + byte];

                    // Isolate the bit
                    uint8_t mask = 1 << bit;
                    value = (value & mask) >> bit;

                    // Add it to the pixel
                    pixel |= (value << plane);
                }

                pixels.push_back(pixel);
            }
        }

        return pixels;
    }

    pixel_data chunky_to_planar(simple_image const& image)
    {
        auto const row_length = ((image.width + 15u) / 16u) * 2u;
        auto const scanline_length = row_length * image.bit_depth;
        auto const total_size = scanline_length * image.height;

        // Since we don't support them, we don't have to account for a mask row
        auto planar = pixel_data(total_size, 0);

        for (auto y = 0u; y < image.height; y++)
        {
            for (auto x = 0u; x < image.width; x++)
            {
                auto const pixel = image.pixel_data[static_cast<size_t>(y) * image.width + x];

                for (auto plane = 0u; plane < image.bit_depth; plane++)
                {
                    // Get the bit that corresponds to this plane
                    auto const color_bit_mask = 1u << plane;
                    auto const color_bit = (pixel & color_bit_mask) != 0 ? 1u : 0u;

                    // Figure out which byte and bit corresponds to this x value
                    auto const byte = x / 8u;
                    auto const bit = 7 - (x % 8u);
                    auto const index = (y * scanline_length) + (row_length * plane) + byte;

                    // Get the byte already there and add the color_bit in the right place
                    auto value = planar[index];
                    auto const planar_bit = color_bit << bit;
                    value |= planar_bit;

                    planar[index] = value;
                }
            }
        }

        return planar;
    }

    ilbm_bmhd_chunk get_bmhd_chunk(simple_image const& image)
    {
        return ilbm_bmhd_chunk
        {
            static_cast<uint16_t>(image.width),         // width
            static_cast<uint16_t>(image.height),        // height
            0, 0,                                       // x and y
            static_cast<uint8_t>(image.bit_depth),      // number of bitplanes
            ilbm_mask_type::none,                       // no support for writing masks
            ilbm_compression_type::none,                // no support for compression
            0,                                          // padding, should be 0
            0,                                          // no support for writing transparent colors... yet
            1, 1,                                       // aspect ratio 1:1
            static_cast<uint16_t>(image.width),         // page width
            static_cast<uint16_t>(image.height),        // page height
        };
    }

    simple_image load_simple_ilbm(std::filesystem::path const& filename)
    {
        auto result = simple_image{};

        auto bitmap_file = std::ifstream{ filename, std::ios::binary };
        if (!bitmap_file)
            throw std::runtime_error("Can't open bitmap file.");

        // Read the required chunks: FORM, ILBM, and BMHD. These must always exist in this order.
        auto const form = read_chunk_header(bitmap_file);
        if (form.name != iff_form_name)
            throw std::runtime_error("Not an ILBM image, missing FORM chunk.");

        auto const ilbm_name = read_swap_u32(bitmap_file);
        if (ilbm_name != iff_ilbm_name)
            throw std::runtime_error("Not an ILBM image, missing ILBM chunk.");

        auto const bmhd_header = read_chunk_header(bitmap_file);
        if (bmhd_header.name != ilbm_bmhd_name)
            throw std::runtime_error("Not an ILBM image, missing BMHD chunk.");

        auto const bmhd = read_bmhd_chunk(bitmap_file);
        result.width = bmhd.width;
        result.height = bmhd.height;
        result.bit_depth = bmhd.bitplanes;

        // Finally, read all remaining chunks.
        while (!bitmap_file.eof())
        {
            auto chunk = read_chunk_header(bitmap_file);
            switch (chunk.name)
            {
            case ilbm_cmap_name:
                result.color_palette = read_cmap_colors(bitmap_file, chunk.size);
                break;
            case ilbm_camg_name:
                read_camg_flags(bitmap_file);
                break;
            case ilbm_body_name:
                result.pixel_data = read_body_pixels(bitmap_file, bmhd, chunk.size);
                return result;
                break;
            default:
                // Unsupported header, skip it.
                bitmap_file.seekg(chunk.size, std::ios_base::cur);
            }
        }

        return result;
    }

    void save_simple_ilbm(std::filesystem::path const& filename, simple_image const& image)
    {
        // At the moment, we only support pre-palettized images
        if (image.bit_depth > 8 || image.color_palette.size() == 0)
            throw std::runtime_error("Only palettized images are supported.");

        // Prepare our chunks and data
        auto const bmhd_data = get_bmhd_chunk(image);
        auto const bmhd_chunk = iff_chunk{ ilbm_bmhd_name, sizeof(ilbm_bmhd_chunk) };
        auto const cmap_chunk = iff_chunk{ ilbm_cmap_name, static_cast<uint32_t>(image.color_palette.size() * sizeof(ilbm_cmap_color))};
        auto const camg_chunk = iff_chunk{ ilbm_camg_name, sizeof(uint32_t) };
        auto const body_data = chunky_to_planar(image);
        auto const body_chunk = iff_chunk{ ilbm_body_name, static_cast<uint32_t>(body_data.size()) };

        auto const total_size =
            sizeof iff_ilbm_name                // ILBM
            + sizeof bmhd_chunk                 // BODY
            + bmhd_chunk.size                   // BODY data
            + sizeof cmap_chunk                 // CMAP
            + cmap_chunk.size                   // CMAP data
            + sizeof camg_chunk                 // CAMG
            + camg_chunk.size                   // CAMG data
            + sizeof body_chunk                 // BODY
            + body_chunk.size                   // BODY data
            ;

        auto const form_chunk = iff_chunk
        {
            iff_form_name,
            static_cast<uint32_t>(total_size)
        };

        auto ofs = std::ofstream{ filename, std::ios::binary, std::ios::trunc };
        if (!ofs)
            throw std::runtime_error{ "Could not create file for writing." };

        write_chunk_header(ofs, form_chunk);
        write_swap_u32(ofs, iff_ilbm_name);
        
        write_chunk_header(ofs, bmhd_chunk);
        write_bmhd_chunk(ofs, bmhd_data);

        write_chunk_header(ofs, cmap_chunk);
        write_cmap_colors(ofs, image.color_palette);

        write_chunk_header(ofs, camg_chunk);
        write_swap_u32(ofs, 0u);

        if (body_data.size() > 0)
        {
            write_chunk_header(ofs, body_chunk);
            ofs.write(reinterpret_cast<char const*>(&body_data[0]), body_data.size());
        }
    }

    void save_ilbm_palette(std::filesystem::path const& filename, simple_image const& image)
    {
        if (image.bit_depth > 8 || image.color_palette.size() == 0)
            throw std::runtime_error("Can't save the palette of an image without one.");

        auto palette_image = simple_image
        {
            0, 0,
            image.bit_depth,
            image.color_palette,
            {}
        };

        save_simple_ilbm(filename, palette_image);
    }

    blitz_shapes image_to_blitz_shapes(simple_image const& image)
    {
        auto shape = blitz_shapes
        {
            static_cast<uint16_t>(image.width),
            static_cast<uint16_t>(image.height),
            static_cast<uint16_t>(image.bit_depth),
        };

        // Scanlines are aligned to the next word
        shape.ebwidth = static_cast<uint16_t>(((image.width + 15) / 16) * 2);

        // I barely understand why this is, but the blit size is actually a packing of the width
        // and height. Was put on the right path by: https://eab.abime.net/showpost.php?p=1312463&postcount=8
        shape.blitsize = static_cast<uint16_t>((image.height << 6) | (shape.ebwidth >> 1) + 1);

        // Number of bytes used up by one bitplane
        shape.onebpmem = static_cast<uint16_t>(shape.ebwidth * image.height);
        shape.allbpmem = static_cast<uint16_t>(shape.onebpmem * image.bit_depth);

        // Nonsense. Just, nonsense.
        shape.onebpmemx = static_cast<uint16_t>(shape.onebpmem + (image.height * 2));
        shape.allbpmemx = static_cast<uint16_t>(shape.onebpmemx * image.bit_depth);

        // Would it that ILBMs and shapes used the same planar format... but they don't.
        // Where in ILBMs, each scanline is split by planes, in Shapes, each plane is stored
        // in its entirety, followed by the next and so on.
        shape.data = pixel_data(shape.allbpmem, 0);

        for (auto plane = 0u; plane < shape.bit_depth; plane++)
        {
            for (auto y = 0u; y < shape.height; y++)
            {
                for (auto x = 0u; x < shape.width; x++)
                {
                    // Figure out which byte and bit corresponds to this x value
                    auto const byte = x / 8u;
                    auto const bit = 7 - (x % 8u);

                    // Get the bit that corresponds to this plane
                    auto pixel = image.pixel_data[static_cast<size_t>(y) * image.width + x];
                    auto const color_bit_mask = 1u << plane;
                    auto const color_bit = (pixel & color_bit_mask) != 0 ? 1u : 0u;
                    auto const planar_bit = color_bit << bit;

                    auto const index = (shape.onebpmem * plane) + (y * shape.ebwidth) + byte;
                    auto value = shape.data[index];
                    value |= planar_bit;

                    shape.data[index] = value;
                }
            }
        }

        return shape;
    }

    void save_blitz_shapes(std::filesystem::path const& filename, std::vector<simple_image> const& images)
    {
        auto ofs = std::ofstream{ filename, std::ios::binary, std::ios::trunc };
        if (!ofs)
            throw std::runtime_error{ "Could not create file for writing." };

        for (auto const& image : images)
        {
            auto shape = image_to_blitz_shapes(image);

            // Write the shape header
            write_swap_u16(ofs, shape.width);
            write_swap_u16(ofs, shape.height);
            write_swap_u16(ofs, shape.bit_depth);
            write_swap_u16(ofs, shape.ebwidth);
            write_swap_u16(ofs, shape.blitsize);

            // Handle is in the top left. Perhaps I can add support for moving them later
            write_swap_u16(ofs, 0);     // x
            write_swap_u16(ofs, 0);     // y

            // Data and cookie pointers. They seem to always be nonsense values in the shapes files created by Blitz
            write_swap_u32(ofs, 0);     // data
            write_swap_u32(ofs, 0);     // cookie

            write_swap_u16(ofs, shape.onebpmem);
            write_swap_u16(ofs, shape.onebpmemx);
            write_swap_u16(ofs, shape.allbpmem);
            write_swap_u16(ofs, shape.allbpmemx);

            write_swap_u16(ofs, 0);     // padding

            // Write out the shape's bitplanes
            if (shape.data.size() > 0)
            {
                ofs.write(reinterpret_cast<char const*>(&shape.data[0]), shape.data.size());
            }
        }
    }

    simple_image depalettize_image(simple_image const& source)
    {
        auto result = simple_image{ source.width, source.height, 32 };

        //if (source.bit_depth != 8)
        //    throw std::runtime_error{ "Source image is not an 8bit image." };
        if (source.color_palette.size() == 0)
            throw std::runtime_error{ "Source image doesn't have a palette" };

        result.pixel_data.reserve(static_cast<size_t>(source.width) * source.height * 4);

        auto inflate = [&source, &result](auto const& pixel)
        {
            auto const& color = source.color_palette[pixel];
            result.pixel_data.push_back(color.r);
            result.pixel_data.push_back(color.g);
            result.pixel_data.push_back(color.b);
            result.pixel_data.push_back(color.a);
        };

        std::ranges::for_each(source.pixel_data, inflate);

        return result;
    }

    simple_image flip_vertical(simple_image const& source)
    {
        auto result = simple_image
        {
            source.width,
            source.height,
            source.bit_depth,
            source.color_palette
        };

        result.pixel_data.reserve(source.pixel_data.size());

        auto const bytes_per_pixel = source.bit_depth >> 3;
        auto const width = static_cast<int64_t>(source.width) * bytes_per_pixel;
        auto const height = static_cast<int64_t>(source.height) * bytes_per_pixel;

        for (auto y = height - 1; y >= 0; y--)
        {
            for (int x = 0; x < width; x++)
            {
                result.pixel_data.push_back(source.pixel_data[y * width + x]);
            }
        }

        return result;
    }

    simple_image crop(simple_image const& source, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        if (source.width < width || source.height < height || x >= source.width || y >= source.height)
            throw std::exception("The crop is out of image bounds");

        auto result = simple_image
        {
            width, height,
            source.bit_depth,
            source.color_palette
        };

        auto const bytes_per_pixel = std::max(source.bit_depth >> 3, 1u);
        auto const scan_width = static_cast<int64_t>(source.width) * bytes_per_pixel;
        auto const crop_start_x = static_cast<int64_t>(x) * bytes_per_pixel;
        auto const crop_start_y = static_cast<int64_t>(y) * bytes_per_pixel;
        auto const crop_end_x = crop_start_x + (static_cast<int64_t>(width) * bytes_per_pixel);
        auto const crop_end_y = crop_start_y + (static_cast<int64_t>(height) * bytes_per_pixel);

        result.pixel_data.reserve(static_cast<size_t>(width) * height * bytes_per_pixel);

        for (auto y_pos = crop_start_y; y_pos < crop_end_y; y_pos++)
        {
            for (auto x_pos = crop_start_x; x_pos < crop_end_x; x_pos++)
            {
                for (auto byte = 0u; byte < bytes_per_pixel; byte++)
                {
                    result.pixel_data.push_back(source.pixel_data[y_pos * scan_width + x_pos + byte]);
                }
            }
        }

        return result;
    }

    simple_image crop_palette(simple_image const& source, uint8_t bit_depth, uint8_t overflow_color)
    {
        auto const color_count = 1 << bit_depth;

        if (bit_depth > 8)
            throw std::runtime_error("Bit-depth can't be more than 8");

        auto result = simple_image
        {
            source.width,
            source.height,
            bit_depth
        };

        // Create a new palette with the requested colors
        auto const max_colors = std::min(color_count, static_cast<int>(source.color_palette.size()));

        result.color_palette.resize(color_count);
        std::copy(
            source.color_palette.begin(),
            source.color_palette.begin() + max_colors,
            result.color_palette.begin()
        );

        if (source.color_palette.size() < color_count || source.bit_depth <= bit_depth)
        {
            for (auto i = source.color_palette.size(); i < color_count; i++)
            {
                result.color_palette[i] = rgba_color{};
            }
        }

        // Adjust any colors that are out of range
        result.pixel_data.resize(static_cast<size_t>(source.width) * source.height);
        std::transform(
            source.pixel_data.begin(),
            source.pixel_data.end(),
            result.pixel_data.begin(),
            [&color_count, &overflow_color](uint8_t pixel)
            {
                return (pixel >= color_count) ? overflow_color : pixel;
            });

        return result;
    }

    simple_image_format determine_image_format(std::filesystem::path const& image_path)
    {
        auto file_stream = std::ifstream{ image_path, std::ios::binary };
        if (!file_stream)
            throw std::runtime_error("Could not open file.");

        auto magic = 0u;
        file_stream.read(reinterpret_cast<char*>(&magic), sizeof(uint32_t));

        if (magic == 0x4D524F46) // 'FORM', this could be an ILBM file
            return simple_image_format::ilbm;

        if ((magic & 0x0000FFFF) == 0x4D42) // 'BM' this could be a BMP file
            return simple_image_format::bitmap;

        return simple_image_format::unknown;
    }

    simple_image load_image(std::filesystem::path const& filename)
    {
        switch (determine_image_format(filename))
        {
        case simple_image_format::bitmap:
            return load_simple_bitmap(filename);

        case simple_image_format::ilbm:
            return load_simple_ilbm(filename);

        default:
            throw std::runtime_error("File is not a recognized image format.");
        }
    }
}

#endif // SIMPLE_IMAGE_IMPL