/**
 * @file		engine.cpp
 * @brief	Graphics engine main file
 *
 * @author	Alessio Gervasini
 */

   #include "engine.h"

   #include <iostream>
   #include <source_location>

   #include <GLFW/glfw3.h>

namespace
{
   const int WINDOW_WIDTH = 1024;
   const int WINDOW_HEIGHT = 768;
   const float SHADOW_LIFT = 0.002f;
   const float SHADOW_OPACITY = 0.5f;
}

/**
 * @brief Base class reserved structure (PIMPL): hides GLFW from the engine users.
 */
struct Eng::Base::Reserved
{
   bool initFlag;                ///< Engine initialized
   GLFWwindow *window;           ///< Output window
   double lastTime;              ///< Time of the previous frame
   float deltaTime;              ///< Seconds between the last two frames
   double fpsTime;               ///< Start of the current FPS measurement
   int fpsFrames;                ///< Frames drawn since fpsTime
   glm::dvec2 lastMouse;         ///< Previous cursor position
   std::vector<int> pressedKeys; ///< Keys pressed since the last check

   /**
    * Constructor of the reserved structure.
    */
   Reserved() :
      initFlag{ false }, window{ nullptr }, lastTime{ 0.0 }, deltaTime{ 0.0f }, fpsTime{ 0.0 }, fpsFrames{ 0 },
      lastMouse{ glm::dvec2(0.0) }
   {}

   /**
    * GLFW keyboard callback: stores the pressed keys, ESC closes the window.
    */
   static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
   {
      if (action != GLFW_PRESS)
         return;
      if (key == GLFW_KEY_ESCAPE)
         glfwSetWindowShouldClose(window, GLFW_TRUE);
      Base::getInstance().reserved->pressedKeys.push_back(key);
   }
};

/**
 * Constructor.
 */
ENG_API Eng::Base::Base() : reserved(std::make_unique<Eng::Base::Reserved>())
{
#ifdef _DEBUG
   std::cout << "[+] " << std::source_location::current().function_name() << " invoked" << std::endl;
#endif
}

/**
 * Destructor.
 */
ENG_API Eng::Base::~Base()
{
#ifdef _DEBUG
   std::cout << "[-] " << std::source_location::current().function_name() << " invoked" << std::endl;
#endif
}

/**
 * Gets a reference to the (unique) singleton instance.
 * @return reference to singleton instance
 */
Eng::Base ENG_API &Eng::Base::getInstance()
{
   static Base instance;
   return instance;
}

/**
 * Opens the window, creates the OpenGL context and sets the fixed-pipeline state.
 * @return TF
 */
bool ENG_API Eng::Base::init()
{
   if (reserved->initFlag)
   {
      std::cout << "ERROR: engine already initialized" << std::endl;
      return false;
   }

   if (!glfwInit())
   {
      std::cout << "ERROR: unable to initialize GLFW" << std::endl;
      return false;
   }

   glfwWindowHint(GLFW_STENCIL_BITS, 8);
   reserved->window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, LIB_NAME, nullptr, nullptr);
   if (reserved->window == nullptr)
   {
      std::cout << "ERROR: unable to create the window" << std::endl;
      glfwTerminate();
      return false;
   }

   glfwMakeContextCurrent(reserved->window);
   glfwSwapInterval(1);
   glfwSetKeyCallback(reserved->window, Reserved::keyCallback);
   glfwSetInputMode(reserved->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
   glfwGetCursorPos(reserved->window, &reserved->lastMouse.x, &reserved->lastMouse.y);
   reserved->lastTime = glfwGetTime();
   reserved->fpsTime = reserved->lastTime;

   std::cout << "   OpenGL version: " << reinterpret_cast<const char *>(glGetString(GL_VERSION)) << std::endl;

   glEnable(GL_DEPTH_TEST);
   glEnable(GL_CULL_FACE);
   glFrontFace(GL_CCW);
   glEnable(GL_LIGHTING);
   glEnable(GL_NORMALIZE);
   glShadeModel(GL_SMOOTH);
   glClearColor(0.1f, 0.1f, 0.12f, 1.0f);

   std::cout << "[>] " << LIB_NAME << " initialized" << std::endl;
   reserved->initFlag = true;
   return true;
}

/**
 * Closes the window and releases GLFW.
 * @return TF
 */
bool ENG_API Eng::Base::free()
{
   if (!reserved->initFlag)
   {
      std::cout << "ERROR: engine not initialized" << std::endl;
      return false;
   }

   glfwDestroyWindow(reserved->window);
   glfwTerminate();

   std::cout << "[<] " << LIB_NAME << " deinitialized" << std::endl;
   reserved->initFlag = false;
   return true;
}

/**
 * Tells if the window is still open.
 * @return TF
 */
bool ENG_API Eng::Base::isRunning() const
{
   return !glfwWindowShouldClose(reserved->window);
}

/**
 * Shows the frame, reads the input, measures time and writes the FPS in the window title.
 */
void ENG_API Eng::Base::update()
{
   const double now = glfwGetTime();

   glfwSwapBuffers(reserved->window);
   reserved->pressedKeys.clear();
   glfwPollEvents();

   reserved->deltaTime = static_cast<float>(now - reserved->lastTime);
   reserved->lastTime = now;

   reserved->fpsFrames++;
   if (now - reserved->fpsTime >= 1.0)
   {
      const std::string title = std::string(LIB_NAME) + " - " + std::to_string(reserved->fpsFrames) + " FPS";
      glfwSetWindowTitle(reserved->window, title.c_str());
      reserved->fpsTime = now;
      reserved->fpsFrames = 0;
   }
}

/**
 * Gets the time elapsed between the last two frames.
 * @return seconds
 */
float ENG_API Eng::Base::getDeltaTime() const
{
   return reserved->deltaTime;
}

/**
 * Tells if a key is held down.
 * @param key uppercase letter or digit ('W', '1', ...)
 * @return TF
 */
bool ENG_API Eng::Base::isKeyDown(int key) const
{
   return glfwGetKey(reserved->window, key) == GLFW_PRESS;
}

/**
 * Tells if a key has been pressed in the last frame (one time only).
 * @param key uppercase letter or digit ('C', '1', ...)
 * @return TF
 */
bool ENG_API Eng::Base::wasKeyPressed(int key)
{
   for (int pressed : reserved->pressedKeys)
      if (pressed == key)
         return true;
   return false;
}

/**
 * Gets how much the mouse moved since the previous call.
 * @return movement in pixels (x right, y down)
 */
glm::vec2 ENG_API Eng::Base::getMouseDelta()
{
   glm::dvec2 mouse(0.0);
   glfwGetCursorPos(reserved->window, &mouse.x, &mouse.y);
   const glm::vec2 delta = glm::vec2(mouse - reserved->lastMouse);
   reserved->lastMouse = mouse;
   return delta;
}

/**
 * Renders a scene graph from a camera: lights first, then meshes, then the planar shadow.
 * The shadow is drawn on the flat meshes (floor) with the shadow matrix of the OpenGL SuperBible
 * (MakeShadowMatrix, chapter 9): M = (plane . light) I - light plane^T, clipped with the stencil buffer.
 * @param camera active camera
 * @param root root of the scene graph
 * @param shadowLight light that casts the shadow (nullptr for no shadow)
 */
void ENG_API Eng::Base::render(Camera *camera, Node *root, Light *shadowLight)
{
   std::vector<Node *> list;
   std::vector<Mesh *> floors;
   std::vector<Mesh *> casters;
   int width, height;

   glfwGetFramebufferSize(reserved->window, &width, &height);
   if (height == 0)
      return;

   glViewport(0, 0, width, height);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

   glMatrixMode(GL_PROJECTION);
   glLoadMatrixf(glm::value_ptr(camera->getProjectionMatrix(static_cast<float>(width) / static_cast<float>(height))));
   glMatrixMode(GL_MODELVIEW);

   const glm::mat4 view = camera->getViewMatrix();
   root->collect(list);

   for (Node *node : list)
      if (dynamic_cast<Light *>(node) != nullptr)
         node->render(view * node->getWorldMatrix());

   glEnable(GL_STENCIL_TEST);
   glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
   for (Node *node : list)
   {
      Mesh *mesh = dynamic_cast<Mesh *>(node);
      if (mesh == nullptr)
         continue;

      if (mesh->isFlat())
         floors.push_back(mesh);
      else
         casters.push_back(mesh);

      glStencilFunc(GL_ALWAYS, mesh->isFlat() ? 1 : 0, 0xFF);
      mesh->render(view * mesh->getWorldMatrix());
   }

   if (shadowLight != nullptr && shadowLight->isEnabled() && !floors.empty())
   {
      const glm::vec4 light = shadowLight->getWorldPosition();
      const glm::vec4 plane(0.0f, 1.0f, 0.0f, -(floors[0]->getWorldMinY() + SHADOW_LIFT));
      const glm::mat4 shadow = glm::dot(plane, light) * glm::mat4(1.0f) - glm::outerProduct(light, plane);

      glDisable(GL_LIGHTING);
      glDisable(GL_TEXTURE_2D);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glColor4f(0.0f, 0.0f, 0.0f, SHADOW_OPACITY);
      glStencilFunc(GL_EQUAL, 1, 0xFF);
      glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

      for (Mesh *mesh : casters)
      {
         glLoadMatrixf(glm::value_ptr(view * shadow * mesh->getWorldMatrix()));
         mesh->renderGeometry();
      }

      glDisable(GL_BLEND);
      glEnable(GL_LIGHTING);
   }
   glDisable(GL_STENCIL_TEST);
}
