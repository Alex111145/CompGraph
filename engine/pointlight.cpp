/**
 * @file		pointlight.cpp
 * @brief	Point light: emits in every direction from the origin of the node
 *
 * @author	Alessio Gervasini
 */

   #include "engine.h"

   #include <GL/freeglut.h>

/**
 * Constructor. The local position is the origin of the node with w = 1, no attenuation.
 * @param name light name
 * @param color diffuse and specular color
 */
ENG_API Eng::PointLight::PointLight(const std::string &name, const glm::vec3 &color) :
   Light(name, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), color), attenuation{ glm::vec3(1.0f, 0.0f, 0.0f) }
{}

/**
 * Destructor.
 */
ENG_API Eng::PointLight::~PointLight()
{}

/**
 * Sets the distance attenuation: 1 / (constant + linear d + quadratic d^2).
 * @param constant constant factor
 * @param linear linear factor
 * @param quadratic quadratic factor
 */
void ENG_API Eng::PointLight::setAttenuation(float constant, float linear, float quadratic)
{
   attenuation = glm::vec3(constant, linear, quadratic);
}

/**
 * Sends the light and its attenuation to OpenGL.
 * @param modelView view matrix x world matrix
 */
void ENG_API Eng::PointLight::render(const glm::mat4 &modelView)
{
   Light::render(modelView);
   glLightf(lightId, GL_CONSTANT_ATTENUATION, attenuation.x);
   glLightf(lightId, GL_LINEAR_ATTENUATION, attenuation.y);
   glLightf(lightId, GL_QUADRATIC_ATTENUATION, attenuation.z);
}
