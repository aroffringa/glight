#ifndef NAMEDOBJECT_H
#define NAMEDOBJECT_H

#include <memory>
#include <stdexcept>
#include <vector>

/**
	@author Andre Offringa
*/
class NamedObject {
	public:
		NamedObject() = default;
		NamedObject(const std::string &name) : _name(name)
		{ }

		const std::string& Name() const { return _name; }
		void SetName(const std::string &name) { _name = name; }

		template<typename NamedObjectType>
		static NamedObjectType& FindNamedObject(const std::vector<std::unique_ptr<NamedObjectType>>& container, const std::string &name)
		{
			for(const std::unique_ptr<NamedObjectType>& obj : container)
			{
				if(obj->_name == name)
					return *obj;
			}
			throw std::runtime_error(std::string("Could not find named object ") + name + " in container.");
		}
	private:
		std::string _name;
};

#endif
