/**
 * @file		main.cpp
 * @brief	Client application (that uses the graphics engine) — OpenGL 1.1 fixed-function pipeline
 *
 * @author	Alessio Gervasini
 */

   #include "engine.h"

   #include <iostream>
   #include <cmath>
   #include <string>
   #include <algorithm>

   #include <GLFW/glfw3.h>

namespace
{
   /**
    * 5x7 bitmap font (uppercase + digits + '/' only -- everything the controls legend needs),
    * hand-authored: OpenGL 1.1 has no text rendering of its own (glut bitmap fonts would be a
    * new dependency, and this project has none beyond glfw3/assimp), so the legend is drawn
    * the same way any 1.x-era HUD was -- a handful of GL_QUADS per character. 5x7 (rather than
    * the original cramped 3x5) so lookalike letters such as M/N/V stay visually distinct.
    */
   struct Glyph5x7 { char c; const char *rows[7]; };
   const Glyph5x7 FONT_5X7[] = {
      {' ', {".....", ".....", ".....", ".....", ".....", ".....", "....."}},
      {'A', {".###.", "#...#", "#...#", "#####", "#...#", "#...#", "#...#"}},
      {'C', {".####", "#....", "#....", "#....", "#....", "#....", ".####"}},
      {'D', {"####.", "#...#", "#...#", "#...#", "#...#", "#...#", "####."}},
      {'E', {"#####", "#....", "#....", "####.", "#....", "#....", "#####"}},
      {'G', {".####", "#....", "#....", "#.###", "#...#", "#...#", ".####"}},
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
      {'Y', {"#...#", "#...#", ".#.#.", "..#..", "..#..", "..#..", "..#.."}},
      {'1', {"..#..", ".##..", "..#..", "..#..", "..#..", "..#..", ".###."}},
      {'2', {".###.", "#...#", "....#", "...#.", "..#..", ".#...", "#####"}},
      {'/', {"....#", "....#", "...#.", "..#..", ".#...", "#....", "#...."}},
   };

   const Glyph5x7 *findGlyph(char c)
   {
      for (const Glyph5x7 &g : FONT_5X7)
         if (g.c == c)
            return &g;
      return nullptr;   // unsupported char -- drawn as blank, never crashes on a stray character
   }

   /**
    * Emits GL_QUADS vertices (must be called between glBegin(GL_QUADS)/glEnd()) for one line of
    * uppercase text, top-left corner at (x,y) in whatever 2D space the caller set up.
    */
   void emitText(const char *text, float x, float y, float pixelSize)
   {
      float penX = x;
      for (const char *p = text; *p; p++)
      {
         const Glyph5x7 *g = findGlyph(static_cast<char>(std::toupper(static_cast<unsigned char>(*p))));
         if (g != nullptr)
         {
            for (int row = 0; row < 7; row++)
               for (int col = 0; col < 5; col++)
                  if (g->rows[row][col] == '#')
                  {
                     const float px = penX + col * pixelSize;
                     const float py = y + row * pixelSize;
                     glVertex2f(px, py);
                     glVertex2f(px + pixelSize, py);
                     glVertex2f(px + pixelSize, py + pixelSize);
                     glVertex2f(px, py + pixelSize);
                  }
         }
         penX += 6.0f * pixelSize;   // 5-wide glyph + 1 pixel of spacing
      }
   }
}

/**
 * Application entry point.
 * @param argc number of command-line arguments passed
 * @param argv array containing up to argc passed arguments
 * @return error code (0 on success, error code otherwise)
 */
int main(int argc, char *argv[])
{
   std::cout << "Client application, Alessio Gervasini (C) SUPSI" << std::endl;
   std::cout << std::endl;

   Eng::Base &eng = Eng::Base::getInstance();
   if (!eng.init())
      return 1;

   // Everything below owns a GPU texture id and must be destroyed while the GL context is
   // still alive -- hence this nested scope, which closes (running every destructor) before
   // eng.free() tears the context down.
   {
   Eng::Loader modelLoader;
   if (!modelLoader.load("assets/bedroom_furniture_set/bedroom_furniture_set_-_game_ready.glb"))
      return 1;

   unsigned int totalTriangles = 0;
   for (unsigned int i = 0; i < modelLoader.getMeshCount(); i++)
      totalTriangles += modelLoader.getMesh(i)->getTriangleCount();
   std::cout << "[i] Scene triangle count: " << totalTriangles << " (budget: 100000)" << std::endl;

   std::cout << "[i] Loaded " << modelLoader.getMeshCount() << " mesh(es):" << std::endl;
   for (unsigned int i = 0; i < modelLoader.getMeshCount(); i++)
   {
      const float *world = modelLoader.getMeshNode(i)->getWorldMatrix();
      std::cout << "    - \"" << modelLoader.getMeshNode(i)->getName() << "\": "
                << modelLoader.getMesh(i)->getTriangleCount() << " triangles, position ("
                << world[12] << ", " << world[13] << ", " << world[14] << ")" << std::endl;
   }

   // The lowest flat mesh in the scene doubles as the reflective surface: found purely from
   // geometry (see Eng::Loader::getGroundMeshIndex), not by looking for any particular name.
   const int groundMeshIndex = modelLoader.getGroundMeshIndex();
   const float groundPlaneY = modelLoader.getGroundPlaneY();

   // The scene's own furniture bounding box (getFurnitureTopCenter -- everything except the
   // ground mesh), used below for the orbiting point light and the camera/scene scale. Never
   // by looking for a specific object's name. The lamp itself is real geometry baked into the
   // .glb file (standing on the table), not placed by code -- drawScene() renders it like any
   // other mesh, and its near-white shade material is auto-detected as self-lit below.
   float furnitureX = 0.0f, furnitureTopY = groundPlaneY, furnitureZ = 0.0f;
   float furnitureHalfWidthX = 0.0f, furnitureHalfDepthZ = 0.0f;
   const bool hasFurniture = modelLoader.getFurnitureTopCenter(furnitureX, furnitureTopY, furnitureZ, furnitureHalfWidthX, furnitureHalfDepthZ);

   // The point light sits at the center of whichever mesh getMeshEmission() flagged as self-lit
   // (the lamp's shade, found by material alone -- see Loader::getMeshEmission), not at any
   // name-checked object. Falls back to the furniture's own top-center if the scene has no
   // emissive mesh at all, so the light still ends up somewhere sensible.
   float lampBulbX = furnitureX, lampBulbY = furnitureTopY, lampBulbZ = furnitureZ;
   for (unsigned int i = 0; i < modelLoader.getMeshCount(); i++)
   {
      float emissionR, emissionG, emissionB;
      if (!modelLoader.getMeshEmission(i, emissionR, emissionG, emissionB))
         continue;
      float minX, minY, minZ, maxX, maxY, maxZ;
      modelLoader.getMeshBounds(i, minX, minY, minZ, maxX, maxY, maxZ);
      lampBulbX = (minX + maxX) * 0.5f;
      lampBulbY = (minY + maxY) * 0.5f;
      lampBulbZ = (minZ + maxZ) * 0.5f;
      break;
   }

   // Fixed downward angle used ONLY for the planar-shadow projection matrix below -- kept
   // constant regardless of what lampLight's own direction does each frame (see the render
   // loop: lampLight now follows the camera), so the shadow never spins or drops out as the
   // player looks around.
   const float lampDirLen = std::sqrt(2.0f * 2.0f + 2.0f * 2.0f + 1.0f * 1.0f);
   const float lampDirX = -2.0f / lampDirLen, lampDirY = -2.0f / lampDirLen, lampDirZ = -1.0f / lampDirLen;

   // The "static" light of the required pair (Directional, no falloff) is a torch attached to
   // the camera: direction re-set every frame from Camera::getForward() (see the render loop),
   // intensity pulsing over time -- moving + changing brightness is what makes it read as the
   // "dynamic" one to look at, even though its GL type stays Directional (satisfies "2 lights,
   // different types" against pointLight, which is Point).
   Eng::Light lampLight;
   lampLight.setType(Eng::LightType::Directional);
   lampLight.setColor(0.85f, 0.92f, 1.0f);

   const float sceneScale = hasFurniture ? std::max(0.3f, furnitureTopY - groundPlaneY) : 1.0f;

   // Fixed at the lamp's own bulb position (lampBulbX/Y/Z above), not orbiting -- the only
   // light meant to be on right now, with attenuation tight enough that it alone lights the
   // desk and fades to black a couple of scene-units out (no global ambient is set below, so
   // anything the falloff doesn't reach stays black).
   Eng::Light pointLight;
   pointLight.setType(Eng::LightType::Point);
   pointLight.setColor(1.0f, 0.55f, 0.20f);
   pointLight.setIntensity(40.0f);
   pointLight.setPosition(lampBulbX, lampBulbY, lampBulbZ);
   // Falloff scales with the scene's own size (sceneScale), same reasoning as before: fixed
   // numbers tuned for one scene size would either never dim or blow out to white up close.
   // Steep on purpose -- a gentler curve left the far side of the bed still visibly lit.
   pointLight.setAttenuation(1.0f, 6.0f / sceneScale, 25.0f / (sceneScale * sceneScale));

   // Fixed-function pipeline setup: global states set once, outside the render loop.
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_LIGHTING);
   glEnable(GL_NORMALIZE);      // slides: "all lighting models must be normalized" -- let OpenGL do it every frame
   glShadeModel(GL_SMOOTH);
   glEnable(GL_TEXTURE_2D);
   glEnable(GL_CULL_FACE);
   glFrontFace(GL_CCW);

   // No global ambient: the only light in the scene must be the two Eng::Light sources
   // (the "lamp"/static and the orbiting/dynamic one) -- anything neither one reaches
   // stays black, i.e. in shadow, rather than getting a uniform ambient fill.
   const GLfloat globalAmbient[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
   glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

   // Material ambient/diffuse tracks glColor and is kept at white: the mesh's own color
   // already lives in its texture (a real image, or a 1x1 flat-Kd color -- see Loader),
   // so a colored material here would just re-tint it.
   glEnable(GL_COLOR_MATERIAL);
   glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
   glColor3f(1.0f, 1.0f, 1.0f);

   glClearColor(0.08f, 0.09f, 0.11f, 1.0f);

   // Camera distances scaled to the loaded scene's own bounding box, not fixed numbers.
   const float cameraDistance = std::max(2.0f, sceneScale * 4.0f);

   Eng::Camera freeCamera;
   Eng::Camera fixedCamera;
   freeCamera.setPosition(0.0f, cameraDistance * 0.5f, cameraDistance);
   freeCamera.look(0.0f, -24.0f);
   freeCamera.setPerspective(60.0f, 4.0f / 3.0f, 0.05f, 300.0f);
   fixedCamera.setPosition(cameraDistance * 0.8f, cameraDistance * 0.55f, cameraDistance * 0.8f);
   fixedCamera.look(-45.0f, -26.5f);
   fixedCamera.setPerspective(60.0f, 4.0f / 3.0f, 0.05f, 300.0f);

   Eng::Camera *activeCamera = &freeCamera;
   bool cKeyWasDown = false;

   // '1' toggles the static light (lampLight, directional), '2' the dynamic one (pointLight,
   // fixed at the lamp's own bulb) -- independent on/off switches, both start on.
   bool staticLightOn = true, dynamicLightOn = true;
   bool key1WasDown = false, key2WasDown = false;

   const float moveSpeed = cameraDistance * 0.7f;
   const float mouseSensitivity = 0.1f;

   float fpsTimer = 0.0f;
   int fpsFrameCount = 0;

   // Draws every mesh in the loaded scene at its node's world position/orientation. A mesh
   // whose material is close to white (getMeshEmission) is drawn self-lit -- a lamp bulb's
   // material is authored that way for exactly this reason, so this needs no per-object name.
   auto drawScene = [&]()
   {
      for (unsigned int i = 0; i < modelLoader.getMeshCount(); i++)
      {
         glPushMatrix();
         glMultMatrixf(modelLoader.getMeshNode(i)->getWorldMatrix());

         float emissionR, emissionG, emissionB;
         // Gated on dynamicLightOn too: the emissive shade is the lamp's own bulb glowing --
         // turning the lamp's light off ('2') should turn this off as well, not leave it lit
         // as a second, untoggleable light source.
         const bool isEmissive = dynamicLightOn && modelLoader.getMeshEmission(i, emissionR, emissionG, emissionB);
         if (isEmissive)
         {
            const GLfloat glow[4] = { emissionR, emissionG, emissionB, 1.0f };
            glMaterialfv(GL_FRONT, GL_EMISSION, glow);
         }

         modelLoader.getMeshTexture(i)->bind();
         modelLoader.getMesh(i)->render();

         if (isEmissive)
         {
            const GLfloat noEmission[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
            glMaterialfv(GL_FRONT, GL_EMISSION, noEmission);
         }

         glPopMatrix();
      }
   };

   while (eng.isRunning())
   {
      int fbWidth, fbHeight;
      eng.getFramebufferSize(fbWidth, fbHeight);
      if (fbHeight > 0)
      {
         activeCamera->setAspect(static_cast<float>(fbWidth) / static_cast<float>(fbHeight));
         glViewport(0, 0, fbWidth, fbHeight);
      }

      bool cKeyIsDown = eng.isKeyPressed('C');
      if (cKeyIsDown && !cKeyWasDown)
         activeCamera = (activeCamera == &freeCamera) ? &fixedCamera : &freeCamera;
      cKeyWasDown = cKeyIsDown;

      bool key1IsDown = eng.isKeyPressed('1');
      if (key1IsDown && !key1WasDown)
         staticLightOn = !staticLightOn;
      key1WasDown = key1IsDown;

      bool key2IsDown = eng.isKeyPressed('2');
      if (key2IsDown && !key2WasDown)
         dynamicLightOn = !dynamicLightOn;
      key2WasDown = key2IsDown;

      if (eng.isKeyPressed('X'))
         eng.requestClose();

      // Always drain the cursor delta, even on the fixed camera, so the engine's internal
      // last-position sample never goes stale -- otherwise switching back to the free camera
      // would apply one huge accumulated jump from however long the mouse moved unread.
      float mouseDeltaX, mouseDeltaY;
      eng.getCursorDelta(mouseDeltaX, mouseDeltaY);

      if (activeCamera == &freeCamera)
      {
         float distance = moveSpeed * eng.getDeltaTime();
         if (eng.isKeyPressed('W')) freeCamera.moveForward(distance);
         if (eng.isKeyPressed('S')) freeCamera.moveForward(-distance);
         if (eng.isKeyPressed('D')) freeCamera.moveRight(distance);
         if (eng.isKeyPressed('A')) freeCamera.moveRight(-distance);
         if (eng.isKeyPressed('E')) freeCamera.moveUp(distance);
         if (eng.isKeyPressed('Q')) freeCamera.moveUp(-distance);
         if (eng.isKeyPressed(GLFW_KEY_UP)) freeCamera.moveUp(distance);
         if (eng.isKeyPressed(GLFW_KEY_DOWN)) freeCamera.moveUp(-distance);

         freeCamera.look(mouseDeltaX * mouseSensitivity, -mouseDeltaY * mouseSensitivity);
      }

      float dt = eng.getDeltaTime();

      fpsTimer += dt;
      fpsFrameCount++;
      if (fpsTimer >= 0.5f)
      {
         float fps = static_cast<float>(fpsFrameCount) / fpsTimer;
         eng.setWindowTitle("My Graphics Engine v0.2 GL1.1 - " +
                             std::to_string(static_cast<int>(fps)) + " FPS");
         fpsTimer = 0.0f;
         fpsFrameCount = 0;
      }

      // Torch light follows wherever the active camera is looking -- "dynamic" through motion
      // alone. Intensity kept constant (no pulse): a pulsing brightness made both the camera
      // tracking harder to notice (confounded with the pulse) and toggling it off with '1'
      // look like a fade whenever the pulse happened to be mid-dip at that moment.
      float torchForwardX, torchForwardY, torchForwardZ;
      activeCamera->getForward(torchForwardX, torchForwardY, torchForwardZ);
      lampLight.setDirection(torchForwardX, torchForwardY, torchForwardZ);
      lampLight.setIntensity(1.2f);

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

      // Camera matrices, loaded straight from GLM's column-major storage -- exactly the format
      // glLoadMatrixf expects (Superbible Ch.7 "Advanced Matrix Manipulation", and the GLM
      // slides' own column-major warning).
      glMatrixMode(GL_PROJECTION);
      glLoadMatrixf(activeCamera->getProjectionMatrix());
      glMatrixMode(GL_MODELVIEW);
      glLoadMatrixf(activeCamera->getViewMatrix());

      // Lights are positioned/enabled AFTER the view matrix is loaded, so OpenGL bakes the
      // current camera transform into their eye-space position -- the same order used by the
      // Superbible's Sun/Earth/Moon example (Listing 7-3: glLightfv(..GL_POSITION..) right
      // after glTranslatef for the viewing transform).
      // Both lights on by default -- lampLight (static/directional) + pointLight (dynamic/
      // point, fixed at the lamp's own lit bulb) -- each toggled independently by '1'/'2'.
      // apply() only enables its GL_LIGHTn; explicitly disable it when toggled off, since the
      // enabled state would otherwise persist from whatever it was last frame.
      if (staticLightOn)
         lampLight.apply(0);
      else
         glDisable(GL_LIGHT0);

      if (dynamicLightOn)
         pointLight.apply(1);
      else
         glDisable(GL_LIGHT1);

      // ---- pass 1: the scene, drawn normally ----
      drawScene();

      // ---- pass 2: a flat dark silhouette of the whole scene, projected onto the ground along
      // the static light's own direction -- the classic OpenGL 1.x planar-shadow technique
      // (Blinn 1988, in every fixed-function-era graphics text): for a plane Y=groundPlaneY and
      // light direction L, a point P casts its shadow at P - ((P.y-groundPlaneY)/L.y) * L. That
      // is an affine map, so it can be loaded as one 4x4 matrix and left for the GL 1.1 matrix
      // stack to apply to every vertex -- no per-vertex CPU work needed. Clipped to the ground
      // mesh's own footprint (found purely from geometry, see getGroundMeshIndex()) via the
      // stencil buffer, the same way a planar reflection would be. Satisfies the project's
      // "casts shadows OR produces reflections on a planar surface" requirement; shadow
      // *mapping* would need a framebuffer object, which doesn't exist in GL 1.1. ----
      if (groundMeshIndex >= 0 && lampDirY < -0.001f)
      {
         glEnable(GL_STENCIL_TEST);

         // 1) Stencil-only pass: mark every pixel covered by the ground mesh, without touching
         // the color or depth buffers.
         glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
         glDepthMask(GL_FALSE);
         glStencilFunc(GL_ALWAYS, 1, 0xFF);
         glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
         glPushMatrix();
         glMultMatrixf(modelLoader.getMeshNode(groundMeshIndex)->getWorldMatrix());
         modelLoader.getMesh(groundMeshIndex)->render();
         glPopMatrix();
         glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
         glDepthMask(GL_TRUE);

         // 2) Draw the whole scene flattened onto the ground by the shadow matrix, as a plain
         // dark blend (lighting/texturing off -- a shadow has no material of its own), only
         // where the stencil test above passed -- i.e. only inside the ground's own footprint.
         glStencilFunc(GL_EQUAL, 1, 0xFF);
         glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

         glDisable(GL_LIGHTING);
         glDisable(GL_TEXTURE_2D);
         glEnable(GL_BLEND);
         glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
         glColor4f(0.0f, 0.0f, 0.0f, 0.45f);
         glDepthMask(GL_FALSE);   // shadow triangles overlap a lot (every mesh onto one flat plane); no need for them to depth-test each other

         const GLfloat shadowMatrix[16] = {
            1.0f,                 0.0f, 0.0f,                 0.0f,
            -lampDirX / lampDirY, 0.0f, -lampDirZ / lampDirY, 0.0f,
            0.0f,                 0.0f, 1.0f,                 0.0f,
            0.0f,                 0.0f, 0.0f,                 1.0f
         };
         glPushMatrix();
         glTranslatef(0.0f, groundPlaneY, 0.0f);
         glMultMatrixf(shadowMatrix);
         glTranslatef(0.0f, -groundPlaneY, 0.0f);
         drawScene();
         glPopMatrix();

         glDepthMask(GL_TRUE);
         glColor3f(1.0f, 1.0f, 1.0f);
         glDisable(GL_BLEND);
         glEnable(GL_LIGHTING);
         glEnable(GL_TEXTURE_2D);
         glDisable(GL_STENCIL_TEST);
      }

      // ---- controls legend, top-right corner, on top of everything else ----
      {
         glMatrixMode(GL_PROJECTION);
         glPushMatrix();
         glLoadIdentity();
         glOrtho(0.0, fbWidth, fbHeight, 0.0, -1.0, 1.0);   // origin top-left, Y down: screen space
         glMatrixMode(GL_MODELVIEW);
         glPushMatrix();
         glLoadIdentity();

         glDisable(GL_LIGHTING);
         glDisable(GL_TEXTURE_2D);
         glDisable(GL_DEPTH_TEST);
         glDisable(GL_CULL_FACE);   // the ortho Y-flip above reverses quad winding -- would
                                    // otherwise get silently backface-culled, invisible

         // Arrow keys not listed: same effect as Q/E (already on the line above), and
         // self-explanatory -- redundant to spell out.
         struct LegendEntry { const char *key; const char *action; };
         static const LegendEntry legend[] = {
            {"WASD",   "MOVE"},
            {"Q/E",    "UP DOWN"},
            {"MOUSE",  "LOOK"},
            {"C",      "CAMERA"},
            {"1",      "TORCH"},
            {"2",      "LAMP"},
            {"X",      "EXIT"},
         };
         const int lineCount = static_cast<int>(sizeof(legend) / sizeof(legend[0]));

         const float pixelSize      = 4.0f;
         const float lineHeight     = 9.0f * pixelSize;        // 7-row glyph + 2 rows of spacing
         const float margin         = 12.0f;
         const float pad            = 3.0f * pixelSize;
         const float titleScale     = 1.5f;
         const float gapAfterTitle  = 2.0f * pixelSize;
         const float gapAfterDivider = 2.0f * pixelSize;

         auto textWidth = [pixelSize](const char *text, float scale)
         {
            return static_cast<float>(std::string(text).size()) * 6.0f * pixelSize * scale;
         };

         float widestAction = 0.0f, widestKey = 0.0f;
         for (const LegendEntry &e : legend)
         {
            widestAction = std::max(widestAction, textWidth(e.action, 1.0f));
            widestKey = std::max(widestKey, textWidth(e.key, 1.0f));
         }
         const float keyColW = widestKey + 6.0f * pixelSize;   // +1 glyph of slack before the action column

         const float titleHeight   = 7.0f * pixelSize * titleScale;
         const float blockWidth    = std::max(keyColW + widestAction, textWidth("CONTROLS", titleScale));
         const float contentHeight = titleHeight + gapAfterTitle + gapAfterDivider
                                    + static_cast<float>(lineCount) * lineHeight;

         const float panelX1 = static_cast<float>(fbWidth) - margin;
         const float panelX0 = panelX1 - blockWidth - 2.0f * pad;
         const float panelY0 = margin;
         const float panelY1 = panelY0 + 2.0f * pad + contentHeight;
         const float textX   = panelX0 + pad;

         // panel: translucent dark backing + faint warm border, so the legend reads clearly
         // against any part of the scene behind it instead of floating gray text on nothing
         glEnable(GL_BLEND);
         glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

         glColor4f(0.05f, 0.05f, 0.07f, 0.55f);
         glBegin(GL_QUADS);
            glVertex2f(panelX0, panelY0);
            glVertex2f(panelX1, panelY0);
            glVertex2f(panelX1, panelY1);
            glVertex2f(panelX0, panelY1);
         glEnd();

         glLineWidth(1.5f);
         glColor4f(1.0f, 0.55f, 0.20f, 0.35f);   // same warm hue as the dynamic point light -- ties the HUD to the scene
         glBegin(GL_LINE_LOOP);
            glVertex2f(panelX0, panelY0);
            glVertex2f(panelX1, panelY0);
            glVertex2f(panelX1, panelY1);
            glVertex2f(panelX0, panelY1);
         glEnd();

         float penY = panelY0 + pad;

         // title, drawn twice with a 1px horizontal offset to fake a bold weight -- OpenGL 1.1
         // has no font weights, this is the cheapest way to make it read as a header at this size
         glColor3f(1.0f, 1.0f, 1.0f);
         glBegin(GL_QUADS);
            emitText("CONTROLS", textX, penY, pixelSize * titleScale);
            emitText("CONTROLS", textX + 1.0f, penY, pixelSize * titleScale);
         glEnd();
         penY += titleHeight + gapAfterTitle;

         glColor4f(1.0f, 0.55f, 0.20f, 0.6f);
         glBegin(GL_LINES);
            glVertex2f(textX, penY);
            glVertex2f(textX + blockWidth, penY);
         glEnd();
         glLineWidth(1.0f);
         penY += gapAfterDivider;

         for (const LegendEntry &e : legend)
         {
            glColor3f(1.0f, 0.72f, 0.40f);      // key: warm accent, scannable at a glance
            glBegin(GL_QUADS);
               emitText(e.key, textX, penY, pixelSize);
            glEnd();

            glColor3f(0.80f, 0.80f, 0.82f);     // action: neutral, secondary
            glBegin(GL_QUADS);
               emitText(e.action, textX + keyColW, penY, pixelSize);
            glEnd();

            penY += lineHeight;
         }

         glColor3f(1.0f, 1.0f, 1.0f);
         glDisable(GL_BLEND);
         glEnable(GL_DEPTH_TEST);
         glEnable(GL_TEXTURE_2D);
         glEnable(GL_LIGHTING);
         glEnable(GL_CULL_FACE);

         glMatrixMode(GL_PROJECTION);
         glPopMatrix();
         glMatrixMode(GL_MODELVIEW);
         glPopMatrix();
      }

      eng.swapBuffers();
   }

   } // end GPU-resource scope -- all textures above are destroyed here, context still alive

   eng.free();

   std::cout << "\n[application terminated]" << std::endl;
   return 0;
}
