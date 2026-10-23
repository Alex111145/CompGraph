/**
 * @file		list.h
 * @brief	Render list: the scene graph flattened in drawing order (lights first, then meshes)
 *
 * @author	Alessio Gervasini
 */
#pragma once

namespace Eng {

/**
 * @brief Flat list of the lights and meshes of a scene graph. The lights are drawn before the meshes,
 * so that every mesh is lit by all of them. The list does not own the nodes.
 */
class ENG_API List final : public Object
{
public:

   List(const std::string &name);
   ~List();

   void pass(Node *node);
   void clear();

   void render(const glm::mat4 &modelView) override;
   void renderShadow(const glm::mat4 &viewMatrix, Light *light);

private:

   std::vector<Light *> lights;   ///< Lights, drawn first
   std::vector<Mesh *> meshes;    ///< Meshes, drawn after the lights
};

};
