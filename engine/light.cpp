/**
 * @file		light.cpp
 * @brief	Light source, base class of the light types
 *
 * @author	Alessio Gervasini
 */

   #include "engine.h"

   #include <GL/freeglut.h>

int Eng::Light::lightCount = 0;

/**
 * Constructor. Each light takes the next free OpenGL light (GL_LIGHT0, GL_LIGHT1, ...).
 * @param name light name
 * @param position homogeneous local position (w = 0 directional, w = 1 point)
 * @param color diffuse and specular color
 */
ENG_API Eng::Light::Light(const std::string &name, const glm::vec4 &position, const glm::vec3 &color) :
   Node(name), lightId{ GL_LIGHT0 + lightCount++ }, position{ position }, color{ glm::vec4(color, 1.0f) }, enabled{ true }
{}

/**
 * Destructor.
 */
ENG_API Eng::Light::~Light()
{}

/**
 * Turns the light on or off.
 * @param enabled TF
 */
void ENG_API Eng::Light::setEnabled(bool enabled)
{
   this->enabled = enabled;
}

/**
 * Tells if the light is on.
 * @return TF
 */
bool ENG_API Eng::Light::isEnabled() const
{
   return enabled;
}

/**
 * Gets the homogeneous position in world coordinates (world matrix x position).
 * @return world position (w = 0 for a direction)
 */
glm::vec4 ENG_API Eng::Light::getWorldPosition() const
{
   return getWorldMatrix() * position;
}

/**
 * Sends the light to OpenGL. The position is transformed by the current modelview matrix.
 * @param modelView view matrix x world matrix
 */
void ENG_API Eng::Light::render(const glm::mat4 &modelView)
{
   const glm::vec4 noAmbient(0.0f, 0.0f, 0.0f, 1.0f);

   if (!enabled)
   {
      glDisable(lightId);
      return;
   }

   glLoadMatrixf(glm::value_ptr(modelView));
   glLightfv(lightId, GL_POSITION, glm::value_ptr(position));
   glLightfv(lightId, GL_AMBIENT, glm::value_ptr(noAmbient));
   glLightfv(lightId, GL_DIFFUSE, glm::value_ptr(color));
   glLightfv(lightId, GL_SPECULAR, glm::value_ptr(color));
   glEnable(lightId);
}
