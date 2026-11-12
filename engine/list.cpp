/**
 * @file		list.cpp
 * @brief	List of the nodes to render
 *
 * @author	Alessio Gervasini (C) SUPSI
 */



//////////////
// #INCLUDE //
//////////////

   // Main include:
   #include "engine.h"



////////////////////////
// BODY OF CLASS List //
////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Constructor.
 * @param name list name
 */
ENG_API Eng::List::List(const std::string &name) : Object(name)
{}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Visits a node and, recursively, its children (depth-first). The world matrix of each node is the world matrix
 * of its parent multiplied by its own matrix. Lights go to the front of the list, meshes to the back,
 * other nodes (root, cameras) are not added.
 * @param node node to visit (nullptr = nothing to do)
 * @param parentMatrix world matrix of the parent (identity for the root)
 */
void ENG_API Eng::List::pass(const Node *node, const glm::mat4 &parentMatrix)
{
   if (node == nullptr)
      return;

   glm::mat4 worldMatrix = parentMatrix * node->getMatrix();

   if (dynamic_cast<const Light *>(node) != nullptr)
      instances.insert(instances.begin(), Instance{ node, worldMatrix });
   else if (dynamic_cast<const Mesh *>(node) != nullptr)
      instances.push_back(Instance{ node, worldMatrix });

   for (unsigned int i = 0; i < node->getNumberOfChildren(); i++)
      pass(node->getChild(i), worldMatrix);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Removes all the nodes from the list (to be called at every frame, before pass()).
 */
void ENG_API Eng::List::clear()
{
   instances.clear();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Renders the nodes in the order of the list. The modelview matrix of each node is the inverse camera matrix
 * multiplied by the world matrix of the node.
 * @param cameraInverseMatrix inverse of the camera matrix in world coordinates
 */
void ENG_API Eng::List::render(const glm::mat4 &cameraInverseMatrix) const
{
   for (const Instance &instance : instances)
      instance.nodePtr->render(cameraInverseMatrix * instance.nodeWorldMatrix);
}
