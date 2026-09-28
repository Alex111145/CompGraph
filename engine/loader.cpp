/**
 * @file		loader.cpp
 * @brief	Loads a 3D model file (via Assimp) into a scene graph
 *
 * @author	Alessio Gervasini
 */

   #include "engine.h"

   #include <iostream>

   #include <assimp/Importer.hpp>
   #include <assimp/postprocess.h>
   #include <assimp/scene.h>

namespace
{
   /**
    * Reads a color property of an Assimp material.
    * @param material Assimp material
    * @param key property key (AI_MATKEY_COLOR_...)
    * @param type property type
    * @param index property index
    * @return color, or a zero vector if missing
    */
   glm::vec3 readColor(const aiMaterial *material, const char *key, unsigned int type, unsigned int index)
   {
      aiColor3D color(0.0f, 0.0f, 0.0f);
      material->Get(key, type, index, color);
      return glm::vec3(color.r, color.g, color.b);
   }

   /**
    * Loads the diffuse texture of a material, stored inside the file.
    * @param scene Assimp scene
    * @param material Assimp material
    * @return texture, or empty if the material has none
    */
   std::shared_ptr<Eng::Texture> readTexture(const aiScene *scene, const aiMaterial *material)
   {
      aiString path;
      if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) != AI_SUCCESS)
         return nullptr;

      const aiTexture *embedded = scene->GetEmbeddedTexture(path.C_Str());
      if (embedded == nullptr || embedded->mHeight != 0)
         return nullptr;

      std::shared_ptr<Eng::Texture> texture = std::make_shared<Eng::Texture>();
      if (!texture->load(reinterpret_cast<const unsigned char *>(embedded->pcData), static_cast<int>(embedded->mWidth)))
         return nullptr;
      return texture;
   }

   /**
    * Converts an Assimp material into an engine material.
    * @param scene Assimp scene
    * @param source Assimp material
    * @return engine material
    */
   std::shared_ptr<Eng::Material> readMaterial(const aiScene *scene, const aiMaterial *source)
   {
      std::shared_ptr<Eng::Material> material = std::make_shared<Eng::Material>();
      const glm::vec3 diffuse = readColor(source, AI_MATKEY_COLOR_DIFFUSE);
      float shininess = 0.0f;

      source->Get(AI_MATKEY_SHININESS, shininess);
      material->setAmbient(diffuse);
      material->setDiffuse(diffuse);
      material->setSpecular(readColor(source, AI_MATKEY_COLOR_SPECULAR));
      material->setEmission(readColor(source, AI_MATKEY_COLOR_EMISSIVE));
      material->setShininess(shininess);
      material->setTexture(readTexture(scene, source));
      return material;
   }

   /**
    * Converts an Assimp mesh into an engine mesh (one vertex per triangle corner).
    * @param source Assimp mesh
    * @param mesh engine mesh, already attached to its parent
    */
   void readMesh(const aiMesh *source, Eng::Mesh *mesh)
   {
      std::vector<glm::vec3> positions;
      std::vector<glm::vec3> normals;
      std::vector<glm::vec2> uvs;

      for (unsigned int f = 0; f < source->mNumFaces; f++)
         for (unsigned int c = 0; c < source->mFaces[f].mNumIndices; c++)
         {
            const unsigned int v = source->mFaces[f].mIndices[c];
            glm::vec2 uv(0.0f);
            if (source->HasTextureCoords(0))
               uv = glm::vec2(source->mTextureCoords[0][v].x, source->mTextureCoords[0][v].y);

            positions.push_back(glm::vec3(source->mVertices[v].x, source->mVertices[v].y, source->mVertices[v].z));
            normals.push_back(glm::vec3(source->mNormals[v].x, source->mNormals[v].y, source->mNormals[v].z));
            uvs.push_back(uv);
         }

      mesh->build(positions, normals, uvs);
   }

   /**
    * Converts an Assimp node (and recursively its children) into engine nodes.
    * The Assimp matrix is row-major: it is transposed into GLM column-major order.
    * @param scene Assimp scene
    * @param source Assimp node
    * @param materials engine materials, same order as in the file
    * @param parent engine parent node (nullptr for the root)
    * @return engine node
    */
   Eng::Node *readNode(const aiScene *scene, const aiNode *source, const std::vector<std::shared_ptr<Eng::Material>> &materials, Eng::Node *parent)
   {
      Eng::Node *node = new Eng::Node(source->mName.C_Str());
      node->setMatrix(glm::transpose(glm::make_mat4(&source->mTransformation.a1)));
      if (parent != nullptr)
         parent->addChild(node);

      for (unsigned int i = 0; i < source->mNumMeshes; i++)
      {
         const aiMesh *sourceMesh = scene->mMeshes[source->mMeshes[i]];
         Eng::Mesh *mesh = new Eng::Mesh(sourceMesh->mName.C_Str());
         node->addChild(mesh);
         mesh->setMaterial(materials[sourceMesh->mMaterialIndex]);
         readMesh(sourceMesh, mesh);
      }

      for (unsigned int i = 0; i < source->mNumChildren; i++)
         readNode(scene, source->mChildren[i], materials, node);

      return node;
   }
}

/**
 * Loads a 3D model file. Requires an initialized engine (OpenGL context).
 * @param filename path of the file
 * @return root node of the loaded scene graph (owned by the caller), nullptr on error
 */
Eng::Node ENG_API *Eng::Loader::load(const std::string &filename)
{
   Assimp::Importer importer;
   const aiScene *scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_GenSmoothNormals);
   std::vector<std::shared_ptr<Material>> materials;

   if (scene == nullptr || scene->mRootNode == nullptr)
   {
      std::cout << "ERROR: unable to load '" << filename << "': " << importer.GetErrorString() << std::endl;
      return nullptr;
   }

   for (unsigned int i = 0; i < scene->mNumMaterials; i++)
      materials.push_back(readMaterial(scene, scene->mMaterials[i]));

   return readNode(scene, scene->mRootNode, materials, nullptr);
}
