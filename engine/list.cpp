/**
 * @file		list.cpp
 * @brief	Render list: the scene graph flattened in drawing order (lights first, then meshes)
 *
 * @author	Alessio Gervasini
 */

   #include "engine.h"

   #include <GL/freeglut.h>

namespace
{
   const float SHADOW_LIFT = 0.002f;
}

/**
 * Constructor.
 * @param name list name
 */
ENG_API Eng::List::List(const std::string &name) : Object(name)
{}

/**
 * Destructor. The nodes belong to the scene graph and are not deleted here.
 */
ENG_API Eng::List::~List()
{}

/**
 * Adds the lights and meshes of a node and of all its descendants. The world matrices are read
 * at every render, so the nodes can move without passing them again.
 * @param node root of the (sub)graph to add
 */
void ENG_API Eng::List::pass(Node *node)
{
   if (Light *light = dynamic_cast<Light *>(node))
      lights.push_back(light);
   else if (Mesh *mesh = dynamic_cast<Mesh *>(node))
      meshes.push_back(mesh);

   for (Node *child : node->getChildren())
      pass(child);
}

/**
 * Empties the list.
 */
void ENG_API Eng::List::clear()
{
   lights.clear();
   meshes.clear();
}

/**
 * Renders the lights, then the meshes. The flat meshes (floor) write 1 into the stencil buffer,
 * the others 0, so that the shadow can be clipped to the floor.
 * @param modelView view matrix of the camera (the world matrix of each node is added here)
 */
void ENG_API Eng::List::render(const glm::mat4 &modelView)
{
   for (Light *light : lights)
      light->render(modelView * light->getWorldMatrix());

   glEnable(GL_STENCIL_TEST);
   glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
   for (Mesh *mesh : meshes)
   {
      glStencilFunc(GL_ALWAYS, mesh->isFlat() ? 1 : 0, 0xFF);
      mesh->render(modelView * mesh->getWorldMatrix());
   }
   glDisable(GL_STENCIL_TEST);
}

/**
 * Renders the planar shadow cast by a light on the floor. The objects are flattened on the floor with the
 * shadow matrix of the OpenGL SuperBible (MakeShadowMatrix, chapter 9): M = (plane . light) I - light plane^T,
 * drawn only into the stencil buffer. The floor is then drawn again, without the light, where the stencil
 * marks the shadow. A point above a point light cannot be projected on the floor: the shadow matrix gives
 * every vertex w = light height - vertex height, so the user clip plane (0, 0, 0, 1), set with the identity
 * modelview (eye coordinates), keeps only w >= 0 and cuts each mesh at the height of the light.
 * @param viewMatrix view matrix of the camera
 * @param light light that casts the shadow (nothing happens if nullptr or off)
 */
void ENG_API Eng::List::renderShadow(const glm::mat4 &viewMatrix, Light *light)
{
   std::vector<Mesh *> floors;
   std::vector<Mesh *> casters;

   for (Mesh *mesh : meshes)
      if (mesh->isFlat())
         floors.push_back(mesh);
      else
         casters.push_back(mesh);

   if (light == nullptr || !light->isEnabled() || floors.empty())
      return;

   const glm::vec4 position = light->getWorldPosition();
   const glm::vec4 plane(0.0f, 1.0f, 0.0f, -(floors[0]->getWorldMinY() + SHADOW_LIFT));
   const glm::mat4 shadow = glm::dot(plane, position) * glm::mat4(1.0f) - glm::outerProduct(position, plane);

   glEnable(GL_STENCIL_TEST);
   glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
   glDepthMask(GL_FALSE);
   glStencilFunc(GL_EQUAL, 1, 0xFF);
   glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

   const double belowLight[4] = { 0.0, 0.0, 0.0, 1.0 };
   glLoadIdentity();
   glClipPlane(GL_CLIP_PLANE0, belowLight);
   glEnable(GL_CLIP_PLANE0);
   for (Mesh *mesh : casters)
   {
      glLoadMatrixf(glm::value_ptr(viewMatrix * shadow * mesh->getWorldMatrix()));
      mesh->renderGeometry();
   }
   glDisable(GL_CLIP_PLANE0);

   glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
   glDepthMask(GL_TRUE);

   light->setEnabled(false);
   light->render(viewMatrix * light->getWorldMatrix());
   glStencilFunc(GL_EQUAL, 2, 0xFF);
   glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
   glDepthFunc(GL_LEQUAL);
   for (Mesh *floor : floors)
      floor->render(viewMatrix * floor->getWorldMatrix());
   glDepthFunc(GL_LESS);
   light->setEnabled(true);
   glDisable(GL_STENCIL_TEST);
}
