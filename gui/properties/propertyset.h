#ifndef PROPERTY_SET_H
#define PROPERTY_SET_H

#include "property.h"

#include "../../libtheatre/controlvalue.h"
#include "../../libtheatre/namedobject.h"

#include <memory>
#include <vector>

class PropertySet
{
public:
	using iterator = std::vector<Property>::iterator;
	using const_iterator = std::vector<Property>::const_iterator;
	
	iterator begin() { return _properties.begin(); }
	const_iterator begin() const { return _properties.begin(); }
	
	static std::unique_ptr<PropertySet> Make(const Effect& object);
	
	void SetControlValue(NamedObject& object, const Property& property, double value) const
	{
		if(value < 0.0)
			value = 0.0;
		unsigned uv = value*ControlValue::MaxUInt()/100.0;
		if(uv > ControlValue::MaxUInt())
			uv = ControlValue::MaxUInt();
		setControlValue(object, property._setIndex, uv);
	}
	
	double GetControlValue(const NamedObject& object, const Property& property) const
	{
		unsigned uv = getControlValue(object, property._setIndex);
		return 100.0*uv/ControlValue::MaxUInt();
	}
	
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
	std::vector<Property> _properties;
};

#endif
