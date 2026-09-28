/**
 * @file		mesh.cpp
 * @brief	Triangle mesh drawn with OpenGL 1.1 vertex arrays
 *
 * @author	Alessio Gervasini
 */

   #include "engine.h"

   #include <vector>

   #include <GLFW/glfw3.h>

struct Eng::Mesh::Reserved
{
   std::vector<Vertex> vertices;
   std::vector<unsigned int> indices;
};

ENG_API Eng::Mesh::Mesh() : reserved(std::make_unique<Eng::Mesh::Reserved>())
{}

ENG_API Eng::Mesh::~Mesh()
{}

void ENG_API Eng::Mesh::setGeometry(const Vertex *vertices, unsigned int vertexCount, const unsigned int *indices, unsigned int indexCount)
{
   reserved->vertices.assign(vertices, vertices + vertexCount);
   reserved->indices.assign(indices, indices + indexCount);
}

void ENG_API Eng::Mesh::render() const
{
   if (reserved->indices.empty())
      return;

   const Vertex *firstVertex = reserved->vertices.data();

   glEnableClientState(GL_VERTEX_ARRAY);
   glEnableClientState(GL_NORMAL_ARRAY);
   glEnableClientState(GL_TEXTURE_COORD_ARRAY);

   glVertexPointer(3, GL_FLOAT, sizeof(Vertex), firstVertex->position);
   glNormalPointer(GL_FLOAT, sizeof(Vertex), firstVertex->normal);
   glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), firstVertex->uv);

   glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(reserved->indices.size()), GL_UNSIGNED_INT, reserved->indices.data());

   glDisableClientState(GL_VERTEX_ARRAY);
   glDisableClientState(GL_NORMAL_ARRAY);
   glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

unsigned int ENG_API Eng::Mesh::getTriangleCount() const
{
   return static_cast<unsigned int>(reserved->indices.size()) / 3;
}
