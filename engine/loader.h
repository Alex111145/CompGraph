/**
 * @file		loader.h
 * @brief	Loads a 3D model file (via Assimp) into a scene graph
 *
 * @author	Alessio Gervasini
 */
#pragma once

namespace Eng {

/**
 * @brief Converts the nodes, meshes, materials and textures of a file into engine objects.
 */
class ENG_API Loader final
{
public:

   static Node *load(const std::string &filename);
};

};
