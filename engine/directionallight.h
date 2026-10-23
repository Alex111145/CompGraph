/**
 * @file		directionallight.h
 * @brief	Directional light: parallel rays along the -Z axis of the node
 *
 * @author	Alessio Gervasini
 */
#pragma once

namespace Eng {

/**
 * @brief Light infinitely far away (like the sun): only its direction matters.
 * The light comes from the +Z axis of the node, so the rays travel along -Z, like a camera looks.
 */
class ENG_API DirectionalLight final : public Light
{
public:

   DirectionalLight(const std::string &name, const glm::vec3 &color);
   ~DirectionalLight();
};

};
