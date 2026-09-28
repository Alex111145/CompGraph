/**
 * @file		camera.cpp
 * @brief	3D camera (position, orientation, projection)
 *
 * @author	Alessio Gervasini
 */

   #include "engine.h"

   #include <algorithm>

   #include <glm/glm.hpp>
   #include <glm/gtc/matrix_transform.hpp>
   #include <glm/gtc/type_ptr.hpp>

namespace
{
   const float MAX_PITCH_DEGREES = 89.0f;
   const glm::vec3 WORLD_UP(0.0f, 1.0f, 0.0f);
}

struct Eng::Camera::Reserved
{
   glm::vec3 position;
   float yawDegrees;
   float pitchDegrees;
   float fovDegrees;
   float aspect;
   float nearPlane;
   float farPlane;
   glm::mat4 viewMatrix;
   glm::mat4 projectionMatrix;

   Reserved() :
      position{ 0.0f, 0.0f, 3.0f }, yawDegrees{ -90.0f }, pitchDegrees{ 0.0f },
      fovDegrees{ 60.0f }, aspect{ 4.0f / 3.0f }, nearPlane{ 0.1f }, farPlane{ 100.0f }
   {
      updateMatrices();
   }

   glm::vec3 getForward() const
   {
      const float yaw = glm::radians(yawDegrees);
      const float pitch = glm::radians(pitchDegrees);
      return glm::normalize(glm::vec3(cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch)));
   }

   void updateMatrices()
   {
      viewMatrix = glm::lookAt(position, position + getForward(), WORLD_UP);
      projectionMatrix = glm::perspective(glm::radians(fovDegrees), aspect, nearPlane, farPlane);
   }
};

ENG_API Eng::Camera::Camera() : reserved(std::make_unique<Eng::Camera::Reserved>())
{}

ENG_API Eng::Camera::~Camera()
{}

void ENG_API Eng::Camera::setPosition(float x, float y, float z)
{
   reserved->position = glm::vec3(x, y, z);
   reserved->updateMatrices();
}

void ENG_API Eng::Camera::moveForward(float distance)
{
   reserved->position += reserved->getForward() * distance;
   reserved->updateMatrices();
}

void ENG_API Eng::Camera::moveRight(float distance)
{
   const glm::vec3 right = glm::normalize(glm::cross(reserved->getForward(), WORLD_UP));
   reserved->position += right * distance;
   reserved->updateMatrices();
}

void ENG_API Eng::Camera::moveUp(float distance)
{
   reserved->position += WORLD_UP * distance;
   reserved->updateMatrices();
}

void ENG_API Eng::Camera::look(float yawDeltaDegrees, float pitchDeltaDegrees)
{
   reserved->yawDegrees += yawDeltaDegrees;
   reserved->pitchDegrees = std::clamp(reserved->pitchDegrees + pitchDeltaDegrees, -MAX_PITCH_DEGREES, MAX_PITCH_DEGREES);
   reserved->updateMatrices();
}

void ENG_API Eng::Camera::getForward(float &x, float &y, float &z) const
{
   const glm::vec3 forward = reserved->getForward();
   x = forward.x;
   y = forward.y;
   z = forward.z;
}

void ENG_API Eng::Camera::setPerspective(float fovDegrees, float aspect, float nearPlane, float farPlane)
{
   reserved->fovDegrees = fovDegrees;
   reserved->aspect = aspect;
   reserved->nearPlane = nearPlane;
   reserved->farPlane = farPlane;
   reserved->updateMatrices();
}

void ENG_API Eng::Camera::setAspect(float aspect)
{
   reserved->aspect = aspect;
   reserved->updateMatrices();
}

const float ENG_API *Eng::Camera::getViewMatrix() const
{
   return glm::value_ptr(reserved->viewMatrix);
}

const float ENG_API *Eng::Camera::getProjectionMatrix() const
{
   return glm::value_ptr(reserved->projectionMatrix);
}
