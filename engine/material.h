/**
 * @file		material.h
 * @brief	Material (emission, ambient, diffuse, specular, shininess)
 *
 * @author	Alessio Gervasini (C) SUPSI
 */
#pragma once



///////////////
// NAMESPACE //
///////////////

namespace Eng {



////////////////////
// CLASS Material //
////////////////////

/**
 * @brief Material properties of the lighting model, passed to OpenGL when a mesh is drawn.
 */
class ENG_API Material : public Object
{
//////////
public: //
//////////

   // Const/dest:
   explicit Material(const std::string &name);

   // Properties:
   void setEmission(const glm::vec4 &emission);
   void setAmbient(const glm::vec4 &ambient);
   void setDiffuse(const glm::vec4 &diffuse);
   void setSpecular(const glm::vec4 &specular);
   void setShininess(float shininess);

   // Rendering:
   void render(const glm::mat4 &modelViewMatrix) const override;


///////////
private: //
///////////

   glm::vec4 emission;   ///< Emissive color (RGBA)
   glm::vec4 ambient;    ///< Ambient color (RGBA)
   glm::vec4 diffuse;    ///< Diffuse color (RGBA)
   glm::vec4 specular;   ///< Specular color (RGBA)
   float shininess;      ///< Specular exponent, between 0 and 128
};

}; // end of namespace Eng::
