/**
 * @file		light.cpp
 * @brief	Light source (directional, omnidirectional or spot)
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

   // C/C++:
   #include <iostream>



/////////////////////////
// BODY OF CLASS Light //
/////////////////////////

int Eng::Light::nextLightNumber = 0;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Constructor. Takes the next free OpenGL light (at most 8: GL_LIGHT0 to GL_LIGHT7).
 * Default colors: ambient black, diffuse and specular white; spot cutoff 30 degrees.
 * @param name light name
 * @param type type of light
 */
ENG_API Eng::Light::Light(const std::string &name, Type type) : Node(name), type{ type }, lightNumber{ -1 },
   ambient{ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) }, diffuse{ glm::vec4(1.0f) }, specular{ glm::vec4(1.0f) }, cutoff{ 30.0f }
{
   if (nextLightNumber >= 8)
   {
      std::cout << "ERROR: too many lights, light " << name << " will not be used" << std::endl;
      return;
   }

   lightNumber = nextLightNumber++;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Sets the ambient color.
 * @param ambient RGBA color, each component between 0 and 1
 */
void ENG_API Eng::Light::setAmbient(const glm::vec4 &ambient)
{
   this->ambient = glm::clamp(ambient, 0.0f, 1.0f);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Sets the diffuse color.
 * @param diffuse RGBA color, each component between 0 and 1
 */
void ENG_API Eng::Light::setDiffuse(const glm::vec4 &diffuse)
{
   this->diffuse = glm::clamp(diffuse, 0.0f, 1.0f);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Sets the specular color.
 * @param specular RGBA color, each component between 0 and 1
 */
void ENG_API Eng::Light::setSpecular(const glm::vec4 &specular)
{
   this->specular = glm::clamp(specular, 0.0f, 1.0f);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Sets the half angle of the cone of a spot light.
 * @param cutoff angle in degrees, between 0 and 90
 * @return TF (false if the light is not a spot light or the angle is not valid)
 */
bool ENG_API Eng::Light::setCutoff(float cutoff)
{
   if (type != Type::SPOT || cutoff < 0.0f || cutoff > 90.0f)
   {
      std::cout << "ERROR: invalid cutoff for light " << getName() << std::endl;
      return false;
   }

   this->cutoff = cutoff;
   return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Loads the modelview matrix and passes the light settings to OpenGL, then switches the light on.
 * Directional: position with w = 0. Omni: position with w = 1 and cutoff 180 (special value). Spot: w = 1 and its cutoff.
 * @param modelViewMatrix matrix that brings the light coordinates into eye coordinates
 */
void ENG_API Eng::Light::render(const glm::mat4 &modelViewMatrix) const
{
   if (lightNumber < 0)
      return;

   Node::render(modelViewMatrix);

   GLenum light = GL_LIGHT0 + lightNumber;
   glm::vec4 position = type == Type::DIRECTIONAL ? glm::vec4(0.0f, 0.0f, 1.0f, 0.0f) : glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
   glm::vec3 direction(0.0f, 0.0f, -1.0f);
   float lightCutoff = type == Type::SPOT ? cutoff : 180.0f;

   glLightfv(light, GL_AMBIENT, glm::value_ptr(ambient));
   glLightfv(light, GL_DIFFUSE, glm::value_ptr(diffuse));
   glLightfv(light, GL_SPECULAR, glm::value_ptr(specular));
   glLightfv(light, GL_POSITION, glm::value_ptr(position));
   glLightfv(light, GL_SPOT_DIRECTION, glm::value_ptr(direction));
   glLightf(light, GL_SPOT_CUTOFF, lightCutoff);
   glEnable(light);
}
