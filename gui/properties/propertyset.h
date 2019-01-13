#ifndef PROPERTY_SET_H
#define PROPERTY_SET_H

#include "property.h"

#include "../../libtheatre/controlvalue.h"
#include "../../libtheatre/namedobject.h"

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
	
	static std::unique_ptr<PropertySet> Make(NamedObject& object);
	
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
	
	std::string GetTypeDescription() const;
	
protected:
	virtual void setControlValue(NamedObject& object, size_t index, unsigned value) const
	{ setterNotImplemented(); }
	virtual unsigned getControlValue(const NamedObject& object, size_t index) const
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
	NamedObject* _object;
	std::vector<Property> _properties;
};

#endif
