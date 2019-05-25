#ifndef PROPERTY_SET_H
#define PROPERTY_SET_H

#include "property.h"

#include "../../libtheatre/controlvalue.h"
#include "../../libtheatre/folderobject.h"

#include <memory>
#include <vector>
#include <string>

class PropertySet
{
public:
	using iterator = std::vector<Property>::iterator;
	using const_iterator = std::vector<Property>::const_iterator;
	
	iterator begin() { return _properties.begin(); }
	const_iterator begin() const { return _properties.begin(); }
	
	iterator end() { return _properties.end(); }
	const_iterator end() const { return _properties.end(); }
	
	size_t size() const { return _properties.size(); }
	
	Property& operator[](size_t index) { return _properties[index]; }
	const Property& operator[](size_t index) const { return _properties[index]; }
	
	static std::unique_ptr<PropertySet> Make(FolderObject& object);
	
	static std::unique_ptr<PropertySet> Make(const FolderObject& object)
	{ return Make(const_cast<FolderObject&>(object)); }
	
	void SetControlValue(const Property& property, unsigned value) const
	{
		if(value > ControlValue::MaxUInt())
			value = ControlValue::MaxUInt();
		setControlValue(*_object, property._setIndex, value);
	}
	
	unsigned GetControlValue(const Property& property) const
	{
		return getControlValue(*_object, property._setIndex);
	}
	
	void SetDuration(const Property& property, double value) const
	{
		setDuration(*_object, property._setIndex, value);
	}
	
	double GetDuration(const Property& property) const
	{
		return getDuration(*_object, property._setIndex);
	}
	
	void SetBool(const Property& property, double value)
	{
		setBool(*_object, property._setIndex, value);
	}
	
	bool GetBool(const Property& property) const
	{
		return getBool(*_object, property._setIndex);
	}
	
	FolderObject& Object() const {
		return *_object;
	}
	
	Property& GetProperty(const std::string& name)
	{
		for(Property& p : _properties)
		{
			if(p.Name() == name)
				return p;
		}
		throw std::runtime_error("Property not found: " + name);
	}
	
	void AssignProperty(const Property& to, const Property& from, const PropertySet& fromSet);
	
protected:
	virtual void setControlValue(FolderObject& object, size_t index, unsigned value) const
	{ setterNotImplemented(); }
	virtual unsigned getControlValue(const FolderObject& object, size_t index) const
	{ getterNotImplemented(); return 0; }
	
	virtual void setDuration(FolderObject& object, size_t index, double value) const
	{ setterNotImplemented(); }
	virtual double getDuration(const FolderObject& object, size_t index) const
	{ getterNotImplemented(); return 0; }
	
	virtual void setBool(FolderObject& object, size_t index, bool value) const
	{ setterNotImplemented(); }
	virtual bool getBool(const FolderObject& object, size_t index) const
	{ getterNotImplemented(); return 0; }
	
	size_t addProperty(const Property& property)
	{
		_properties.emplace_back(property);
		_properties.back()._setIndex = _properties.size()-1;
		return _properties.back()._setIndex;
	}
	size_t addProperty(Property&& property)
	{
		property._setIndex = _properties.size();
		_properties.emplace_back(std::move(property));
		return _properties.back()._setIndex;
	}
	
private:
	void setterNotImplemented() const
	{
		throw std::runtime_error("A method of the property set was called for which the set method was not implemented");
	}
	void getterNotImplemented() const
	{
		throw std::runtime_error("A method of the property set was called for which the get method was not implemented");
	}
	FolderObject* _object;
	std::vector<Property> _properties;
};

#endif
