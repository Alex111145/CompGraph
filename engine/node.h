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
class ENG_API Node
{
public:

   Node(const std::string &name);
   Node(Node const &) = delete;
   virtual ~Node();

   void operator=(Node const &) = delete;

   const std::string &getName() const;

   void setMatrix(const glm::mat4 &matrix);
   const glm::mat4 &getMatrix() const;
   glm::mat4 getWorldMatrix() const;

   void addChild(Node *child);
   Node *getParent() const;
   const std::vector<Node *> &getChildren() const;
   void collect(std::vector<Node *> &list);

   virtual void render(const glm::mat4 &modelView);

protected:

   std::string name;       ///< Node name (as found in the file)
   glm::mat4 matrix;       ///< Local matrix, relative to the parent
   Node *parent;           ///< Parent node (nullptr for the root)
   std::vector<Node *> children;   ///< Owned children
};

};
