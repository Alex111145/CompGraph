/**
 * @file		material.cpp
 * @brief	Material (emission, ambient, diffuse, specular, shininess)
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

   // GLM:
   #include <glm/gtc/type_ptr.hpp>



////////////////////////////
// BODY OF CLASS Material //
////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Constructor. Default values: no emission, ambient 0.2, diffuse 0.8, specular 0.5, shininess 128.
 * @param name material name
 */
ENG_API Eng::Material::Material(const std::string &name) : Object(name),
   emission{ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) }, ambient{ glm::vec4(0.2f, 0.2f, 0.2f, 1.0f) },
   diffuse{ glm::vec4(0.8f, 0.8f, 0.8f, 1.0f) }, specular{ glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) }, shininess{ 128.0f }
{}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Sets the emissive color.
 * @param emission RGBA color, each component between 0 and 1
 */
void ENG_API Eng::Material::setEmission(const glm::vec4 &emission)
{
   this->emission = glm::clamp(emission, 0.0f, 1.0f);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Sets the ambient color.
 * @param ambient RGBA color, each component between 0 and 1
 */
void ENG_API Eng::Material::setAmbient(const glm::vec4 &ambient)
{
   this->ambient = glm::clamp(ambient, 0.0f, 1.0f);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Sets the diffuse color.
 * @param diffuse RGBA color, each component between 0 and 1
 */
void ENG_API Eng::Material::setDiffuse(const glm::vec4 &diffuse)
{
   this->diffuse = glm::clamp(diffuse, 0.0f, 1.0f);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Sets the specular color.
 * @param specular RGBA color, each component between 0 and 1
 */
void ENG_API Eng::Material::setSpecular(const glm::vec4 &specular)
{
   this->specular = glm::clamp(specular, 0.0f, 1.0f);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Sets the shininess.
 * @param shininess specular exponent, between 0 and 128 (higher = smaller and sharper highlight)
 */
void ENG_API Eng::Material::setShininess(float shininess)
{
   this->shininess = glm::clamp(shininess, 0.0f, 128.0f);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Passes the material properties to OpenGL.
 * @param modelViewMatrix not used: a material has no position
 */
void ENG_API Eng::Material::render(const glm::mat4 &modelViewMatrix) const
{
   glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, glm::value_ptr(emission));
   glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, glm::value_ptr(ambient));
   glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, glm::value_ptr(diffuse));
   glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, glm::value_ptr(specular));
   glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
}
