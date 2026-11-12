/**
 * @file		list.h
 * @brief	List of the nodes to render
 *
 * @author	Alessio Gervasini (C) SUPSI
 */
#pragma once



///////////////
// NAMESPACE //
///////////////

namespace Eng {



////////////////
// CLASS List //
////////////////

/**
 * @brief List of the nodes to render, each one with its final matrix in world coordinates.
 * It is filled by visiting the scene graph (pass), lights first and meshes after, then rendered in this order.
 */
class ENG_API List : public Object
{
//////////
public: //
//////////

   // Const/dest:
   explicit List(const std::string &name);

   // Filling:
   void pass(const Node *node, const glm::mat4 &parentMatrix = glm::mat4(1.0f));
   void clear();

   // Rendering:
   void render(const glm::mat4 &cameraInverseMatrix) const override;


///////////
private: //
///////////

   /**
    * @brief One node to render.
    */
   struct Instance
   {
      const Node *nodePtr;         ///< Node to render
      glm::mat4 nodeWorldMatrix;   ///< Matrix of the node in world coordinates
   };

   std::vector<Instance> instances;   ///< Nodes to render: lights first, then meshes
};

}; // end of namespace Eng::
