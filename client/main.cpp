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
   const int WINDOW_WIDTH = 1024;
   const int WINDOW_HEIGHT = 768;
   const int CAMERA_COUNT = 3;
   const float MOVE_SPEED = 1.5f;
   const float MOUSE_SENSITIVITY = 0.1f;
   const glm::vec3 BACKGROUND_COLOR(0.1f, 0.1f, 0.12f);
   const glm::vec3 SCENE_CENTER(0.0f, 0.0f, 0.0f);
   const glm::vec3 UP(0.0f, 1.0f, 0.0f);
   const std::vector<std::pair<std::string, std::string>> LEGEND = {
      { "WASD", "MOVE" },
      { "Q/E", "DOWN UP" },
      { "MOUSE", "LOOK" },
      { "C", "CAMERA" },
      { "1", "TORCH" },
      { "2", "LAMP" },
      { "F", "WIREFRAME" },
      { "P", "PRINT" },
      { "ESC", "EXIT" }
   };

   Eng::Camera *cameras[CAMERA_COUNT] = {};   ///< Free, front and corner camera
   int activeCamera = 0;                      ///< Index of the camera in use
   Eng::DirectionalLight *torch = nullptr;    ///< Dynamic light that follows the active camera
   Eng::Light *lamp = nullptr;                ///< Static light of the lamp, read from the file (can be nullptr)
   Eng::Mesh *lampShade = nullptr;            ///< Glowing mesh of the lamp (can be nullptr)
   glm::vec3 lampGlow(0.0f);                  ///< Emission of the lamp shade when the lamp is on
   glm::vec3 freePosition(0.0f, 1.5f, 4.5f);  ///< Position of the free camera
   float yaw = 0.0f;                          ///< Free camera rotation around Y in degrees
   float pitch = -15.0f;                      ///< Free camera rotation around X in degrees

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
    * @param node root of the (sub)graph to search
    * @return glowing mesh, or nullptr if the scene has none
    */
   Eng::Mesh *findGlowingMesh(Eng::Node *node)
   {
      Eng::Mesh *mesh = dynamic_cast<Eng::Mesh *>(node);
      if (mesh != nullptr && mesh->getMaterial() && glm::length(mesh->getMaterial()->getEmission()) > 0.0f)
         return mesh;

      for (Eng::Node *child : node->getChildren())
         if (Eng::Mesh *found = findGlowingMesh(child))
            return found;
      return nullptr;
   }

   /**
    * Finds the first light of a (sub)graph.
    * @param node root of the (sub)graph to search
    * @return light, or nullptr if there is none
    */
   Eng::Light *findLight(Eng::Node *node)
   {
      if (Eng::Light *light = dynamic_cast<Eng::Light *>(node))
         return light;

      for (Eng::Node *child : node->getChildren())
         if (Eng::Light *found = findLight(child))
            return found;
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

   /**
    * Keyboard callback: C changes camera, 1 and 2 switch the lights, F switches between wireframe and solid,
    * P prints the camera position.
    * @param key pressed key (uppercase)
    */
   void keyboardCallback(unsigned char key)
   {
      switch (key)
      {
      case 'C':
         activeCamera = (activeCamera + 1) % CAMERA_COUNT;
         break;

      case '1':
         torch->setEnabled(!torch->isEnabled());
         break;

      case '2':
         if (lamp == nullptr)
            break;
         lamp->setEnabled(!lamp->isEnabled());
         if (lampShade != nullptr)
            lampShade->getMaterial()->setEmission(lamp->isEnabled() ? lampGlow : glm::vec3(0.0f));
         break;

      case 'F':
         Eng::Base::getInstance().setWireframe(!Eng::Base::getInstance().isWireframe());
         break;

      case 'P':
         std::cout << cameras[activeCamera]->getName() << " position: "
                   << glm::to_string(glm::vec3(cameras[activeCamera]->getWorldMatrix()[3])) << std::endl;
         break;
      }
   }

   /**
    * Moves the free camera: the mouse turns it (yaw, pitch), WASD/QE move it in the direction it looks.
    * @param eng graphics engine
    */
   void moveFreeCamera(Eng::Base &eng)
   {
      const float step = MOVE_SPEED * eng.getDeltaTime();
      const glm::vec2 mouse = eng.getMouseDelta();

      if (activeCamera != 0)
         return;

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
   if (!eng.init("3D model viewer", WINDOW_WIDTH, WINDOW_HEIGHT))
      return 1;
   eng.setKeyboardCallback(keyboardCallback);
   eng.setBackgroundColor(BACKGROUND_COLOR);

   Eng::Node *root = new Eng::Node("root");
   Eng::Node *model = Eng::Loader::load(MODEL_FILE);
   if (model == nullptr)
   {
      delete root;
      eng.free();
      return 1;
   }
   root->addChild(model);

   lamp = findLight(model);
   lampShade = findGlowingMesh(model);
   if (lampShade != nullptr)
      lampGlow = lampShade->getMaterial()->getEmission();
   torch = new Eng::DirectionalLight("torch", glm::vec3(0.85f, 0.92f, 1.0f) * 1.2f);
   root->addChild(torch);

   cameras[0] = new Eng::Camera("free camera", 60.0f, 0.05f, 100.0f);
   cameras[1] = new Eng::Camera("front camera", 60.0f, 0.05f, 100.0f);
   cameras[2] = new Eng::Camera("corner camera", 60.0f, 0.05f, 100.0f);
   cameras[1]->setMatrix(lookFrom(glm::vec3(0.0f, 2.0f, 4.5f), SCENE_CENTER));
   cameras[2]->setMatrix(lookFrom(glm::vec3(4.0f, 3.5f, 4.0f), SCENE_CENTER));
   for (Eng::Camera *camera : cameras)
      root->addChild(camera);

   Eng::List list("render list");
   list.pass(root);

   std::cout << std::endl << "Scene graph (world positions):" << std::endl;
   printNode(root, 1);

   while (eng.isRunning())
   {
      moveFreeCamera(eng);
      torch->setMatrix(cameras[activeCamera]->getWorldMatrix());

      eng.clearWindow();
      eng.render(cameras[activeCamera], &list, lamp);
      eng.renderLegend(LEGEND);
      eng.swapBuffers();
   }

   list.clear();
   delete root;
   eng.free();

   std::cout << std::endl << "[application terminated]" << std::endl;
   return 0;
}
