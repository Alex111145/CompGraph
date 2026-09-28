/**
 * @file		material.cpp
 * @brief	Surface material for the OpenGL lighting model (glMaterial)
 *
 * @author	Alessio Gervasini
 */

   #include "engine.h"

   #include <GLFW/glfw3.h>

/**
 * Constructor. All the colors start as zero vectors (alpha 1).
 */
ENG_API Eng::Material::Material() :
   ambient{ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) }, diffuse{ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) },
   specular{ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) }, emission{ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) },
   shininess{ 0.0f }
{}

/**
 * Destructor.
 */
ENG_API Eng::Material::~Material()
{}

/**
 * Sets the ambient reflectance.
 * @param color RGB color
 */
void ENG_API Eng::Material::setAmbient(const glm::vec3 &color)
{
   ambient = glm::vec4(color, 1.0f);
}

/**
 * Sets the diffuse reflectance.
 * @param color RGB color
 */
void ENG_API Eng::Material::setDiffuse(const glm::vec3 &color)
{
   diffuse = glm::vec4(color, 1.0f);
}

/**
 * Sets the specular reflectance.
 * @param color RGB color
 */
void ENG_API Eng::Material::setSpecular(const glm::vec3 &color)
{
   specular = glm::vec4(color, 1.0f);
}

/**
 * Sets the emitted color (a surface that glows without lights).
 * @param color RGB color
 */
void ENG_API Eng::Material::setEmission(const glm::vec3 &color)
{
   emission = glm::vec4(color, 1.0f);
}

/**
 * Sets the specular exponent, clamped to the OpenGL range [0, 128].
 * @param shininess specular exponent
 */
void ENG_API Eng::Material::setShininess(float shininess)
{
   this->shininess = glm::clamp(shininess, 0.0f, 128.0f);
}

/**
 * Sets the diffuse texture.
 * @param texture texture shared with other materials
 */
void ENG_API Eng::Material::setTexture(std::shared_ptr<Texture> texture)
{
   this->texture = texture;
}

/**
 * Sends the material to OpenGL and binds its texture.
 */
void ENG_API Eng::Material::apply() const
{
   glMaterialfv(GL_FRONT, GL_AMBIENT, glm::value_ptr(ambient));
   glMaterialfv(GL_FRONT, GL_DIFFUSE, glm::value_ptr(diffuse));
   glMaterialfv(GL_FRONT, GL_SPECULAR, glm::value_ptr(specular));
   glMaterialfv(GL_FRONT, GL_EMISSION, glm::value_ptr(emission));
   glMaterialf(GL_FRONT, GL_SHININESS, shininess);

   if (texture)
   {
      glEnable(GL_TEXTURE_2D);
      texture->bind();
   }
   else
      glDisable(GL_TEXTURE_2D);
}
