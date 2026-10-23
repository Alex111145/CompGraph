/**
 * @file		object.h
 * @brief	Base class of every engine object: unique id, name and render method
 *
 * @author	Alessio Gervasini
 */
#pragma once

namespace Eng {

/**
 * @brief Abstract base class of everything the engine can render (nodes, materials, textures, lists).
 */
class ENG_API Object
{
public:

   Object(const std::string &name);
   Object(Object const &) = delete;
   virtual ~Object();

   void operator=(Object const &) = delete;

   unsigned int getId() const;
   const std::string &getName() const;

   virtual void render(const glm::mat4 &modelView) = 0;

protected:

   unsigned int id;     ///< Unique identifier
   std::string name;    ///< Object name (as found in the file)

   static unsigned int idCounter;   ///< Next free identifier
};

};
