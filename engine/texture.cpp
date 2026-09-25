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

/** @brief Texture class reserved structure (PIMPL/Bridge pattern, same as Eng::Base). */
struct Eng::Texture::Reserved
{
   GLuint id;

   Reserved() : id{ 0 }
   {}

   /**
    * Shared by load() and loadFromMemory(): uploads raw pixels and applies the default
    * filter/wrapping settings. Assumes id is already bound-worthy (glGenTextures done).
    * No mipmaps: automatic mipmap generation (glGenerateMipmap) is OpenGL 3.0, and the manual
    * OpenGL 1.1 way (building each reduced level by hand) is not worth it for this project.
    */
   void upload(int width, int height, int channels, const unsigned char *data)
   {
      GLenum format = GL_RGB;
      if (channels == 1)
         format = GL_LUMINANCE;
      else if (channels == 3)
         format = GL_RGB;
      else if (channels == 4)
         format = GL_RGBA;

      glBindTexture(GL_TEXTURE_2D, id);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);   // stb_image rows are tightly packed; default 4 would skew RGB widths not multiple of 4
      glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(format), width, height, 0, format, GL_UNSIGNED_BYTE, data);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   }
};

/** Constructor: allocates the GL texture id (still empty, load() fills it). */
ENG_API Eng::Texture::Texture() : reserved(std::make_unique<Eng::Texture::Reserved>())
{
   glGenTextures(1, &reserved->id);
}

/** Destructor. */
ENG_API Eng::Texture::~Texture()
{
   glDeleteTextures(1, &reserved->id);
}

/**
 * Loads an image file, uploads it to the GPU, and configures filtering/wrapping.
 * @return TF
 */
bool ENG_API Eng::Texture::load(const std::string &filename)
{
   stbi_set_flip_vertically_on_load(true);

   int width, height, channels;
   unsigned char *data = stbi_load(filename.c_str(), &width, &height, &channels, 0);
   if (data == nullptr)
   {
      std::cout << "ERROR: unable to load texture '" << filename << "': " << stbi_failure_reason() << std::endl;
      return false;
   }

   reserved->upload(width, height, channels, data);
   stbi_image_free(data);
   return true;
}

/**
 * Uploads raw pixels already in memory (no file involved) — used for the built-in
 * "missing texture" placeholder, but usable for any generated texture.
 * @return TF
 */
bool ENG_API Eng::Texture::loadFromMemory(int width, int height, int channels, const unsigned char *pixels)
{
   reserved->upload(width, height, channels, pixels);
   return true;
}

/**
 * Decodes image bytes (PNG/JPG/...) already sitting in memory -- what a texture embedded
 * inside a single-file model (e.g. a .glb's binary chunk) needs.
 * @return TF
 */
bool ENG_API Eng::Texture::loadFromCompressedMemory(const unsigned char *compressedData, int byteLength)
{
   stbi_set_flip_vertically_on_load(true);

   int width, height, channels;
   unsigned char *data = stbi_load_from_memory(compressedData, byteLength, &width, &height, &channels, 0);
   if (data == nullptr)
   {
      std::cout << "ERROR: unable to decode embedded texture: " << stbi_failure_reason() << std::endl;
      return false;
   }

   reserved->upload(width, height, channels, data);
   stbi_image_free(data);
   return true;
}

/** Binds this texture to the (only) texture unit that OpenGL 1.1 has. */
void ENG_API Eng::Texture::bind() const
{
   glBindTexture(GL_TEXTURE_2D, reserved->id);
}
