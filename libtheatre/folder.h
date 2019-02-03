#ifndef FOLDER_H
#define FOLDER_H

#include <algorithm>
#include <string>
#include <vector>

#include "namedobject.h"

class Folder : public NamedObject
{
public:
	Folder() : NamedObject() { }
	
	Folder(const std::string& name) : NamedObject(name) { }
	
	static void Move(NamedObject& object, Folder& destination)
	{
		object._parent->Remove(object);
		
		destination._objects.emplace_back(&object);
		object._parent = &destination;
	}
	
	static std::string ParentPath(const std::string& path)
	{
		auto separator = std::find(path.rbegin(), path.rend(), '/');
		if(separator == path.rend())
			return std::string();
		else
		{
			size_t nChar = path.size() - (separator - path.rbegin()) - 1;
			return path.substr(0, nChar);
		}
	}
	
	static std::string ParentPath(std::string&& path)
	{
		auto separator = std::find(path.rbegin(), path.rend(), '/');
		if(separator == path.rend())
			return std::string();
		else
		{
			size_t nChar = path.size() - (separator - path.rbegin()) - 1;
			path.resize(nChar);
			return std::move(path);
		}
	}
	
	static std::string LastName(const std::string& path)
	{
		auto separator = std::find(path.rbegin(), path.rend(), '/');
		if(separator == path.rend())
			return path;
		else
		{
			size_t sepIndex = path.size() - (separator - path.rbegin());
			return path.substr(sepIndex);
		}
	}
	
	static std::string LastName(std::string&& path)
	{
		auto separator = std::find(path.rbegin(), path.rend(), '/');
		if(separator == path.rend())
			return std::move(path);
		else
		{
			size_t sepIndex = path.size() - (separator - path.rbegin());
			return std::move(path).substr(sepIndex);
		}
	}
	
	/**
	 * This also sets the parent of the object to this.
	 */
	void Add(NamedObject& object)
	{
		_objects.emplace_back(&object);
		_objects.back()->SetParent(*this);
	}
	
	void Remove(NamedObject& object)
	{
		std::vector<NamedObject*>::iterator srciter = std::find(_objects.begin(), _objects.end(), &object);
		_objects.erase(srciter);
	}
	
	const std::vector<NamedObject*> Children() const { return _objects; }
	
	Folder& FollowDown(const std::string& path)
	{
		if(path.empty())
			return *this;
		else
			return *followDown(path, 0);
	}
	
	Folder& FollowDown(std::string&& path)
	{
		if(path.empty())
			return *this;
		else
			return *followDown(std::move(path), 0);
	}
	
	NamedObject& GetChild(const std::string& name)
	{
		return FindNamedObject(_objects, name);
	}
	
	NamedObject& FollowRelPath(const std::string& path)
	{
		Folder& parentFolder = FollowDown(ParentPath(path));
		return parentFolder.GetChild(LastName(path));
	}
	
	NamedObject& FollowRelPath(std::string&& path)
	{
		Folder& parentFolder = FollowDown(ParentPath(path));
		return parentFolder.GetChild(LastName(std::move(path)));
	}

private:
	Folder* followDown(const std::string& path, size_t strPos)
	{
		auto sep = std::find(path.begin()+strPos, path.end(), '/');
		std::string subpath;
		if(sep == path.end())
		{
			NamedObject& obj = FindNamedObject(_objects, path.substr(strPos));
			return &static_cast<Folder&>(obj);
		}
		else {
			NamedObject& obj = FindNamedObject(_objects, path.substr(strPos, sep-path.begin()));
			return static_cast<Folder&>(obj).followDown(path, sep+1-path.begin());
		}
	}
	
	Folder* followDown(std::string&& path, size_t strPos)
	{
		auto sep = std::find(path.begin()+strPos, path.end(), '/');
		std::string subpath;
		if(sep == path.end())
		{
			NamedObject& obj = FindNamedObject(_objects, std::move(path).substr(strPos));
			return &static_cast<Folder&>(obj);
		}
		else {
			NamedObject& obj = FindNamedObject(_objects, path.substr(strPos, sep-path.begin()));
			return static_cast<Folder&>(obj).followDown(std::move(path), sep+1-path.begin());
		}
	}
	
	std::vector<NamedObject*> _objects;
};

#endif
