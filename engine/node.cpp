/**
 * @file		node.cpp
 * @brief	Scene graph node
 *
 * @author	Alessio Gervasini (C) SUPSI
 */



//////////////
// #INCLUDE //
//////////////

   // Main include:
   #include "engine.h"

   // FreeGLUT:
   #include <GL/freeglut.h>

   // GLM:
   #include <glm/gtc/type_ptr.hpp>

   // C/C++:
   #include <algorithm>



////////////////////////
// BODY OF CLASS Node //
////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Constructor.
 * @param name node name
 */
ENG_API Eng::Node::Node(const std::string &name) : Object(name), matrix{ glm::mat4(1.0f) }, parent{ nullptr }
{}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Destructor. Detaches the node from its parent and from its children.
 */
ENG_API Eng::Node::~Node()
{
   if (parent != nullptr)
      parent->removeChild(this);

   for (Node *child : children)
      child->parent = nullptr;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Sets the matrix relative to the parent node.
 * @param matrix new local matrix
 */
void ENG_API Eng::Node::setMatrix(const glm::mat4 &matrix)
{
   this->matrix = matrix;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Gets the matrix relative to the parent node.
 * @return local matrix
 */
const glm::mat4 ENG_API &Eng::Node::getMatrix() const
{
   return matrix;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Gets the matrix in world coordinates, i.e. the matrices of all the parents multiplied by the local one.
 * @return world matrix
 */
glm::mat4 ENG_API Eng::Node::getWorldMatrix() const
{
   if (parent == nullptr)
      return matrix;
   return parent->getWorldMatrix() * matrix;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Adds a child. A child that already has a parent is moved under this node.
 * @param child node to add
 * @return TF (false if the child is nullptr, this node or one of its parents)
 */
bool ENG_API Eng::Node::addChild(Node *child)
{
   if (child == nullptr)
      return false;

   for (const Node *ancestor = this; ancestor != nullptr; ancestor = ancestor->parent)
      if (ancestor == child)
         return false;

   if (child->parent != nullptr)
      child->parent->removeChild(child);

   child->parent = this;
   children.push_back(child);
   return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Removes a child.
 * @param child node to remove
 * @return TF (false if the node is not a child of this node)
 */
bool ENG_API Eng::Node::removeChild(Node *child)
{
   auto position = std::find(children.begin(), children.end(), child);
   if (position == children.end())
      return false;

   children.erase(position);
   child->parent = nullptr;
   return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Gets a child.
 * @param index position of the child, from 0 to getNumberOfChildren() - 1
 * @return child node, or nullptr if the index is out of range
 */
Eng::Node ENG_API *Eng::Node::getChild(unsigned int index) const
{
   if (index >= children.size())
      return nullptr;
   return children[index];
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Gets the number of children.
 * @return number of children
 */
unsigned int ENG_API Eng::Node::getNumberOfChildren() const
{
   return static_cast<unsigned int>(children.size());
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Gets the parent.
 * @return parent node, or nullptr for the root
 */
Eng::Node ENG_API *Eng::Node::getParent() const
{
   return parent;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Loads the modelview matrix of the node into OpenGL. An empty node has nothing else to draw.
 * @param modelViewMatrix matrix that brings the node coordinates into eye coordinates
 */
void ENG_API Eng::Node::render(const glm::mat4 &modelViewMatrix) const
{
   glLoadMatrixf(glm::value_ptr(modelViewMatrix));
}
