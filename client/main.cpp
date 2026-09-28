/**
 * @file		main.cpp
 * @brief	Client application (3D viewer) that uses the graphics engine
 *
 * @author	Alessio Gervasini
 */

   #include "engine.h"

   #include <iostream>

namespace
{
   const char *MODEL_FILE = "assets/bedroom_furniture_set/bedroom_furniture_set_-_game_ready.glb";
   const float MOVE_SPEED = 1.5f;
   const float MOUSE_SENSITIVITY = 0.1f;
   const glm::vec3 SCENE_CENTER(0.0f, 0.0f, 0.0f);
   const glm::vec3 UP(0.0f, 1.0f, 0.0f);
   const std::vector<std::pair<std::string, std::string>> LEGEND = {
      { "WASD", "MOVE" },
      { "Q/E", "DOWN UP" },
      { "MOUSE", "LOOK" },
      { "C", "CAMERA" },
      { "1", "TORCH" },
      { "2", "LAMP" },
      { "P", "PRINT" },
      { "ESC", "EXIT" }
   };

   /**
    * Prints the world position of every node of the scene graph, using glm::to_string.
    * @param node node to print (with its children)
    * @param depth indentation level
    */
   void printNode(const Eng::Node *node, int depth)
   {
      const glm::vec3 position = glm::vec3(node->getWorldMatrix()[3]);

      std::cout << std::string(depth * 3, ' ') << node->getName() << "  " << glm::to_string(position);
      if (const Eng::Mesh *mesh = dynamic_cast<const Eng::Mesh *>(node))
         std::cout << "  (" << mesh->getTriangleCount() << " triangles)";
      std::cout << std::endl;

      for (const Eng::Node *child : node->getChildren())
         printNode(child, depth + 1);
   }

   /**
    * Finds the lamp shade: the first mesh whose material emits light.
    * @param root root of the scene graph
    * @return glowing mesh, or nullptr if the scene has none
    */
   Eng::Mesh *findGlowingMesh(Eng::Node *root)
   {
      std::vector<Eng::Node *> list;
      root->collect(list);
      for (Eng::Node *node : list)
      {
         Eng::Mesh *mesh = dynamic_cast<Eng::Mesh *>(node);
         if (mesh != nullptr && mesh->getMaterial() && glm::length(mesh->getMaterial()->getEmission()) > 0.0f)
            return mesh;
      }
      return nullptr;
   }

   /**
    * Builds the world matrix of a fixed camera looking at a target (inverse of the lookAt view matrix).
    * @param eye camera position
    * @param target point to look at
    * @return camera world matrix
    */
   glm::mat4 lookFrom(const glm::vec3 &eye, const glm::vec3 &target)
   {
      return glm::inverse(glm::lookAt(eye, target, UP));
   }
}

/**
 * Application entry point.
 * @param argc number of command-line arguments passed
 * @param argv array containing up to argc passed arguments
 * @return error code (0 on success, error code otherwise)
 */
int main(int argc, char *argv[])
{
   std::cout << "Client application, Alessio Gervasini (C) SUPSI" << std::endl << std::endl;

   Eng::Base &eng = Eng::Base::getInstance();
   if (!eng.init())
      return 1;

   Eng::Node *root = new Eng::Node("root");
   Eng::Node *model = Eng::Loader::load(MODEL_FILE);
   if (model == nullptr)
   {
      delete root;
      eng.free();
      return 1;
   }
   root->addChild(model);

   Eng::Mesh *lampShade = findGlowingMesh(root);
   const glm::vec3 lampGlow = lampShade != nullptr ? lampShade->getMaterial()->getEmission() : glm::vec3(0.0f);
   Eng::Light *torch = new Eng::Light("torch", glm::vec4(0.0f, 0.0f, 1.0f, 0.0f), glm::vec3(0.85f, 0.92f, 1.0f) * 1.2f);
   Eng::Light *lamp = new Eng::Light("lamp", glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.55f, 0.2f) * 40.0f);
   lamp->setAttenuation(1.0f, 4.2f, 12.4f);
   if (lampShade != nullptr)
      lamp->setMatrix(glm::translate(glm::mat4(1.0f), lampShade->getWorldCenter()));
   root->addChild(torch);
   root->addChild(lamp);

   Eng::Camera *cameras[3] = {
      new Eng::Camera("free camera", 60.0f, 0.05f, 100.0f),
      new Eng::Camera("front camera", 60.0f, 0.05f, 100.0f),
      new Eng::Camera("top camera", 60.0f, 0.05f, 100.0f)
   };
   cameras[1]->setMatrix(lookFrom(glm::vec3(0.0f, 2.0f, 4.5f), SCENE_CENTER));
   cameras[2]->setMatrix(lookFrom(glm::vec3(4.0f, 3.5f, 4.0f), SCENE_CENTER));
   for (Eng::Camera *camera : cameras)
      root->addChild(camera);

   glm::vec3 freePosition(0.0f, 1.5f, 4.5f);
   float yaw = 0.0f;
   float pitch = -15.0f;
   int activeCamera = 0;

   std::cout << std::endl << "Scene graph (world positions):" << std::endl;
   printNode(root, 1);

   while (eng.isRunning())
   {
      const float step = MOVE_SPEED * eng.getDeltaTime();
      const glm::vec2 mouse = eng.getMouseDelta();

      if (eng.wasKeyPressed('C'))
         activeCamera = (activeCamera + 1) % 3;
      if (eng.wasKeyPressed('1'))
         torch->setEnabled(!torch->isEnabled());
      if (eng.wasKeyPressed('2'))
      {
         lamp->setEnabled(!lamp->isEnabled());
         if (lampShade != nullptr)
            lampShade->getMaterial()->setEmission(lamp->isEnabled() ? lampGlow : glm::vec3(0.0f));
      }

      if (activeCamera == 0)
      {
         yaw -= mouse.x * MOUSE_SENSITIVITY;
         pitch = glm::clamp(pitch - mouse.y * MOUSE_SENSITIVITY, -89.0f, 89.0f);

         glm::mat4 rotation = glm::mat4(1.0f);
         rotation = glm::rotate(rotation, glm::radians(yaw), UP);
         rotation = glm::rotate(rotation, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));

         const glm::vec3 right = glm::vec3(rotation[0]);
         const glm::vec3 forward = -glm::vec3(rotation[2]);

         if (eng.isKeyDown('W')) freePosition += forward * step;
         if (eng.isKeyDown('S')) freePosition -= forward * step;
         if (eng.isKeyDown('D')) freePosition += right * step;
         if (eng.isKeyDown('A')) freePosition -= right * step;
         if (eng.isKeyDown('E')) freePosition += UP * step;
         if (eng.isKeyDown('Q')) freePosition -= UP * step;

         cameras[0]->setMatrix(glm::translate(glm::mat4(1.0f), freePosition) * rotation);
      }

      if (eng.wasKeyPressed('P'))
         std::cout << cameras[activeCamera]->getName() << " position: "
                   << glm::to_string(glm::vec3(cameras[activeCamera]->getWorldMatrix()[3])) << std::endl;

      torch->setMatrix(cameras[activeCamera]->getWorldMatrix());

      eng.render(cameras[activeCamera], root, lamp);
      eng.renderLegend(LEGEND);
      eng.update();
   }

   delete root;
   eng.free();

   std::cout << std::endl << "[application terminated]" << std::endl;
   return 0;
}
