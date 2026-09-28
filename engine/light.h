/**
 * @file		light.h
 * @brief	Light source (directional or point) for the OpenGL fixed pipeline (glLight)
 *
 * @author	Alessio Gervasini
 */
#pragma once

namespace Eng {

/**
 * @brief Scene-graph node that is a light. The homogeneous position decides the type:
 * w = 0 directional (xyz = direction towards the light), w = 1 point light.
 */
class ENG_API Light final : public Node
{
public:

   Light(const std::string &name, const glm::vec4 &position, const glm::vec3 &color);
   ~Light();

   void setAttenuation(float constant, float linear, float quadratic);
   void setEnabled(bool enabled);
   bool isEnabled() const;

   glm::vec4 getWorldPosition() const;

   void render(const glm::mat4 &modelView) override;

private:

   int lightId;            ///< GL_LIGHT0 + n
   glm::vec4 position;     ///< Homogeneous position in local coordinates
   glm::vec4 color;        ///< Diffuse and specular color
   glm::vec3 attenuation;  ///< Constant, linear, quadratic attenuation
   bool enabled;           ///< On/off

   static int lightCount;  ///< Number of lights created so far
};

};
