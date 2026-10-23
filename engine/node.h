/**
 * @file		node.h
 * @brief	Scene-graph node: local matrix + parent/children hierarchy
 *
 * @author	Alessio Gervasini
 */
#pragma once

namespace Eng {

/**
 * @brief Generic element of the scene graph. Every node has a local matrix relative to its parent.
 */
class ENG_API Node : public Object
{
public:

   Node(const std::string &name);
   virtual ~Node();

   void setMatrix(const glm::mat4 &matrix);
   const glm::mat4 &getMatrix() const;
   glm::mat4 getWorldMatrix() const;

   void addChild(Node *child);
   Node *getParent() const;
   const std::vector<Node *> &getChildren() const;

   void render(const glm::mat4 &modelView) override;

protected:

   glm::mat4 matrix;               ///< Local matrix, relative to the parent
   Node *parent;                   ///< Parent node (nullptr for the root)
   std::vector<Node *> children;   ///< Owned children
};

};
