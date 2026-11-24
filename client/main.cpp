/**
 * @file		main.cpp
 * @brief	Client application (that uses the graphics engine)
 *
 * @author	Alessio Gervasini (Master) SUPSI
 */



//////////////
// #INCLUDE //
//////////////

   // Library header:
   #include "engine.h"

   // GLM:
   #include <glm/gtc/matrix_transform.hpp>

   // C/C++:
   #include <algorithm>
   #include <cctype>
   #include <chrono>
   #include <iostream>
   #include <string>



/////////////
// GLOBALS //
/////////////

   const std::string defaultFileName = "bedroom_furniture_set_-_game_ready.glb";   ///< 3D file loaded when none is given on the command line: model "Bedroom Furniture Set - Game Ready" by General of Thailand (CC BY 4.0), taken from https://sketchfab.com/
   bool orthographic = false;   ///< True when the orthographic camera is used, false for the perspective one
   const float cameraSpeed = 2.0f;   ///< Camera speed while a movement key is held down, in units per second
   const float maxFrameSeconds = 0.1f;   ///< Longest frame time used for the movement, so the camera does not jump after the window was blocked
   const float turnSpeed = 45.0f;   ///< Camera turning speed while an arrow key is held down, in degrees per second
   bool heldKeys[256] = {};   ///< True for each key held down (index = lowercase key)
   bool heldSpecialKeys[256] = {};   ///< True for each special key held down (index = Eng::Base::KEY_LEFT, KEY_RIGHT, ...)
   const std::string legend = "F = wireframe / solid\nC = perspective / orthographic camera\nW, A, S, D = move forward, left, back, right\nE, Q = move up, down\nLEFT, RIGHT arrows = turn left, right";   ///< Mini legend of the keys, drawn in the window



///////////////
// FUNCTIONS //
///////////////

/**
 * Keyboard callback: F switches between wireframe and solid mode, C between perspective and orthographic camera,
 * W, A, S, D, E, Q start moving the camera (it keeps moving while the key is held down).
 * @param key pressed key
 * @param mouseX mouse x position, in pixels
 * @param mouseY mouse y position, in pixels
 */
void keyboardCallback(unsigned char key, int mouseX, int mouseY)
{
   Eng::Base &eng = Eng::Base::getInstance();
   if (key == 'f' || key == 'F')
      eng.setWireframe(!eng.isWireframe());
   if (key == 'c' || key == 'C')
      orthographic = !orthographic;
   heldKeys[std::tolower(key)] = true;
}


/**
 * Key release callback: releasing W, A, S, D, E, Q stops the camera movement in that direction.
 * @param key released key
 * @param mouseX mouse x position, in pixels
 * @param mouseY mouse y position, in pixels
 */
void keyboardUpCallback(unsigned char key, int mouseX, int mouseY)
{
   heldKeys[std::tolower(key)] = false;
}


/**
 * Special key callback: the left and right arrows start turning the camera (it keeps turning while the key is held down).
 * @param key pressed special key
 * @param mouseX mouse x position, in pixels
 * @param mouseY mouse y position, in pixels
 */
void specialKeyCallback(int key, int mouseX, int mouseY)
{
   if (key >= 0 && key < 256)
      heldSpecialKeys[key] = true;
}


/**
 * Special key release callback: releasing an arrow stops the camera turning.
 * @param key released special key
 * @param mouseX mouse x position, in pixels
 * @param mouseY mouse y position, in pixels
 */
void specialKeyUpCallback(int key, int mouseX, int mouseY)
{
   if (key >= 0 && key < 256)
      heldSpecialKeys[key] = false;
}



//////////
// MAIN //
//////////

/**
 * Application entry point: loads a 3D scene from a file (defaultFileName, or the file given on the command line), puts it
 * in a scene graph with two cameras and a light and renders it through a list until the window is closed.
 * @param argc number of command-line arguments passed
 * @param argv array containing up to argc passed arguments
 * @return error code (0 on success, error code otherwise)
 */
int main(int argc, char *argv[])
{
   std::cout << "3D scene loaded from file, Alessio Gervasini (Master) SUPSI" << std::endl;
   std::cout << legend << std::endl;
   std::cout << std::endl;

   std::string fileName = argc > 1 ? argv[1] : defaultFileName;

   Eng::Base &eng = Eng::Base::getInstance();
   if (!eng.init("3D scene loaded from file", 800, 600))
      return 1;
   eng.setKeyboardCallback(keyboardCallback);
   eng.setKeyboardUpCallback(keyboardUpCallback);
   eng.setSpecialKeyCallback(specialKeyCallback);
   eng.setSpecialKeyUpCallback(specialKeyUpCallback);
   eng.setBackgroundColor(glm::vec3(0.2f, 0.2f, 0.7f));

   Eng::Node root("root");
   glm::mat4 cameraMatrix = glm::inverse(glm::lookAt(glm::vec3(-3.0f, 3.6f, 5.5f), glm::vec3(0.2f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
   Eng::Camera perspectiveCamera("perspective camera");
   Eng::Camera orthographicCamera("orthographic camera");
   perspectiveCamera.setMatrix(cameraMatrix);
   orthographicCamera.setMatrix(cameraMatrix);
   root.addChild(&perspectiveCamera);
   root.addChild(&orthographicCamera);

   Eng::Light light("light", Eng::Light::Type::OMNI);
   light.setMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 3.0f, 2.0f)));
   root.addChild(&light);

   Eng::Node *scene = eng.load(fileName);
   if (scene == nullptr)
   {
      eng.free();
      return 1;
   }
   root.addChild(scene);

   Eng::List list("list");
   std::chrono::steady_clock::time_point previousTime = std::chrono::steady_clock::now();
   while (eng.isRunning())
   {
      std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
      float elapsedSeconds = std::min(std::chrono::duration<float>(currentTime - previousTime).count(), maxFrameSeconds);
      previousTime = currentTime;
      glm::vec3 direction = glm::vec3(float(heldKeys['d']) - float(heldKeys['a']), float(heldKeys['e']) - float(heldKeys['q']), float(heldKeys['s']) - float(heldKeys['w']));
      if (glm::length(direction) > 0.0f)
      {
         glm::vec3 offset = glm::normalize(direction) * cameraSpeed * elapsedSeconds;
         perspectiveCamera.move(offset);
         orthographicCamera.move(offset);
      }
      float turning = float(heldSpecialKeys[Eng::Base::KEY_LEFT]) - float(heldSpecialKeys[Eng::Base::KEY_RIGHT]);
      if (turning != 0.0f)
      {
         perspectiveCamera.turn(turning * turnSpeed * elapsedSeconds);
         orthographicCamera.turn(turning * turnSpeed * elapsedSeconds);
      }

      float aspectRatio = eng.getAspectRatio();
      perspectiveCamera.setProjectionMatrix(glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f));
      orthographicCamera.setProjectionMatrix(glm::ortho(-2.6f * aspectRatio, 2.6f * aspectRatio, -2.6f, 2.6f, 0.1f, 100.0f));

      list.clear();
      list.pass(&root);

      eng.clearWindow();
      eng.render(orthographic ? orthographicCamera : perspectiveCamera, list);
      eng.drawText(legend, 10, 20, glm::vec3(1.0f, 1.0f, 1.0f));
      eng.swapBuffers();
   }

   eng.free();

   std::cout << "\n[application terminated]" << std::endl;
   return 0;
}
