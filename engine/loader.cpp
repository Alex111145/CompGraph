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

      std::shared_ptr<Eng::Texture> texture = std::make_shared<Eng::Texture>(path.C_Str());
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
      std::shared_ptr<Eng::Material> material = std::make_shared<Eng::Material>(source->GetName().C_Str());
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
    * Faces that are not triangles (points, lines) are skipped.
    * @param source Assimp mesh
    * @param mesh engine mesh, already attached to its parent
    */
   void readMesh(const aiMesh *source, Eng::Mesh *mesh)
   {
      std::vector<glm::vec3> positions;
      std::vector<glm::vec3> normals;
      std::vector<glm::vec2> texCoords;

      for (unsigned int face = 0; face < source->mNumFaces; face++)
      {
         if (source->mFaces[face].mNumIndices != 3)
            continue;

         for (unsigned int corner = 0; corner < 3; corner++)
         {
            const unsigned int vertex = source->mFaces[face].mIndices[corner];
            glm::vec2 texCoord(0.0f);
            if (source->HasTextureCoords(0))
               texCoord = glm::vec2(source->mTextureCoords[0][vertex].x, source->mTextureCoords[0][vertex].y);

            positions.push_back(glm::vec3(source->mVertices[vertex].x, source->mVertices[vertex].y, source->mVertices[vertex].z));
            normals.push_back(glm::vec3(source->mNormals[vertex].x, source->mNormals[vertex].y, source->mNormals[vertex].z));
            texCoords.push_back(texCoord);
         }
      }

      mesh->build(positions, normals, texCoords);
   }

   /**
    * Converts an Assimp light into an engine light placed at the origin of its node.
    * A directional light shines along -Z of its node (glTF convention, same as DirectionalLight).
    * @param source Assimp light
    * @return engine light, or nullptr if the type is not supported (spot, area)
    */
   Eng::Light *readLight(const aiLight *source)
   {
      const glm::vec3 color(source->mColorDiffuse.r, source->mColorDiffuse.g, source->mColorDiffuse.b);

      if (source->mType == aiLightSource_DIRECTIONAL)
         return new Eng::DirectionalLight(source->mName.C_Str(), color);

      if (source->mType != aiLightSource_POINT)
         return nullptr;

      Eng::PointLight *light = new Eng::PointLight(source->mName.C_Str(), color);
      light->setAttenuation(source->mAttenuationConstant, source->mAttenuationLinear, source->mAttenuationQuadratic);
      light->setMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(source->mPosition.x, source->mPosition.y, source->mPosition.z)));
      return light;
   }

   /**
    * Converts an Assimp node (and recursively its children) into engine nodes.
    * Meshes without triangles (only points or lines) are skipped. A light belongs to the node with its name.
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
         if ((sourceMesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE) == 0)
            continue;

         Eng::Mesh *mesh = new Eng::Mesh(sourceMesh->mName.C_Str());
         node->addChild(mesh);
         mesh->setMaterial(materials[sourceMesh->mMaterialIndex]);
         readMesh(sourceMesh, mesh);
      }

      for (unsigned int i = 0; i < scene->mNumLights; i++)
         if (scene->mLights[i]->mName == source->mName)
            if (Eng::Light *light = readLight(scene->mLights[i]))
               node->addChild(light);

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
