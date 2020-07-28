#include "Precomp.h"
#include "Object.h"

using namespace sf;

#if 0
#define DBG_REFCOUNT(fmt, ...) debugPrint(fmt, __VA_ARGS__)
#else
#define DBG_REFCOUNT(fmt, ...) 
#endif

uint Object::s_count = 0;

//--------------------------------------------------------------------------
Object::Object(const sf::String & _name) :
	m_name(_name)
{
	DBG_REFCOUNT("Create Object \"%s\"\n", m_name.toAnsiString().c_str());
	s_count++;
}

//--------------------------------------------------------------------------
Object::~Object()
{
	DBG_REFCOUNT("Delete Object \"%s\"\n", m_name.toAnsiString().c_str());
	s_count--;
}

//--------------------------------------------------------------------------
uint Object::increaseRefCount()
{
	DBG_REFCOUNT("Increase RefCount of Object \"%s\" (RefCount %u => %u)\n", m_name.toAnsiString().c_str(), m_refCount, m_refCount+1);
	return ++m_refCount;
}

//--------------------------------------------------------------------------
void Object::release()
{
	DBG_REFCOUNT("Release Object \"%s\" (RefCount %u => %u)\n", m_name.toAnsiString().c_str(), m_refCount, m_refCount - 1);
	if (--m_refCount == 0)
	{
		delete this;
	}
}