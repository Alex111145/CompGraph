/**
 * @file		engine.cpp
 * @brief	Graphics engine main file
 *
 * @author	Alessio Gervasini (C) SUPSI
 */



//////////////
// #INCLUDE //
//////////////

   // Main include:
   #include "engine.h"

   // FreeGLUT:
   #include <GL/freeglut.h>

   // GLM:
   #include <glm/gtc/type_ptr.hpp>

   // C/C++:
   #include <iostream>
   #include <source_location>



/////////////////////////
// RESERVED STRUCTURES //
/////////////////////////

/**
 * @brief Base class reserved structure (using PIMPL/Bridge design pattern https://en.wikipedia.org/wiki/Opaque_pointer).
 */
struct Eng::Base::Reserved
{
   // Flags:
   bool initFlag;    ///< True between init() and free()
   bool running;     ///< True until the window is closed
   bool wireframe;   ///< True when polygons are drawn as lines

   // Window:
   int width;        ///< Window width, in pixels
   int height;       ///< Window height, in pixels (never 0)


   /**
    * Constructor.
    */
   Reserved() : initFlag{ false }, running{ false }, wireframe{ false }, width{ 0 }, height{ 1 }
   {}
};



////////////////////////
// BODY OF CLASS Base //
////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Constructor.
 */
ENG_API Eng::Base::Base() : reserved(std::make_unique<Eng::Base::Reserved>())
{
#ifdef _DEBUG
   std::cout << "[+] " << std::source_location::current().function_name() << " invoked" << std::endl;
#endif
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Destructor.
 */
ENG_API Eng::Base::~Base()
{
#ifdef _DEBUG
   std::cout << "[-] " << std::source_location::current().function_name() << " invoked" << std::endl;
#endif
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Gets a reference to the (unique) singleton instance.
 * @return reference to singleton instance
 */
Eng::Base ENG_API &Eng::Base::getInstance()
{
   static Base instance;
   return instance;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Init internal components: creates the window and the OpenGL context, registers the callbacks
 * and enables depth test, face culling, lighting and normalization of normals. The lights are switched on by the Light class.
 * @param title window title
 * @param width window width, in pixels
 * @param height window height, in pixels
 * @return TF
 */
bool ENG_API Eng::Base::init(const std::string &title, int width, int height)
{
   if (reserved->initFlag)
   {
      std::cout << "ERROR: engine already initialized" << std::endl;
      return false;
   }

   if (width <= 0 || height <= 0)
   {
      std::cout << "ERROR: invalid window size" << std::endl;
      return false;
   }

   int argc = 1;
   char programName[] = "engine";
   char *argv[] = { programName };
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
   glutInitWindowSize(width, height);
   glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);
   glutCreateWindow(title.c_str());

   glutDisplayFunc(displayCallback);
   glutReshapeFunc(reshapeCallback);
   glutCloseFunc(closeCallback);

   glEnable(GL_DEPTH_TEST);
   glEnable(GL_CULL_FACE);
   glEnable(GL_LIGHTING);
   glEnable(GL_NORMALIZE);
   glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, 1.0f);

   reserved->width = width;
   reserved->height = height;
   reserved->running = true;

   std::cout << "[>] " << LIB_NAME << " initialized" << std::endl;
   reserved->initFlag = true;
   return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Free internal components: closes the window and releases FreeGLUT.
 * @return TF
 */
bool ENG_API Eng::Base::free()
{
   if (!reserved->initFlag)
   {
      std::cout << "ERROR: engine not initialized" << std::endl;
      return false;
   }

   glutExit();
   reserved->running = false;

   std::cout << "[<] " << LIB_NAME << " deinitialized" << std::endl;
   reserved->initFlag = false;
   return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Sets the function called when a key is pressed. Using it is optional: the client decides.
 * @param callback function receiving the key and the mouse position, in pixels (nullptr = no keyboard callback)
 */
void ENG_API Eng::Base::setKeyboardCallback(void (*callback)(unsigned char key, int mouseX, int mouseY))
{
   if (!reserved->initFlag)
   {
      std::cout << "ERROR: engine not initialized" << std::endl;
      return;
   }

   glutKeyboardFunc(callback);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Sets the color used by clearWindow().
 * @param color RGB color, each component between 0 and 1
 */
void ENG_API Eng::Base::setBackgroundColor(const glm::vec3 &color)
{
   if (!reserved->initFlag)
   {
      std::cout << "ERROR: engine not initialized" << std::endl;
      return;
   }

   glClearColor(color.r, color.g, color.b, 1.0f);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Processes the pending window events and tells whether the window is still open.
 * @return TF (false once the window has been closed)
 */
bool ENG_API Eng::Base::isRunning()
{
   if (!reserved->running)
      return false;

   glutMainLoopEvent();
   return reserved->running;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Clears the color and depth buffers.
 */
void ENG_API Eng::Base::clearWindow()
{
   if (!reserved->running)
      return;

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Shows the back buffer on the screen.
 */
void ENG_API Eng::Base::swapBuffers()
{
   if (!reserved->running)
      return;

   glutSwapBuffers();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Gets the aspect ratio of the window, needed by the client to build the projection matrix of a camera.
 * @return window width divided by window height
 */
float ENG_API Eng::Base::getAspectRatio() const
{
   return static_cast<float>(reserved->width) / static_cast<float>(reserved->height);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Switches between wireframe and solid mode.
 * @param enabled true for wireframe, false for solid
 */
void ENG_API Eng::Base::setWireframe(bool enabled)
{
   if (!reserved->running)
      return;

   glPolygonMode(GL_FRONT_AND_BACK, enabled ? GL_LINE : GL_FILL);
   reserved->wireframe = enabled;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Tells whether wireframe mode is active.
 * @return TF
 */
bool ENG_API Eng::Base::isWireframe() const
{
   return reserved->wireframe;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Renders a list seen from a camera: loads the projection matrix of the camera, then renders the list
 * with the inverse camera matrix.
 * @param camera camera used to look at the scene
 * @param list list of the nodes to render, already filled with List::pass()
 */
void ENG_API Eng::Base::render(const Camera &camera, const List &list)
{
   if (!reserved->running)
      return;

   glMatrixMode(GL_PROJECTION);
   glLoadMatrixf(glm::value_ptr(camera.getProjectionMatrix()));

   glMatrixMode(GL_MODELVIEW);
   list.render(camera.getInverseMatrix());
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Display callback required by FreeGLUT. Empty: the client draws inside its own loop.
 */
void ENG_API Eng::Base::displayCallback()
{}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Reshape callback: updates the viewport and the size used for the aspect ratio.
 * @param width new window width, in pixels
 * @param height new window height, in pixels
 */
void ENG_API Eng::Base::reshapeCallback(int width, int height)
{
   Base &eng = getInstance();
   eng.reserved->width = width;
   eng.reserved->height = height > 0 ? height : 1;
   glViewport(0, 0, width, height);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Close callback: the window has been closed by the user.
 */
void ENG_API Eng::Base::closeCallback()
{
   getInstance().reserved->running = false;
}
