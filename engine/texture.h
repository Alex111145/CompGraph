/**
 * @file		texture.h
 * @brief	2D texture (material painted with an image)
 *
 * @author	Alessio Gervasini (Master) SUPSI
 */
#pragma once



///////////////
// NAMESPACE //
///////////////

namespace Eng {



///////////////////
// CLASS Texture //
///////////////////

/**
 * @brief Material painted with a 2D image. The image is loaded only once into the memory of the graphics card, when
 * the texture is created; then each render() only binds it again. The image is released when the texture is deleted.
 */
class ENG_API Texture : public Material
{
//////////
public: //
//////////

   // Const/dest:
   Texture(const std::string &name, int width, int height, const unsigned char *bitmap);
   ~Texture();

   // Rendering:
   void render(const glm::mat4 &modelViewMatrix) const override;


///////////
private: //
///////////

   unsigned int texId;   ///< Name (identifier) of the OpenGL texture object
};

}; // end of namespace Eng::
