/**
 * @file		mesh.h
 * @brief	Triangle mesh drawn with OpenGL 1.1 vertex arrays
 *
 * @author	Alessio Gervasini
 */
#pragma once

   #include <memory>

namespace Eng {

struct Vertex
{
   float position[3];
   float normal[3];
   float uv[2];
};

class ENG_API Mesh final
{
public:

   Mesh();
   Mesh(Mesh const &) = delete;
   ~Mesh();

   void operator=(Mesh const &) = delete;

   void setGeometry(const Vertex *vertices, unsigned int vertexCount, const unsigned int *indices, unsigned int indexCount);
   void render() const;
   unsigned int getTriangleCount() const;

private:

   struct Reserved;
   std::unique_ptr<Reserved> reserved;
};

};
