#include "folderobject.h"
#include "folder.h"

#include <sstream>

std::string FolderObject::FullPath() const
{
	// This is more easy to do recursively (basically with
	// return Name + "/" + Parent()->FullPath() ), but I've
	// tried to avoid recursion. I'm actually not sure below
	// is faster though.
	std::vector<const std::string*> list;
	const FolderObject* obj = this;
	do {
		list.emplace_back(&obj->Name());
		obj = obj->_parent;
	} while(obj != nullptr);
	std::ostringstream str;
	auto iter = list.rbegin();
	if(iter != list.rend())
	{
		str << **iter;
		++iter;
		while(iter != list.rend())
		{
			str << '/' << **iter;
			++iter;
		}
	}
	return str.str();
}
