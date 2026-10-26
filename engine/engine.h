/**
 * @file		engine.h
 * @brief	Graphics engine main include file
 *
 * @author	Alessio Gervasini
 */
#pragma once

   #include <memory>
   #include <string>
   #include <utility>
   #include <vector>

   #define GLM_ENABLE_EXPERIMENTAL
   #include <glm/glm.hpp>
   #include <glm/gtc/matrix_transform.hpp>
   #include <glm/gtc/type_ptr.hpp>
   #include <glm/gtx/string_cast.hpp>

#ifdef _DEBUG
   #define LIB_NAME      "Graphics Engine v0.4 (debug)"   ///< Library credits
#else
   #define LIB_NAME      "Graphics Engine v0.4"           ///< Library credits
#endif

#ifdef _WINDOWS
   #ifdef GRAPHICS_ENGINE_EXPORTS
      #define ENG_API __declspec(dllexport)
   #else
      #define ENG_API __declspec(dllimport)
   #endif

   #pragma warning(disable : 4251)
#else
   #define ENG_API
#endif

   #include "object.h"
   #include "node.h"
   #include "texture.h"
   #include "material.h"
   #include "mesh.h"
   #include "light.h"
   #include "directionallight.h"
   #include "pointlight.h"
   #include "camera.h"
   #include "list.h"
   #include "loader.h"

namespace Eng {

/**
 * @brief Base engine main class (singleton): owns the window and the rendering context, reads the input
 * and renders a render list. The libraries used for these tasks stay hidden inside the engine.
 */
class ENG_API Base final
{
public:

   /**
    * @brief Function called once every time a key is pressed (letters in uppercase).
    */
   using KeyboardCallback = void (*)(unsigned char key);

   Base(Base const &) = delete;
   ~Base();

   void operator=(Base const &) = delete;

   static Base &getInstance();

   bool init(const std::string &title, int width, int height);
   bool free();

   bool isRunning() const;
   void setBackgroundColor(const glm::vec3 &color);
   void setKeyboardCallback(KeyboardCallback callback);

   void clearWindow();
   void render(Camera *camera, List *list, Light *shadowLight);
   void renderLegend(const std::vector<std::pair<std::string, std::string>> &entries);
   void swapBuffers();

   float getDeltaTime() const;
   bool isKeyDown(unsigned char key) const;
   glm::vec2 getMouseDelta();

private:

   struct Reserved;
   std::unique_ptr<Reserved> reserved;

   Base();
};

};
