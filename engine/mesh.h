/**
 * @file		mesh.h
 * @brief	Triangle mesh with its material
 *
 * @author	Alessio Gervasini
 */
#pragma once

namespace Eng {

/**
 * @brief Scene-graph node holding triangles (positions, normals, texture coordinates) and a material.
 */
class ENG_API Mesh final : public Node
{
public:

   Mesh(const std::string &name);
   ~Mesh();

   void build(const std::vector<glm::vec3> &positions, const std::vector<glm::vec3> &normals, const std::vector<glm::vec2> &texCoords);
   void setMaterial(std::shared_ptr<Material> material);
   std::shared_ptr<Material> getMaterial() const;
   unsigned int getTriangleCount() const;

   bool isFlat() const;
   float getWorldMinY() const;

   void render(const glm::mat4 &modelView) override;

private:

   friend class List;   ///< The render list draws the bare triangles of the shadow

   void renderGeometry() const;
   glm::vec2 getWorldRangeY() const;

   unsigned int displayList;            ///< Compiled triangles (internal handle)
   unsigned int triangleCount;          ///< Number of triangles
   glm::vec3 localMin;                  ///< Bounding box minimum, in the mesh coordinates
   glm::vec3 localMax;                  ///< Bounding box maximum, in the mesh coordinates
   std::shared_ptr<Material> material;  ///< Surface material
};

};
