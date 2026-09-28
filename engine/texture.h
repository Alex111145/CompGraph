/**
 * @file		texture.h
 * @brief	2D texture (loaded from an image file via stb_image)
 *
 * @author	Alessio Gervasini
 */
#pragma once

   #include <memory>
   #include <string>

namespace Eng {

class ENG_API Texture final
{
public:

   Texture();
   Texture(Texture const &) = delete;
   ~Texture();

   void operator=(Texture const &) = delete;

   bool loadFromFile(const std::string &filename);
   bool loadFromEncodedMemory(const unsigned char *encodedBytes, int byteCount);
   void loadFromPixels(int width, int height, int channels, const unsigned char *pixels);

   void bind() const;

private:

   struct Reserved;
   std::unique_ptr<Reserved> reserved;
};

};
