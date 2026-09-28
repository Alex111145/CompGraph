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
   const int WINDOW_WIDTH = 800;
   const int WINDOW_HEIGHT = 600;

   void glfwErrorCallback(int error, const char *description)
   {
      std::cerr << "GLFW error " << error << ": " << description << std::endl;
   }
}

struct Eng::Base::Reserved
{
   bool initFlag;
   GLFWwindow *window;
   double lastFrameTime;
   float deltaTime;
   double lastMouseX;
   double lastMouseY;
   bool firstMouseSample;

   Reserved() :
      initFlag{ false }, window{ nullptr }, lastFrameTime{ 0.0 }, deltaTime{ 0.0f },
      lastMouseX{ 0.0 }, lastMouseY{ 0.0 }, firstMouseSample{ true }
   {}
};

ENG_API Eng::Base::Base() : reserved(std::make_unique<Eng::Base::Reserved>())
{
#ifdef _DEBUG
   std::cout << "[+] " << std::source_location::current().function_name() << " invoked" << std::endl;
#endif
}

ENG_API Eng::Base::~Base()
{
#ifdef _DEBUG
   std::cout << "[-] " << std::source_location::current().function_name() << " invoked" << std::endl;
#endif
}

Eng::Base ENG_API &Eng::Base::getInstance()
{
   static Base instance;
   return instance;
}

bool ENG_API Eng::Base::init()
{
   if (reserved->initFlag)
   {
      std::cout << "ERROR: engine already initialized" << std::endl;
      return false;
   }

   glfwSetErrorCallback(glfwErrorCallback);
   if (!glfwInit())
   {
      std::cout << "ERROR: unable to initialize GLFW" << std::endl;
      return false;
   }

   glfwWindowHint(GLFW_STENCIL_BITS, 8);
   reserved->window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, LIB_NAME, nullptr, nullptr);
   if (reserved->window == nullptr)
   {
      std::cout << "ERROR: unable to create the GLFW window" << std::endl;
      glfwTerminate();
      return false;
   }

   glfwMakeContextCurrent(reserved->window);
   glfwSwapInterval(1);
   glfwSetInputMode(reserved->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
   reserved->lastFrameTime = glfwGetTime();

   std::cout << "   OpenGL vendor:   " << reinterpret_cast<const char *>(glGetString(GL_VENDOR)) << std::endl;
   std::cout << "   OpenGL renderer: " << reinterpret_cast<const char *>(glGetString(GL_RENDERER)) << std::endl;
   std::cout << "   OpenGL version:  " << reinterpret_cast<const char *>(glGetString(GL_VERSION)) << std::endl;

   glEnable(GL_DEPTH_TEST);

   std::cout << "[>] " << LIB_NAME << " initialized" << std::endl;
   reserved->initFlag = true;
   return true;
}

bool ENG_API Eng::Base::free()
{
   if (!reserved->initFlag)
   {
      std::cout << "ERROR: engine not initialized" << std::endl;
      return false;
   }

   glfwDestroyWindow(reserved->window);
   reserved->window = nullptr;
   glfwTerminate();

   std::cout << "[<] " << LIB_NAME << " deinitialized" << std::endl;
   reserved->initFlag = false;
   return true;
}

bool ENG_API Eng::Base::isRunning() const
{
   return reserved->window != nullptr && !glfwWindowShouldClose(reserved->window);
}

void ENG_API Eng::Base::requestClose()
{
   glfwSetWindowShouldClose(reserved->window, GLFW_TRUE);
}

void ENG_API Eng::Base::swapBuffers()
{
   glfwSwapBuffers(reserved->window);
   glfwPollEvents();

   const double now = glfwGetTime();
   reserved->deltaTime = static_cast<float>(now - reserved->lastFrameTime);
   reserved->lastFrameTime = now;
}

void ENG_API Eng::Base::getFramebufferSize(int &width, int &height) const
{
   glfwGetFramebufferSize(reserved->window, &width, &height);
}

float ENG_API Eng::Base::getDeltaTime() const
{
   return reserved->deltaTime;
}

void ENG_API Eng::Base::setWindowTitle(const std::string &title)
{
   glfwSetWindowTitle(reserved->window, title.c_str());
}

bool ENG_API Eng::Base::isKeyPressed(int key) const
{
   return glfwGetKey(reserved->window, key) == GLFW_PRESS;
}

void ENG_API Eng::Base::getCursorDelta(float &deltaX, float &deltaY)
{
   double cursorX, cursorY;
   glfwGetCursorPos(reserved->window, &cursorX, &cursorY);

   if (reserved->firstMouseSample)
   {
      reserved->lastMouseX = cursorX;
      reserved->lastMouseY = cursorY;
      reserved->firstMouseSample = false;
   }

   deltaX = static_cast<float>(cursorX - reserved->lastMouseX);
   deltaY = static_cast<float>(cursorY - reserved->lastMouseY);
   reserved->lastMouseX = cursorX;
   reserved->lastMouseY = cursorY;
}
