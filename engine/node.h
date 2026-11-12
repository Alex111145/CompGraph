/**
 * @file		node.h
 * @brief	Scene graph node
 *
 * @author	Alessio Gervasini (C) SUPSI
 */
#pragma once



///////////////
// NAMESPACE //
///////////////

namespace Eng {



////////////////
// CLASS Node //
////////////////

/**
 * @brief Scene graph node: has its own matrix (relative to the parent), one parent and a list of children.
 * The node does not take ownership of its children.
 */
class ENG_API Node : public Object
{
//////////
public: //
//////////

   // Const/dest:
   explicit Node(const std::string &name);
   ~Node() override;

   // Matrix:
   void setMatrix(const glm::mat4 &matrix);
   const glm::mat4 &getMatrix() const;
   glm::mat4 getWorldMatrix() const;

   // Hierarchy:
   bool addChild(Node *child);
   bool removeChild(Node *child);
   Node *getChild(unsigned int index) const;
   unsigned int getNumberOfChildren() const;
   Node *getParent() const;

   // Rendering:
   void render(const glm::mat4 &modelViewMatrix) const override;


///////////
private: //
///////////

   glm::mat4 matrix;              ///< Matrix relative to the parent node
   Node *parent;                  ///< Parent node (nullptr for the root)
   /**
    * Child nodes.
    * @warning std::vector moves its elements in memory when it grows: pointers and references to its elements
    * become invalid. Here it stores only pointers to the nodes (the nodes themselves never move); if this ever
    * gives problems, use std::list, which never moves its elements.
    */
   std::vector<Node *> children;
};

}; // end of namespace Eng::
