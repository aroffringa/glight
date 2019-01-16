#include "management.h"

#include "../beatfinder.h"

#include "chase.h"
#include "controllable.h"
#include "dmxdevice.h"
#include "dummydevice.h"
#include "effect.h"
#include "effectcontrol.h"
#include "fixturefunctioncontrol.h"
#include "presetcollection.h"
#include "presetvalue.h"
#include "sequence.h"
#include "show.h"
#include "theatre.h"
#include "valuesnapshot.h"

Management::Management() : 
	_thread(),
	_isQuitting(false),
	_createTime(boost::posix_time::microsec_clock::local_time()),
	_nextPresetValueId(1),
	_theatre(new class Theatre()),
	_snapshot(new ValueSnapshot()),
	_beatFinder(new BeatFinder()),
	_show(new class Show(*this))
{
	_beatFinder->Start();
}

Management::~Management()
{
	if(_thread != nullptr)
	{
		Quit();
		_thread->join();
		_thread.reset();
	}
}

void Management::Clear()
{
	_show->Clear();

	_controllables.clear();
	_presetValues.clear();
	_sequences.clear();
	_effects.clear();

	_theatre->Clear();

	_nextPresetValueId = 1;
}

void Management::AddDevice(std::unique_ptr<class DmxDevice> device)
{
	std::lock_guard<std::mutex> lock(_mutex);
	_devices.emplace_back(std::move(device));
	_snapshot->SetUniverseCount(_devices.size());
}

void Management::Run()
{
	if(_thread == nullptr)
	{
		_isQuitting = false;
		ManagementThread threadFunc;
		threadFunc.parent = this;
		_thread.reset(new std::thread(threadFunc));
	}
	else throw std::runtime_error("Invalid call to Run(): already running");
}

void Management::ManagementThread::operator()()
{
	std::unique_ptr<ValueSnapshot> nextSnapshot(new ValueSnapshot());
	nextSnapshot->SetUniverseCount(parent->_devices.size());
	while(!parent->IsQuitting())
	{
		for(unsigned universe=0; universe < parent->_devices.size(); ++universe)
		{
			unsigned values[512];
			unsigned char valuesChar[512];

			for(unsigned i=0;i<512;++i)
				values[i] = 0;
	
			parent->GetChannelValues(values, universe);
	
			for(unsigned i=0;i<512;++i)
			{
				unsigned val = (values[i] >> 16);
				if(val > 255) val = 255;
				valuesChar[i] = static_cast<unsigned char>(val);
			}

			ValueUniverseSnapshot& universeValues =
				nextSnapshot->GetUniverseSnapshot(universe);
			universeValues.SetValues(valuesChar, parent->_theatre->HighestChannel()+1);

			parent->_devices[universe]->WaitForNextSync();
			parent->_devices[universe]->SetValues(valuesChar, 512);
		}

		std::lock_guard<std::mutex> lock(parent->_mutex);
		std::swap(parent->_snapshot, nextSnapshot);
	}
}

void Management::AbortAllDevices()
{
	for(std::unique_ptr<DmxDevice>& device : _devices)
	{
		device->Abort();
	}
}

void Management::GetChannelValues(unsigned* values, unsigned universe)
{
	double relTimeInMs = GetOffsetTimeInMS();
	double beatValue, beatConfidence;
	_beatFinder->GetBeatValue(beatValue, beatConfidence);
	unsigned audioLevel = _beatFinder->GetAudioLevel();
	Timing relTiming(relTimeInMs, beatValue, audioLevel);

	std::lock_guard<std::mutex> lock(_mutex);

	for(const std::unique_ptr<class Effect>& effect : _effects)
		effect->StartIteration();

	_show->Mix(values, universe, relTiming);
	
	for(const std::unique_ptr<class PresetValue>& pv : _presetValues)
		pv->Controllable().Mix(pv->Value(), values, universe, relTiming);
}

PresetCollection& Management::AddPresetCollection()
{
	_controllables.emplace_back(new PresetCollection());
	return static_cast<PresetCollection&>(*_controllables.back());
}

void Management::RemoveControllable(Controllable& controllable)
{
	for(std::vector<std::unique_ptr<Controllable>>::iterator i=_controllables.begin();
		i!=_controllables.end(); ++i)
	{
		if(i->get() == &controllable)
		{
			removeControllable(i);
			return;
		}
	}
}

void Management::removeControllable(std::vector<std::unique_ptr<Controllable>>::iterator controllablePtr)
{
	std::unique_ptr<Controllable> controllable = std::move(*controllablePtr);

	_controllables.erase(controllablePtr);

	for(std::vector<std::unique_ptr<PresetValue>>::iterator i=_presetValues.begin();
		i!=_presetValues.end(); ++i)
	{
		PresetValue *p = i->get();
		if(&p->Controllable() == controllable.get())
		{
			--i;
			removePreset(i+1);
		}
	}

	PresetCollection*
		presetCollection = dynamic_cast<PresetCollection*>(controllable.get());
	if(presetCollection != nullptr)
	{
		for(std::vector<std::unique_ptr<Sequence>>::iterator i=_sequences.begin();
			i!=_sequences.end(); ++i)
		{
			Sequence *s = i->get();
			if(s->IsUsing(*presetCollection))
			{
				--i;
				removeSequence(i+1);
			}
		}
	}

	for(std::vector<std::unique_ptr<Controllable>>::iterator i=_controllables.begin();
		i!=_controllables.end(); ++i)
	{
		PresetCollection *p = dynamic_cast<PresetCollection*>(i->get());
		if(p!=0 && p->IsUsing(*controllable))
		{
			--i;
			removeControllable(i+1);
		}
	}
}

bool Management::Contains(Controllable &controllable) const
{
	for(const std::unique_ptr<Controllable>& contr : _controllables)
	{
		if(contr.get() == &controllable)
			return true;
	}
	return false;
}

FixtureFunctionControl& Management::AddFixtureFunctionControl(FixtureFunction &function)
{
	_controllables.emplace_back(new FixtureFunctionControl(function));
	return static_cast<FixtureFunctionControl&>(*_controllables.back());
}

PresetValue &Management::AddPreset(unsigned id, Controllable &controllable)
{
	_presetValues.emplace_back(new PresetValue(id, controllable));
	if(_nextPresetValueId <= id) _nextPresetValueId = id+1;
	return *_presetValues.back();
}

PresetValue &Management::AddPreset(Controllable &controllable)
{
	_presetValues.emplace_back(new PresetValue(_nextPresetValueId, controllable));
	++_nextPresetValueId;
	return *_presetValues.back();
}

void Management::RemovePreset(PresetValue& presetValue)
{
	for(std::vector<std::unique_ptr<PresetValue>>::iterator i=_presetValues.begin();
		i!=_presetValues.end(); ++i)
	{
		if(i->get() == &presetValue)
		{
			--i;
			removePreset(i+1);
		}
	}
}

void Management::removePreset(std::vector<std::unique_ptr<PresetValue>>::iterator presetValuePtr)
{
	_presetValues.erase(presetValuePtr);
}

bool Management::Contains(PresetValue& presetValue) const
{
	for(const std::unique_ptr<PresetValue>& pv : _presetValues)
	{
		if(pv.get() == &presetValue)
			return true;
	}
	return false;
}

Sequence& Management::AddSequence()
{
	_sequences.emplace_back(new Sequence());
	return *_sequences.back();
}

void Management::RemoveSequence(Sequence &sequence)
{
	for(std::vector<std::unique_ptr<Sequence>>::iterator i=_sequences.begin();
		i!=_sequences.end(); ++i)
	{
		if(i->get() == &sequence)
		{
			--i;
			removeSequence(i+1);
		}
	}
}

void Management::removeSequence(std::vector<std::unique_ptr<Sequence>>::iterator sequencePtr)
{
	std::unique_ptr<Sequence> sequence = std::move(*sequencePtr);

	for(std::vector<std::unique_ptr<Controllable>>::iterator i=_controllables.begin();
		i!=_controllables.end(); ++i)
	{
		Chase *chase = dynamic_cast<Chase*>(i->get());
		if(chase != 0)
		{
			if(&chase->Sequence() == sequence.get())
			{
				--i;
				removeControllable(i+1);
			}
		}
	}
	_sequences.erase(sequencePtr);
}

Chase &Management::AddChase(Sequence &sequence)
{
	_controllables.emplace_back(new Chase(sequence));
	return static_cast<Chase&>(*_controllables.back());
}

Effect& Management::AddEffect(std::unique_ptr<Effect> effect)
{
	std::vector<std::unique_ptr<EffectControl>> controls = effect->ConstructControls();
	for(std::unique_ptr<EffectControl>& control : controls)
		_controllables.emplace_back(std::move(control));
	_effects.emplace_back(std::move(effect));
	return *_effects.back();
}

Controllable& Management::GetControllable(const std::string &name) const
{
	return NamedObject::FindNamedObject(_controllables, name);
}

size_t Management::ControllableIndex(const Controllable* controllable) const
{
	return NamedObject::FindIndex(_controllables, controllable);
}

Sequence& Management::GetSequence(const std::string &name) const
{
	return NamedObject::FindNamedObject(_sequences, name);
}

size_t Management::SequenceIndex(const Sequence* sequence) const
{
	return NamedObject::FindIndex(_sequences, sequence);
}

PresetValue* Management::GetPresetValue(unsigned id) const
{
	for(const std::unique_ptr<PresetValue>& pv : _presetValues)
		if(pv->Id() == id)
			return pv.get();
	return nullptr;
}

PresetValue* Management::GetPresetValue(Controllable& controllable) const
{
	for(const std::unique_ptr<PresetValue>& pv : _presetValues)
		if(&pv->Controllable() == &controllable)
			return pv.get();
	return nullptr;
}

size_t Management::PresetValueIndex(const PresetValue* presetValue) const
{
	return NamedObject::FindIndex(_presetValues, presetValue);
}


ValueSnapshot Management::Snapshot()
{
	std::lock_guard<std::mutex> lock(_mutex);
	return *_snapshot;
}

size_t Management::EffectIndex(const Effect* effect) const
{
	return NamedObject::FindIndex(_effects, effect);
}

/**
 * Copy constructor for making a dry mode copy.
 */
Management::Management(const Management& forDryCopy, std::shared_ptr<class BeatFinder>& beatFinder) :
	_thread(),
	_isQuitting(false),
	_createTime(forDryCopy._createTime),
	_nextPresetValueId(forDryCopy._nextPresetValueId),
	_theatre(new class Theatre(*forDryCopy._theatre)),
	_beatFinder(beatFinder)
{
	for(size_t i=0; i!=forDryCopy._devices.size(); ++i)
		_devices.emplace_back(new DummyDevice());

	_snapshot.reset(new ValueSnapshot(*forDryCopy._snapshot));
	_show.reset(new class Show(*this)); // TODO For now we don't copy the show
	
	// The controllables can have dependencies to other controllables, hence dependencies
	// need to be resolved and copied first.
	_controllables.resize(forDryCopy._controllables.size());
	_presetValues.resize(forDryCopy._presetValues.size());
	_sequences.resize(forDryCopy._sequences.size());
	_effects.resize(forDryCopy._effects.size());
	for(size_t i=0; i!=forDryCopy._controllables.size(); ++i)
	{
		if(_controllables[i] == nullptr) // not already resolved?
			dryCopyControllerDependency(forDryCopy, i);
	}
	for(size_t i=0; i!=forDryCopy._presetValues.size(); ++i)
	{
		if(_presetValues[i] == nullptr)
		{
			size_t cIndex = forDryCopy.ControllableIndex(&forDryCopy._presetValues[i]->Controllable());
			_presetValues[i].reset(new PresetValue(*forDryCopy._presetValues[i], *_controllables[cIndex]));
		}
	}
	for(size_t i=0; i!=forDryCopy._sequences.size(); ++i)
	{
		if(_sequences[i] == nullptr)
			dryCopySequenceDependency(forDryCopy, i);
	}
	for(size_t i=0; i!=forDryCopy._effects.size(); ++i)
	{
		if(_effects[i] == nullptr)
			dryCopyEffectDependency(forDryCopy, i);
	}
}

void Management::dryCopyControllerDependency(const Management& forDryCopy, size_t index)
{
	Controllable* controllable = forDryCopy._controllables[index].get();
	FixtureFunctionControl* ffc = dynamic_cast<FixtureFunctionControl*>(controllable);
	const Chase* chase = dynamic_cast<const Chase *>(controllable);
	const PresetCollection* presetCollection = dynamic_cast<const PresetCollection *>(controllable);
	const EffectControl* effectControl = dynamic_cast<const EffectControl*>(controllable);
	if(ffc != nullptr)
	{
		FixtureFunction& ff = _theatre->GetFixtureFunction(ffc->Function().Name());
		_controllables[index].reset(new FixtureFunctionControl(ff));
	}
	else if(chase != nullptr)
	{
		size_t sIndex = forDryCopy.SequenceIndex(&chase->Sequence());
		if(_sequences[sIndex] == nullptr)
			dryCopySequenceDependency(forDryCopy, sIndex);
		_controllables[index].reset(new Chase(*chase, *_sequences[sIndex]));
	}
	else if(presetCollection != nullptr)
	{
		_controllables[index].reset(new PresetCollection(presetCollection->Name()));
		PresetCollection& pc = static_cast<PresetCollection&>(*_controllables[index]);
		for(const std::unique_ptr<PresetValue>& value : presetCollection->PresetValues())
		{
			// This preset is owned by the preset collection, not by management.
			size_t cIndex = forDryCopy.ControllableIndex(&value->Controllable());
			if(_controllables[cIndex] == nullptr)
				dryCopyControllerDependency(forDryCopy, cIndex);
			pc.AddPresetValue(*value, *_controllables[cIndex]);
		}
	}
	else if(effectControl != nullptr)
	{
		size_t eIndex = forDryCopy.EffectIndex(&effectControl->GetEffect());
		dryCopyEffectDependency(forDryCopy, eIndex);
	}
	else throw std::runtime_error("Unknown controllable in manager");
}

void Management::dryCopySequenceDependency(const Management& forDryCopy, size_t index)
{
	Sequence* sourceSequence = forDryCopy.Sequences()[index].get();
	_sequences[index] = sourceSequence->CopyWithoutPresets();
	Sequence* destSequence = static_cast<Sequence*>(_sequences[index].get());
	for(const PresetCollection* preset : sourceSequence->Presets())
	{
		size_t pIndex = forDryCopy.ControllableIndex(preset);
		if(_controllables[pIndex] == nullptr)
			dryCopyControllerDependency(forDryCopy, pIndex);
		destSequence->AddPreset(static_cast<PresetCollection*>(_controllables[pIndex].get()));
	}
}

void Management::dryCopyEffectDependency(const Management& forDryCopy, size_t index)
{
	const Effect* effect = forDryCopy._effects[index].get();
	_effects[index] = effect->Copy();
	std::vector<std::unique_ptr<EffectControl>> controls = _effects[index]->ConstructControls();
	for(size_t i=0; i!=controls.size(); ++i)
	{
		size_t cIndex = forDryCopy.ControllableIndex(effect->Controls()[i]);
		_controllables[cIndex] = std::move(controls[i]);
	}
	for(Controllable* c : effect->Connections())
	{
		size_t cIndex = forDryCopy.ControllableIndex(c);
		if(_controllables[cIndex] == nullptr)
			dryCopyControllerDependency(forDryCopy, cIndex);
		_effects[index]->AddConnection(_controllables[cIndex].get());
	}
}

std::unique_ptr<Management> Management::MakeDryMode()
{
	std::lock_guard<std::mutex> guard(_mutex);
	std::unique_ptr<Management> dryMode(new Management(*this, _beatFinder));
	return dryMode;
}

void Management::SwapDevices(Management& source)
{
	bool
		sourceRunning = source._thread != nullptr,
		thisRunning = _thread != nullptr;
	if(thisRunning)
	{
		Quit();
		_thread->join();
		_thread.reset();
	}
	if(sourceRunning)
	{
		source.Quit();
		source._thread->join();
		source._thread.reset();
	}
	
	std::unique_lock<std::mutex> guard(_mutex);
#ifndef NDEBUG
	if(source._devices.size() != _devices.size())
		throw std::runtime_error("Something went wrong: device lists were not of same size in call to SwapDevices()");
#endif
	std::swap(source._devices, _devices);
	guard.unlock();
	
	if(sourceRunning)
		source.Run();
	if(thisRunning)
		Run();
}
