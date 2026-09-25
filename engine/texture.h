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

/**
 * @brief Wraps one GPU texture. Loads from any format stb_image supports (PNG, JPG, ...).
 * OpenGL 1.1 exposes exactly one texture unit (multi-texturing/glActiveTexture only arrived
 * in 1.3), so bind() just binds GL_TEXTURE_2D -- there is no unit to select.
 */
class ENG_API Texture final
{
public:

   Texture();
   Texture(Texture const &) = delete;
   ~Texture();

   void operator=(Texture const &) = delete;

   bool load(const std::string &filename);
   bool loadFromMemory(int width, int height, int channels, const unsigned char *pixels);

   /**
    * Decodes an image file's bytes (PNG/JPG/...) already sitting in memory rather than on
    * disk -- what a texture embedded inside a single-file model (e.g. a .glb's binary chunk)
    * needs, since there is no separate image file to load() by path.
    */
   bool loadFromCompressedMemory(const unsigned char *compressedData, int byteLength);

   void bind() const;

private:

   struct Reserved;
   std::unique_ptr<Reserved> reserved;
};

};
