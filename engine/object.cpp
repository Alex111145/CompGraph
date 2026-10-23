/**
 * @file		object.cpp
 * @brief	Base class of every engine object: unique id, name and render method
 *
 * @author	Alessio Gervasini
 */

   #include "engine.h"

unsigned int Eng::Object::idCounter = 1;

/**
 * Constructor. Gives the object the next free identifier.
 * @param name object name
 */
ENG_API Eng::Object::Object(const std::string &name) : id{ idCounter++ }, name{ name }
{}

/**
 * Destructor.
 */
ENG_API Eng::Object::~Object()
{}

/**
 * Gets the unique identifier.
 * @return identifier (never 0)
 */
unsigned int ENG_API Eng::Object::getId() const
{
   return id;
}

/**
 * Gets the object name.
 * @return object name
 */
const std::string ENG_API &Eng::Object::getName() const
{
   return name;
}
