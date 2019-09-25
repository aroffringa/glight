#include "management.h"

#include "../system/beatfinder.h"

#include "chase.h"
#include "controllable.h"
#include "dmxdevice.h"
#include "dummydevice.h"
#include "effect.h"
#include "fixturecontrol.h"
#include "folder.h"
#include "presetcollection.h"
#include "presetvalue.h"
#include "sequence.h"
#include "show.h"
#include "theatre.h"
#include "timesequence.h"
#include "valuesnapshot.h"

Management::Management() : 
	_thread(),
	_isQuitting(false),
	_createTime(boost::posix_time::microsec_clock::local_time()),
	_rndDistribution(0, ControlValue::MaxUInt()+1),
	_overridenBeat(0),
	_lastOverridenBeatTime(0.0),
	
	_theatre(new class Theatre()),
	_snapshot(new ValueSnapshot()),
	_show(new class Show(*this))
{
	_folders.emplace_back(new Folder());
	_rootFolder = _folders.back().get();
	_rootFolder->SetName("Root");
}

Management::~Management()
{
	if(_thread != nullptr)
	{
		_isQuitting = true;
		abortAllDevices();
		_thread->join();
		_thread.reset();
	}
}

void Management::StartBeatFinder()
{
	_beatFinder.reset(new BeatFinder());
	_beatFinder->Start();
}

void Management::Clear()
{
	_show->Clear();

	_controllables.clear();
	_presetValues.clear();
	_folders.clear();
	_folders.emplace_back(new Folder());
	_rootFolder = _folders.back().get();
	_rootFolder->SetName("Root");

	_theatre->Clear();
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
	unsigned timestepNumber = 0;
	while(!parent->_isQuitting)
	{
		for(unsigned universe=0; universe < parent->_devices.size(); ++universe)
		{
			unsigned values[512];
			unsigned char valuesChar[512];

			std::fill_n(values, 512, 0);
	
			parent->getChannelValues(timestepNumber, values, universe);
	
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
		
		++timestepNumber;
	}
}

void Management::abortAllDevices()
{
	for(std::unique_ptr<DmxDevice>& device : _devices)
	{
		device->Abort();
	}
}

void Management::getChannelValues(unsigned timestepNumber, unsigned* values, unsigned universe)
{
	double relTimeInMs = GetOffsetTimeInMS();
	double beatValue, beatConfidence;
	unsigned audioLevel;
	if(relTimeInMs - _lastOverridenBeatTime < 3000.0 && _overridenBeat != 0)
	{
		beatConfidence = 1.0;
		beatValue = _overridenBeat;
		if(_beatFinder)
			audioLevel = _beatFinder->GetAudioLevel();
		else
			audioLevel = 0;
	}
	else if(_beatFinder)
	{
		_beatFinder->GetBeatValue(beatValue, beatConfidence);
		audioLevel = _beatFinder->GetAudioLevel();
	}
	else {
		beatValue = 0.0;
		beatConfidence = 0.0;
		audioLevel = 0;
	}
	unsigned randomValue = _rndDistribution(_randomGenerator);
	Timing timing(relTimeInMs, timestepNumber, beatValue, audioLevel, randomValue);

	std::lock_guard<std::mutex> lock(_mutex);

	_show->Mix(values, universe, timing);
	
	// Reset all inputs
	for(const std::unique_ptr<class PresetValue>& pv : _presetValues)
	{
		for(size_t inputIndex=0; inputIndex != pv->Controllable().NInputs(); ++inputIndex)
		{
			pv->Controllable().InputValue(inputIndex) = 0;
		}
	}
	
	for(const std::unique_ptr<class PresetValue>& pv : _presetValues)
		pv->Controllable().MixInput(pv->InputIndex(), pv->Value());
	
	// Solve dependency graph of effects
	std::vector<Controllable*> unorderedList, orderedList;
	for(const std::unique_ptr<Controllable>& c : _controllables)
		unorderedList.emplace_back(c.get());
	if(!topologicalSort(unorderedList, orderedList))
		throw std::runtime_error("Cycle in dependencies");
	
	for(auto c = orderedList.rbegin(); c != orderedList.rend(); ++c)
	{
		(*c)->Mix(values, universe, timing);
	}
}

bool Management::HasCycle() const
{
	std::vector<Controllable*> unorderedList, orderedList;
	for(const std::unique_ptr<Controllable>& c : _controllables)
		unorderedList.emplace_back(c.get());
	return !topologicalSort(unorderedList, orderedList);
}

PresetCollection& Management::AddPresetCollection()
{
	_controllables.emplace_back(new PresetCollection());
	return static_cast<PresetCollection&>(*_controllables.back());
}

Folder& Management::AddFolder(Folder& parent, const std::string& name)
{
	_folders.emplace_back(new Folder(name));
	parent.Add(*_folders.back());
	return *_folders.back();
}

Folder& Management::GetFolder(const std::string& path)
{
	return *_rootFolder->FollowDown(Folder::RemoveRoot(path));
}

void Management::RemoveObject(FolderObject& object)
{
	Folder* folder = dynamic_cast<Folder*>(&object);
	if(folder)
		RemoveFolder(*folder);
	else {
		Controllable* controllable = dynamic_cast<Controllable*>(&object);
		if(controllable)
			RemoveControllable(*controllable);
		else
			throw std::runtime_error("Can not remove unknown object " + object.Name());
	}
}

void Management::RemoveFolder(Folder& folder)
{
	if(&folder == _rootFolder)
		throw std::runtime_error("Can not remove root folder");
	// Removing a child might remove dependent children from the same folder
	// so we have to recheck whether the folder is empty after each removal
	while(!folder.Children().empty())
	{
		RemoveObject(*folder.Children().back());
	}
	folder.Parent().Remove(folder);
	for(auto iter = _folders.begin(); iter != _folders.end(); ++iter)
	{
		if(iter->get() == &folder)
		{
			_folders.erase(iter);
			break;
		}
	}
}

void Management::RemoveControllable(Controllable& controllable)
{
	removeControllable(_controllables.begin() +
		FolderObject::FindIndex(_controllables, &controllable));
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
	
	controllable->Parent().Remove(*controllable.get());

	for(std::vector<std::unique_ptr<Controllable>>::iterator i=_controllables.begin();
		i!=_controllables.end(); ++i)
	{
		if((*i)->HasOutputConnection(*controllable))
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

FixtureControl& Management::AddFixtureControl(Fixture& fixture)
{
	_controllables.emplace_back(new FixtureControl(fixture));
	return static_cast<FixtureControl&>(*_controllables.back());
}

FixtureControl& Management::AddFixtureControl(Fixture& fixture, Folder& parent)
{
	_controllables.emplace_back(new FixtureControl(fixture));
	parent.Add(*_controllables.back());
	return static_cast<FixtureControl&>(*_controllables.back());
}

FixtureControl& Management::GetFixtureControl(class Fixture& fixture)
{
	for(const std::unique_ptr<Controllable>& contr : _controllables)
	{
		FixtureControl* fc = dynamic_cast<FixtureControl*>(contr.get());
		if(fc)
		{
			if(&fc->Fixture() == &fixture)
				return *fc;
		}
	}
	throw std::runtime_error("GetFixtureControl() : Fixture control not found");
}

void Management::RemoveFixture(Fixture& fixture)
{
	FixtureControl& control = GetFixtureControl(fixture);
	_theatre->RemoveFixture(fixture);
	RemoveControllable(control);
}

PresetValue &Management::AddPreset(Controllable &controllable, size_t inputIndex)
{
	_presetValues.emplace_back(new PresetValue(controllable, inputIndex));
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

Chase &Management::AddChase()
{
	_controllables.emplace_back(new Chase());
	return static_cast<Chase&>(*_controllables.back());
}

TimeSequence &Management::AddTimeSequence()
{
	_controllables.emplace_back(new TimeSequence());
	return static_cast<TimeSequence&>(*_controllables.back());
}

Effect& Management::AddEffect(std::unique_ptr<Effect> effect)
{
	_controllables.emplace_back(std::move(effect));
	return static_cast<Effect&>(*_controllables.back());
}

Effect& Management::AddEffect(std::unique_ptr<Effect> effect, Folder& folder)
{
	Effect& newEffect = AddEffect(std::move(effect));
	folder.Add(newEffect);
	return newEffect;
}

FolderObject* Management::GetObjectFromPathIfExists(const std::string& path) const
{
	auto sep = std::find(path.begin(), path.end(), '/');
	if(sep == path.end())
	{
		if(path == _rootFolder->Name())
			return _rootFolder;
	}
	else {
		std::string left = path.substr(0, sep-path.begin());
		std::string right = path.substr(sep+1-path.begin());
		if(left == _rootFolder->Name())
			return _rootFolder->FollowRelPath(right);
	}
	return nullptr;
}

FolderObject& Management::GetObjectFromPath(const std::string& path) const
{
	FolderObject* result = GetObjectFromPathIfExists(path);
	if(result)
		return *result;
	else
		throw std::runtime_error("Could not find object with path " + path);
}

size_t Management::ControllableIndex(const Controllable* controllable) const
{
	return FolderObject::FindIndex(_controllables, controllable);
}

PresetValue* Management::GetPresetValue(Controllable& controllable, size_t inputIndex) const
{
	for(const std::unique_ptr<PresetValue>& pv : _presetValues)
		if(&pv->Controllable() == &controllable && pv->InputIndex() == inputIndex)
			return pv.get();
	return nullptr;
}

size_t Management::PresetValueIndex(const PresetValue* presetValue) const
{
	return FolderObject::FindIndex(_presetValues, presetValue);
}

ValueSnapshot Management::Snapshot()
{
	std::lock_guard<std::mutex> lock(_mutex);
	return *_snapshot;
}

/**
 * Copy constructor for making a dry mode copy.
 */
Management::Management(const Management& forDryCopy, std::shared_ptr<class BeatFinder>& beatFinder) :
	_thread(),
	_isQuitting(false),
	_createTime(forDryCopy._createTime),
	_theatre(new class Theatre(*forDryCopy._theatre)),
	_beatFinder(beatFinder)
{
	for(size_t i=0; i!=forDryCopy._devices.size(); ++i)
		_devices.emplace_back(new DummyDevice());

	_snapshot.reset(new ValueSnapshot(*forDryCopy._snapshot));
	_show.reset(new class Show(*this)); // TODO For now we don't copy the show
	
	_rootFolder = forDryCopy._rootFolder->CopyHierarchy(_folders);
	
	// The controllables can have dependencies to other controllables, hence dependencies
	// need to be resolved and copied first.
	_controllables.resize(forDryCopy._controllables.size());
	_presetValues.resize(forDryCopy._presetValues.size());
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
}

void Management::dryCopyControllerDependency(const Management& forDryCopy, size_t index)
{
	Controllable* controllable = forDryCopy._controllables[index].get();
	FixtureControl* fc = dynamic_cast<FixtureControl*>(controllable);
	const Chase* chase = dynamic_cast<const Chase *>(controllable);
	const TimeSequence* timeSequence = dynamic_cast<const TimeSequence *>(controllable);
	const PresetCollection* presetCollection = dynamic_cast<const PresetCollection *>(controllable);
	const Effect* effect = dynamic_cast<const Effect*>(controllable);
	if(fc)
	{
		Fixture& fixture = _theatre->GetFixture(fc->Fixture().Name());
		_controllables[index].reset(new FixtureControl(fixture));
		GetFolder(fc->Parent().FullPath()).Add(*_controllables[index]);
	}
	else if(chase || timeSequence)
	{
		const Sequence* sequence;
		Sequence* newSequence;
		if(chase)
		{
			_controllables[index] = chase->CopyWithoutSequence();
			Chase& newChase = static_cast<Chase&>(*_controllables[index]);
			sequence = &chase->Sequence();
			newSequence = &newChase.Sequence();
		}
		else {
			_controllables[index] = timeSequence->CopyWithoutSequence();
			TimeSequence& newTimeSequence = static_cast<TimeSequence&>(*_controllables[index]);
			sequence = &timeSequence->Sequence();
			newSequence = &newTimeSequence.Sequence();
		}
		for(const std::pair<Controllable*, size_t>& input : sequence->List())
		{
			size_t cIndex = forDryCopy.ControllableIndex(input.first);
			if(_controllables[cIndex] == nullptr)
				dryCopyControllerDependency(forDryCopy, cIndex);
			newSequence->Add(*_controllables[cIndex], input.second);
		}
		GetFolder(controllable->Parent().FullPath()).Add(*_controllables[index]);
	}
	else if(presetCollection)
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
		GetFolder(presetCollection->Parent().FullPath()).Add(pc);
	}
	else if(effect)
	{
		size_t eIndex = forDryCopy.ControllableIndex(effect);
		dryCopyEffectDependency(forDryCopy, eIndex);
	}
	else throw std::runtime_error("Unknown controllable in manager");
}

void Management::dryCopyEffectDependency(const Management& forDryCopy, size_t index)
{
	const Effect* effect = static_cast<const Effect*>(forDryCopy._controllables[index].get());
	_controllables[index] = effect->Copy();
	GetFolder(effect->Parent().FullPath()).Add(*_controllables[index]);
	for(const std::pair<Controllable*, size_t>& c : effect->Connections())
	{
		size_t cIndex = forDryCopy.ControllableIndex(c.first);
		if(_controllables[cIndex] == nullptr)
			dryCopyControllerDependency(forDryCopy, cIndex);
		static_cast<Effect&>(*_controllables[index]).AddConnection(*_controllables[cIndex], c.second);
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
		_isQuitting = true;
		abortAllDevices();
		_thread->join();
		_thread.reset();
	}
	if(sourceRunning)
	{
		source._isQuitting = true;
		source.abortAllDevices();
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

bool Management::topologicalSort(const std::vector<Controllable*>& input, std::vector<Controllable*>& output)
{
	for(Controllable* controllable : input)
		controllable->SetVisitLevel(0);
	for(Controllable* controllable : input)
	{
		if(!topologicalSortVisit(*controllable, output))
			return false;
	}
	return true;
}

bool Management::topologicalSortVisit(Controllable& controllable, std::vector<Controllable*>& list)
{
	if(controllable.VisitLevel() == 0)
	{
		controllable.SetVisitLevel(1);
		for(size_t i=0; i!=controllable.NOutputs(); ++i)
		{
			Controllable* other = controllable.Output(i).first;
			if(!topologicalSortVisit(*other, list))
				return false;
		}
		controllable.SetVisitLevel(2);
		list.emplace_back(&controllable);
	}
	else if(controllable.VisitLevel() == 1)
		return false;
	return true;
}

void Management::BlackOut()
{
	for(std::unique_ptr<class PresetValue>& p : _presetValues)
	{
		p->SetValue(ControlValue::Zero());
	}
}
