/**
 * @file		camera.cpp
 * @brief	3D camera (position, orientation, projection)
 *
 * @author	Alessio Gervasini
 */

   #include "engine.h"

   #include <glm/glm.hpp>
   #include <glm/gtc/matrix_transform.hpp>
   #include <glm/gtc/type_ptr.hpp>

/**
 * @brief Camera class reserved structure (PIMPL/Bridge pattern, same as Eng::Base).
 */
struct Eng::Camera::Reserved
{
   glm::vec3 position;
   glm::vec3 up;
   float yaw;
   float pitch;

   float fovDegrees;
   float aspect;
   float nearPlane;
   float farPlane;

   glm::mat4 viewMatrix;
   glm::mat4 projectionMatrix;

   Reserved() :
      position{ 0.0f, 0.0f, 3.0f }, up{ 0.0f, 1.0f, 0.0f }, yaw{ -90.0f }, pitch{ 0.0f },
      fovDegrees{ 60.0f }, aspect{ 4.0f / 3.0f }, nearPlane{ 0.1f }, farPlane{ 100.0f }
   {
      updateMatrices();
   }

   /** Direction the camera is looking at, derived from yaw/pitch. */
   glm::vec3 getFront() const
   {
      glm::vec3 front;
      front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
      front.y = sin(glm::radians(pitch));
      front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
      return glm::normalize(front);
   }

   /** Recomputes view and projection from the current position/orientation/lens. */
   void updateMatrices()
   {
      viewMatrix = glm::lookAt(position, position + getFront(), up);
      projectionMatrix = glm::perspective(glm::radians(fovDegrees), aspect, nearPlane, farPlane);
   }
};

/** Constructor. */
ENG_API Eng::Camera::Camera() : reserved(std::make_unique<Eng::Camera::Reserved>())
{}

/** Destructor. */
ENG_API Eng::Camera::~Camera()
{}

/** Sets the camera's world-space position. */
void ENG_API Eng::Camera::setPosition(float x, float y, float z)
{
   reserved->position = glm::vec3(x, y, z);
   reserved->updateMatrices();
}

/** Moves the camera along its current look direction (negative = backward). */
void ENG_API Eng::Camera::moveForward(float distance)
{
   reserved->position += reserved->getFront() * distance;
   reserved->updateMatrices();
}

/** Strafes the camera sideways, perpendicular to both look direction and up (negative = left). */
void ENG_API Eng::Camera::moveRight(float distance)
{
   glm::vec3 right = glm::normalize(glm::cross(reserved->getFront(), reserved->up));
   reserved->position += right * distance;
   reserved->updateMatrices();
}

/** Moves the camera straight up along the world up vector (negative = down). */
void ENG_API Eng::Camera::moveUp(float distance)
{
   reserved->position += reserved->up * distance;
   reserved->updateMatrices();
}

/** Applies a mouse-look rotation; pitch clamped to +/-89 degrees to avoid flipping over the poles. */
void ENG_API Eng::Camera::look(float yawDeltaDegrees, float pitchDeltaDegrees)
{
   reserved->yaw += yawDeltaDegrees;
   reserved->pitch += pitchDeltaDegrees;

   if (reserved->pitch > 89.0f)
      reserved->pitch = 89.0f;
   if (reserved->pitch < -89.0f)
      reserved->pitch = -89.0f;

   reserved->updateMatrices();
}

/** @return the direction this camera is currently looking (world-space, normalized). */
void ENG_API Eng::Camera::getForward(float &x, float &y, float &z) const
{
   const glm::vec3 front = reserved->getFront();
   x = front.x; y = front.y; z = front.z;
}

/** Sets the perspective lens (field of view, aspect ratio, near/far clip planes). */
void ENG_API Eng::Camera::setPerspective(float fovDegrees, float aspect, float nearPlane, float farPlane)
{
   reserved->fovDegrees = fovDegrees;
   reserved->aspect = aspect;
   reserved->nearPlane = nearPlane;
   reserved->farPlane = farPlane;
   reserved->updateMatrices();
}

/** Updates only the aspect ratio (call whenever the window/framebuffer is resized). */
void ENG_API Eng::Camera::setAspect(float aspect)
{
   reserved->aspect = aspect;
   reserved->updateMatrices();
}

/** @return pointer to 16 column-major floats, ready for glLoadMatrixf/glMultMatrixf. */
const float ENG_API *Eng::Camera::getViewMatrix() const
{
   return glm::value_ptr(reserved->viewMatrix);
}

/** @return pointer to 16 column-major floats, ready for glLoadMatrixf/glMultMatrixf. */
const float ENG_API *Eng::Camera::getProjectionMatrix() const
{
   return glm::value_ptr(reserved->projectionMatrix);
}
