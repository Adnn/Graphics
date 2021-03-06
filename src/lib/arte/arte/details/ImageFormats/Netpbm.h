#pragma once

#include "../../Image.h"

#include <math/Color.h>


static constexpr std::size_t gChunkSize = 256/*kB*/ * 1024/*B*/;


namespace ad {
namespace detail {


    using Rgb = math::sdr::Rgb;
    using Grayscale = math::sdr::Grayscale;


    enum class NetpbmFormat
    {
        Pgm,
        Ppm,
    };

    constexpr const char * magicNumber(NetpbmFormat aFormat)
    {
        switch(aFormat)
        {
        case NetpbmFormat::Pgm:
            return "P5";
        case NetpbmFormat::Ppm:
            return "P6";
        }
    };

    std::string to_string(NetpbmFormat aFormat)
    {
        switch(aFormat)
        {
        case NetpbmFormat::Pgm:
            return "PGM";
        case NetpbmFormat::Ppm:
            return "PPM";
        }
    };


    template <NetpbmFormat N_format>
    struct format_trait
    {};

    template <>
    struct format_trait<NetpbmFormat::Pgm>
    { using pixel_type = Grayscale; };

    template <>
    struct format_trait<NetpbmFormat::Ppm>
    { using pixel_type = Rgb; };


    template <NetpbmFormat N_format>
    struct Netpbm
    {
        static constexpr const char * magic = magicNumber(N_format);
        using pixel_type = typename format_trait<N_format>::pixel_type;

        static math::Size<2, int> ReadHeader(std::istream & aIn)
        {
            char magic1{0}, magic2{0};
            aIn >> magic1 >> magic2;
            if (magic1 != magic[0] || magic2 != magic[1])
            {
                throw std::runtime_error("Invalid header for " + to_string(N_format) + " content");
            }

            int width{-1}, height{-1}, maxValue{0};
            aIn >> width >> height >> maxValue;

            if (width <= 0 || height <= 0)
            {
                throw std::runtime_error("Invalid PPM dimensions");
            }
            if (maxValue != 255)
            {
                throw std::runtime_error("Unhandled " + to_string(N_format) + " maximum value of "
                                         + std::to_string(maxValue));
            }

            aIn.get(); // consume the one byte appearing after max value and before raster data
                       // (carriage return or line feed)

            return {width, height};
        }

        static Image<pixel_type> Read(std::istream & aIn)
        {
            auto dimensions = ReadHeader(aIn);

            std::size_t remainingBytes = dimensions.area() * sizeof(pixel_type);
            auto data = std::make_unique<char[]>(remainingBytes);
            char * currentDestination = data.get();

            while (remainingBytes)
            {
                int readSize = std::min(remainingBytes, gChunkSize);
                if (!aIn.read(currentDestination, readSize).good())
                {
                    // If the stream is not good, but its converts to "true", it means it reached eof
                    // see: https://en.cppreference.com/w/cpp/io/basic_ios/good#see_also
                    throw std::runtime_error("Invalid " + to_string(N_format) + " content: " +
                        (aIn ? "truncated content" : "read error"));
                }
                remainingBytes -= readSize;
                currentDestination += readSize;
            }

            if (aIn.peek(), !aIn.eof())
            {
                throw std::runtime_error("Invalid " + to_string(N_format)
                                         + " content: trailing data");
            }

            return {dimensions, std::move(data)};
        }

        static void Write(std::ostream & aOut, const Image<pixel_type> & aImage)
        {
            if(!aOut.good())
            {
                throw std::runtime_error("Output stream is not valid for writing");
            }

            aOut << magic << '\n'
                << aImage.width() << ' ' << aImage.height() << '\n'
                << "255\n"
                ;

            std::size_t remainingBytes = aImage.size_bytes();
            const char * currentSource = reinterpret_cast<const char *>(aImage.data());

            while (remainingBytes)
            {
                int writeSize = std::min(remainingBytes, gChunkSize);
                if (!aOut.write(currentSource, writeSize).good())
                {
                    throw std::runtime_error("Error writing "+ to_string(N_format)
                                             + " pixel data to stream");
                }
                remainingBytes -= writeSize;
                currentSource += writeSize;
            }
        }
    };

} // namespace detail
} // namespace ad
