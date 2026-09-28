/**
 * @file		engine.h
 * @brief	Graphics engine main include file
 *
 * @author	Alessio Gervasini
 */
#pragma once

   #include <memory>
   #include <string>

#ifdef _DEBUG
   #define LIB_NAME      "Graphics Engine OpenGL 1.1 (debug)"
#else
   #define LIB_NAME      "Graphics Engine OpenGL 1.1"
#endif

#ifdef _WINDOWS
   #ifdef GRAPHICS_ENGINE_EXPORTS
      #define ENG_API __declspec(dllexport)
   #else
      #define ENG_API __declspec(dllimport)
   #endif

   #ifdef _MSC_VER
      #pragma warning(disable : 4251)
   #endif
#else
   #define ENG_API
#endif

   #include "camera.h"
   #include "light.h"
   #include "mesh.h"
   #include "node.h"
   #include "texture.h"
   #include "loader.h"

namespace Eng {

class ENG_API Base final
{
public:

   Base(Base const &) = delete;
   ~Base();

   void operator=(Base const &) = delete;

   static Base &getInstance();

   bool init();
   bool free();

   bool isRunning() const;
   void requestClose();
   void swapBuffers();
   void getFramebufferSize(int &width, int &height) const;
   float getDeltaTime() const;
   void setWindowTitle(const std::string &title);

   bool isKeyPressed(int key) const;
   void getCursorDelta(float &deltaX, float &deltaY);

private:

   struct Reserved;
   std::unique_ptr<Reserved> reserved;

   Base();
};

};
