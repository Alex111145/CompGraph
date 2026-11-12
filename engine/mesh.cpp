/**
 * @file		mesh.cpp
 * @brief	Mesh (a 3D cube for now)
 *
 * @author	Alessio Gervasini (C) SUPSI
 */



//////////////
// #INCLUDE //
//////////////

   // Main include:
   #include "engine.h"

   // FreeGLUT:
   #include <GL/freeglut.h>



////////////////////////
// BODY OF CLASS Mesh //
////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Constructor.
 * @param name mesh name
 * @param size edge length of the cube
 */
ENG_API Eng::Mesh::Mesh(const std::string &name, float size) : Node(name), size{ size }, material{ nullptr }
{}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Sets the material.
 * @param material material used to draw the cube (nullptr = OpenGL default material)
 */
void ENG_API Eng::Mesh::setMaterial(const Material *material)
{
   this->material = material;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Loads the modelview matrix, applies the material and draws the 6 faces of the cube.
 * Each face has its normal and its 4 vertices in counterclockwise order, as seen from outside.
 * @param modelViewMatrix matrix that brings the object coordinates into eye coordinates
 */
void ENG_API Eng::Mesh::render(const glm::mat4 &modelViewMatrix) const
{
   Node::render(modelViewMatrix);

   if (material != nullptr)
      material->render(modelViewMatrix);

   float h = size / 2.0f;
   glBegin(GL_QUADS);
   glNormal3f(0.0f, 0.0f, 1.0f);
   glVertex3f(-h, -h, h); glVertex3f(h, -h, h); glVertex3f(h, h, h); glVertex3f(-h, h, h);
   glNormal3f(0.0f, 0.0f, -1.0f);
   glVertex3f(h, -h, -h); glVertex3f(-h, -h, -h); glVertex3f(-h, h, -h); glVertex3f(h, h, -h);
   glNormal3f(1.0f, 0.0f, 0.0f);
   glVertex3f(h, -h, h); glVertex3f(h, -h, -h); glVertex3f(h, h, -h); glVertex3f(h, h, h);
   glNormal3f(-1.0f, 0.0f, 0.0f);
   glVertex3f(-h, -h, -h); glVertex3f(-h, -h, h); glVertex3f(-h, h, h); glVertex3f(-h, h, -h);
   glNormal3f(0.0f, 1.0f, 0.0f);
   glVertex3f(-h, h, h); glVertex3f(h, h, h); glVertex3f(h, h, -h); glVertex3f(-h, h, -h);
   glNormal3f(0.0f, -1.0f, 0.0f);
   glVertex3f(-h, -h, -h); glVertex3f(h, -h, -h); glVertex3f(h, -h, h); glVertex3f(-h, -h, h);
   glEnd();
}
