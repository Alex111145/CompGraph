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
   #include <iostream>
   #include <string>



/////////////
// GLOBALS //
/////////////

   bool orthographic = false;   ///< True when the orthographic camera is used, false for the perspective one
   const std::string legend = "F = wireframe / solid\nC = perspective / orthographic camera";   ///< Mini legend of the keys, drawn in the window



///////////////
// FUNCTIONS //
///////////////

/**
 * Keyboard callback: F switches between wireframe and solid mode, C between perspective and orthographic camera.
 * W, A, S, D stay free for moving the camera.
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
}



//////////
// MAIN //
//////////

/**
 * Application entry point: loads a 3D scene from a file (scene.glb, or the file given on the command line), puts it in
 * a scene graph with two cameras and renders it through a list until the window is closed.
 * @param argc number of command-line arguments passed
 * @param argv array containing up to argc passed arguments
 * @return error code (0 on success, error code otherwise)
 */
int main(int argc, char *argv[])
{
   std::cout << "3D scene loaded from file, Alessio Gervasini (Master) SUPSI" << std::endl;
   std::cout << legend << std::endl;
   std::cout << std::endl;

   std::string fileName = argc > 1 ? argv[1] : "scene.glb";

   Eng::Base &eng = Eng::Base::getInstance();
   if (!eng.init("3D scene loaded from file", 800, 600))
      return 1;
   eng.setKeyboardCallback(keyboardCallback);
   eng.setBackgroundColor(glm::vec3(0.2f, 0.2f, 0.7f));

   Eng::Node root("root");
   glm::mat4 cameraMatrix = glm::inverse(glm::lookAt(glm::vec3(-4.0f, 4.5f, 7.0f), glm::vec3(0.5f, 0.3f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
   Eng::Camera perspectiveCamera("perspective camera");
   Eng::Camera orthographicCamera("orthographic camera");
   perspectiveCamera.setMatrix(cameraMatrix);
   orthographicCamera.setMatrix(cameraMatrix);
   root.addChild(&perspectiveCamera);
   root.addChild(&orthographicCamera);

   Eng::Node *scene = eng.load(fileName);
   if (scene == nullptr)
   {
      eng.free();
      return 1;
   }
   root.addChild(scene);

   Eng::List list("list");
   while (eng.isRunning())
   {
      float aspectRatio = eng.getAspectRatio();
      perspectiveCamera.setProjectionMatrix(glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f));
      orthographicCamera.setProjectionMatrix(glm::ortho(-4.0f * aspectRatio, 4.0f * aspectRatio, -4.0f, 4.0f, 0.1f, 100.0f));

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
