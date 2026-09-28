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

   glm::mat4 getLocalMatrix() const
   {
      glm::mat4 local(1.0f);
      local = glm::translate(local, position);
      local = glm::rotate(local, glm::radians(rotationDegrees.y), glm::vec3(0.0f, 1.0f, 0.0f));
      local = glm::rotate(local, glm::radians(rotationDegrees.x), glm::vec3(1.0f, 0.0f, 0.0f));
      local = glm::rotate(local, glm::radians(rotationDegrees.z), glm::vec3(0.0f, 0.0f, 1.0f));
      local = glm::scale(local, scale);
      return local;
   }
};

ENG_API Eng::Node::Node() : reserved(std::make_unique<Eng::Node::Reserved>())
{}

ENG_API Eng::Node::~Node()
{}

void ENG_API Eng::Node::setPosition(float x, float y, float z)
{
   reserved->position = glm::vec3(x, y, z);
}

void ENG_API Eng::Node::setRotation(float pitchDegrees, float yawDegrees, float rollDegrees)
{
   reserved->rotationDegrees = glm::vec3(pitchDegrees, yawDegrees, rollDegrees);
}

void ENG_API Eng::Node::setScale(float x, float y, float z)
{
   reserved->scale = glm::vec3(x, y, z);
}

void ENG_API Eng::Node::setName(const std::string &name)
{
   reserved->name = name;
}

const std::string ENG_API &Eng::Node::getName() const
{
   return reserved->name;
}

Eng::Node ENG_API *Eng::Node::addChild()
{
   auto child = std::make_unique<Node>();
   child->reserved->parent = this;
   Node *childPtr = child.get();
   reserved->children.push_back(std::move(child));
   return childPtr;
}

const float ENG_API *Eng::Node::getWorldMatrix() const
{
   const glm::mat4 local = reserved->getLocalMatrix();

   if (reserved->parent != nullptr)
      reserved->worldMatrix = glm::make_mat4(reserved->parent->getWorldMatrix()) * local;
   else
      reserved->worldMatrix = local;

   return glm::value_ptr(reserved->worldMatrix);
}
