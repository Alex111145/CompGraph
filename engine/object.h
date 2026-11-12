/**
 * @file		object.h
 * @brief	Base class of every engine object
 *
 * @author	Alessio Gervasini (C) SUPSI
 */
#pragma once



///////////////
// NAMESPACE //
///////////////

namespace Eng {



//////////////////
// CLASS Object //
//////////////////

/**
 * @brief Abstract base class: gives every object a unique id and a name, and declares the render method.
 */
class ENG_API Object
{
//////////
public: //
//////////

   // Const/dest:
   Object(const Object &) = delete;
   virtual ~Object();

   // Operators:
   Object &operator=(const Object &) = delete;

   // Get/set:
   unsigned int getId() const;
   const std::string &getName() const;
   void setName(const std::string &name);

   // Rendering:
   /**
    * Renders the object.
    * @param modelViewMatrix matrix that brings the object coordinates into eye coordinates
    */
   virtual void render(const glm::mat4 &modelViewMatrix) const = 0;


/////////////
protected: //
/////////////

   // Const/dest:
   explicit Object(const std::string &name);


///////////
private: //
///////////

   unsigned int id;              ///< Unique id
   std::string name;             ///< Name
   static unsigned int nextId;   ///< Id given to the next object created
};

}; // end of namespace Eng::
