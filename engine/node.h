/**
 * @file		node.h
 * @brief	Scene-graph node: local transform + parent/children hierarchy
 *
 * @author	Alessio Gervasini
 */
#pragma once

   #include <memory>
   #include <string>

namespace Eng {

/**
 * @brief One node of the scene graph. Owns its children; knows its parent but does not own it.
 * A node carries only a transform (position/rotation/scale) — attaching geometry (Eng::Mesh) or
 * anything else to a node is the caller's job, done alongside the graph rather than inside it.
 */
class ENG_API Node final
{
public:

   Node();
   Node(Node const &) = delete;
   ~Node();

   void operator=(Node const &) = delete;

   void setPosition(float x, float y, float z);
   void setRotation(float pitchDegrees, float yawDegrees, float rollDegrees);
   void setScale(float x, float y, float z);

   void setName(const std::string &name);
   const std::string &getName() const;

   Node *addChild();

   const float *getWorldMatrix() const;

private:

   struct Reserved;
   std::unique_ptr<Reserved> reserved;
};

};
