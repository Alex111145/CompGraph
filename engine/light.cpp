/**
 * @file		light.cpp
 * @brief	Light source (directional or point), color + intensity + placement
 *
 * @author	Alessio Gervasini
 */

   #include "engine.h"

   #include <glm/glm.hpp>

   #include <GLFW/glfw3.h>

/**
 * @brief Light class reserved structure (PIMPL/Bridge pattern, same as Eng::Base).
 */
struct Eng::Light::Reserved
{
   LightType type;
   glm::vec3 color;
   float intensity;
   glm::vec3 position;
   glm::vec3 direction;
   float attenConstant, attenLinear, attenQuadratic;

   Reserved() :
      type{ LightType::Directional }, color{ 1.0f, 1.0f, 1.0f }, intensity{ 1.0f },
      position{ 0.0f, 0.0f, 0.0f }, direction{ 0.0f, -1.0f, 0.0f },
      attenConstant{ 1.0f }, attenLinear{ 0.0f }, attenQuadratic{ 0.0f }
   {}
};

/** Constructor. */
ENG_API Eng::Light::Light() : reserved(std::make_unique<Eng::Light::Reserved>())
{}

/** Destructor. */
ENG_API Eng::Light::~Light()
{}

/** Sets whether this light is Directional (parallel rays) or Point (radiates from a position). */
void ENG_API Eng::Light::setType(LightType type)
{
   reserved->type = type;
}

/** Sets the light's color (left unclamped, so intensity can still overbright it above 1.0). */
void ENG_API Eng::Light::setColor(float r, float g, float b)
{
   reserved->color = glm::vec3(r, g, b);
}

/** Reads back the light's color. */
void ENG_API Eng::Light::getColor(float &r, float &g, float &b) const
{
   r = reserved->color.r;
   g = reserved->color.g;
   b = reserved->color.b;
}

/** Sets the light's brightness multiplier (1.0 = color used as-is). */
void ENG_API Eng::Light::setIntensity(float intensity)
{
   reserved->intensity = intensity;
}

/** Sets the position a Point light radiates from (no effect on a Directional light). */
void ENG_API Eng::Light::setPosition(float x, float y, float z)
{
   reserved->position = glm::vec3(x, y, z);
}

/** Sets a Directional light's ray direction, light->scene (no effect on a Point light). */
void ENG_API Eng::Light::setDirection(float x, float y, float z)
{
   reserved->direction = glm::normalize(glm::vec3(x, y, z));
}

/** Sets a Point light's distance falloff coefficients (no effect on a Directional light). */
void ENG_API Eng::Light::setAttenuation(float constant, float linear, float quadratic)
{
   reserved->attenConstant = constant;
   reserved->attenLinear = linear;
   reserved->attenQuadratic = quadratic;
}

/**
 * Enables GL_LIGHTn and uploads its color/position, following the fixed-function model from
 * the slides: w=0 marks a Directional light (GL_POSITION then holds a direction, not a point,
 * and the light is treated as infinitely far away), w=1 marks a Point light (GL_POSITION holds
 * an actual world-space point). Diffuse and specular both carry intensity * color, per the
 * "color = intensity * base color" rule; ambient is left at (0,0,0) since only one global
 * ambient term is used for the whole scene.
 */
void ENG_API Eng::Light::apply(int index) const
{
   const GLenum glLight = GL_LIGHT0 + index;

   const glm::vec3 scaledColor = reserved->color * reserved->intensity;
   const GLfloat diffuseAndSpecular[4] = { scaledColor.r, scaledColor.g, scaledColor.b, 1.0f };
   const GLfloat ambient[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

   glLightfv(glLight, GL_AMBIENT, ambient);
   glLightfv(glLight, GL_DIFFUSE, diffuseAndSpecular);
   glLightfv(glLight, GL_SPECULAR, diffuseAndSpecular);

   if (reserved->type == LightType::Directional)
   {
      // w = 0: OpenGL treats (x,y,z) as a direction the light comes FROM, at infinite distance.
      const GLfloat position[4] = { -reserved->direction.x, -reserved->direction.y, -reserved->direction.z, 0.0f };
      glLightfv(glLight, GL_POSITION, position);
   }
   else
   {
      // w = 1: OpenGL treats (x,y,z) as an actual world-space point.
      const GLfloat position[4] = { reserved->position.x, reserved->position.y, reserved->position.z, 1.0f };
      glLightfv(glLight, GL_POSITION, position);
      glLightf(glLight, GL_CONSTANT_ATTENUATION, reserved->attenConstant);
      glLightf(glLight, GL_LINEAR_ATTENUATION, reserved->attenLinear);
      glLightf(glLight, GL_QUADRATIC_ATTENUATION, reserved->attenQuadratic);
   }

   glEnable(glLight);
}
