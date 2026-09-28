/**
 * @file		camera.cpp
 * @brief	Perspective camera placed in the scene graph
 *
 * @author	Alessio Gervasini
 */

   #include "engine.h"

/**
 * Constructor.
 * @param name camera name
 * @param fovDegrees vertical field of view in degrees
 * @param nearPlane near clipping plane
 * @param farPlane far clipping plane
 */
ENG_API Eng::Camera::Camera(const std::string &name, float fovDegrees, float nearPlane, float farPlane) :
   Node(name), fovDegrees{ fovDegrees }, nearPlane{ nearPlane }, farPlane{ farPlane }
{}

/**
 * Destructor.
 */
ENG_API Eng::Camera::~Camera()
{}

/**
 * Gets the view matrix, the inverse of the camera world matrix.
 * @return view matrix
 */
glm::mat4 ENG_API Eng::Camera::getViewMatrix() const
{
   return glm::inverse(getWorldMatrix());
}

/**
 * Gets the perspective projection matrix (right-handed, like gluPerspective).
 * @param aspect width / height of the window
 * @return projection matrix
 */
glm::mat4 ENG_API Eng::Camera::getProjectionMatrix(float aspect) const
{
   return glm::perspective(glm::radians(fovDegrees), aspect, nearPlane, farPlane);
}
