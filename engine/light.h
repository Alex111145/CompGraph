/**
 * @file		light.h
 * @brief	Light source (directional or point), color + intensity + placement
 *
 * @author	Alessio Gervasini
 */
#pragma once

   #include <memory>

namespace Eng {

/**
 * @brief Which physical model this light follows: Directional has no position (parallel
 * rays, like the sun — "static" light of the pair required by the project), Point radiates
 * from a single world-space position in every direction (falls off with distance — used
 * here as the "dynamic" light of the pair).
 */
enum class LightType
{
   Directional,
   Point
};

/**
 * @brief A single light source: type + color + intensity + the position/direction it needs.
 * Under the OpenGL 1.1 fixed-function pipeline there is no shader to push uniforms into,
 * so Eng::Light applies itself directly to one of the 8 fixed-function lights (GL_LIGHT0..7)
 * via apply(). Ambient is left at zero here (the scene's global ambient is set once through
 * glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ...) by the client) and intensity simply scales the
 * diffuse/specular color, matching the "color = intensity * base color" model from the slides.
 */
class ENG_API Light final
{
public:

   Light();
   Light(Light const &) = delete;
   ~Light();

   void operator=(Light const &) = delete;

   void setType(LightType type);

   void setColor(float r, float g, float b);
   void getColor(float &r, float &g, float &b) const;
   void setIntensity(float intensity);

   void setPosition(float x, float y, float z);
   void setDirection(float x, float y, float z);

   /**
    * Point-light falloff (no effect on a Directional light): 1/(constant + linear*d +
    * quadratic*d^2). Defaults (1, 0, 0) mean no falloff at all -- the caller should scale
    * linear/quadratic to the scene's own size, since a light bright enough for a large room
    * would blow out a small one at the same distance.
    */
   void setAttenuation(float constant, float linear, float quadratic);

   /**
    * Enables GL_LIGHTn (n = index, 0..7) and uploads this light's diffuse/specular color and
    * GL_POSITION to it. Call after the modelview matrix is set to the current camera's view,
    * so the position/direction ends up correctly transformed into eye space by OpenGL itself.
    */
   void apply(int index) const;

private:

   struct Reserved;
   std::unique_ptr<Reserved> reserved;
};

};
