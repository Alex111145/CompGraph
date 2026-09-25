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
   /**
    * GLFW error callback: prints GLFW-level errors to stderr.
    */
   void glfwErrorCallback(int error, const char *description)
   {
      std::cerr << "GLFW error " << error << ": " << description << std::endl;
   }

   /** GLFW framebuffer resize callback: keeps the GL viewport matching the window's framebuffer. */
   void framebufferSizeCallback(GLFWwindow *window, int width, int height)
   {
      glViewport(0, 0, width, height);
   }
}

/**
 * @brief Base class reserved structure (using PIMPL/Bridge design pattern https://en.wikipedia.org/wiki/Opaque_pointer).
 */
struct Eng::Base::Reserved
{
   bool initFlag;

   GLFWwindow *window;
   int windowWidth;
   int windowHeight;

   double lastFrameTime;
   float deltaTime;

   double lastMouseX;
   double lastMouseY;
   bool firstMouseSample;

   /**
    * Constructor.
    */
   Reserved() :
      initFlag{ false }, window{ nullptr }, windowWidth{ 800 }, windowHeight{ 600 },
      lastFrameTime{ 0.0 }, deltaTime{ 0.0f },
      lastMouseX{ 0.0 }, lastMouseY{ 0.0 }, firstMouseSample{ true }
   {}
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
 * Init internal components. 
 * @return TF
 */
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

   glfwWindowHint(GLFW_STENCIL_BITS, 8);   // needed by the client's stencil-clipped planar reflection

   // No GLFW_CONTEXT_VERSION_* / GLFW_OPENGL_PROFILE hints on purpose: this engine only
   // uses OpenGL 1.1 fixed-function calls (glBegin/glVertex/glLight/matrix stack, no
   // shaders). Asking GLFW for a specific old version or a core profile can make some
   // drivers refuse the context outright; leaving the hints unset gives the platform's
   // default desktop-GL context, which on Windows/Linux is compatibility-profile and
   // still exposes the full 1.1 API we need.
   reserved->window = glfwCreateWindow(reserved->windowWidth, reserved->windowHeight, LIB_NAME, nullptr, nullptr);
   if (reserved->window == nullptr)
   {
      std::cout << "ERROR: unable to create the GLFW window" << std::endl;
      glfwTerminate();
      return false;
   }

   glfwMakeContextCurrent(reserved->window);
   glfwSwapInterval(1);
   glfwSetFramebufferSizeCallback(reserved->window, framebufferSizeCallback);

   glfwSetInputMode(reserved->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
   glfwGetCursorPos(reserved->window, &reserved->lastMouseX, &reserved->lastMouseY);
   reserved->lastFrameTime = glfwGetTime();

   // No extension loader (GLEW/glad) needed: every symbol used by this engine is core
   // OpenGL 1.1, which is guaranteed to be exported directly by opengl32.lib on Windows
   // and libGL on Linux -- no runtime extension lookup involved.

   std::cout << "   OpenGL vendor:   " << reinterpret_cast<const char *>(glGetString(GL_VENDOR)) << std::endl;
   std::cout << "   OpenGL renderer: " << reinterpret_cast<const char *>(glGetString(GL_RENDERER)) << std::endl;
   std::cout << "   OpenGL version:  " << reinterpret_cast<const char *>(glGetString(GL_VERSION)) << std::endl;

   glEnable(GL_DEPTH_TEST);

   std::cout << "[>] " << LIB_NAME << " initialized" << std::endl;
   reserved->initFlag = true;
   return true;
}

/**
 * Free internal components.
 * @return TF
 */
bool ENG_API Eng::Base::free()
{
   if (!reserved->initFlag)
   {
      std::cout << "ERROR: engine not initialized" << std::endl;
      return false;
   }

   if (reserved->window != nullptr)
   {
      glfwDestroyWindow(reserved->window);
      reserved->window = nullptr;
   }
   glfwTerminate();

   std::cout << "[<] " << LIB_NAME << " deinitialized" << std::endl;
   reserved->initFlag = false;
   return true;
}

/**
 * Tells whether the engine window is still open (i.e. the render loop should keep going).
 * @return TF
 */
bool ENG_API Eng::Base::isRunning() const
{
   return reserved->window != nullptr && !glfwWindowShouldClose(reserved->window);
}

/**
 * Asks the render loop to stop after this frame — same effect as clicking the window's
 * close button (sets GLFW's own close flag, which isRunning() already checks).
 */
void ENG_API Eng::Base::requestClose()
{
   glfwSetWindowShouldClose(reserved->window, GLFW_TRUE);
}

/**
 * Presents the rendered frame and processes pending window/input events. Call once per render loop iteration.
 */
void ENG_API Eng::Base::swapBuffers()
{
   glfwSwapBuffers(reserved->window);
   glfwPollEvents();

   double now = glfwGetTime();
   reserved->deltaTime = static_cast<float>(now - reserved->lastFrameTime);
   reserved->lastFrameTime = now;
}

/**
 * Current framebuffer size in pixels (not the same as window size on HiDPI/Retina displays).
 */
void ENG_API Eng::Base::getFramebufferSize(int &width, int &height) const
{
   glfwGetFramebufferSize(reserved->window, &width, &height);
}

/**
 * Changes the OS window's title bar text — used to surface a live FPS counter without
 * spamming the console every frame.
 */
void ENG_API Eng::Base::setWindowTitle(const std::string &title)
{
   glfwSetWindowTitle(reserved->window, title.c_str());
}

/**
 * Seconds elapsed since the previous swapBuffers() call. Use to scale per-frame movement.
 */
float ENG_API Eng::Base::getDeltaTime() const
{
   return reserved->deltaTime;
}

/**
 * Tells whether a key is currently held down.
 * @param key ASCII code of a printable key (e.g. 'W'), matching GLFW's own convention
 */
bool ENG_API Eng::Base::isKeyPressed(int key) const
{
   return glfwGetKey(reserved->window, key) == GLFW_PRESS;
}

/**
 * Mouse movement in pixels since the last call (the cursor is captured/hidden for FPS-style look).
 */
void ENG_API Eng::Base::getCursorDelta(float &deltaX, float &deltaY)
{
   double x, y;
   glfwGetCursorPos(reserved->window, &x, &y);

   if (reserved->firstMouseSample)
   {
      reserved->lastMouseX = x;
      reserved->lastMouseY = y;
      reserved->firstMouseSample = false;
   }

   deltaX = static_cast<float>(x - reserved->lastMouseX);
   deltaY = static_cast<float>(y - reserved->lastMouseY);
   reserved->lastMouseX = x;
   reserved->lastMouseY = y;
}
