/**
 * @file		texture.h
 * @brief	2D image texture (PNG/JPG)
 *
 * @author	Alessio Gervasini
 */
#pragma once

namespace Eng {

/**
 * @brief 2D image applied to the surfaces through their texture coordinates.
 */
class ENG_API Texture final : public Object
{
public:

   Texture(const std::string &name);
   ~Texture();

   bool load(const unsigned char *fileData, int fileSize);
   void render(const glm::mat4 &modelView) override;

private:

   unsigned int textureId;   ///< Internal texture handle
};

};
