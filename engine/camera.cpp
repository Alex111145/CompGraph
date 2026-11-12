/**
 * @file		camera.cpp
 * @brief	Camera (perspective or orthographic, chosen by the user)
 *
 * @author	Alessio Gervasini (C) SUPSI
 */



//////////////
// #INCLUDE //
//////////////

   // Main include:
   #include "engine.h"



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
