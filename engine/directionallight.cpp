/**
 * @file		directionallight.cpp
 * @brief	Directional light: parallel rays along the -Z axis of the node
 *
 * @author	Alessio Gervasini
 */

   #include "engine.h"

/**
 * Constructor. The local position is the direction +Z with w = 0 (already normalized).
 * @param name light name
 * @param color diffuse and specular color
 */
ENG_API Eng::DirectionalLight::DirectionalLight(const std::string &name, const glm::vec3 &color) :
   Light(name, glm::vec4(0.0f, 0.0f, 1.0f, 0.0f), color)
{}

/**
 * Destructor.
 */
ENG_API Eng::DirectionalLight::~DirectionalLight()
{}
