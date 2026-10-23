/**
 * @file		texture.cpp
 * @brief	2D texture decoded with stb_image and uploaded with glTexImage2D
 *
 * @author	Alessio Gervasini
 */

   #include "engine.h"

   #include <GL/freeglut.h>

   #define STB_IMAGE_IMPLEMENTATION
   #include <stb/stb_image.h>

/**
 * Constructor. Reserves an OpenGL texture name.
 * @param name texture name (path inside the file)
 */
ENG_API Eng::Texture::Texture(const std::string &name) : Object(name), textureId{ 0 }
{
   glGenTextures(1, &textureId);
}

/**
 * Destructor. Releases the OpenGL texture name.
 */
ENG_API Eng::Texture::~Texture()
{
   glDeleteTextures(1, &textureId);
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

   glBindTexture(GL_TEXTURE_2D, textureId);
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
 * @param modelView not used: a texture has no position
 */
void ENG_API Eng::Texture::render(const glm::mat4 &modelView)
{
   glBindTexture(GL_TEXTURE_2D, textureId);
}
