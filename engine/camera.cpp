/**
 * @file		camera.cpp
 * @brief	Camera (perspective or orthographic, chosen by the user)
 *
 * @author	Alessio Gervasini (Master) SUPSI
 */



//////////////
// #INCLUDE //
//////////////

   // Main include:
   #include "engine.h"

   // GLM:
   #include <glm/gtc/matrix_transform.hpp>



//////////////////////////
// BODY OF CLASS Camera //
//////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Constructor. The projection matrix starts as the identity: the user must set it.
 * @param name camera name
 */
ENG_API Eng::Camera::Camera(const std::string &name) : Node(name), projectionMatrix{ glm::mat4(1.0f) }
{}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Sets the projection matrix.
 * @param projectionMatrix new projection matrix (e.g. from glm::perspective() or glm::ortho())
 */
void ENG_API Eng::Camera::setProjectionMatrix(const glm::mat4 &projectionMatrix)
{
   this->projectionMatrix = projectionMatrix;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Gets the projection matrix.
 * @return projection matrix
 */
const glm::mat4 ENG_API &Eng::Camera::getProjectionMatrix() const
{
   return projectionMatrix;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Gets the inverse of the camera world matrix, i.e. the view matrix.
 * @return inverse camera matrix
 */
glm::mat4 ENG_API Eng::Camera::getInverseMatrix() const
{
   return glm::inverse(getWorldMatrix());
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Moves the camera as a free camera in a 3D game: forward/backward and left/right follow its current orientation,
 * up/down follow the vertical axis of the parent node, so the camera rises without going forward even when it looks down.
 * @param offset movement: x to the right, -z forward (where the camera looks), both in camera coordinates; y up, vertical
 */
void ENG_API Eng::Camera::move(const glm::vec3 &offset)
{
   glm::mat4 vertical = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, offset.y, 0.0f));
   glm::mat4 horizontal = glm::translate(glm::mat4(1.0f), glm::vec3(offset.x, 0.0f, offset.z));
   setMatrix(vertical * getMatrix() * horizontal);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Turns the camera left or right around the vertical axis that passes through its position (it does not move).
 * @param angle rotation angle in degrees (positive = to the left, negative = to the right)
 */
void ENG_API Eng::Camera::turn(float angle)
{
   glm::vec3 position = glm::vec3(getMatrix()[3]);
   glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
   setMatrix(glm::translate(glm::mat4(1.0f), position) * rotation * glm::translate(glm::mat4(1.0f), -position) * getMatrix());
}
