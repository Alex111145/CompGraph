/**
 * @file		light.cpp
 * @brief	Light source (directional or point), color + intensity + placement
 *
 * @author	Alessio Gervasini
 */

   #include "engine.h"

   #include <glm/glm.hpp>

   #include <GLFW/glfw3.h>

struct Eng::Light::Reserved
{
   LightType type;
   glm::vec3 color;
   float intensity;
   glm::vec3 position;
   glm::vec3 direction;
   float constantAttenuation;
   float linearAttenuation;
   float quadraticAttenuation;

   Reserved() :
      type{ LightType::Directional }, color{ 1.0f, 1.0f, 1.0f }, intensity{ 1.0f },
      position{ 0.0f, 0.0f, 0.0f }, direction{ 0.0f, -1.0f, 0.0f },
      constantAttenuation{ 1.0f }, linearAttenuation{ 0.0f }, quadraticAttenuation{ 0.0f }
   {}
};

ENG_API Eng::Light::Light() : reserved(std::make_unique<Eng::Light::Reserved>())
{}

ENG_API Eng::Light::~Light()
{}

void ENG_API Eng::Light::setType(LightType type)
{
   reserved->type = type;
}

void ENG_API Eng::Light::setColor(float r, float g, float b)
{
   reserved->color = glm::vec3(r, g, b);
}

void ENG_API Eng::Light::setIntensity(float intensity)
{
   reserved->intensity = intensity;
}

void ENG_API Eng::Light::setPosition(float x, float y, float z)
{
   reserved->position = glm::vec3(x, y, z);
}

void ENG_API Eng::Light::setDirection(float x, float y, float z)
{
   reserved->direction = glm::normalize(glm::vec3(x, y, z));
}

void ENG_API Eng::Light::setAttenuation(float constant, float linear, float quadratic)
{
   reserved->constantAttenuation = constant;
   reserved->linearAttenuation = linear;
   reserved->quadraticAttenuation = quadratic;
}

void ENG_API Eng::Light::apply(int index) const
{
   const GLenum lightId = GL_LIGHT0 + index;
   const glm::vec3 scaledColor = reserved->color * reserved->intensity;
   const GLfloat diffuseAndSpecular[4] = { scaledColor.r, scaledColor.g, scaledColor.b, 1.0f };
   const GLfloat ambient[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

   glLightfv(lightId, GL_AMBIENT, ambient);
   glLightfv(lightId, GL_DIFFUSE, diffuseAndSpecular);
   glLightfv(lightId, GL_SPECULAR, diffuseAndSpecular);

   if (reserved->type == LightType::Directional)
   {
      const GLfloat towardsLight[4] = { -reserved->direction.x, -reserved->direction.y, -reserved->direction.z, 0.0f };
      glLightfv(lightId, GL_POSITION, towardsLight);
   }
   else
   {
      const GLfloat position[4] = { reserved->position.x, reserved->position.y, reserved->position.z, 1.0f };
      glLightfv(lightId, GL_POSITION, position);
      glLightf(lightId, GL_CONSTANT_ATTENUATION, reserved->constantAttenuation);
      glLightf(lightId, GL_LINEAR_ATTENUATION, reserved->linearAttenuation);
      glLightf(lightId, GL_QUADRATIC_ATTENUATION, reserved->quadraticAttenuation);
   }

   glEnable(lightId);
}
