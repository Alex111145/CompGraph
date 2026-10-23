/**
 * @file		pointlight.h
 * @brief	Point light: emits in every direction from the origin of the node
 *
 * @author	Alessio Gervasini
 */
#pragma once

namespace Eng {

/**
 * @brief Light placed in a point (like a bulb), weaker with the distance.
 */
class ENG_API PointLight final : public Light
{
public:

   PointLight(const std::string &name, const glm::vec3 &color);
   ~PointLight();

   void setAttenuation(float constant, float linear, float quadratic);

   void render(const glm::mat4 &modelView) override;

private:

   glm::vec3 attenuation;  ///< Constant, linear, quadratic attenuation
};

};
