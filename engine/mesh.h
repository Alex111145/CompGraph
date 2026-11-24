/**
 * @file		mesh.h
 * @brief	Mesh made of triangles (vertex list + face list)
 *
 * @author	Alessio Gervasini (Master) SUPSI
 */
#pragma once



///////////////
// NAMESPACE //
///////////////

namespace Eng {



////////////////
// CLASS Mesh //
////////////////

/**
 * @brief Scene graph node that draws a triangle mesh with a material. The geometry is stored as in most 3D file formats:
 * a list of vertices (each one defined once, with its position and normal) and a list of faces, where each face is
 * a triangle given by the indices of its 3 vertices.
 */
class ENG_API Mesh : public Node
{
//////////
public: //
//////////

   // Const/dest:
   explicit Mesh(const std::string &name);

   // Geometry:
   void addVertex(const glm::vec3 &position, const glm::vec3 &normal);
   bool addFace(unsigned int vertex0, unsigned int vertex1, unsigned int vertex2);

   // Material:
   void setMaterial(const Material *material);

   // Rendering:
   void render(const glm::mat4 &modelViewMatrix) const override;


///////////
private: //
///////////

   std::vector<glm::vec3> positions;   ///< Vertex list: position of each vertex
   std::vector<glm::vec3> normals;     ///< Normal of each vertex (same index as in positions)
   std::vector<glm::uvec3> faces;      ///< Face list: each face is a triangle, given by the indices of its 3 vertices
   const Material *material;           ///< Material used (nullptr = OpenGL default material)
};

}; // end of namespace Eng::
