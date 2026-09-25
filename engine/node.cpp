/**
 * @file		node.cpp
 * @brief	Scene-graph node: local transform + parent/children hierarchy
 *
 * @author	Alessio Gervasini
 */

   #include "engine.h"

   #include <vector>

   #include <glm/glm.hpp>
   #include <glm/gtc/matrix_transform.hpp>
   #include <glm/gtc/type_ptr.hpp>

/**
 * @brief Node class reserved structure (PIMPL/Bridge pattern, same as Eng::Base).
 */
struct Eng::Node::Reserved
{
   glm::vec3 position;
   glm::vec3 rotationDegrees;
   glm::vec3 scale;
   std::string name;

   Node *parent;
   std::vector<std::unique_ptr<Node>> children;

   mutable glm::mat4 worldMatrix;

   Reserved() :
      position{ 0.0f, 0.0f, 0.0f }, rotationDegrees{ 0.0f, 0.0f, 0.0f }, scale{ 1.0f, 1.0f, 1.0f },
      parent{ nullptr }, worldMatrix{ 1.0f }
   {}

   /**
    * This node's transform alone, ignoring the whole ancestry above it.
    */
   glm::mat4 getLocalMatrix() const
   {
      glm::mat4 m(1.0f);
      m = glm::translate(m, position);
      m = glm::rotate(m, glm::radians(rotationDegrees.y), glm::vec3(0.0f, 1.0f, 0.0f));
      m = glm::rotate(m, glm::radians(rotationDegrees.x), glm::vec3(1.0f, 0.0f, 0.0f));
      m = glm::rotate(m, glm::radians(rotationDegrees.z), glm::vec3(0.0f, 0.0f, 1.0f));
      m = glm::scale(m, scale);
      return m;
   }
};

/** Constructor: new node is parentless, childless, unrotated, unscaled, at the origin. */
ENG_API Eng::Node::Node() : reserved(std::make_unique<Eng::Node::Reserved>())
{}

/** Destructor: destroys the whole subtree too (children are owned via unique_ptr). */
ENG_API Eng::Node::~Node()
{}

/** Sets this node's local position (relative to its parent, or world if it has none). */
void ENG_API Eng::Node::setPosition(float x, float y, float z)
{
   reserved->position = glm::vec3(x, y, z);
}

/** Sets this node's local rotation in degrees (pitch/yaw/roll around X/Y/Z). */
void ENG_API Eng::Node::setRotation(float pitchDegrees, float yawDegrees, float rollDegrees)
{
   reserved->rotationDegrees = glm::vec3(pitchDegrees, yawDegrees, rollDegrees);
}

/** Sets this node's local scale. */
void ENG_API Eng::Node::setScale(float x, float y, float z)
{
   reserved->scale = glm::vec3(x, y, z);
}

/** Sets this node's name (purely informational -- the graph itself never looks at it). */
void ENG_API Eng::Node::setName(const std::string &name)
{
   reserved->name = name;
}

/** @return this node's name, or an empty string if it was never set. */
const std::string ENG_API &Eng::Node::getName() const
{
   return reserved->name;
}

/**
 * Creates a new child node owned by this one.
 * @return raw (non-owning) pointer to the new child — valid as long as this node (or an ancestor) is alive
 */
Eng::Node ENG_API *Eng::Node::addChild()
{
   auto child = std::make_unique<Node>();
   child->reserved->parent = this;
   Node *childPtr = child.get();
   reserved->children.push_back(std::move(child));
   return childPtr;
}

/**
 * Composes this node's local transform with every ancestor's, walking up to the root.
 * @return pointer to 16 column-major floats, ready for glLoadMatrixf/glMultMatrixf.
 */
const float ENG_API *Eng::Node::getWorldMatrix() const
{
   glm::mat4 local = reserved->getLocalMatrix();

   if (reserved->parent != nullptr)
   {
      glm::mat4 parentWorld = glm::make_mat4(reserved->parent->getWorldMatrix());
      reserved->worldMatrix = parentWorld * local;
   }
   else
   {
      reserved->worldMatrix = local;
   }

   return glm::value_ptr(reserved->worldMatrix);
}
