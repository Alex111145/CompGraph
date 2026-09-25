/**
 * @file		mesh.cpp
 * @brief	Triangle mesh drawn with OpenGL 1.1 vertex arrays
 *
 * @author	Alessio Gervasini
 */

   #include "engine.h"

   #include <vector>

   #include <GLFW/glfw3.h>

/** @brief Mesh class reserved structure (PIMPL/Bridge pattern, same as Eng::Base). */
struct Eng::Mesh::Reserved
{
   std::vector<Vertex> vertices;
   std::vector<unsigned int> indices;

   Reserved() = default;
};

/** Constructor: no GPU resources to allocate -- OpenGL 1.1 vertex arrays are client-side. */
ENG_API Eng::Mesh::Mesh() : reserved(std::make_unique<Eng::Mesh::Reserved>())
{}

/** Destructor: nothing to free explicitly, the vectors clean up themselves. */
ENG_API Eng::Mesh::~Mesh()
{}

/**
 * Copies vertex/index data into this mesh's CPU-side storage, ready for render().
 * @return TF
 */
bool ENG_API Eng::Mesh::loadFromVertices(const Vertex *vertices, unsigned int vertexCount, const unsigned int *indices, unsigned int indexCount)
{
   reserved->vertices.assign(vertices, vertices + vertexCount);
   reserved->indices.assign(indices, indices + indexCount);
   return true;
}

/**
 * Draws the mesh via OpenGL 1.1 vertex arrays: point the fixed-function pipeline at this
 * mesh's own arrays (glVertexPointer/glNormalPointer/glTexCoordPointer, all core in 1.1,
 * see Appendix C of the Superbible), then issue one indexed draw call.
 */
void ENG_API Eng::Mesh::render() const
{
   if (reserved->indices.empty())
      return;

   glEnableClientState(GL_VERTEX_ARRAY);
   glEnableClientState(GL_NORMAL_ARRAY);
   glEnableClientState(GL_TEXTURE_COORD_ARRAY);

   const Vertex *base = reserved->vertices.data();
   glVertexPointer(3, GL_FLOAT, sizeof(Vertex), &base->position);
   glNormalPointer(GL_FLOAT, sizeof(Vertex), &base->normal);
   glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &base->uv);

   glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(reserved->indices.size()), GL_UNSIGNED_INT, reserved->indices.data());

   glDisableClientState(GL_VERTEX_ARRAY);
   glDisableClientState(GL_NORMAL_ARRAY);
   glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

/** @return triangle count (index count / 3 -- indices are always a GL_TRIANGLES list, see loadFromVertices). */
unsigned int ENG_API Eng::Mesh::getTriangleCount() const
{
   return static_cast<unsigned int>(reserved->indices.size()) / 3;
}
