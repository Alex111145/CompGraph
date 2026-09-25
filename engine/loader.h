/**
 * @file		loader.h
 * @brief	Loads a 3D model from an external file into a Node hierarchy + Mesh list
 *
 * @author	Alessio Gervasini
 */
#pragma once

   #include <memory>
   #include <string>

namespace Eng {

/**
 * @brief Reads a 3D model file via Assimp (so glTF/.glb, FBX and most other common formats all
 * work through the same code path) and builds a matching Eng::Node hierarchy plus one
 * Eng::Mesh per sub-mesh found in the file, preserving each node's own position/rotation/scale
 * from the source file (a full scene graph, not a flat list). Materials resolve to a texture:
 * the model's own image if its material has one (loaded from an external file, or decoded
 * straight from memory for an embedded .glb texture), else a flat color synthesized from the
 * material's base/diffuse color, else a neutral gray placeholder if the mesh has no material
 * at all. The Loader owns everything it
 * creates — nodes, meshes and textures stay valid as long as the Loader (or whoever it was
 * moved/kept alive by) is alive.
 */
class ENG_API Loader final
{
public:

   Loader();
   Loader(Loader const &) = delete;
   ~Loader();

   void operator=(Loader const &) = delete;

   bool load(const std::string &filename);

   unsigned int getMeshCount() const;
   Mesh *getMesh(unsigned int index) const;
   Node *getMeshNode(unsigned int index) const;
   Texture *getMeshTexture(unsigned int index) const;

   /**
    * Identifies the scene's reflective ground purely from geometry, not from any name in the
    * file: among every mesh whose vertices are all coplanar (flat, within a small epsilon on Y),
    * this returns the one sitting lowest -- the usual place for a floor/ground plane in a Y-up
    * scene. @return mesh index (same numbering as getMesh()), or -1 if no such mesh exists.
    */
   int getGroundMeshIndex() const;

   /** @return the Y coordinate of the plane identified by getGroundMeshIndex(). Only meaningful if that returned >= 0. */
   float getGroundPlaneY() const;

   /**
    * Materials close to white (Kd luminance above ~0.9, like a lamp bulb's) are treated as
    * mildly self-lit -- a common cheap stand-in for a real emission map, and, unlike checking
    * an object's name, works on any material data. @return TF (false = no glow, r/g/b untouched)
    */
   bool getMeshEmission(unsigned int index, float &r, float &g, float &b) const;

   /**
    * World-space axis-aligned bounding box of one mesh alone (not combined with any other) --
    * for placing something relative to a single object found by a geometric trait, e.g. a light
    * at the position of whichever mesh getMeshEmission() flagged as self-lit, without the code
    * ever checking an object's name.
    * @param index 0-based, < getMeshCount()
    */
   void getMeshBounds(unsigned int index, float &minX, float &minY, float &minZ, float &maxX, float &maxY, float &maxZ) const;

   /**
    * Combined bounding box of every mesh EXCEPT the ground (see getGroundMeshIndex()) -- i.e.
    * "wherever the furniture is", found purely from geometry, without the code ever checking
    * an object's name: its top-center point (x/y/z) plus its half-width/half-depth on the
    * ground plane (halfWidthX/halfDepthZ), so a caller can place something above the middle of
    * it or offset towards one of its edges.
    * @return TF (false = nothing but ground in the scene, all outputs untouched)
    */
   bool getFurnitureTopCenter(float &x, float &y, float &z, float &halfWidthX, float &halfDepthZ) const;

private:

   struct Reserved;
   std::unique_ptr<Reserved> reserved;
};

};
