/**
 * @file		mesh.cpp
 * @brief	Triangle mesh with its material
 *
 * @author	Alessio Gervasini
 */

   #include "engine.h"

   #include <GL/freeglut.h>

namespace
{
   const float FLAT_TOLERANCE = 0.001f;
}

/**
 * Constructor.
 * @param name mesh name
 */
ENG_API Eng::Mesh::Mesh(const std::string &name) :
   Node(name), displayList{ 0 }, triangleCount{ 0 }, localMin{ glm::vec3(0.0f) }, localMax{ glm::vec3(0.0f) }
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
 * computes the bounding box in the mesh coordinates. Nothing happens if there are no triangles.
 * @param positions vertex positions, three per triangle
 * @param normals vertex normals (normalized here)
 * @param texCoords texture coordinates
 */
void ENG_API Eng::Mesh::build(const std::vector<glm::vec3> &positions, const std::vector<glm::vec3> &normals, const std::vector<glm::vec2> &texCoords)
{
   if (positions.empty())
      return;

   triangleCount = static_cast<unsigned int>(positions.size() / 3);
   localMin = positions[0];
   localMax = positions[0];

   displayList = glGenLists(1);
   glNewList(displayList, GL_COMPILE);
   glBegin(GL_TRIANGLES);
   for (size_t i = 0; i < positions.size(); i++)
   {
      const glm::vec3 normal = glm::normalize(normals[i]);

      glNormal3f(normal.x, normal.y, normal.z);
      glTexCoord2f(texCoords[i].x, texCoords[i].y);
      glVertex3f(positions[i].x, positions[i].y, positions[i].z);

      localMin = glm::min(localMin, positions[i]);
      localMax = glm::max(localMax, positions[i]);
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
   const glm::vec2 range = getWorldRangeY();
   return range.y - range.x < FLAT_TOLERANCE;
}

/**
 * Gets the lowest world-space height of the mesh.
 * @return minimum Y
 */
float ENG_API Eng::Mesh::getWorldMinY() const
{
   return getWorldRangeY().x;
}

/**
 * Renders the mesh with its material.
 * @param modelView view matrix x world matrix
 */
void ENG_API Eng::Mesh::render(const glm::mat4 &modelView)
{
   glLoadMatrixf(glm::value_ptr(modelView));
   if (material)
      material->render(modelView);
   glCallList(displayList);
}

/**
 * Draws only the triangles (no material), used for the shadow.
 */
void ENG_API Eng::Mesh::renderGeometry() const
{
   glCallList(displayList);
}

/**
 * Computes the world-space height range of the mesh: the 8 corners of the bounding box are moved
 * with the current world matrix, so the range follows the mesh when it (or a parent) moves.
 * @return minimum Y in x, maximum Y in y
 */
glm::vec2 ENG_API Eng::Mesh::getWorldRangeY() const
{
   const glm::mat4 world = getWorldMatrix();
   glm::vec2 range((world * glm::vec4(localMin, 1.0f)).y);

   for (int corner = 1; corner < 8; corner++)
   {
      const glm::vec3 point((corner & 1) ? localMax.x : localMin.x,
                            (corner & 2) ? localMax.y : localMin.y,
                            (corner & 4) ? localMax.z : localMin.z);
      const float height = (world * glm::vec4(point, 1.0f)).y;

      range = glm::vec2(glm::min(range.x, height), glm::max(range.y, height));
   }
   return range;
}
