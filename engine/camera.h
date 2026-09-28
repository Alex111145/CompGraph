/**
 * @file		camera.h
 * @brief	Perspective camera placed in the scene graph
 *
 * @author	Alessio Gervasini
 */
#pragma once

namespace Eng {

/**
 * @brief Scene-graph node that is a camera. Its world matrix places it in the scene,
 * the view matrix is the inverse of that world matrix.
 */
class ENG_API Camera final : public Node
{
public:

   Camera(const std::string &name, float fovDegrees, float nearPlane, float farPlane);
   ~Camera();

   glm::mat4 getViewMatrix() const;
   glm::mat4 getProjectionMatrix(float aspect) const;

private:

   float fovDegrees;   ///< Vertical field of view
   float nearPlane;    ///< Near clipping plane
   float farPlane;     ///< Far clipping plane
};

};
