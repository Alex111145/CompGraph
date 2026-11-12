/**
 * @file		mesh.h
 * @brief	Mesh (a 3D cube for now)
 *
 * @author	Alessio Gervasini (C) SUPSI
 */
#pragma once



///////////////
// NAMESPACE //
///////////////

namespace Eng {



////////////////
// CLASS Mesh //
////////////////

/**
 * @brief Scene graph node that draws a cube centered at its origin, with one normal per face and a material.
 */
class ENG_API Mesh : public Node
{
//////////
public: //
//////////

   // Const/dest:
   Mesh(const std::string &name, float size);

   // Material:
   void setMaterial(const Material *material);

   // Rendering:
   void render(const glm::mat4 &modelViewMatrix) const override;


///////////
private: //
///////////

   float size;                 ///< Edge length of the cube
   const Material *material;   ///< Material used (nullptr = OpenGL default material)
};

}; // end of namespace Eng::
