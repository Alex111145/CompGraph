/**
 * @file		light.h
 * @brief	Light source for the OpenGL fixed pipeline (glLight), base of the light types
 *
 * @author	Alessio Gervasini
 */
#pragma once

namespace Eng {

/**
 * @brief Abstract scene-graph node that is a light. The homogeneous local position decides the type:
 * w = 0 directional (xyz = direction towards the light), w = 1 point light.
 */
class ENG_API Light : public Node
{
public:

   virtual ~Light();

   void setEnabled(bool enabled);
   bool isEnabled() const;

   glm::vec4 getWorldPosition() const;

   void render(const glm::mat4 &modelView) override;

protected:

   Light(const std::string &name, const glm::vec4 &position, const glm::vec3 &color);

   int lightId;            ///< GL_LIGHT0 + n
   glm::vec4 position;     ///< Homogeneous position in local coordinates
   glm::vec4 color;        ///< Diffuse and specular color
   bool enabled;           ///< On/off

   static int lightCount;  ///< Number of lights created so far
};

};
