/**
 * @file		loader.cpp
 * @brief	Loads a 3D model from an external file into a Node hierarchy + Mesh list, via Assimp
 *
 * @author	Alessio Gervasini
 */

   #include "engine.h"

   #include <iostream>
   #include <vector>
   #include <unordered_map>
   #include <string>
   #include <algorithm>
   #include <cmath>
   #include <cstdlib>

   #include <assimp/Importer.hpp>
   #include <assimp/scene.h>
   #include <assimp/postprocess.h>
   #include <assimp/material.h>

   #include <glm/glm.hpp>
   #include <glm/gtc/quaternion.hpp>
   #include <glm/gtc/type_ptr.hpp>

namespace
{
   /** Directory part of a path (including the trailing slash), or "" if path has none. */
   std::string dirOf(const std::string &path)
   {
      const size_t slash = path.find_last_of("/\\");
      return slash == std::string::npos ? std::string() : path.substr(0, slash + 1);
   }
}

/** @brief Loader class reserved structure (PIMPL/Bridge pattern, same as Eng::Base). */
struct Eng::Loader::Reserved
{
   std::unique_ptr<Node> rootNode;
   std::vector<std::unique_ptr<Mesh>> meshes;
   std::vector<Node *> meshNodes;
   std::vector<Texture *> meshTextures;
   std::vector<glm::vec3> meshEmission;   // (0,0,0) = not self-lit
   std::vector<glm::vec3> meshBoundsMin;
   std::vector<glm::vec3> meshBoundsMax;

   std::vector<std::unique_ptr<Texture>> ownedTextures;
   std::unordered_map<std::string, Texture *> textureCache;   // keyed by "path" or "*embeddedIndex"
   Texture *fallbackTexture = nullptr;

   int groundMeshIndex = -1;
   float groundPlaneY = 0.0f;
   bool groundFound = false;

   /**
    * Called once per mesh with its vertex data already transformed to WORLD space (a glTF/FBX
    * node carries its own position/rotation/scale, so "is this mesh low and flat in the scene"
    * has to be checked after applying that transform, not on raw local-space coordinates): if every vertex
    * shares the same world-space Y (a flat mesh) and that Y is the lowest seen so far, remembers
    * it as the scene's ground -- geometry-based, no dependency on how anything is named.
    */
   void considerAsGround(unsigned int meshIndex, const std::vector<glm::vec3> &worldPositions)
   {
      if (worldPositions.empty())
         return;

      const float flatEpsilon = 0.001f;
      float minY = worldPositions[0].y;
      float maxY = minY;
      for (const glm::vec3 &p : worldPositions)
      {
         minY = std::min(minY, p.y);
         maxY = std::max(maxY, p.y);
      }
      if (maxY - minY > flatEpsilon)
         return;   // not flat

      if (!groundFound || minY < groundPlaneY)
      {
         groundMeshIndex = static_cast<int>(meshIndex);
         groundPlaneY = minY;
         groundFound = true;
      }
   }

   /** Records one mesh's own world-space axis-aligned bounding box, for getFurnitureTopCenter(). */
   void recordBounds(const std::vector<glm::vec3> &worldPositions)
   {
      glm::vec3 boundsMin(worldPositions.empty() ? glm::vec3(0.0f) : worldPositions[0]);
      glm::vec3 boundsMax = boundsMin;
      for (const glm::vec3 &p : worldPositions)
      {
         boundsMin = glm::min(boundsMin, p);
         boundsMax = glm::max(boundsMax, p);
      }
      meshBoundsMin.push_back(boundsMin);
      meshBoundsMax.push_back(boundsMax);
   }

   /**
    * Union of every mesh's bounding box EXCEPT the ground (once it's known, after the whole
    * file is loaded) -- i.e. "wherever the furniture actually is", purely from geometry.
    * @return TF (false = nothing to report, e.g. the scene is only a ground plane)
    */
   bool getFurnitureTopCenter(float &x, float &y, float &z, float &halfWidthX, float &halfDepthZ) const
   {
      bool any = false;
      glm::vec3 boundsMin(0.0f), boundsMax(0.0f);
      for (size_t i = 0; i < meshBoundsMin.size(); i++)
      {
         if (static_cast<int>(i) == groundMeshIndex)
            continue;
         if (!any)
         {
            boundsMin = meshBoundsMin[i];
            boundsMax = meshBoundsMax[i];
            any = true;
         }
         else
         {
            boundsMin = glm::min(boundsMin, meshBoundsMin[i]);
            boundsMax = glm::max(boundsMax, meshBoundsMax[i]);
         }
      }
      if (!any)
         return false;

      halfWidthX = (boundsMax.x - boundsMin.x) * 0.5f;
      halfDepthZ = (boundsMax.z - boundsMin.z) * 0.5f;
      x = (boundsMin.x + boundsMax.x) * 0.5f;
      y = boundsMax.y;
      z = (boundsMin.z + boundsMax.z) * 0.5f;
      return true;
   }

   /** Creates, registers (so it's freed with the Loader), and returns a new 1x1 solid-color texture. */
   Texture *makeSolidTexture(const glm::vec3 &color)
   {
      const unsigned char rgba[4] = {
         static_cast<unsigned char>(std::clamp(color.r, 0.0f, 1.0f) * 255.0f),
         static_cast<unsigned char>(std::clamp(color.g, 0.0f, 1.0f) * 255.0f),
         static_cast<unsigned char>(std::clamp(color.b, 0.0f, 1.0f) * 255.0f),
         255
      };
      auto tex = std::make_unique<Texture>();
      tex->loadFromMemory(1, 1, 4, rgba);
      Texture *texPtr = tex.get();
      ownedTextures.push_back(std::move(tex));
      return texPtr;
   }

   /**
    * A 1x1 neutral-gray texture, created once and reused for every mesh whose material has no
    * diffuse map or color -- matches the same gray a material with no explicit color falls
    * back to below, so an object with literally no material looks like a plain lit surface
    * instead of standing out.
    */
   Texture *getFallbackTexture()
   {
      if (fallbackTexture == nullptr)
         fallbackTexture = makeSolidTexture(glm::vec3(0.8f, 0.8f, 0.8f));
      return fallbackTexture;
   }

   /** Loads (or reuses, if already loaded) the external image file at the given path as a Texture. */
   Texture *getOrLoadTexture(const std::string &path)
   {
      auto found = textureCache.find(path);
      if (found != textureCache.end())
         return found->second;

      auto tex = std::make_unique<Texture>();
      if (!tex->load(path))
         return nullptr;

      Texture *texPtr = tex.get();
      ownedTextures.push_back(std::move(tex));
      textureCache[path] = texPtr;
      return texPtr;
   }

   /**
    * Decodes (or reuses, if already decoded) one texture embedded directly inside the model
    * file itself -- how a single-file .glb carries its images, with no external file to load()
    * by path. aiTexture::mHeight == 0 means "compressed" (PNG/JPG bytes, mWidth = byte count);
    * otherwise it's already-decoded BGRA8 texels, mWidth x mHeight of them.
    */
   Texture *getOrDecodeEmbeddedTexture(const aiScene *scene, int index)
   {
      const std::string cacheKey = "*" + std::to_string(index);
      auto found = textureCache.find(cacheKey);
      if (found != textureCache.end())
         return found->second;

      const aiTexture *embedded = scene->mTextures[index];
      auto tex = std::make_unique<Texture>();
      bool ok;
      if (embedded->mHeight == 0)
      {
         ok = tex->loadFromCompressedMemory(reinterpret_cast<const unsigned char *>(embedded->pcData), static_cast<int>(embedded->mWidth));
      }
      else
      {
         const unsigned int pixelCount = embedded->mWidth * embedded->mHeight;
         std::vector<unsigned char> rgba(static_cast<size_t>(pixelCount) * 4);
         for (unsigned int i = 0; i < pixelCount; i++)
         {
            const aiTexel &texel = embedded->pcData[i];
            rgba[i * 4 + 0] = texel.r; rgba[i * 4 + 1] = texel.g;
            rgba[i * 4 + 2] = texel.b; rgba[i * 4 + 3] = texel.a;
         }
         ok = tex->loadFromMemory(static_cast<int>(embedded->mWidth), static_cast<int>(embedded->mHeight), 4, rgba.data());
      }
      if (!ok)
         return nullptr;

      Texture *texPtr = tex.get();
      ownedTextures.push_back(std::move(tex));
      textureCache[cacheKey] = texPtr;
      return texPtr;
   }

   /** Picks the texture for one material: its diffuse map (external file or embedded in the model) if any, else a flat base color, else the gray fallback. */
   Texture *resolveMaterialTexture(const aiScene *scene, const aiMaterial *material, const std::string &baseDir)
   {
      if (material == nullptr)
         return getFallbackTexture();

      aiString texPath;
      if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS)
      {
         if (texPath.C_Str()[0] == '*')
         {
            Texture *tex = getOrDecodeEmbeddedTexture(scene, std::atoi(texPath.C_Str() + 1));
            if (tex != nullptr)
               return tex;
         }
         else
         {
            Texture *tex = getOrLoadTexture(baseDir + texPath.C_Str());
            if (tex != nullptr)
               return tex;
         }
      }

      aiColor3D kd;
      if (material->Get(AI_MATKEY_COLOR_DIFFUSE, kd) == AI_SUCCESS)
         return makeSolidTexture(glm::vec3(kd.r, kd.g, kd.b));

      return getFallbackTexture();
   }

   /**
    * A material's own declared emissive color (AI_MATKEY_COLOR_EMISSIVE, the real "Ke" a glTF/
    * FBX material can carry) if it has one and it's not black; otherwise, a material with NO
    * diffuse texture map whose flat base color is close to white (luminance > 0.9, the way a
    * lamp bulb's material is conventionally authored even with no emission data at all) is
    * treated as mildly self-lit. The no-texture condition matters: a textured material's Kd is
    * conventionally left at (1,1,1) as a neutral multiplier for the image (the glTF/PBR
    * baseColorFactor default) -- that white is about the texture pipeline, not the material
    * glowing, so it must never trip this heuristic. Either way this never looks at an object's
    * name.
    */
   glm::vec3 computeEmission(const aiMaterial *material)
   {
      if (material == nullptr)
         return glm::vec3(0.0f);

      aiColor3D ke;
      if (material->Get(AI_MATKEY_COLOR_EMISSIVE, ke) == AI_SUCCESS)
      {
         const glm::vec3 emissive(ke.r, ke.g, ke.b);
         if (glm::length(emissive) > 0.001f)
            return emissive;
      }

      aiString texPath;
      if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS)
         return glm::vec3(0.0f);   // Kd's whiteness here is a texture multiplier, not a glow

      aiColor3D kd;
      if (material->Get(AI_MATKEY_COLOR_DIFFUSE, kd) != AI_SUCCESS)
         return glm::vec3(0.0f);

      const float luminance = 0.299f * kd.r + 0.587f * kd.g + 0.114f * kd.b;
      const float brightThreshold = 0.9f;
      if (luminance <= brightThreshold)
         return glm::vec3(0.0f);

      // Full base color, not scaled down by how far past the threshold it is -- a graded
      // strength left a lamp shade looking like a dim tint instead of a lit bulb.
      return glm::vec3(kd.r, kd.g, kd.b);
   }

   /**
    * Converts one aiMesh (a single material's worth of geometry) into an Eng::Mesh, resolves
    * its material to a texture + emission color, and remembers which node it belongs to.
    */
   void processMesh(const aiScene *scene, const aiMesh *aMesh, Node *node, const glm::mat4 &worldMatrix, const std::string &baseDir)
   {
      std::vector<Vertex> vertices(aMesh->mNumVertices);
      std::vector<glm::vec3> worldPositions(aMesh->mNumVertices);
      for (unsigned int i = 0; i < aMesh->mNumVertices; i++)
      {
         vertices[i].position[0] = aMesh->mVertices[i].x;
         vertices[i].position[1] = aMesh->mVertices[i].y;
         vertices[i].position[2] = aMesh->mVertices[i].z;

         const glm::vec4 worldPos = worldMatrix * glm::vec4(aMesh->mVertices[i].x, aMesh->mVertices[i].y, aMesh->mVertices[i].z, 1.0f);
         worldPositions[i] = glm::vec3(worldPos);

         if (aMesh->HasNormals())
         {
            vertices[i].normal[0] = aMesh->mNormals[i].x;
            vertices[i].normal[1] = aMesh->mNormals[i].y;
            vertices[i].normal[2] = aMesh->mNormals[i].z;
         }
         else
         {
            vertices[i].normal[0] = vertices[i].normal[1] = vertices[i].normal[2] = 0.0f;
         }

         if (aMesh->HasTextureCoords(0))
         {
            vertices[i].uv[0] = aMesh->mTextureCoords[0][i].x;
            vertices[i].uv[1] = aMesh->mTextureCoords[0][i].y;
         }
         else
         {
            vertices[i].uv[0] = vertices[i].uv[1] = 0.0f;
         }
      }

      std::vector<unsigned int> indices;
      indices.reserve(static_cast<size_t>(aMesh->mNumFaces) * 3);
      for (unsigned int i = 0; i < aMesh->mNumFaces; i++)
      {
         const aiFace &face = aMesh->mFaces[i];
         for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
      }

      auto mesh = std::make_unique<Mesh>();
      mesh->loadFromVertices(vertices.data(), static_cast<unsigned int>(vertices.size()),
                              indices.data(), static_cast<unsigned int>(indices.size()));

      const aiMaterial *material = (aMesh->mMaterialIndex < scene->mNumMaterials) ? scene->mMaterials[aMesh->mMaterialIndex] : nullptr;

      considerAsGround(static_cast<unsigned int>(meshes.size()), worldPositions);
      recordBounds(worldPositions);
      meshes.push_back(std::move(mesh));
      meshNodes.push_back(node);
      meshTextures.push_back(resolveMaterialTexture(scene, material, baseDir));
      meshEmission.push_back(computeEmission(material));
   }

   /**
    * Mirrors one aiNode (and its whole subtree) into the Eng::Node hierarchy, converting
    * every aiMesh it references along the way.
    */
   void processNode(const aiScene *scene, const aiNode *aNode, Node *node, const std::string &baseDir)
   {
      aiVector3D translation, scaling;
      aiQuaternion rotation;
      aNode->mTransformation.Decompose(scaling, rotation, translation);
      node->setPosition(translation.x, translation.y, translation.z);
      node->setScale(scaling.x, scaling.y, scaling.z);
      node->setName(aNode->mName.C_Str());

      // Node::setRotation expects pitch/yaw/roll matching the Y*X*Z composition order
      // Node::Reserved::getLocalMatrix builds internally, so the quaternion from Assimp is
      // decomposed back into that same order rather than a generic (order-mismatched) one.
      glm::mat3 rot = glm::mat3_cast(glm::quat(rotation.w, rotation.x, rotation.y, rotation.z));
      float pitch = glm::degrees(std::asin(std::clamp(-rot[2][1], -1.0f, 1.0f)));
      float yaw   = glm::degrees(std::atan2(rot[2][0], rot[2][2]));
      float roll  = glm::degrees(std::atan2(rot[0][1], rot[1][1]));
      node->setRotation(pitch, yaw, roll);

      const glm::mat4 worldMatrix = glm::make_mat4(node->getWorldMatrix());

      for (unsigned int i = 0; i < aNode->mNumMeshes; i++)
      {
         const aiMesh *aMesh = scene->mMeshes[aNode->mMeshes[i]];
         processMesh(scene, aMesh, node, worldMatrix, baseDir);
      }

      for (unsigned int i = 0; i < aNode->mNumChildren; i++)
      {
         Node *child = node->addChild();
         processNode(scene, aNode->mChildren[i], child, baseDir);
      }
   }
};

/** Constructor. */
ENG_API Eng::Loader::Loader() : reserved(std::make_unique<Eng::Loader::Reserved>())
{}

/** Destructor. */
ENG_API Eng::Loader::~Loader()
{}

/**
 * Reads the given file and builds the node hierarchy + mesh list from it.
 * @param filename path to a 3D model file (any format Assimp understands: .glb, .gltf, .fbx, ...)
 * @return TF
 */
bool ENG_API Eng::Loader::load(const std::string &filename)
{
   Assimp::Importer importer;
   const aiScene *scene = importer.ReadFile(filename,
      aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GenSmoothNormals);

   if (scene == nullptr || scene->mRootNode == nullptr)
   {
      std::cout << "ERROR: Assimp failed to load '" << filename << "': " << importer.GetErrorString() << std::endl;
      return false;
   }

   const std::string baseDir = dirOf(filename);

   reserved->rootNode = std::make_unique<Node>();
   reserved->processNode(scene, scene->mRootNode, reserved->rootNode.get(), baseDir);

   if (reserved->meshes.empty())
   {
      std::cout << "ERROR: '" << filename << "' produced no geometry" << std::endl;
      return false;
   }
   return true;
}

/** @return how many separate Eng::Mesh objects this file produced (one per material/sub-object). */
unsigned int ENG_API Eng::Loader::getMeshCount() const
{
   return static_cast<unsigned int>(reserved->meshes.size());
}

/** @param index 0-based, < getMeshCount(). @return that mesh. */
Eng::Mesh ENG_API *Eng::Loader::getMesh(unsigned int index) const
{
   return reserved->meshes[index].get();
}

/** @param index 0-based, < getMeshCount(). @return the node that mesh should be drawn at (world matrix already includes its place in the hierarchy). */
Eng::Node ENG_API *Eng::Loader::getMeshNode(unsigned int index) const
{
   return reserved->meshNodes[index];
}

/**
 * @param index 0-based, < getMeshCount()
 * @return that mesh's diffuse texture — the model's own image, a flat color synthesized from
 * its material, or the built-in gray placeholder if it has no material at all. Never nullptr.
 */
Eng::Texture ENG_API *Eng::Loader::getMeshTexture(unsigned int index) const
{
   return reserved->meshTextures[index];
}

/** @return index of the lowest flat mesh found while loading (see Reserved::considerAsGround), or -1. */
int ENG_API Eng::Loader::getGroundMeshIndex() const
{
   return reserved->groundMeshIndex;
}

/** @return the Y coordinate of the ground plane identified by getGroundMeshIndex(). */
float ENG_API Eng::Loader::getGroundPlaneY() const
{
   return reserved->groundPlaneY;
}

/** @return TF -- true and fills r/g/b if this mesh's material is emissive or bright enough to be treated as self-lit. */
bool ENG_API Eng::Loader::getMeshEmission(unsigned int index, float &r, float &g, float &b) const
{
   const glm::vec3 &emission = reserved->meshEmission[index];
   if (emission == glm::vec3(0.0f))
      return false;

   r = emission.r; g = emission.g; b = emission.b;
   return true;
}

/** @return that one mesh's own world-space bounding box (min/max), recorded while loading. */
void ENG_API Eng::Loader::getMeshBounds(unsigned int index, float &minX, float &minY, float &minZ, float &maxX, float &maxY, float &maxZ) const
{
   const glm::vec3 &bmin = reserved->meshBoundsMin[index];
   const glm::vec3 &bmax = reserved->meshBoundsMax[index];
   minX = bmin.x; minY = bmin.y; minZ = bmin.z;
   maxX = bmax.x; maxY = bmax.y; maxZ = bmax.z;
}

/** @return TF -- true and fills every output with the non-ground meshes' combined bounding box. */
bool ENG_API Eng::Loader::getFurnitureTopCenter(float &x, float &y, float &z, float &halfWidthX, float &halfDepthZ) const
{
   return reserved->getFurnitureTopCenter(x, y, z, halfWidthX, halfDepthZ);
}
