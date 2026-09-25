/**
 * @file		camera.h
 * @brief	3D camera (position, orientation, projection)
 *
 * @author	Alessio Gervasini
 */
#pragma once

   #include <memory>

namespace Eng {

/**
 * @brief First-person style 3D camera: position + yaw/pitch orientation + perspective projection.
 */
class ENG_API Camera final
{
public:

   Camera();
   Camera(Camera const &) = delete;
   ~Camera();

   void operator=(Camera const &) = delete;

   void setPosition(float x, float y, float z);
   void moveForward(float distance);
   void moveRight(float distance);
   void moveUp(float distance);
   void look(float yawDeltaDegrees, float pitchDeltaDegrees);

   /** @return the direction this camera is currently looking (world-space, normalized). */
   void getForward(float &x, float &y, float &z) const;

   void setPerspective(float fovDegrees, float aspect, float nearPlane, float farPlane);
   void setAspect(float aspect);

   const float *getViewMatrix() const;
   const float *getProjectionMatrix() const;

private:

   struct Reserved;
   std::unique_ptr<Reserved> reserved;
};

};
