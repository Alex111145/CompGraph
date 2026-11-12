/**
 * @file		engine.h
 * @brief	Graphics engine main include file
 *
 * @author	Alessio Gervasini (C) SUPSI
 */
#pragma once



//////////////
// #INCLUDE //
//////////////

   // C/C++:
   #include <memory>
   #include <string>
   #include <vector>

   // GLM:
   #include <glm/glm.hpp>



/////////////
// VERSION //
/////////////

   // Generic info:
#ifdef _DEBUG
   #define LIB_NAME      "Gervasini Project v0.1a (debug)"   ///< Library credits
#else
   #define LIB_NAME      "Gervasini Project v0.1a"   ///< Library credits
#endif
   #define LIB_VERSION   10                           ///< Library version (divide by 10)

   // Export API:
#ifdef _WINDOWS
   // Specifies i/o linkage (VC++ spec):
   #ifdef GRAPHICS_ENGINE_EXPORTS
      #define ENG_API __declspec(dllexport)
   #else
      #define ENG_API __declspec(dllimport)
   #endif

   // Get rid of annoying warnings:
   #pragma warning(disable : 4251)
#else // Under linux
   #define ENG_API
#endif



//////////////
// #INCLUDE //
//////////////

   // Engine classes:
   #include "object.h"
   #include "node.h"
   #include "material.h"
   #include "mesh.h"
   #include "light.h"
   #include "camera.h"
   #include "list.h"



///////////////
// NAMESPACE //
///////////////

namespace Eng {



///////////////////////
// MAIN ENGINE CLASS //
///////////////////////

/**
 * @brief Base engine main class. This class is a singleton that wraps FreeGLUT and the OpenGL context.
 */
class ENG_API Base final
{
//////////
public: //
//////////

   // Const/dest:
   Base(Base const &) = delete;
   ~Base();

   // Operators:
   void operator=(Base const &) = delete;

   // Singleton:
   static Base &getInstance();

   // Init/free:
   bool init(const std::string &title, int width, int height);
   bool free();

   // Window:
   void setKeyboardCallback(void (*callback)(unsigned char key, int mouseX, int mouseY));
   void setBackgroundColor(const glm::vec3 &color);
   bool isRunning();
   void clearWindow();
   void swapBuffers();
   float getAspectRatio() const;

   // Rendering:
   void setWireframe(bool enabled);
   bool isWireframe() const;
   void render(const Camera &camera, const List &list);


///////////
private: //
///////////

   // Reserved:
   struct Reserved;
   std::unique_ptr<Reserved> reserved;

   // Const/dest:
   Base();

   // FreeGLUT callbacks:
   static void displayCallback();
   static void reshapeCallback(int width, int height);
   static void closeCallback();
};

}; // end of namespace Eng::
