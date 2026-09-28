/**
 * @file		texture.cpp
 * @brief	2D texture decoded with stb_image and uploaded with glTexImage2D
 *
 * @author	Alessio Gervasini
 */

   #include "engine.h"

   #include <GLFW/glfw3.h>

   #define STB_IMAGE_IMPLEMENTATION
   #include <stb/stb_image.h>

/**
 * Constructor. Reserves an OpenGL texture name.
 */
ENG_API Eng::Texture::Texture() : id{ 0 }
{
   glGenTextures(1, &id);
}

/**
 * Destructor. Releases the OpenGL texture name.
 */
ENG_API Eng::Texture::~Texture()
{
   glDeleteTextures(1, &id);
}

/**
 * Decodes an image file (PNG/JPG) kept in memory and uploads it as RGBA.
 * @param fileData encoded image bytes
 * @param fileSize number of bytes
 * @return TF
 */
bool ENG_API Eng::Texture::load(const unsigned char *fileData, int fileSize)
{
   int width, height, channels;
   stbi_set_flip_vertically_on_load(true);
   unsigned char *pixels = stbi_load_from_memory(fileData, fileSize, &width, &height, &channels, 4);
   if (pixels == nullptr)
      return false;

   glBindTexture(GL_TEXTURE_2D, id);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

   stbi_image_free(pixels);
   return true;
}

/**
 * Makes this texture the current one.
 */
void ENG_API Eng::Texture::bind() const
{
   glBindTexture(GL_TEXTURE_2D, id);
}
