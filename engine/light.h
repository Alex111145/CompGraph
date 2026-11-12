/**
 * @file		light.h
 * @brief	Light source (directional, omnidirectional or spot)
 *
 * @author	Alessio Gervasini (C) SUPSI
 */
#pragma once



///////////////
// NAMESPACE //
///////////////

namespace Eng {



/////////////////
// CLASS Light //
/////////////////

/**
 * @brief Scene graph node that emits light. Its position and direction come from its matrix:
 * the light sits at the origin of the node and points along its -Z axis.
 */
class ENG_API Light : public Node
{
//////////
public: //
//////////

   /**
    * @brief Main types of light.
    */
   enum class Type
   {
      DIRECTIONAL,   ///< Infinite light, all rays parallel (e.g. the sun)
      OMNI,          ///< Point light, same light in all directions (e.g. a candle)
      SPOT           ///< Light with a cone of effect (e.g. a flashlight)
   };

   // Const/dest:
   Light(const std::string &name, Type type);

   // Properties:
   void setAmbient(const glm::vec4 &ambient);
   void setDiffuse(const glm::vec4 &diffuse);
   void setSpecular(const glm::vec4 &specular);
   bool setCutoff(float cutoff);

   // Rendering:
   void render(const glm::mat4 &modelViewMatrix) const override;


///////////
private: //
///////////

   Type type;                         ///< Type of light
   int lightNumber;                   ///< OpenGL light used, from 0 to 7 (-1 = no light left)
   glm::vec4 ambient;                 ///< Ambient color (RGBA)
   glm::vec4 diffuse;                 ///< Diffuse color (RGBA)
   glm::vec4 specular;                ///< Specular color (RGBA)
   float cutoff;                      ///< Half angle of the cone, in degrees (spot light only)
   static int nextLightNumber;        ///< OpenGL light given to the next light created
};

}; // end of namespace Eng::
