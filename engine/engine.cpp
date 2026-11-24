/**
 * @file		engine.cpp
 * @brief	Graphics engine main file
 *
 * @author	Alessio Gervasini (Master) SUPSI
 */



//////////////
// #INCLUDE //
//////////////

   // Main include:
   #include "engine.h"

   // FreeGLUT:
   #include <GL/freeglut.h>

   // Assimp:
   #include <assimp/Importer.hpp>
   #include <assimp/scene.h>
   #include <assimp/postprocess.h>

   // GLM:
   #include <glm/gtc/matrix_transform.hpp>
   #include <glm/gtc/type_ptr.hpp>

   // C/C++:
   #include <algorithm>
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

   // Loaded scenes:
   std::vector<std::unique_ptr<Object>> loadedObjects;   ///< Nodes, meshes, lights, materials and textures created by load(), released by free()

   // Textures:
   static constexpr int checkerboardSize = 256;   ///< Width and height of the procedural checkerboard image, in pixels
   static constexpr int checkerboardSquare = 32;  ///< Side of one square of the checkerboard, in pixels


   /**
    * Constructor.
    */
   Reserved() : initFlag{ false }, running{ false }, wireframe{ false }, width{ 0 }, height{ 1 }
   {}

   /**
    * Keeps an object created by load(), so that free() can release it.
    * @param object new object
    * @return pointer to the object
    */
   template <typename T> T *keep(std::unique_ptr<T> object)
   {
      T *objectPtr = object.get();
      loadedObjects.push_back(std::move(object));
      return objectPtr;
   }

   // Loading:
   static std::vector<unsigned char> createCheckerboard();
   Material *loadMaterial(const aiMaterial *fileMaterial);
   Light *loadLight(const aiLight *fileLight);
   Mesh *loadMesh(const aiMesh *fileMesh, const std::string &name, const std::vector<Material *> &materials);
   Node *loadNode(const aiScene *fileScene, const aiNode *fileNode, const std::vector<Material *> &materials);
};



///////////////////////////////////
// BODY OF STRUCT Base::Reserved //
///////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Creates procedurally, in memory, an RGB image with a white and gray checkerboard, used as the image of the textures
 * until the images of the file can be read.
 * @return image of checkerboardSize x checkerboardSize pixels, 3 bytes per pixel, row by row
 */
std::vector<unsigned char> Eng::Base::Reserved::createCheckerboard()
{
   std::vector<unsigned char> bitmap;
   for (int row = 0; row < checkerboardSize; row++)
      for (int column = 0; column < checkerboardSize; column++)
      {
         unsigned char value = (row / checkerboardSquare + column / checkerboardSquare) % 2 == 0 ? 255 : 96;
         bitmap.insert(bitmap.end(), { value, value, value });
      }
   return bitmap;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Converts a material of the file into an engine material. If the material of the file has a diffuse (base color)
 * texture, a Texture is created instead, loaded once here with the procedural checkerboard image. Diffuse, specular and
 * emission colors and shininess are copied when the file has them (otherwise the default values of Material stay).
 * Ambient: the color of the file if there is one, otherwise 0.2 times the diffuse color (glTF has no ambient color).
 * @param fileMaterial material read by Assimp
 * @return new material (a Texture if the material of the file has a diffuse texture)
 */
Eng::Material *Eng::Base::Reserved::loadMaterial(const aiMaterial *fileMaterial)
{
   std::string name = fileMaterial->GetName().C_Str();
   Material *material = nullptr;
   if (fileMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
   {
      std::vector<unsigned char> bitmap = createCheckerboard();
      material = keep(std::make_unique<Texture>(name, checkerboardSize, checkerboardSize, bitmap.data()));
   }
   else
      material = keep(std::make_unique<Material>(name));

   aiColor4D color;
   glm::vec4 diffuse(1.0f);
   if (fileMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
      diffuse = glm::vec4(color.r, color.g, color.b, color.a);
   material->setDiffuse(diffuse);

   if (fileMaterial->Get(AI_MATKEY_COLOR_AMBIENT, color) == AI_SUCCESS)
      material->setAmbient(glm::vec4(color.r, color.g, color.b, color.a));
   else
      material->setAmbient(glm::vec4(glm::vec3(diffuse) * 0.2f, diffuse.a));

   if (fileMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS)
      material->setSpecular(glm::vec4(color.r, color.g, color.b, color.a));

   if (fileMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, color) == AI_SUCCESS)
      material->setEmission(glm::vec4(color.r, color.g, color.b, color.a));

   float shininess;
   if (fileMaterial->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS)
      material->setShininess(shininess);

   return material;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Converts a light of the file into an engine light. Directional, point and spot lights are supported. The diffuse and
 * specular colors are scaled so that the brightest component is at most 1 (a file can store colors multiplied by an
 * intensity). The ambient color stays black: glTF lights have no ambient term (the scene ambient is the global one of OpenGL).
 * @param fileLight light read by Assimp
 * @return new light, or nullptr if the type of light is not supported
 */
Eng::Light *Eng::Base::Reserved::loadLight(const aiLight *fileLight)
{
   Light::Type type;
   if (fileLight->mType == aiLightSource_DIRECTIONAL)
      type = Light::Type::DIRECTIONAL;
   else if (fileLight->mType == aiLightSource_POINT)
      type = Light::Type::OMNI;
   else if (fileLight->mType == aiLightSource_SPOT)
      type = Light::Type::SPOT;
   else
      return nullptr;

   Light *light = keep(std::make_unique<Light>(fileLight->mName.C_Str(), type));

   glm::vec3 diffuse(fileLight->mColorDiffuse.r, fileLight->mColorDiffuse.g, fileLight->mColorDiffuse.b);
   glm::vec3 specular(fileLight->mColorSpecular.r, fileLight->mColorSpecular.g, fileLight->mColorSpecular.b);
   light->setDiffuse(glm::vec4(diffuse / std::max({ diffuse.r, diffuse.g, diffuse.b, 1.0f }), 1.0f));
   light->setSpecular(glm::vec4(specular / std::max({ specular.r, specular.g, specular.b, 1.0f }), 1.0f));

   if (type == Light::Type::SPOT)
      light->setCutoff(glm::degrees(fileLight->mAngleOuterCone));

   return light;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Converts a mesh of the file into an engine mesh: copies the vertex list (positions, normals and texture coordinates)
 * and the face list. Only faces with 3 indices (triangles) are copied. A mesh without texture coordinates gets (0, 0).
 * @param fileMesh mesh read by Assimp
 * @param name name of the new mesh
 * @param materials engine materials, in the same order as in the file
 * @return new mesh
 */
Eng::Mesh *Eng::Base::Reserved::loadMesh(const aiMesh *fileMesh, const std::string &name, const std::vector<Material *> &materials)
{
   Mesh *mesh = keep(std::make_unique<Mesh>(name));

   for (unsigned int i = 0; i < fileMesh->mNumVertices; i++)
   {
      glm::vec2 texCoord(0.0f);
      if (fileMesh->HasTextureCoords(0))
         texCoord = glm::vec2(fileMesh->mTextureCoords[0][i].x, fileMesh->mTextureCoords[0][i].y);
      mesh->addVertex(glm::vec3(fileMesh->mVertices[i].x, fileMesh->mVertices[i].y, fileMesh->mVertices[i].z),
                      glm::vec3(fileMesh->mNormals[i].x, fileMesh->mNormals[i].y, fileMesh->mNormals[i].z), texCoord);
   }

   for (unsigned int i = 0; i < fileMesh->mNumFaces; i++)
      if (fileMesh->mFaces[i].mNumIndices == 3)
         mesh->addFace(fileMesh->mFaces[i].mIndices[0], fileMesh->mFaces[i].mIndices[1], fileMesh->mFaces[i].mIndices[2]);

   mesh->setMaterial(materials[fileMesh->mMaterialIndex]);
   return mesh;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Converts a node of the file into an engine node and, recursively, its children. The node becomes a Light if a light
 * of the file has its name, a Mesh if it has one mesh, otherwise a Node (with one child Mesh for each of its meshes).
 * The matrix of the node, relative to its parent, is copied: Assimp stores it by rows, GLM by columns, so it is transposed.
 * @param fileScene scene read by Assimp
 * @param fileNode node to convert
 * @param materials engine materials, in the same order as in the file
 * @return new node
 */
Eng::Node *Eng::Base::Reserved::loadNode(const aiScene *fileScene, const aiNode *fileNode, const std::vector<Material *> &materials)
{
   std::string name = fileNode->mName.C_Str();
   glm::mat4 matrix = glm::transpose(glm::make_mat4(&fileNode->mTransformation.a1));
   Node *node = nullptr;

   for (unsigned int i = 0; i < fileScene->mNumLights && node == nullptr; i++)
      if (fileScene->mLights[i]->mName == fileNode->mName)
      {
         const aiLight *fileLight = fileScene->mLights[i];
         glm::vec3 position(fileLight->mPosition.x, fileLight->mPosition.y, fileLight->mPosition.z);
         glm::vec3 direction(fileLight->mDirection.x, fileLight->mDirection.y, fileLight->mDirection.z);
         glm::vec3 up(fileLight->mUp.x, fileLight->mUp.y, fileLight->mUp.z);
         node = loadLight(fileLight);
         if (node != nullptr && glm::length(glm::cross(direction, up)) > 0.0f)
            matrix = matrix * glm::inverse(glm::lookAt(position, position + direction, up));
         else if (node != nullptr)
            matrix = matrix * glm::translate(glm::mat4(1.0f), position);
      }

   if (node == nullptr && fileNode->mNumMeshes == 1)
      node = loadMesh(fileScene->mMeshes[fileNode->mMeshes[0]], name, materials);

   if (node == nullptr)
   {
      node = keep(std::make_unique<Node>(name));
      for (unsigned int i = 0; i < fileNode->mNumMeshes; i++)
      {
         const aiMesh *fileMesh = fileScene->mMeshes[fileNode->mMeshes[i]];
         node->addChild(loadMesh(fileMesh, fileMesh->mName.C_Str(), materials));
      }
   }

   node->setMatrix(matrix);
   for (unsigned int i = 0; i < fileNode->mNumChildren; i++)
      node->addChild(loadNode(fileScene, fileNode->mChildren[i], materials));

   return node;
}



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
   glutIgnoreKeyRepeat(1);

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
 * Free internal components: deletes the objects created by load() (the textures release their images while the OpenGL
 * context still exists), then closes the window and releases FreeGLUT.
 * @return TF
 */
bool ENG_API Eng::Base::free()
{
   if (!reserved->initFlag)
   {
      std::cout << "ERROR: engine not initialized" << std::endl;
      return false;
   }

   reserved->loadedObjects.clear();
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
 * Sets the function called when a key is released. Together with setKeyboardCallback() it tells which keys are held
 * down (the key repeat of the system is ignored: a held key gives one press and one release). Using it is optional.
 * @param callback function receiving the key and the mouse position, in pixels (nullptr = no key release callback)
 */
void ENG_API Eng::Base::setKeyboardUpCallback(void (*callback)(unsigned char key, int mouseX, int mouseY))
{
   if (!reserved->initFlag)
   {
      std::cout << "ERROR: engine not initialized" << std::endl;
      return;
   }

   glutKeyboardUpFunc(callback);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Sets the function called when a special key (e.g. an arrow: KEY_LEFT, KEY_UP, KEY_RIGHT, KEY_DOWN) is pressed.
 * Using it is optional.
 * @param callback function receiving the key and the mouse position, in pixels (nullptr = no special key callback)
 */
void ENG_API Eng::Base::setSpecialKeyCallback(void (*callback)(int key, int mouseX, int mouseY))
{
   if (!reserved->initFlag)
   {
      std::cout << "ERROR: engine not initialized" << std::endl;
      return;
   }

   static_assert(KEY_LEFT == GLUT_KEY_LEFT && KEY_UP == GLUT_KEY_UP && KEY_RIGHT == GLUT_KEY_RIGHT && KEY_DOWN == GLUT_KEY_DOWN);
   glutSpecialFunc(callback);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Sets the function called when a special key (e.g. an arrow) is released. Using it is optional.
 * @param callback function receiving the key and the mouse position, in pixels (nullptr = no special key release callback)
 */
void ENG_API Eng::Base::setSpecialKeyUpCallback(void (*callback)(int key, int mouseX, int mouseY))
{
   if (!reserved->initFlag)
   {
      std::cout << "ERROR: engine not initialized" << std::endl;
      return;
   }

   glutSpecialUpFunc(callback);
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
 * Loads a whole 3D scene from a file (glTF/GLB or any other format read by Assimp) and converts it into engine objects:
 * scene graph (nodes and their matrices), meshes, materials, textures and lights. Assimp turns every polygon into triangles,
 * removes points, lines and degenerate triangles (area 0) and computes the normals when the file has none.
 * The objects belong to the engine and are deleted by free().
 * @param fileName name of the file
 * @return root node of the loaded scene, or nullptr on error
 */
Eng::Node ENG_API *Eng::Base::load(const std::string &fileName)
{
   if (!reserved->initFlag)
   {
      std::cout << "ERROR: engine not initialized" << std::endl;
      return nullptr;
   }

   Assimp::Importer importer;
   importer.SetPropertyInteger(AI_CONFIG_PP_FD_REMOVE, 1);
   importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);
   const aiScene *fileScene = importer.ReadFile(fileName, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices |
      aiProcess_FindDegenerates | aiProcess_SortByPType | aiProcess_GenSmoothNormals);
   if (fileScene == nullptr || fileScene->mRootNode == nullptr)
   {
      std::cout << "ERROR: unable to load " << fileName << " (" << importer.GetErrorString() << ")" << std::endl;
      return nullptr;
   }

   std::vector<Material *> materials;
   for (unsigned int i = 0; i < fileScene->mNumMaterials; i++)
      materials.push_back(reserved->loadMaterial(fileScene->mMaterials[i]));

   Node *root = reserved->loadNode(fileScene, fileScene->mRootNode, materials);

   std::cout << "[>] " << fileName << " loaded: " << fileScene->mNumMeshes << " meshes, " << fileScene->mNumMaterials
             << " materials, " << fileScene->mNumLights << " lights" << std::endl;
   return root;
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
 * Draws a 2D text over the rendered scene (e.g. a legend of the keys), with the text function of FreeGLUT.
 * Call it after render() and before swapBuffers(). Lighting, depth test and texture mapping are off while drawing the text.
 * @param text text to draw (a newline character starts a new line)
 * @param x distance from the left border of the window, in pixels
 * @param y distance from the top border of the window to the first line, in pixels
 * @param color RGB color, each component between 0 and 1
 */
void ENG_API Eng::Base::drawText(const std::string &text, int x, int y, const glm::vec3 &color)
{
   if (!reserved->running)
      return;

   glMatrixMode(GL_PROJECTION);
   glLoadMatrixf(glm::value_ptr(glm::ortho(0.0f, static_cast<float>(reserved->width), 0.0f, static_cast<float>(reserved->height), -1.0f, 1.0f)));
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   glDisable(GL_LIGHTING);
   glDisable(GL_DEPTH_TEST);
   glDisable(GL_TEXTURE_2D);
   glColor3f(color.r, color.g, color.b);
   glRasterPos2i(x, reserved->height - y);
   glutBitmapString(GLUT_BITMAP_8_BY_13, reinterpret_cast<const unsigned char *>(text.c_str()));
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_LIGHTING);
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
