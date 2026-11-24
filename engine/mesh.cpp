/**
 * @file		mesh.cpp
 * @brief	Mesh made of triangles (vertex list + face list)
 *
 * @author	Alessio Gervasini (Master) SUPSI
 */



//////////////
// #INCLUDE //
//////////////

   // Main include:
   #include "engine.h"

   // FreeGLUT:
   #include <GL/freeglut.h>

   // GLM:
   #include <glm/gtc/type_ptr.hpp>

   // C/C++:
   #include <iostream>



////////////////////////
// BODY OF CLASS Mesh //
////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Constructor. The mesh starts empty: no vertices, no faces and no material.
 * @param name mesh name
 */
ENG_API Eng::Mesh::Mesh(const std::string &name) : Node(name), material{ nullptr }
{}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Adds a vertex at the end of the vertex list. Its index is the number of vertices added before it.
 * @param position position of the vertex, in object coordinates
 * @param normal normal of the vertex
 */
void ENG_API Eng::Mesh::addVertex(const glm::vec3 &position, const glm::vec3 &normal)
{
   positions.push_back(position);
   normals.push_back(normal);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Adds a face (a triangle) at the end of the face list. Its vertices are given in counterclockwise order, as seen from
 * the front side of the face.
 * @param vertex0 index of the first vertex
 * @param vertex1 index of the second vertex
 * @param vertex2 index of the third vertex
 * @return TF (false if an index is not in the vertex list)
 */
bool ENG_API Eng::Mesh::addFace(unsigned int vertex0, unsigned int vertex1, unsigned int vertex2)
{
   if (vertex0 >= positions.size() || vertex1 >= positions.size() || vertex2 >= positions.size())
   {
      std::cout << "ERROR: invalid face for mesh " << getName() << std::endl;
      return false;
   }

   faces.push_back(glm::uvec3(vertex0, vertex1, vertex2));
   return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Sets the material.
 * @param material material used to draw the mesh (nullptr = OpenGL default material)
 */
void ENG_API Eng::Mesh::setMaterial(const Material *material)
{
   this->material = material;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Loads the modelview matrix, applies the material and draws the faces. Starting from the vertex list and the face list,
 * the coordinates of each face are rebuilt inside one glBegin(GL_TRIANGLES) / glEnd() block: for each corner of the face,
 * its normal and its position are taken from the vertex list through the index stored in the face.
 * @param modelViewMatrix matrix that brings the object coordinates into eye coordinates
 */
void ENG_API Eng::Mesh::render(const glm::mat4 &modelViewMatrix) const
{
   Node::render(modelViewMatrix);

   if (material != nullptr)
      material->render(modelViewMatrix);

   glBegin(GL_TRIANGLES);
   for (const glm::uvec3 &face : faces)
      for (int corner = 0; corner < 3; corner++)
      {
         glNormal3fv(glm::value_ptr(normals[face[corner]]));
         glVertex3fv(glm::value_ptr(positions[face[corner]]));
      }
   glEnd();
}
