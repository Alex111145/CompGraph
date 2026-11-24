/**
 * @file		texture.cpp
 * @brief	2D texture (material painted with an image)
 *
 * @author	Alessio Gervasini (Master) SUPSI
 */



//////////////
// #INCLUDE //
//////////////

   // Main include:
   #include "engine.h"

   // FreeGLUT:
   #include <GL/freeglut.h>



///////////////////////////
// BODY OF CLASS Texture //
///////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Constructor: creates and configures the texture, then loads the image into it. This is done only once: the image stays
 * in the memory of the graphics card and render() only binds it again. Texture coordinates outside [0, 1] repeat the
 * image; linear filtering is used both when the image is enlarged and when it is reduced.
 * @param name texture name
 * @param width image width, in pixels
 * @param height image height, in pixels
 * @param bitmap RGB image, 3 bytes per pixel, row by row starting from the bottom row
 */
ENG_API Eng::Texture::Texture(const std::string &name, int width, int height, const unsigned char *bitmap) : Material(name),
   texId{ 0 }
{
   glGenTextures(1, &texId);
   glBindTexture(GL_TEXTURE_2D, texId);

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, bitmap);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Destructor: releases the image from the memory of the graphics card, when the texture is no longer needed.
 */
ENG_API Eng::Texture::~Texture()
{
   glDeleteTextures(1, &texId);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Passes the material properties to OpenGL, then binds the texture and enables texture mapping, so that the faces
 * drawn next are painted with the image.
 * @param modelViewMatrix not used: a texture has no position
 */
void ENG_API Eng::Texture::render(const glm::mat4 &modelViewMatrix) const
{
   Material::render(modelViewMatrix);

   glBindTexture(GL_TEXTURE_2D, texId);
   glEnable(GL_TEXTURE_2D);
}
