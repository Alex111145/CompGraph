/**
 * @file		camera.h
 * @brief	Camera (perspective or orthographic, chosen by the user)
 *
 * @author	Alessio Gervasini (Master) SUPSI
 */
#pragma once



///////////////
// NAMESPACE //
///////////////

namespace Eng {



//////////////////
// CLASS Camera //
//////////////////

/**
 * @brief Scene graph node that looks at the scene. It stores only the projection matrix, built by the user
 * (e.g. with glm::perspective() or glm::ortho()); its inverse world matrix is the view matrix.
 */
class ENG_API Camera : public Node
{
//////////
public: //
//////////

   // Const/dest:
   explicit Camera(const std::string &name);

   // Projection:
   void setProjectionMatrix(const glm::mat4 &projectionMatrix);
   const glm::mat4 &getProjectionMatrix() const;

   // View:
   glm::mat4 getInverseMatrix() const;

   // Movement:
   void move(const glm::vec3 &offset);
   void turn(float angle);


///////////
private: //
///////////

   glm::mat4 projectionMatrix;   ///< Projection matrix (perspective or orthographic)
};

}; // end of namespace Eng::
