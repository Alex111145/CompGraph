/**
 * @file		mesh.h
 * @brief	Triangle mesh compiled into an OpenGL display list
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

   void build(const std::vector<glm::vec3> &positions, const std::vector<glm::vec3> &normals, const std::vector<glm::vec2> &uvs);
   void setMaterial(std::shared_ptr<Material> material);
   std::shared_ptr<Material> getMaterial() const;
   unsigned int getTriangleCount() const;

   bool isFlat() const;
   float getWorldMinY() const;
   float getWorldMaxY() const;
   glm::vec3 getWorldCenter() const;

   void render(const glm::mat4 &modelView) override;
   void renderGeometry() const;

private:

   unsigned int displayList;            ///< OpenGL display list with the triangles
   unsigned int triangleCount;          ///< Number of triangles
   glm::vec3 worldMin;                  ///< World-space bounding box minimum
   glm::vec3 worldMax;                  ///< World-space bounding box maximum
   std::shared_ptr<Material> material;  ///< Surface material
};

};
