#ifndef NAMEDOBJECT_H
#define NAMEDOBJECT_H

#include <memory>
#include <stdexcept>
#include <vector>

#include <sigc++/signal.h>

/**
	@author Andre Offringa
*/
class NamedObject {
public:
	friend class Folder;
	
	NamedObject() : _name()
	{ }
	NamedObject(const std::string& name) : _name(name)
	{ }
	virtual ~NamedObject()
	{
		_signalDelete();
	}

	NamedObject(const NamedObject&) = default;
	NamedObject(NamedObject&&) = default;
	NamedObject& operator=(const NamedObject&) = default;
	NamedObject& operator=(NamedObject&&) = default;
	
	const std::string& Name() const { return _name; }
	void SetName(const std::string& name) { _name = name; }
	
	template<typename NamedObjectType>
	static NamedObjectType& FindNamedObject(const std::vector<std::unique_ptr<NamedObjectType>>& container, const std::string& name)
	{
		for(const std::unique_ptr<NamedObjectType>& obj : container)
		{
			if(obj->_name == name)
				return *obj;
		}
		throw std::runtime_error("Could not find named object " + name + " in container.");
	}
	
	template<typename NamedObjectType>
	static NamedObjectType& FindNamedObject(const std::vector<NamedObjectType*>& container, const std::string& name)
	{
		for(NamedObjectType* obj : container)
		{
			if(obj->_name == name)
				return *obj;
		}
		throw std::runtime_error("Could not find named object " + name + " in container.");
	}
	
	template<typename ObjectType>
	static size_t FindIndex(const std::vector<std::unique_ptr<ObjectType>>& container, const ObjectType* element)
	{
		for(size_t i=0; i!=container.size(); ++i)
		{
			if(container[i].get() == element)
			{
				return i;
			}
		}
		throw std::runtime_error("Could not find object in container.");
	}
	
	template<typename ObjectType>
	static bool Contains(const std::vector<std::unique_ptr<ObjectType>>& container, const ObjectType* element)
	{
		for(const std::unique_ptr<ObjectType>& obj : container)
		{
			if(obj.get() == &element)
				return true;
		}
		return false;
	}
	
	template<typename ObjectType>
	static bool Contains(const std::vector<std::unique_ptr<ObjectType>>& container, const std::string& name)
	{
		for(const std::unique_ptr<ObjectType>& obj : container)
		{
			if(obj->_name == name)
				return true;
		}
		return false;
	}
	
	sigc::signal<void()>& SignalDelete() { return _signalDelete; }
		
private:
	std::string _name;
	sigc::signal<void()> _signalDelete;
};

#endif

