/**
 * @file		light.h
 * @brief	Light source (directional or point), color + intensity + placement
 *
 * @author	Alessio Gervasini
 */
#pragma once

   #include <memory>

namespace Eng {

enum class LightType
{
   Directional,
   Point
};

class ENG_API Light final
{
public:

   Light();
   Light(Light const &) = delete;
   ~Light();

   void operator=(Light const &) = delete;

   void setType(LightType type);
   void setColor(float r, float g, float b);
   void setIntensity(float intensity);
   void setPosition(float x, float y, float z);
   void setDirection(float x, float y, float z);
   void setAttenuation(float constant, float linear, float quadratic);

   void apply(int index) const;

private:

   struct Reserved;
   std::unique_ptr<Reserved> reserved;
};

};
