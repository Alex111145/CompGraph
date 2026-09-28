/**
 * @file		material.h
 * @brief	Surface material for the OpenGL lighting model (glMaterial)
 *
 * @author	Alessio Gervasini
 */
#pragma once

namespace Eng {

/**
 * @brief Ambient/diffuse/specular/emission colors, shininess and optional texture.
 */
class ENG_API Material final
{
public:

   Material();
   Material(Material const &) = delete;
   ~Material();

   void operator=(Material const &) = delete;

   void setAmbient(const glm::vec3 &color);
   void setDiffuse(const glm::vec3 &color);
   void setSpecular(const glm::vec3 &color);
   void setEmission(const glm::vec3 &color);
   glm::vec3 getEmission() const;
   void setShininess(float shininess);
   void setTexture(std::shared_ptr<Texture> texture);

   void apply() const;

private:

   glm::vec4 ambient;      ///< Ambient reflectance
   glm::vec4 diffuse;      ///< Diffuse reflectance
   glm::vec4 specular;     ///< Specular reflectance
   glm::vec4 emission;     ///< Emitted color
   float shininess;        ///< Specular exponent [0, 128]
   std::shared_ptr<Texture> texture;   ///< Diffuse texture (can be empty)
};

};
