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
   #define LIB_NAME      "Graphics Engine OpenGL 1.1 (debug)"   ///< Library credits
#else
   #define LIB_NAME      "Graphics Engine OpenGL 1.1"           ///< Library credits
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

   #include "node.h"
   #include "texture.h"
   #include "material.h"
   #include "mesh.h"
   #include "light.h"
   #include "camera.h"
   #include "loader.h"

namespace Eng {

/**
 * @brief Base engine main class (singleton): owns the window, reads the input and renders a scene graph.
 */
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
   void update();
   float getDeltaTime() const;

   bool isKeyDown(int key) const;
   bool wasKeyPressed(int key);
   glm::vec2 getMouseDelta();

   void render(Camera *camera, Node *root, const glm::vec4 &shadowLight);
   void renderLegend(const std::vector<std::pair<std::string, std::string>> &entries);

private:

   struct Reserved;
   std::unique_ptr<Reserved> reserved;

   Base();
};

};
