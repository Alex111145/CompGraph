/**
 * @file		loader.cpp
 * @brief	Loads a 3D model file into a Node hierarchy + Mesh list via Assimp
 *
 * @author	Alessio Gervasini
 */

   #include "engine.h"

   #include <algorithm>
   #include <cmath>
   #include <cstdlib>
   #include <iostream>
   #include <unordered_map>
   #include <vector>

   #include <assimp/Importer.hpp>
   #include <assimp/material.h>
   #include <assimp/postprocess.h>
   #include <assimp/scene.h>

   #include <glm/glm.hpp>
   #include <glm/gtc/quaternion.hpp>
   #include <glm/gtc/type_ptr.hpp>

namespace
{
   const float FLAT_TOLERANCE = 0.001f;
   const float EMISSION_LUMINANCE_THRESHOLD = 0.9f;
   const glm::vec3 NO_EMISSION(0.0f);
   const glm::vec3 FALLBACK_COLOR(0.8f, 0.8f, 0.8f);

   std::string directoryOf(const std::string &path)
   {
      const size_t lastSeparator = path.find_last_of("/\\");
      return lastSeparator == std::string::npos ? std::string() : path.substr(0, lastSeparator + 1);
   }

   float luminance(const aiColor3D &color)
   {
      return 0.299f * color.r + 0.587f * color.g + 0.114f * color.b;
   }
}

struct Eng::Loader::Reserved
{
   std::unique_ptr<Node> rootNode;
   std::vector<std::unique_ptr<Mesh>> meshes;
   std::vector<Node *> meshNodes;
   std::vector<Texture *> meshTextures;
   std::vector<glm::vec3> meshEmissions;
   std::vector<glm::vec3> meshBoundsMin;
   std::vector<glm::vec3> meshBoundsMax;
   std::vector<std::unique_ptr<Texture>> ownedTextures;
   std::unordered_map<std::string, Texture *> textureCache;
   Texture *fallbackTexture;
   int groundMeshIndex;
   float groundPlaneY;
   std::string baseDirectory;

   Reserved() : fallbackTexture{ nullptr }, groundMeshIndex{ -1 }, groundPlaneY{ 0.0f }
   {}

   Texture *storeTexture(std::unique_ptr<Texture> texture, const std::string &cacheKey)
   {
      Texture *texturePtr = texture.get();
      ownedTextures.push_back(std::move(texture));
      if (!cacheKey.empty())
         textureCache[cacheKey] = texturePtr;
      return texturePtr;
   }

   Texture *createSolidTexture(const glm::vec3 &color)
   {
      const glm::vec3 clamped = glm::clamp(color, 0.0f, 1.0f);
      const unsigned char rgba[4] = {
         static_cast<unsigned char>(clamped.r * 255.0f),
         static_cast<unsigned char>(clamped.g * 255.0f),
         static_cast<unsigned char>(clamped.b * 255.0f),
         255
      };
      auto texture = std::make_unique<Texture>();
      texture->loadFromPixels(1, 1, 4, rgba);
      return storeTexture(std::move(texture), "");
   }

   Texture *getFallbackTexture()
   {
      if (fallbackTexture == nullptr)
         fallbackTexture = createSolidTexture(FALLBACK_COLOR);
      return fallbackTexture;
   }

   Texture *loadExternalTexture(const std::string &path)
   {
      const auto cached = textureCache.find(path);
      if (cached != textureCache.end())
         return cached->second;

      auto texture = std::make_unique<Texture>();
      if (!texture->loadFromFile(path))
         return nullptr;
      return storeTexture(std::move(texture), path);
   }

   Texture *loadEmbeddedTexture(const aiScene *scene, int embeddedIndex)
   {
      const std::string cacheKey = "*" + std::to_string(embeddedIndex);
      const auto cached = textureCache.find(cacheKey);
      if (cached != textureCache.end())
         return cached->second;

      const aiTexture *embedded = scene->mTextures[embeddedIndex];
      auto texture = std::make_unique<Texture>();

      if (embedded->mHeight == 0)
      {
         const unsigned char *encodedBytes = reinterpret_cast<const unsigned char *>(embedded->pcData);
         if (!texture->loadFromEncodedMemory(encodedBytes, static_cast<int>(embedded->mWidth)))
            return nullptr;
      }
      else
      {
         const unsigned int pixelCount = embedded->mWidth * embedded->mHeight;
         std::vector<unsigned char> rgba(static_cast<size_t>(pixelCount) * 4);
         for (unsigned int i = 0; i < pixelCount; i++)
         {
            const aiTexel &texel = embedded->pcData[i];
            rgba[i * 4 + 0] = texel.r;
            rgba[i * 4 + 1] = texel.g;
            rgba[i * 4 + 2] = texel.b;
            rgba[i * 4 + 3] = texel.a;
         }
         texture->loadFromPixels(static_cast<int>(embedded->mWidth), static_cast<int>(embedded->mHeight), 4, rgba.data());
      }

      return storeTexture(std::move(texture), cacheKey);
   }

   Texture *resolveTexture(const aiScene *scene, const aiMaterial *material)
   {
      if (material == nullptr)
         return getFallbackTexture();

      aiString texturePath;
      if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS)
      {
         const bool isEmbedded = texturePath.C_Str()[0] == '*';
         Texture *texture = isEmbedded
            ? loadEmbeddedTexture(scene, std::atoi(texturePath.C_Str() + 1))
            : loadExternalTexture(baseDirectory + texturePath.C_Str());
         if (texture != nullptr)
            return texture;
      }

      aiColor3D diffuseColor;
      if (material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor) == AI_SUCCESS)
         return createSolidTexture(glm::vec3(diffuseColor.r, diffuseColor.g, diffuseColor.b));

      return getFallbackTexture();
   }

   glm::vec3 resolveEmission(const aiMaterial *material) const
   {
      if (material == nullptr)
         return NO_EMISSION;

      aiColor3D emissiveColor;
      if (material->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColor) == AI_SUCCESS)
      {
         const glm::vec3 emission(emissiveColor.r, emissiveColor.g, emissiveColor.b);
         if (glm::length(emission) > FLAT_TOLERANCE)
            return emission;
      }

      aiString texturePath;
      if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS)
         return NO_EMISSION;

      aiColor3D diffuseColor;
      if (material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor) != AI_SUCCESS)
         return NO_EMISSION;

      if (luminance(diffuseColor) <= EMISSION_LUMINANCE_THRESHOLD)
         return NO_EMISSION;

      return glm::vec3(diffuseColor.r, diffuseColor.g, diffuseColor.b);
   }

   void recordBoundsAndGround(unsigned int meshIndex, const std::vector<glm::vec3> &worldPositions)
   {
      glm::vec3 boundsMin = worldPositions.empty() ? glm::vec3(0.0f) : worldPositions.front();
      glm::vec3 boundsMax = boundsMin;
      for (const glm::vec3 &position : worldPositions)
      {
         boundsMin = glm::min(boundsMin, position);
         boundsMax = glm::max(boundsMax, position);
      }
      meshBoundsMin.push_back(boundsMin);
      meshBoundsMax.push_back(boundsMax);

      const bool isFlat = !worldPositions.empty() && boundsMax.y - boundsMin.y <= FLAT_TOLERANCE;
      const bool isLowest = groundMeshIndex < 0 || boundsMin.y < groundPlaneY;
      if (isFlat && isLowest)
      {
         groundMeshIndex = static_cast<int>(meshIndex);
         groundPlaneY = boundsMin.y;
      }
   }

   void processMesh(const aiScene *scene, const aiMesh *sourceMesh, Node *node, const glm::mat4 &worldMatrix)
   {
      const unsigned int vertexCount = sourceMesh->mNumVertices;
      const bool hasNormals = sourceMesh->HasNormals();
      const bool hasTextureCoords = sourceMesh->HasTextureCoords(0);
      std::vector<Vertex> vertices(vertexCount);
      std::vector<glm::vec3> worldPositions(vertexCount);
      std::vector<unsigned int> indices;

      for (unsigned int i = 0; i < vertexCount; i++)
      {
         const aiVector3D &position = sourceMesh->mVertices[i];
         const aiVector3D normal = hasNormals ? sourceMesh->mNormals[i] : aiVector3D(0.0f);
         const aiVector3D textureCoord = hasTextureCoords ? sourceMesh->mTextureCoords[0][i] : aiVector3D(0.0f);

         vertices[i] = { { position.x, position.y, position.z }, { normal.x, normal.y, normal.z }, { textureCoord.x, textureCoord.y } };
         worldPositions[i] = glm::vec3(worldMatrix * glm::vec4(position.x, position.y, position.z, 1.0f));
      }

      indices.reserve(static_cast<size_t>(sourceMesh->mNumFaces) * 3);
      for (unsigned int i = 0; i < sourceMesh->mNumFaces; i++)
      {
         const aiFace &face = sourceMesh->mFaces[i];
         indices.insert(indices.end(), face.mIndices, face.mIndices + face.mNumIndices);
      }

      auto mesh = std::make_unique<Mesh>();
      mesh->setGeometry(vertices.data(), vertexCount, indices.data(), static_cast<unsigned int>(indices.size()));

      const aiMaterial *material = sourceMesh->mMaterialIndex < scene->mNumMaterials ? scene->mMaterials[sourceMesh->mMaterialIndex] : nullptr;

      recordBoundsAndGround(static_cast<unsigned int>(meshes.size()), worldPositions);
      meshes.push_back(std::move(mesh));
      meshNodes.push_back(node);
      meshTextures.push_back(resolveTexture(scene, material));
      meshEmissions.push_back(resolveEmission(material));
   }

   void processNode(const aiScene *scene, const aiNode *sourceNode, Node *node)
   {
      aiVector3D translation, scaling;
      aiQuaternion rotation;
      sourceNode->mTransformation.Decompose(scaling, rotation, translation);

      const glm::mat3 rotationMatrix = glm::mat3_cast(glm::quat(rotation.w, rotation.x, rotation.y, rotation.z));
      const float pitchDegrees = glm::degrees(std::asin(std::clamp(-rotationMatrix[2][1], -1.0f, 1.0f)));
      const float yawDegrees = glm::degrees(std::atan2(rotationMatrix[2][0], rotationMatrix[2][2]));
      const float rollDegrees = glm::degrees(std::atan2(rotationMatrix[0][1], rotationMatrix[1][1]));

      node->setName(sourceNode->mName.C_Str());
      node->setPosition(translation.x, translation.y, translation.z);
      node->setRotation(pitchDegrees, yawDegrees, rollDegrees);
      node->setScale(scaling.x, scaling.y, scaling.z);

      const glm::mat4 worldMatrix = glm::make_mat4(node->getWorldMatrix());
      for (unsigned int i = 0; i < sourceNode->mNumMeshes; i++)
         processMesh(scene, scene->mMeshes[sourceNode->mMeshes[i]], node, worldMatrix);

      for (unsigned int i = 0; i < sourceNode->mNumChildren; i++)
         processNode(scene, sourceNode->mChildren[i], node->addChild());
   }
};

ENG_API Eng::Loader::Loader() : reserved(std::make_unique<Eng::Loader::Reserved>())
{}

ENG_API Eng::Loader::~Loader()
{}

bool ENG_API Eng::Loader::load(const std::string &filename)
{
   Assimp::Importer importer;
   const aiScene *scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GenSmoothNormals);
   if (scene == nullptr || scene->mRootNode == nullptr)
   {
      std::cout << "ERROR: Assimp failed to load '" << filename << "': " << importer.GetErrorString() << std::endl;
      return false;
   }

   reserved->baseDirectory = directoryOf(filename);
   reserved->rootNode = std::make_unique<Node>();
   reserved->processNode(scene, scene->mRootNode, reserved->rootNode.get());

   if (reserved->meshes.empty())
   {
      std::cout << "ERROR: '" << filename << "' contains no geometry" << std::endl;
      return false;
   }
   return true;
}

unsigned int ENG_API Eng::Loader::getMeshCount() const
{
   return static_cast<unsigned int>(reserved->meshes.size());
}

Eng::Mesh ENG_API *Eng::Loader::getMesh(unsigned int index) const
{
   return reserved->meshes[index].get();
}

Eng::Node ENG_API *Eng::Loader::getMeshNode(unsigned int index) const
{
   return reserved->meshNodes[index];
}

Eng::Texture ENG_API *Eng::Loader::getMeshTexture(unsigned int index) const
{
   return reserved->meshTextures[index];
}

bool ENG_API Eng::Loader::getMeshEmission(unsigned int index, float &r, float &g, float &b) const
{
   const glm::vec3 &emission = reserved->meshEmissions[index];
   if (emission == NO_EMISSION)
      return false;

   r = emission.r;
   g = emission.g;
   b = emission.b;
   return true;
}

void ENG_API Eng::Loader::getMeshBounds(unsigned int index, float &minX, float &minY, float &minZ, float &maxX, float &maxY, float &maxZ) const
{
   const glm::vec3 &boundsMin = reserved->meshBoundsMin[index];
   const glm::vec3 &boundsMax = reserved->meshBoundsMax[index];
   minX = boundsMin.x;
   minY = boundsMin.y;
   minZ = boundsMin.z;
   maxX = boundsMax.x;
   maxY = boundsMax.y;
   maxZ = boundsMax.z;
}

int ENG_API Eng::Loader::getGroundMeshIndex() const
{
   return reserved->groundMeshIndex;
}

float ENG_API Eng::Loader::getGroundPlaneY() const
{
   return reserved->groundPlaneY;
}

bool ENG_API Eng::Loader::getFurnitureTopCenter(float &x, float &y, float &z) const
{
   bool foundFurniture = false;
   glm::vec3 boundsMin(0.0f);
   glm::vec3 boundsMax(0.0f);

   for (size_t i = 0; i < reserved->meshBoundsMin.size(); i++)
   {
      if (static_cast<int>(i) == reserved->groundMeshIndex)
         continue;

      boundsMin = foundFurniture ? glm::min(boundsMin, reserved->meshBoundsMin[i]) : reserved->meshBoundsMin[i];
      boundsMax = foundFurniture ? glm::max(boundsMax, reserved->meshBoundsMax[i]) : reserved->meshBoundsMax[i];
      foundFurniture = true;
   }

   if (!foundFurniture)
      return false;

   x = (boundsMin.x + boundsMax.x) * 0.5f;
   y = boundsMax.y;
   z = (boundsMin.z + boundsMax.z) * 0.5f;
   return true;
}
