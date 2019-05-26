#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <vector>

/**
	@author Andre Offringa
*/
class Sequence {
public:
	Sequence() = default;
	
	size_t Size() const { return _list.size(); }

	void Add(class Controllable* controllable)
	{
		_list.push_back(controllable);
	}

	const std::vector<class Controllable *>& List() const
	{
		return _list;
	}

	bool IsUsing(class Controllable& object) const
	{
		for(class Controllable* controllable : _list)
			if(controllable == &object) return true;
		return false;
	}
	
private:
	std::vector<class Controllable*> _list;
};

#endif
