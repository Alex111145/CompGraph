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

   #include <GL/freeglut.h>

namespace
{
   const unsigned char ESCAPE_KEY = 27;
   const int KEY_COUNT = 256;
   char PROGRAM_NAME[] = "engine";

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
      {'F', {"#####", "#....", "#....", "####.", "#....", "#....", "#...."}},
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
 * @brief Base class reserved structure (PIMPL): hides FreeGLUT from the engine users.
 */
struct Eng::Base::Reserved
{
   bool initFlag;                      ///< Engine initialized
   bool running;                       ///< False after ESC or after the window has been closed
   int windowId;                       ///< FreeGLUT window identifier
   std::string title;                  ///< Window title (the FPS are appended)
   glm::vec3 backgroundColor;          ///< Clear color
   KeyboardCallback keyboardCallback;  ///< Application function called at every key press
   bool wireframe;                     ///< Polygons drawn as lines (true) or filled (false)
   bool keyDown[KEY_COUNT];            ///< Keys held down, indexed by uppercase character
   glm::vec2 mouseDelta;               ///< Mouse movement since the last getMouseDelta()
   int lastTime;                       ///< Time of the previous frame in milliseconds
   float deltaTime;                    ///< Seconds between the last two frames
   int fpsTime;                        ///< Start of the current FPS measurement in milliseconds
   int fpsFrames;                      ///< Frames drawn since fpsTime

   /**
    * Constructor of the reserved structure.
    */
   Reserved() :
      initFlag{ false }, running{ false }, windowId{ 0 }, backgroundColor{ glm::vec3(0.0f) }, keyboardCallback{ nullptr },
      wireframe{ false }, keyDown{}, mouseDelta{ glm::vec2(0.0f) }, lastTime{ 0 }, deltaTime{ 0.0f }, fpsTime{ 0 }, fpsFrames{ 0 }
   {}

   /**
    * FreeGLUT display callback. Empty: the frame is drawn by the application loop.
    */
   static void displayCallback()
   {}

   /**
    * FreeGLUT key press callback: ESC stops the engine, the other keys are stored and passed to the application.
    * @param key character of the key
    * @param mouseX mouse position X
    * @param mouseY mouse position Y
    */
   static void keyDownCallback(unsigned char key, int mouseX, int mouseY)
   {
      Reserved &reserved = *getInstance().reserved;
      const unsigned char upperCase = static_cast<unsigned char>(std::toupper(key));

      if (key == ESCAPE_KEY)
      {
         reserved.running = false;
         return;
      }

      reserved.keyDown[upperCase] = true;
      if (reserved.keyboardCallback != nullptr)
         reserved.keyboardCallback(upperCase);
   }

   /**
    * FreeGLUT key release callback.
    * @param key character of the key
    * @param mouseX mouse position X
    * @param mouseY mouse position Y
    */
   static void keyUpCallback(unsigned char key, int mouseX, int mouseY)
   {
      getInstance().reserved->keyDown[static_cast<unsigned char>(std::toupper(key))] = false;
   }

   /**
    * FreeGLUT mouse callback: adds the distance from the window center to the movement, then puts the
    * (hidden) cursor back in the center, so that the mouse can turn without limits.
    * @param mouseX mouse position X
    * @param mouseY mouse position Y
    */
   static void mouseMoveCallback(int mouseX, int mouseY)
   {
      Reserved &reserved = *getInstance().reserved;
      const int centerX = glutGet(GLUT_WINDOW_WIDTH) / 2;
      const int centerY = glutGet(GLUT_WINDOW_HEIGHT) / 2;

      if (mouseX == centerX && mouseY == centerY)
         return;

      reserved.mouseDelta += glm::vec2(static_cast<float>(mouseX - centerX), static_cast<float>(mouseY - centerY));
      glutWarpPointer(centerX, centerY);
   }

   /**
    * FreeGLUT close callback: the window has been closed with the mouse.
    */
   static void windowCloseCallback()
   {
      getInstance().reserved->running = false;
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
 * Opens the window with FreeGLUT (double buffer, depth and stencil buffer), registers the callbacks
 * and sets the fixed-pipeline state.
 * @param title window title
 * @param width window width in pixels
 * @param height window height in pixels
 * @return TF
 */
bool ENG_API Eng::Base::init(const std::string &title, int width, int height)
{
   int argc = 1;
   char *argv[] = { PROGRAM_NAME, nullptr };

   if (reserved->initFlag)
   {
      std::cout << "ERROR: engine already initialized" << std::endl;
      return false;
   }

   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
   glutInitWindowSize(width, height);
   glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);
   reserved->windowId = glutCreateWindow(title.c_str());
   reserved->title = title;

   glutDisplayFunc(Reserved::displayCallback);
   glutKeyboardFunc(Reserved::keyDownCallback);
   glutKeyboardUpFunc(Reserved::keyUpCallback);
   glutPassiveMotionFunc(Reserved::mouseMoveCallback);
   glutMotionFunc(Reserved::mouseMoveCallback);
   glutCloseFunc(Reserved::windowCloseCallback);
   glutIgnoreKeyRepeat(1);
   glutSetCursor(GLUT_CURSOR_NONE);

#ifdef _DEBUG
   std::cout << "   OpenGL version: " << reinterpret_cast<const char *>(glGetString(GL_VERSION)) << std::endl;
#endif

   glEnable(GL_DEPTH_TEST);
   glEnable(GL_CULL_FACE);
   glCullFace(GL_BACK);
   glFrontFace(GL_CCW);
   glEnable(GL_LIGHTING);
   glEnable(GL_NORMALIZE);
   glShadeModel(GL_SMOOTH);

   glutMainLoopEvent();
   reserved->lastTime = glutGet(GLUT_ELAPSED_TIME);
   reserved->fpsTime = reserved->lastTime;
   reserved->running = true;

   std::cout << "[>] " << LIB_NAME << " initialized" << std::endl;
   reserved->initFlag = true;
   return true;
}

/**
 * Closes the window and releases FreeGLUT.
 * @return TF
 */
bool ENG_API Eng::Base::free()
{
   if (!reserved->initFlag)
   {
      std::cout << "ERROR: engine not initialized" << std::endl;
      return false;
   }

   glutDestroyWindow(reserved->windowId);
   glutExit();

   std::cout << "[<] " << LIB_NAME << " deinitialized" << std::endl;
   reserved->running = false;
   reserved->initFlag = false;
   return true;
}

/**
 * Tells if the application should keep running (no ESC pressed, window still open).
 * @return TF
 */
bool ENG_API Eng::Base::isRunning() const
{
   return reserved->running;
}

/**
 * Sets the color used by clearWindow().
 * @param color RGB color
 */
void ENG_API Eng::Base::setBackgroundColor(const glm::vec3 &color)
{
   reserved->backgroundColor = color;
}

/**
 * Sets the function called at every key press (nullptr for none).
 * @param callback application function
 */
void ENG_API Eng::Base::setKeyboardCallback(KeyboardCallback callback)
{
   reserved->keyboardCallback = callback;
}

/**
 * Chooses how the scene is drawn: polygon edges only (wireframe, for debugging) or filled (solid).
 * @param wireframe true for wireframe, false for solid
 */
void ENG_API Eng::Base::setWireframe(bool wireframe)
{
   reserved->wireframe = wireframe;
}

/**
 * Tells if the scene is drawn in wireframe.
 * @return TF
 */
bool ENG_API Eng::Base::isWireframe() const
{
   return reserved->wireframe;
}

/**
 * Clears the color, depth and stencil buffers.
 */
void ENG_API Eng::Base::clearWindow()
{
   glClearColor(reserved->backgroundColor.r, reserved->backgroundColor.g, reserved->backgroundColor.b, 1.0f);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

/**
 * Renders a render list from a camera, then the planar shadow cast by a light.
 * @param camera active camera
 * @param list lights and meshes to draw
 * @param shadowLight light that casts the shadow (nullptr for no shadow)
 */
void ENG_API Eng::Base::render(Camera *camera, List *list, Light *shadowLight)
{
   const int width = glutGet(GLUT_WINDOW_WIDTH);
   const int height = glutGet(GLUT_WINDOW_HEIGHT);

   if (height == 0)
      return;

   glViewport(0, 0, width, height);
   glMatrixMode(GL_PROJECTION);
   glLoadMatrixf(glm::value_ptr(camera->getProjectionMatrix(static_cast<float>(width) / static_cast<float>(height))));
   glMatrixMode(GL_MODELVIEW);

   if (reserved->wireframe)
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   else
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

   const glm::mat4 view = camera->getViewMatrix();
   list->render(view);
   list->renderShadow(view, shadowLight);
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
   const int width = glutGet(GLUT_WINDOW_WIDTH);
   const int height = glutGet(GLUT_WINDOW_HEIGHT);
   float widestKey = 0.0f;
   float widestAction = 0.0f;

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
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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

/**
 * Shows the frame (glutSwapBuffers), measures the time, writes the FPS in the window title
 * and processes the window events (glutMainLoopEvent), which call the input callbacks.
 */
void ENG_API Eng::Base::swapBuffers()
{
   const int now = glutGet(GLUT_ELAPSED_TIME);

   glutSwapBuffers();

   reserved->deltaTime = static_cast<float>(now - reserved->lastTime) / 1000.0f;
   reserved->lastTime = now;

   reserved->fpsFrames++;
   if (now - reserved->fpsTime >= 1000)
   {
      const std::string title = reserved->title + " - " + std::to_string(reserved->fpsFrames) + " FPS";
      glutSetWindowTitle(title.c_str());
      reserved->fpsTime = now;
      reserved->fpsFrames = 0;
   }

   glutMainLoopEvent();
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
bool ENG_API Eng::Base::isKeyDown(unsigned char key) const
{
   return reserved->keyDown[key];
}

/**
 * Gets how much the mouse moved since the previous call.
 * @return movement in pixels (x right, y down)
 */
glm::vec2 ENG_API Eng::Base::getMouseDelta()
{
   const glm::vec2 delta = reserved->mouseDelta;
   reserved->mouseDelta = glm::vec2(0.0f);
   return delta;
}
