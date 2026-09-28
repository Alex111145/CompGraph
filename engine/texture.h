/**
 * @file		texture.h
 * @brief	2D texture decoded with stb_image and uploaded with glTexImage2D
 *
 * @author	Alessio Gervasini
 */
#pragma once

namespace Eng {

/**
 * @brief 2D texture object (OpenGL 1.1 glGenTextures/glBindTexture).
 */
class ENG_API Texture final
{
public:

   Texture();
   Texture(Texture const &) = delete;
   ~Texture();

   void operator=(Texture const &) = delete;

   bool load(const unsigned char *fileData, int fileSize);
   void bind() const;

private:

   unsigned int id;   ///< OpenGL texture name
};

};
