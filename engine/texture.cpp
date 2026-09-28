/**
 * @file		texture.cpp
 * @brief	2D texture (loaded from an image file via stb_image)
 *
 * @author	Alessio Gervasini
 */

   #include "engine.h"

   #include <iostream>

   #include <GLFW/glfw3.h>

   #define STB_IMAGE_IMPLEMENTATION
   #include "thirdparty/stb_image.h"

struct Eng::Texture::Reserved
{
   GLuint id;

   Reserved() : id{ 0 }
   {}

   void upload(int width, int height, int channels, const unsigned char *pixels)
   {
      GLenum format = GL_RGB;
      if (channels == 1)
         format = GL_LUMINANCE;
      else if (channels == 4)
         format = GL_RGBA;

      glBindTexture(GL_TEXTURE_2D, id);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(format), width, height, 0, format, GL_UNSIGNED_BYTE, pixels);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   }

   bool uploadDecoded(unsigned char *decodedPixels, int width, int height, int channels, const std::string &source)
   {
      if (decodedPixels == nullptr)
      {
         std::cout << "ERROR: unable to decode texture " << source << ": " << stbi_failure_reason() << std::endl;
         return false;
      }

      upload(width, height, channels, decodedPixels);
      stbi_image_free(decodedPixels);
      return true;
   }
};

ENG_API Eng::Texture::Texture() : reserved(std::make_unique<Eng::Texture::Reserved>())
{
   glGenTextures(1, &reserved->id);
}

ENG_API Eng::Texture::~Texture()
{
   glDeleteTextures(1, &reserved->id);
}

bool ENG_API Eng::Texture::loadFromFile(const std::string &filename)
{
   int width, height, channels;
   stbi_set_flip_vertically_on_load(true);
   unsigned char *decodedPixels = stbi_load(filename.c_str(), &width, &height, &channels, 0);
   return reserved->uploadDecoded(decodedPixels, width, height, channels, "'" + filename + "'");
}

bool ENG_API Eng::Texture::loadFromEncodedMemory(const unsigned char *encodedBytes, int byteCount)
{
   int width, height, channels;
   stbi_set_flip_vertically_on_load(true);
   unsigned char *decodedPixels = stbi_load_from_memory(encodedBytes, byteCount, &width, &height, &channels, 0);
   return reserved->uploadDecoded(decodedPixels, width, height, channels, "(embedded)");
}

void ENG_API Eng::Texture::loadFromPixels(int width, int height, int channels, const unsigned char *pixels)
{
   reserved->upload(width, height, channels, pixels);
}

void ENG_API Eng::Texture::bind() const
{
   glBindTexture(GL_TEXTURE_2D, reserved->id);
}
