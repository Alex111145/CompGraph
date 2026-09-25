/**
 * @file		mesh.h
 * @brief	Triangle mesh drawn with OpenGL 1.1 vertex arrays
 *
 * @author	Alessio Gervasini
 */
#pragma once

   #include <memory>

namespace Eng {

/** @brief One vertex: position + normal (for lighting) + UV (for texture mapping). */
struct Vertex
{
   float position[3];
   float normal[3];
   float uv[2];
};

/**
 * @brief Owns a CPU-side copy of a mesh's vertex/index data and knows how to draw itself
 * using OpenGL 1.1 vertex arrays (glVertexPointer/glNormalPointer/glTexCoordPointer +
 * glDrawElements) -- there is no VBO, since buffer objects only arrived in OpenGL 1.5.
 * Geometry is shared: several Eng::Node instances can point at (i.e. call render() on) the
 * same Mesh.
 */
class ENG_API Mesh final
{
public:

   Mesh();
   Mesh(Mesh const &) = delete;
   ~Mesh();

   void operator=(Mesh const &) = delete;

   bool loadFromVertices(const Vertex *vertices, unsigned int vertexCount, const unsigned int *indices, unsigned int indexCount);

   void render() const;

   unsigned int getTriangleCount() const;

private:

   struct Reserved;
   std::unique_ptr<Reserved> reserved;
};

};
