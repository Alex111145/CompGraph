/**
 * @file		node.cpp
 * @brief	Scene-graph node: local matrix + parent/children hierarchy
 *
 * @author	Alessio Gervasini
 */

   #include "engine.h"

/**
 * Constructor. The local matrix starts as the identity matrix.
 * @param name node name
 */
ENG_API Eng::Node::Node(const std::string &name) : name{ name }, matrix{ glm::mat4(1.0f) }, parent{ nullptr }
{}

/**
 * Destructor. Deletes all the children.
 */
ENG_API Eng::Node::~Node()
{
   for (Node *child : children)
      delete child;
}

/**
 * Gets the node name.
 * @return node name
 */
const std::string ENG_API &Eng::Node::getName() const
{
   return name;
}

/**
 * Sets the local matrix (relative to the parent).
 * @param matrix new local matrix
 */
void ENG_API Eng::Node::setMatrix(const glm::mat4 &matrix)
{
   this->matrix = matrix;
}

/**
 * Gets the local matrix.
 * @return local matrix
 */
const glm::mat4 ENG_API &Eng::Node::getMatrix() const
{
   return matrix;
}

/**
 * Gets the world matrix: parent world matrix x local matrix (column vectors, post-multiplication).
 * @return world matrix
 */
glm::mat4 ENG_API Eng::Node::getWorldMatrix() const
{
   if (parent == nullptr)
      return matrix;
   return parent->getWorldMatrix() * matrix;
}

/**
 * Adds a child. The node takes ownership of it.
 * @param child node to add
 */
void ENG_API Eng::Node::addChild(Node *child)
{
   child->parent = this;
   children.push_back(child);
}

/**
 * Gets the parent node.
 * @return parent node or nullptr for the root
 */
Eng::Node ENG_API *Eng::Node::getParent() const
{
   return parent;
}

/**
 * Gets the children.
 * @return list of children
 */
const std::vector<Eng::Node *> ENG_API &Eng::Node::getChildren() const
{
   return children;
}

/**
 * Appends this node and all its descendants to a flat list.
 * @param list list to fill
 */
void ENG_API Eng::Node::collect(std::vector<Node *> &list)
{
   list.push_back(this);
   for (Node *child : children)
      child->collect(list);
}

/**
 * Renders the node. A generic node has nothing to draw.
 * @param modelView view matrix x world matrix
 */
void ENG_API Eng::Node::render(const glm::mat4 &modelView)
{}
