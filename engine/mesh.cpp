/**
 * @file		mesh.cpp
 * @brief	Triangle mesh compiled into an OpenGL display list
 *
 * @author	Alessio Gervasini
 */

   #include "engine.h"

   #include <GLFW/glfw3.h>

namespace
{
   const float FLAT_TOLERANCE = 0.001f;
}

/**
 * Constructor.
 * @param name mesh name
 */
ENG_API Eng::Mesh::Mesh(const std::string &name) :
   Node(name), displayList{ 0 }, triangleCount{ 0 }, worldMin{ glm::vec3(0.0f) }, worldMax{ glm::vec3(0.0f) }
{}

/**
 * Destructor. Releases the display list.
 */
ENG_API Eng::Mesh::~Mesh()
{
   if (displayList != 0)
      glDeleteLists(displayList, 1);
}

/**
 * Compiles the triangles into a display list (glNewList + glBegin(GL_TRIANGLES)) and
 * computes the world-space bounding box. Call it after the mesh has been added to its parent.
 * @param positions vertex positions, three per triangle
 * @param normals vertex normals (normalized here)
 * @param uvs texture coordinates
 */
void ENG_API Eng::Mesh::build(const std::vector<glm::vec3> &positions, const std::vector<glm::vec3> &normals, const std::vector<glm::vec2> &uvs)
{
   const glm::mat4 world = getWorldMatrix();

   triangleCount = static_cast<unsigned int>(positions.size() / 3);
   worldMin = glm::vec3(world * glm::vec4(positions[0], 1.0f));
   worldMax = worldMin;

   displayList = glGenLists(1);
   glNewList(displayList, GL_COMPILE);
   glBegin(GL_TRIANGLES);
   for (size_t i = 0; i < positions.size(); i++)
   {
      const glm::vec3 normal = glm::normalize(normals[i]);
      const glm::vec3 worldPosition = glm::vec3(world * glm::vec4(positions[i], 1.0f));

      glNormal3f(normal.x, normal.y, normal.z);
      glTexCoord2f(uvs[i].x, uvs[i].y);
      glVertex3f(positions[i].x, positions[i].y, positions[i].z);

      worldMin = glm::min(worldMin, worldPosition);
      worldMax = glm::max(worldMax, worldPosition);
   }
   glEnd();
   glEndList();
}

/**
 * Sets the material.
 * @param material material shared with other meshes
 */
void ENG_API Eng::Mesh::setMaterial(std::shared_ptr<Material> material)
{
   this->material = material;
}

/**
 * Gets the material.
 * @return material (can be empty)
 */
std::shared_ptr<Eng::Material> ENG_API Eng::Mesh::getMaterial() const
{
   return material;
}

/**
 * Gets the number of triangles.
 * @return triangle count
 */
unsigned int ENG_API Eng::Mesh::getTriangleCount() const
{
   return triangleCount;
}

/**
 * Tells if the mesh is a horizontal plane (all vertices at the same world height), like a floor.
 * @return TF
 */
bool ENG_API Eng::Mesh::isFlat() const
{
   return worldMax.y - worldMin.y < FLAT_TOLERANCE;
}

/**
 * Gets the lowest world-space height of the mesh.
 * @return minimum Y
 */
float ENG_API Eng::Mesh::getWorldMinY() const
{
   return worldMin.y;
}

/**
 * Gets the highest world-space height of the mesh.
 * @return maximum Y
 */
float ENG_API Eng::Mesh::getWorldMaxY() const
{
   return worldMax.y;
}

/**
 * Gets the center of the world-space bounding box.
 * @return center point
 */
glm::vec3 ENG_API Eng::Mesh::getWorldCenter() const
{
   return (worldMin + worldMax) * 0.5f;
}

/**
 * Renders the mesh with its material.
 * @param modelView view matrix x world matrix
 */
void ENG_API Eng::Mesh::render(const glm::mat4 &modelView)
{
   glLoadMatrixf(glm::value_ptr(modelView));
   if (material)
      material->apply();
   glCallList(displayList);
}

/**
 * Draws only the triangles (no material), used for the shadow.
 */
void ENG_API Eng::Mesh::renderGeometry() const
{
   glCallList(displayList);
}
