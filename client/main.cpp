/**
 * @file		main.cpp
 * @brief	Client application that uses the graphics engine
 *
 * @author	Alessio Gervasini
 */

   #include "engine.h"

   #include <algorithm>
   #include <cctype>
   #include <iostream>
   #include <iterator>
   #include <string>

   #include <GLFW/glfw3.h>

namespace
{
   const char *MODEL_PATH = "assets/bedroom_furniture_set/bedroom_furniture_set_-_game_ready.glb";
   const unsigned int TRIANGLE_BUDGET = 100000;

   const float FIELD_OF_VIEW_DEGREES = 60.0f;
   const float NEAR_PLANE = 0.05f;
   const float FAR_PLANE = 300.0f;
   const float MOUSE_SENSITIVITY = 0.1f;
   const float FPS_REFRESH_SECONDS = 0.5f;

   const float SHADOW_DIRECTION_X = -2.0f;
   const float SHADOW_DIRECTION_Y = -2.0f;
   const float SHADOW_DIRECTION_Z = -1.0f;
   const float SHADOW_OPACITY = 0.45f;

   const int GLYPH_COLUMNS = 5;
   const int GLYPH_ROWS = 7;
   const int GLYPH_ADVANCE = GLYPH_COLUMNS + 1;

   struct Glyph
   {
      char character;
      const char *rows[GLYPH_ROWS];
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

   struct LegendEntry
   {
      const char *key;
      const char *action;
   };

   const LegendEntry LEGEND[] = {
      {"WASD",  "MOVE"},
      {"Q/E",   "UP DOWN"},
      {"MOUSE", "LOOK"},
      {"C",     "CAMERA"},
      {"1",     "TORCH"},
      {"2",     "LAMP"},
      {"X",     "EXIT"},
   };

   const Glyph *findGlyph(char character)
   {
      const char upperCase = static_cast<char>(std::toupper(static_cast<unsigned char>(character)));
      for (const Glyph &glyph : FONT)
         if (glyph.character == upperCase)
            return &glyph;
      return nullptr;
   }

   float textWidth(const char *text, float pixelSize)
   {
      return static_cast<float>(std::string(text).size() * GLYPH_ADVANCE) * pixelSize;
   }

   void drawText(const char *text, float x, float y, float pixelSize)
   {
      float penX = x;

      glBegin(GL_QUADS);
      for (const char *character = text; *character != '\0'; character++)
      {
         const Glyph *glyph = findGlyph(*character);
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

   void drawRectangle(GLenum mode, float left, float top, float right, float bottom)
   {
      glBegin(mode);
      glVertex2f(left, top);
      glVertex2f(right, top);
      glVertex2f(right, bottom);
      glVertex2f(left, bottom);
      glEnd();
   }

   void drawControlsPanel(int framebufferWidth, int framebufferHeight)
   {
      const float pixelSize = 4.0f;
      const float titleSize = pixelSize * 1.5f;
      const float lineHeight = (GLYPH_ROWS + 2) * pixelSize;
      const float titleHeight = GLYPH_ROWS * titleSize;
      const float sectionGap = 2.0f * pixelSize;
      const float margin = 12.0f;
      const float padding = 3.0f * pixelSize;
      const int lineCount = static_cast<int>(std::size(LEGEND));

      float widestKey = 0.0f;
      float widestAction = 0.0f;
      for (const LegendEntry &entry : LEGEND)
      {
         widestKey = std::max(widestKey, textWidth(entry.key, pixelSize));
         widestAction = std::max(widestAction, textWidth(entry.action, pixelSize));
      }

      const float keyColumnWidth = widestKey + GLYPH_ADVANCE * pixelSize;
      const float contentWidth = std::max(keyColumnWidth + widestAction, textWidth("CONTROLS", titleSize));
      const float contentHeight = titleHeight + 2.0f * sectionGap + lineCount * lineHeight;
      const float panelRight = static_cast<float>(framebufferWidth) - margin;
      const float panelLeft = panelRight - contentWidth - 2.0f * padding;
      const float panelTop = margin;
      const float panelBottom = panelTop + contentHeight + 2.0f * padding;
      const float textLeft = panelLeft + padding;
      float penY = panelTop + padding;

      glMatrixMode(GL_PROJECTION);
      glPushMatrix();
      glLoadIdentity();
      glOrtho(0.0, framebufferWidth, framebufferHeight, 0.0, -1.0, 1.0);
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
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

      for (const LegendEntry &entry : LEGEND)
      {
         glColor3f(1.0f, 0.72f, 0.40f);
         drawText(entry.key, textLeft, penY, pixelSize);
         glColor3f(0.80f, 0.80f, 0.82f);
         drawText(entry.action, textLeft + keyColumnWidth, penY, pixelSize);
         penY += lineHeight;
      }

      glColor3f(1.0f, 1.0f, 1.0f);
      glDisable(GL_BLEND);
      glEnable(GL_CULL_FACE);
      glEnable(GL_DEPTH_TEST);
      glEnable(GL_TEXTURE_2D);
      glEnable(GL_LIGHTING);

      glMatrixMode(GL_PROJECTION);
      glPopMatrix();
      glMatrixMode(GL_MODELVIEW);
      glPopMatrix();
   }

   void drawMesh(const Eng::Loader &loader, unsigned int index)
   {
      glPushMatrix();
      glMultMatrixf(loader.getMeshNode(index)->getWorldMatrix());
      loader.getMeshTexture(index)->bind();
      loader.getMesh(index)->render();
      glPopMatrix();
   }

   void drawScene(const Eng::Loader &loader, bool lampOn)
   {
      const GLfloat noEmission[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

      for (unsigned int i = 0; i < loader.getMeshCount(); i++)
      {
         float emissionR, emissionG, emissionB;
         const bool isGlowing = lampOn && loader.getMeshEmission(i, emissionR, emissionG, emissionB);
         const GLfloat emission[4] = { isGlowing ? emissionR : 0.0f, isGlowing ? emissionG : 0.0f, isGlowing ? emissionB : 0.0f, 1.0f };

         glMaterialfv(GL_FRONT, GL_EMISSION, emission);
         drawMesh(loader, i);
      }
      glMaterialfv(GL_FRONT, GL_EMISSION, noEmission);
   }

   void drawPlanarShadow(const Eng::Loader &loader)
   {
      const int groundMeshIndex = loader.getGroundMeshIndex();
      const float groundPlaneY = loader.getGroundPlaneY();
      const GLfloat shadowMatrix[16] = {
         1.0f,                                     0.0f, 0.0f,                                     0.0f,
         -SHADOW_DIRECTION_X / SHADOW_DIRECTION_Y, 0.0f, -SHADOW_DIRECTION_Z / SHADOW_DIRECTION_Y, 0.0f,
         0.0f,                                     0.0f, 1.0f,                                     0.0f,
         0.0f,                                     0.0f, 0.0f,                                     1.0f
      };

      if (groundMeshIndex < 0)
         return;

      glEnable(GL_STENCIL_TEST);

      glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
      glDepthMask(GL_FALSE);
      glStencilFunc(GL_ALWAYS, 1, 0xFF);
      glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
      drawMesh(loader, static_cast<unsigned int>(groundMeshIndex));
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

      glStencilFunc(GL_EQUAL, 1, 0xFF);
      glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
      glDisable(GL_LIGHTING);
      glDisable(GL_TEXTURE_2D);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glColor4f(0.0f, 0.0f, 0.0f, SHADOW_OPACITY);

      glPushMatrix();
      glTranslatef(0.0f, groundPlaneY, 0.0f);
      glMultMatrixf(shadowMatrix);
      glTranslatef(0.0f, -groundPlaneY, 0.0f);
      drawScene(loader, false);
      glPopMatrix();

      glColor3f(1.0f, 1.0f, 1.0f);
      glDepthMask(GL_TRUE);
      glDisable(GL_BLEND);
      glEnable(GL_TEXTURE_2D);
      glEnable(GL_LIGHTING);
      glDisable(GL_STENCIL_TEST);
   }

   void printSceneSummary(const Eng::Loader &loader)
   {
      unsigned int totalTriangles = 0;

      std::cout << "[i] Loaded " << loader.getMeshCount() << " mesh(es):" << std::endl;
      for (unsigned int i = 0; i < loader.getMeshCount(); i++)
      {
         const Eng::Node *node = loader.getMeshNode(i);
         const float *worldMatrix = node->getWorldMatrix();
         const unsigned int triangleCount = loader.getMesh(i)->getTriangleCount();

         totalTriangles += triangleCount;
         std::cout << "    - \"" << node->getName() << "\": " << triangleCount << " triangles, position ("
                   << worldMatrix[12] << ", " << worldMatrix[13] << ", " << worldMatrix[14] << ")" << std::endl;
      }
      std::cout << "[i] Scene triangle count: " << totalTriangles << " (budget: " << TRIANGLE_BUDGET << ")" << std::endl;
   }

   void findLampPosition(const Eng::Loader &loader, float &x, float &y, float &z)
   {
      for (unsigned int i = 0; i < loader.getMeshCount(); i++)
      {
         float emissionR, emissionG, emissionB;
         float minX, minY, minZ, maxX, maxY, maxZ;

         if (!loader.getMeshEmission(i, emissionR, emissionG, emissionB))
            continue;

         loader.getMeshBounds(i, minX, minY, minZ, maxX, maxY, maxZ);
         x = (minX + maxX) * 0.5f;
         y = (minY + maxY) * 0.5f;
         z = (minZ + maxZ) * 0.5f;
         return;
      }
   }

   bool wasJustPressed(const Eng::Base &engine, int key, bool &wasDown)
   {
      const bool isDown = engine.isKeyPressed(key);
      const bool justPressed = isDown && !wasDown;
      wasDown = isDown;
      return justPressed;
   }

   void setupFixedFunctionState()
   {
      const GLfloat noGlobalAmbient[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

      glEnable(GL_LIGHTING);
      glEnable(GL_NORMALIZE);
      glShadeModel(GL_SMOOTH);
      glEnable(GL_TEXTURE_2D);
      glEnable(GL_CULL_FACE);
      glFrontFace(GL_CCW);
      glLightModelfv(GL_LIGHT_MODEL_AMBIENT, noGlobalAmbient);
      glEnable(GL_COLOR_MATERIAL);
      glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
      glColor3f(1.0f, 1.0f, 1.0f);
      glClearColor(0.08f, 0.09f, 0.11f, 1.0f);
   }

   bool runViewer(Eng::Base &engine)
   {
      Eng::Loader loader;
      if (!loader.load(MODEL_PATH))
         return false;

      printSceneSummary(loader);

      const float groundPlaneY = loader.getGroundPlaneY();
      float furnitureX = 0.0f, furnitureTopY = groundPlaneY, furnitureZ = 0.0f;
      const bool hasFurniture = loader.getFurnitureTopCenter(furnitureX, furnitureTopY, furnitureZ);
      const float sceneScale = hasFurniture ? std::max(0.3f, furnitureTopY - groundPlaneY) : 1.0f;
      const float cameraDistance = std::max(2.0f, sceneScale * 4.0f);
      const float moveSpeed = cameraDistance * 0.7f;
      float lampX = furnitureX, lampY = furnitureTopY, lampZ = furnitureZ;
      findLampPosition(loader, lampX, lampY, lampZ);

      Eng::Light torchLight;
      Eng::Light lampLight;
      Eng::Camera freeCamera;
      Eng::Camera fixedCamera;
      Eng::Camera *activeCamera = &freeCamera;
      bool torchOn = true, lampOn = true;
      bool cameraKeyWasDown = false, torchKeyWasDown = false, lampKeyWasDown = false;
      float fpsTimer = 0.0f;
      int fpsFrameCount = 0;

      torchLight.setType(Eng::LightType::Directional);
      torchLight.setColor(0.85f, 0.92f, 1.0f);
      torchLight.setIntensity(1.2f);

      lampLight.setType(Eng::LightType::Point);
      lampLight.setColor(1.0f, 0.55f, 0.20f);
      lampLight.setIntensity(40.0f);
      lampLight.setPosition(lampX, lampY, lampZ);
      lampLight.setAttenuation(1.0f, 6.0f / sceneScale, 25.0f / (sceneScale * sceneScale));

      freeCamera.setPosition(0.0f, cameraDistance * 0.5f, cameraDistance);
      freeCamera.look(0.0f, -24.0f);
      freeCamera.setPerspective(FIELD_OF_VIEW_DEGREES, 4.0f / 3.0f, NEAR_PLANE, FAR_PLANE);
      fixedCamera.setPosition(cameraDistance * 0.8f, cameraDistance * 0.55f, cameraDistance * 0.8f);
      fixedCamera.look(-45.0f, -26.5f);
      fixedCamera.setPerspective(FIELD_OF_VIEW_DEGREES, 4.0f / 3.0f, NEAR_PLANE, FAR_PLANE);

      setupFixedFunctionState();

      while (engine.isRunning())
      {
         const float deltaTime = engine.getDeltaTime();
         int framebufferWidth, framebufferHeight;
         float mouseDeltaX, mouseDeltaY;
         float forwardX, forwardY, forwardZ;

         engine.getFramebufferSize(framebufferWidth, framebufferHeight);
         if (framebufferHeight > 0)
         {
            activeCamera->setAspect(static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight));
            glViewport(0, 0, framebufferWidth, framebufferHeight);
         }

         if (wasJustPressed(engine, 'C', cameraKeyWasDown))
            activeCamera = activeCamera == &freeCamera ? &fixedCamera : &freeCamera;
         if (wasJustPressed(engine, '1', torchKeyWasDown))
            torchOn = !torchOn;
         if (wasJustPressed(engine, '2', lampKeyWasDown))
            lampOn = !lampOn;
         if (engine.isKeyPressed('X'))
            engine.requestClose();

         engine.getCursorDelta(mouseDeltaX, mouseDeltaY);
         if (activeCamera == &freeCamera)
         {
            const float step = moveSpeed * deltaTime;
            if (engine.isKeyPressed('W')) freeCamera.moveForward(step);
            if (engine.isKeyPressed('S')) freeCamera.moveForward(-step);
            if (engine.isKeyPressed('D')) freeCamera.moveRight(step);
            if (engine.isKeyPressed('A')) freeCamera.moveRight(-step);
            if (engine.isKeyPressed('E')) freeCamera.moveUp(step);
            if (engine.isKeyPressed('Q')) freeCamera.moveUp(-step);
            freeCamera.look(mouseDeltaX * MOUSE_SENSITIVITY, -mouseDeltaY * MOUSE_SENSITIVITY);
         }

         fpsTimer += deltaTime;
         fpsFrameCount++;
         if (fpsTimer >= FPS_REFRESH_SECONDS)
         {
            engine.setWindowTitle(std::string(LIB_NAME) + " - " + std::to_string(static_cast<int>(fpsFrameCount / fpsTimer)) + " FPS");
            fpsTimer = 0.0f;
            fpsFrameCount = 0;
         }

         activeCamera->getForward(forwardX, forwardY, forwardZ);
         torchLight.setDirection(forwardX, forwardY, forwardZ);

         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
         glMatrixMode(GL_PROJECTION);
         glLoadMatrixf(activeCamera->getProjectionMatrix());
         glMatrixMode(GL_MODELVIEW);
         glLoadMatrixf(activeCamera->getViewMatrix());

         if (torchOn)
            torchLight.apply(0);
         else
            glDisable(GL_LIGHT0);

         if (lampOn)
            lampLight.apply(1);
         else
            glDisable(GL_LIGHT1);

         drawScene(loader, lampOn);
         drawPlanarShadow(loader);
         drawControlsPanel(framebufferWidth, framebufferHeight);

         engine.swapBuffers();
      }

      return true;
   }
}

int main()
{
   Eng::Base &engine = Eng::Base::getInstance();
   bool viewerSucceeded = false;

   std::cout << "Client application, Alessio Gervasini (C) SUPSI" << std::endl << std::endl;

   if (!engine.init())
      return 1;

   viewerSucceeded = runViewer(engine);
   engine.free();

   std::cout << std::endl << "[application terminated]" << std::endl;
   return viewerSucceeded ? 0 : 1;
}
