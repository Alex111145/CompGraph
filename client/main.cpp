/**
 * @file		main.cpp
 * @brief	Client application (that uses the graphics engine)
 *
 * @author	Alessio Gervasini (C) SUPSI
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
   #include <iterator>
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
 * Application entry point: builds a scene graph with a light and a tower of cubes, each one smaller than the cube below
 * and rotating around its Y axis, and renders it through a list until the window is closed.
 * @param argc number of command-line arguments passed
 * @param argv array containing up to argc passed arguments
 * @return error code (0 on success, error code otherwise)
 */
int main(int argc, char *argv[])
{
   std::cout << "Scene graph of cubes, Alessio Gervasini (C) SUPSI" << std::endl;
   std::cout << legend << std::endl;
   std::cout << std::endl;

   Eng::Base &eng = Eng::Base::getInstance();
   if (!eng.init("Scene graph of cubes", 800, 600))
      return 1;
   eng.setKeyboardCallback(keyboardCallback);
   eng.setBackgroundColor(glm::vec3(0.2f, 0.2f, 0.7f));

   Eng::Node root("root");
   glm::mat4 cameraMatrix = glm::inverse(glm::lookAt(glm::vec3(0.0f, 2.0f, 5.0f), glm::vec3(0.0f, 0.9f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
   Eng::Camera perspectiveCamera("perspective camera");
   Eng::Camera orthographicCamera("orthographic camera");
   perspectiveCamera.setMatrix(cameraMatrix);
   orthographicCamera.setMatrix(cameraMatrix);
   root.addChild(&perspectiveCamera);
   root.addChild(&orthographicCamera);

   Eng::Light light("light", Eng::Light::Type::OMNI);
   light.setMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 3.0f, 3.0f)));
   root.addChild(&light);

   Eng::Material materials[] = { Eng::Material("red"), Eng::Material("green"), Eng::Material("yellow"), Eng::Material("white") };
   glm::vec3 colors[] = { glm::vec3(1.0f, 0.2f, 0.2f), glm::vec3(0.2f, 1.0f, 0.2f), glm::vec3(1.0f, 1.0f, 0.2f), glm::vec3(1.0f, 1.0f, 1.0f) };
   for (unsigned int i = 0; i < std::size(materials); i++)
   {
      materials[i].setAmbient(glm::vec4(colors[i] * 0.2f, 1.0f));
      materials[i].setDiffuse(glm::vec4(colors[i] * 0.8f, 1.0f));
   }

   float edge = 1.0f;
   float shrink = 0.7f;
   Eng::Mesh cubes[] = { Eng::Mesh("cube0", edge), Eng::Mesh("cube1", edge), Eng::Mesh("cube2", edge), Eng::Mesh("cube3", edge) };
   Eng::Node *parent = &root;
   for (unsigned int i = 0; i < std::size(cubes); i++)
   {
      cubes[i].setMaterial(&materials[i]);
      parent->addChild(&cubes[i]);
      parent = &cubes[i];
   }

   Eng::List list("list");
   glm::mat4 stackMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, edge / 2.0f * (1.0f + shrink), 0.0f));
   glm::mat4 shrinkMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(shrink));
   float angle = 0.0f;
   while (eng.isRunning())
   {
      angle += 0.5f;
      glm::mat4 spinMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
      cubes[0].setMatrix(spinMatrix);
      for (unsigned int i = 1; i < std::size(cubes); i++)
         cubes[i].setMatrix(stackMatrix * spinMatrix * shrinkMatrix);

      float aspectRatio = eng.getAspectRatio();
      perspectiveCamera.setProjectionMatrix(glm::perspective(glm::radians(45.0f), aspectRatio, 1.0f, 100.0f));
      orthographicCamera.setProjectionMatrix(glm::ortho(-2.0f * aspectRatio, 2.0f * aspectRatio, -2.0f, 2.0f, 1.0f, 100.0f));

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
