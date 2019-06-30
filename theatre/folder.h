#ifndef FOLDER_H
#define FOLDER_H

#include <algorithm>
#include <string>
#include <vector>

#include "folderobject.h"

class Folder : public FolderObject
{
public:
	Folder() : FolderObject() { }
	
	Folder(const std::string& name) : FolderObject(name) { }
	
	Folder* CopyHierarchy(std::vector<std::unique_ptr<Folder>>& newFolders) const
	{
		newFolders.emplace_back(new Folder(_name));
		Folder* copy = newFolders.back().get();
		for(const FolderObject* object : _objects)
		{
			const Folder* child = dynamic_cast<const Folder*>(object);
			if(child)
			{
				copy->_objects.emplace_back(child->CopyHierarchy(newFolders));
				copy->_objects.back()->SetParent(*copy);
			}
		}
		return copy;
	}
	
	static void Move(FolderObject& object, Folder& destination)
	{
		if(&destination != &object.Parent())
		{
			if(destination.GetChildIfExists(object.Name()))
				throw std::runtime_error("Can't move object: the destination folder already contains an object with the same name");
			object._parent->Remove(object);
			
			destination._objects.emplace_back(&object);
			object._parent = &destination;
		}
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
	
	static std::string RemoveRoot(const std::string& path)
	{
		auto separator = std::find(path.begin(), path.end(), '/');
		if(separator == path.end())
			return std::string();
		else
		{
			return path.substr(separator-path.begin()+1);
		}
	}
	
	static std::string RemoveRoot(std::string&& path)
	{
		auto separator = std::find(path.begin(), path.end(), '/');
		if(separator == path.end())
			return std::string();
		else
		{
			return std::move(path).substr(separator-path.begin()+1);
		}
	}
	
	/**
	 * This also sets the parent of the object to this.
	 */
	void Add(FolderObject& object)
	{
		if(GetChildIfExists(object.Name()))
			throw std::runtime_error("Trying to add object " + object.Name() + " to folder " + Name() + ", but an object with that name already exists in this folder");
		_objects.emplace_back(&object);
		_objects.back()->SetParent(*this);
	}
	
	void Remove(FolderObject& object)
	{
		std::vector<FolderObject*>::iterator srciter = std::find(_objects.begin(), _objects.end(), &object);
		_objects.erase(srciter);
	}
	
	void MoveUp(FolderObject& object)
	{
		std::vector<FolderObject*>::iterator srciter = std::find(_objects.begin(), _objects.end(), &object);
		if(srciter != _objects.begin() && srciter != _objects.end())
		{
			std::vector<FolderObject*>::iterator previous = srciter;
			--previous;
			std::swap(*previous, *srciter);
		}
	}
	
	void MoveDown(FolderObject& object)
	{
		std::vector<FolderObject*>::iterator srciter = std::find(_objects.begin(), _objects.end(), &object);
		if(srciter != _objects.end())
		{
			std::vector<FolderObject*>::iterator next = srciter;
			++next;
			if(_objects.end() != next)
				std::swap(*next, *srciter);
		}
	}
	
	const std::vector<FolderObject*> Children() const { return _objects; }
	
	Folder* FollowDown(const std::string& path)
	{
		if(path.empty())
			return this;
		else
			return followDown(path, 0);
	}
	
	Folder* FollowDown(std::string&& path)
	{
		if(path.empty())
			return this;
		else
			return followDown(std::move(path), 0);
	}
	
	FolderObject& GetChild(const std::string& name)
	{
		return FindNamedObject(_objects, name);
	}
	
	FolderObject* GetChildIfExists(const std::string& name)
	{
		return FindNamedObjectIfExists(_objects, name);
	}
	
	const FolderObject& GetChild(const std::string& name) const
	{
		return FindNamedObject(_objects, name);
	}
	
	const FolderObject* GetChildIfExists(const std::string& name) const
	{
		return FindNamedObjectIfExists(_objects, name);
	}
	
	FolderObject* FollowRelPath(const std::string& path)
	{
		Folder* parentFolder = FollowDown(ParentPath(path));
		if(parentFolder)
			return parentFolder->GetChildIfExists(LastName(path));
		else
			return nullptr;
	}
	
	FolderObject* FollowRelPath(std::string&& path) 
	{
		Folder* parentFolder = FollowDown(ParentPath(path));
		if(parentFolder)
			return parentFolder->GetChildIfExists(LastName(std::move(path)));
		else
			return nullptr;
	}

private:
	Folder* followDown(const std::string& path, size_t strPos) const
	{
		auto sep = std::find(path.begin()+strPos, path.end(), '/');
		std::string subpath;
		if(sep == path.end())
		{
			FolderObject* obj = FindNamedObjectIfExists(_objects, path.substr(strPos));
			return dynamic_cast<Folder*>(obj);
		}
		else {
			FolderObject* obj = FindNamedObjectIfExists(_objects, path.substr(strPos, sep-path.begin()));
			Folder* folder = dynamic_cast<Folder*>(obj);
			if(folder)
				return folder->followDown(path, sep+1-path.begin());
			else
				return nullptr;
		}
	}
	
	Folder* followDown(std::string&& path, size_t strPos) const
	{
		auto sep = std::find(path.begin()+strPos, path.end(), '/');
		std::string subpath;
		if(sep == path.end())
		{
			FolderObject* obj = FindNamedObjectIfExists(_objects, std::move(path).substr(strPos));
			return dynamic_cast<Folder*>(obj);
		}
		else {
			FolderObject* obj = FindNamedObjectIfExists(_objects, path.substr(strPos, sep-path.begin()));
			Folder* folder = dynamic_cast<Folder*>(obj);
			if(folder)
				return folder->followDown(std::move(path), sep+1-path.begin());
			else
				return nullptr;
		}
	}
	
	std::vector<FolderObject*> _objects;
};

#endif
