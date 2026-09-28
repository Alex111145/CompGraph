/**
 * @file		loader.h
 * @brief	Loads a 3D model file into a Node hierarchy + Mesh list via Assimp
 *
 * @author	Alessio Gervasini
 */
#pragma once

   #include <memory>
   #include <string>

namespace Eng {

class ENG_API Loader final
{
public:

   Loader();
   Loader(Loader const &) = delete;
   ~Loader();

   void operator=(Loader const &) = delete;

   bool load(const std::string &filename);

   unsigned int getMeshCount() const;
   Mesh *getMesh(unsigned int index) const;
   Node *getMeshNode(unsigned int index) const;
   Texture *getMeshTexture(unsigned int index) const;
   bool getMeshEmission(unsigned int index, float &r, float &g, float &b) const;
   void getMeshBounds(unsigned int index, float &minX, float &minY, float &minZ, float &maxX, float &maxY, float &maxZ) const;

   int getGroundMeshIndex() const;
   float getGroundPlaneY() const;
   bool getFurnitureTopCenter(float &x, float &y, float &z) const;

private:

   struct Reserved;
   std::unique_ptr<Reserved> reserved;
};

};
