/**
 * @file		engine.cpp
 * @brief	Graphics engine main file
 *
 * @author	Alessio Gervasini
 */

   #include "engine.h"

   #include <algorithm>
   #include <cctype>
   #include <iostream>
   #include <source_location>

   #include <GLFW/glfw3.h>

namespace
{
   const int WINDOW_WIDTH = 1024;
   const int WINDOW_HEIGHT = 768;
   const float SHADOW_LIFT = 0.002f;
   const float SHADOW_OPACITY = 0.45f;

   const int GLYPH_COLUMNS = 5;
   const int GLYPH_ROWS = 7;
   const int GLYPH_ADVANCE = GLYPH_COLUMNS + 1;

   /**
    * @brief One character of the legend font: 7 rows of 5 cells, '#' = filled square.
    */
   struct Glyph
   {
      char character;               ///< Character drawn
      const char *rows[GLYPH_ROWS]; ///< Cells, top row first
   };

   const Glyph FONT[] = {
      {' ', {".....", ".....", ".....", ".....", ".....", ".....", "....."}},
      {'A', {".###.", "#...#", "#...#", "#####", "#...#", "#...#", "#...#"}},
      {'C', {".####", "#....", "#....", "#....", "#....", "#....", ".####"}},
      {'D', {"####.", "#...#", "#...#", "#...#", "#...#", "#...#", "####."}},
      {'E', {"#####", "#....", "#....", "####.", "#....", "#....", "#####"}},
      {'H', {"#...#", "#...#", "#...#", "#####", "#...#", "#...#", "#...#"}},
      {'I', {"#####", "..#..", "..#..", "..#..", "..#..", "..#..", "#####"}},
      {'K', {"#...#", "#..#.", "#.#..", "##...", "#.#..", "#..#.", "#...#"}},
      {'L', {"#....", "#....", "#....", "#....", "#....", "#....", "#####"}},
      {'M', {"#...#", "##.##", "#.#.#", "#...#", "#...#", "#...#", "#...#"}},
      {'N', {"#...#", "##..#", "#.#.#", "#..##", "#...#", "#...#", "#...#"}},
      {'O', {".###.", "#...#", "#...#", "#...#", "#...#", "#...#", ".###."}},
      {'P', {"####.", "#...#", "#...#", "####.", "#....", "#....", "#...."}},
      {'Q', {".###.", "#...#", "#...#", "#...#", "#.#.#", "#..#.", ".##.#"}},
      {'R', {"####.", "#...#", "#...#", "####.", "#.#..", "#..#.", "#...#"}},
      {'S', {".####", "#....", "#....", ".###.", "....#", "....#", "####."}},
      {'T', {"#####", "..#..", "..#..", "..#..", "..#..", "..#..", "..#.."}},
      {'U', {"#...#", "#...#", "#...#", "#...#", "#...#", "#...#", ".###."}},
      {'V', {"#...#", "#...#", "#...#", "#...#", "#...#", ".#.#.", "..#.."}},
      {'W', {"#...#", "#...#", "#...#", "#.#.#", "#.#.#", "##.##", "#...#"}},
      {'X', {"#...#", "#...#", ".#.#.", "..#..", ".#.#.", "#...#", "#...#"}},
      {'1', {"..#..", ".##..", "..#..", "..#..", "..#..", "..#..", ".###."}},
      {'2', {".###.", "#...#", "....#", "...#.", "..#..", ".#...", "#####"}},
      {'/', {"....#", "....#", "...#.", "..#..", ".#...", "#....", "#...."}},
   };

   /**
    * Finds the glyph of a character (case insensitive).
    * @param character character to draw
    * @return glyph, or nullptr if the font does not have it
    */
   const Glyph *findGlyph(char character)
   {
      const char upperCase = static_cast<char>(std::toupper(static_cast<unsigned char>(character)));
      for (const Glyph &glyph : FONT)
         if (glyph.character == upperCase)
            return &glyph;
      return nullptr;
   }

   /**
    * Computes the width of a text in pixels.
    * @param text text to measure
    * @param pixelSize side of one font cell in pixels
    * @return width in pixels
    */
   float textWidth(const std::string &text, float pixelSize)
   {
      return static_cast<float>(text.size() * GLYPH_ADVANCE) * pixelSize;
   }

   /**
    * Draws a text as a set of small squares (one GL_QUADS block).
    * @param text text to draw
    * @param x left side in pixels
    * @param y top side in pixels
    * @param pixelSize side of one font cell in pixels
    */
   void drawText(const std::string &text, float x, float y, float pixelSize)
   {
      float penX = x;

      glBegin(GL_QUADS);
      for (char character : text)
      {
         const Glyph *glyph = findGlyph(character);
         for (int row = 0; glyph != nullptr && row < GLYPH_ROWS; row++)
            for (int column = 0; column < GLYPH_COLUMNS; column++)
            {
               if (glyph->rows[row][column] != '#')
                  continue;

               const float left = penX + column * pixelSize;
               const float top = y + row * pixelSize;
               glVertex2f(left, top);
               glVertex2f(left + pixelSize, top);
               glVertex2f(left + pixelSize, top + pixelSize);
               glVertex2f(left, top + pixelSize);
            }
         penX += GLYPH_ADVANCE * pixelSize;
      }
      glEnd();
   }

   /**
    * Draws a rectangle, filled (GL_QUADS) or as an outline (GL_LINE_LOOP).
    * @param mode GL_QUADS or GL_LINE_LOOP
    * @param left left side in pixels
    * @param top top side in pixels
    * @param right right side in pixels
    * @param bottom bottom side in pixels
    */
   void drawRectangle(GLenum mode, float left, float top, float right, float bottom)
   {
      glBegin(mode);
      glVertex2f(left, top);
      glVertex2f(right, top);
      glVertex2f(right, bottom);
      glVertex2f(left, bottom);
      glEnd();
   }
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
 * Only meshes lower than a point light cast a shadow: a point above the light cannot be projected on the floor.
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
         if (light.w != 0.0f && mesh->getWorldMaxY() >= light.y)
            continue;
         glLoadMatrixf(glm::value_ptr(view * shadow * mesh->getWorldMatrix()));
         mesh->renderGeometry();
      }

      glDisable(GL_BLEND);
      glEnable(GL_LIGHTING);
   }
   glDisable(GL_STENCIL_TEST);
}

/**
 * Draws the controls legend in the top-right corner of the window: a semi-transparent panel with
 * the title CONTROLS and one line per entry (key on the left, action on the right).
 * The panel is drawn in pixel coordinates with an orthographic projection (glOrtho).
 * @param entries list of (key, action) pairs
 */
void ENG_API Eng::Base::renderLegend(const std::vector<std::pair<std::string, std::string>> &entries)
{
   const float pixelSize = 4.0f;
   const float titleSize = pixelSize * 1.5f;
   const float lineHeight = (GLYPH_ROWS + 2) * pixelSize;
   const float titleHeight = GLYPH_ROWS * titleSize;
   const float sectionGap = 2.0f * pixelSize;
   const float margin = 12.0f;
   const float padding = 3.0f * pixelSize;
   float widestKey = 0.0f;
   float widestAction = 0.0f;
   int width, height;

   glfwGetFramebufferSize(reserved->window, &width, &height);
   for (const std::pair<std::string, std::string> &entry : entries)
   {
      widestKey = std::max(widestKey, textWidth(entry.first, pixelSize));
      widestAction = std::max(widestAction, textWidth(entry.second, pixelSize));
   }

   const float keyColumnWidth = widestKey + GLYPH_ADVANCE * pixelSize;
   const float contentWidth = std::max(keyColumnWidth + widestAction, textWidth("CONTROLS", titleSize));
   const float contentHeight = titleHeight + 2.0f * sectionGap + entries.size() * lineHeight;
   const float panelRight = static_cast<float>(width) - margin;
   const float panelLeft = panelRight - contentWidth - 2.0f * padding;
   const float panelTop = margin;
   const float panelBottom = panelTop + contentHeight + 2.0f * padding;
   const float textLeft = panelLeft + padding;
   float penY = panelTop + padding;

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(0.0, width, height, 0.0, -1.0, 1.0);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   glDisable(GL_LIGHTING);
   glDisable(GL_TEXTURE_2D);
   glDisable(GL_DEPTH_TEST);
   glDisable(GL_CULL_FACE);
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   glColor4f(0.05f, 0.05f, 0.07f, 0.55f);
   drawRectangle(GL_QUADS, panelLeft, panelTop, panelRight, panelBottom);

   glLineWidth(1.5f);
   glColor4f(1.0f, 0.55f, 0.20f, 0.35f);
   drawRectangle(GL_LINE_LOOP, panelLeft, panelTop, panelRight, panelBottom);

   glColor3f(1.0f, 1.0f, 1.0f);
   drawText("CONTROLS", textLeft, penY, titleSize);
   drawText("CONTROLS", textLeft + 1.0f, penY, titleSize);
   penY += titleHeight + sectionGap;

   glColor4f(1.0f, 0.55f, 0.20f, 0.6f);
   glBegin(GL_LINES);
   glVertex2f(textLeft, penY);
   glVertex2f(textLeft + contentWidth, penY);
   glEnd();
   glLineWidth(1.0f);
   penY += sectionGap;

   for (const std::pair<std::string, std::string> &entry : entries)
   {
      glColor3f(1.0f, 0.72f, 0.40f);
      drawText(entry.first, textLeft, penY, pixelSize);
      glColor3f(0.80f, 0.80f, 0.82f);
      drawText(entry.second, textLeft + keyColumnWidth, penY, pixelSize);
      penY += lineHeight;
   }

   glDisable(GL_BLEND);
   glEnable(GL_CULL_FACE);
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_LIGHTING);
}
