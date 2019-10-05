#include <stdexcept>

#include "writer.h"

#include "../theatre/properties/propertyset.h"

#include "../theatre/chase.h"
#include "../theatre/controllable.h"
#include "../theatre/effect.h"
#include "../theatre/fixture.h"
#include "../theatre/fixturecontrol.h"
#include "../theatre/fixturefunction.h"
#include "../theatre/folder.h"
#include "../theatre/presetvalue.h"
#include "../theatre/show.h"
#include "../theatre/theatre.h"
#include "../theatre/timesequence.h"

#include "../gui/guistate.h"

Writer::Writer(Management &management) : _management(management), _guiState(nullptr), _encoding("UTF-8")
{
}

void Writer::CheckXmlVersion()
{
	LIBXML_TEST_VERSION;
}

void Writer::Write(const Glib::ustring &filename)
{
	_controllablesWritten.clear();
	_folderIds.clear();
	
	/* Create a new XmlWriter for uri, with no compression. */
	_writer = xmlNewTextWriterFilename(filename.c_str(), 0);
	if (_writer == NULL)
		throw WriterException("Writer::Write(): Error creating the xml writer");

	int rc = xmlTextWriterStartDocument(_writer, NULL, _encoding, NULL);
	if (rc < 0)
		throw WriterException("Writer::Write(): Error at xmlTextWriterStartDocument");

	writeGlightShow();

	rc = xmlTextWriterEndDocument(_writer);
	if (rc < 0)
		throw WriterException("Writer::Write(): Error at xmlTextWriterEndDocument");

	xmlFreeTextWriter(_writer);
}

void Writer::startElement(const char *elementName)
{
	if ( xmlTextWriterStartElement(_writer, BAD_CAST elementName) < 0 )
		throw WriterException("Writer: Error at xmlTextWriterStartElement");
}

void Writer::endElement()
{
	if(xmlTextWriterEndElement(_writer) < 0)
		throw WriterException("Writer: Error at xmlTextWriterEndElement");
}

void Writer::writeElement(const char *elementName, const char *elementValue)
{
	if(xmlTextWriterWriteElement(_writer, BAD_CAST elementName, BAD_CAST elementValue) < 0)
		throw WriterException("Writer: Error at xmlTextWriterWriteElement");
}

void Writer::writeAttribute(const char *attributeName, const char *attributeValue)
{
	if(xmlTextWriterWriteAttribute(_writer, BAD_CAST attributeName, BAD_CAST attributeValue) < 0)
		throw WriterException("Writer: Error at xmlTextWriterWriteAttribute");
}

void Writer::writeAttribute(const char *attributeName, int attributeValue)
{
	std::stringstream s;
	s << attributeValue;
	writeAttribute(attributeName, s.str());
}

void Writer::writeAttribute(const char *attributeName, double attributeValue)
{
	std::stringstream s;
	s << attributeValue;
	writeAttribute(attributeName, s.str());
}

void Writer::writeGlightShow()
{
	startElement("glight-show");

	startElement("folders");
	writeFolders();
	endElement();
	
	startElement("theatre");

	Theatre &theatre = _management.Theatre();

	const std::vector<std::unique_ptr<FixtureType>>&
		fixtureTypes = theatre.FixtureTypes();
	for(const std::unique_ptr<FixtureType>& ft : fixtureTypes)
		writeFixtureType(*ft);

	const std::vector<std::unique_ptr<Fixture>>&
		fixtures = theatre.Fixtures();
	for(const std::unique_ptr<Fixture>& f : fixtures)
		writeFixture(*f);

	endElement(); // theatre

	startElement("control");

	const std::vector<std::unique_ptr<Controllable>>&
		controllables = _management.Controllables();
	for(const std::unique_ptr<Controllable>& c : controllables)
		writeControllable(*c);

	const std::vector<std::unique_ptr<PresetValue>>&
		presetValues = _management.PresetValues();
	for(const std::unique_ptr<PresetValue>& pv : presetValues)
		writePresetValue(*pv);

	endElement(); // control

	startElement("show");

	Show& show = _management.Show();

	const std::vector<std::unique_ptr<Scene>>& scenes = show.Scenes();
	for(const std::unique_ptr<Scene>& scene : scenes)
		writeScene(*scene);

	endElement(); // show
	
	startElement("gui");
	
	if(_guiState != nullptr)
		writeGUIState(*_guiState);
	
	endElement(); // gui

	endElement(); // glight-show
}

void Writer::writeFolders()
{
	for(size_t i=0; i!=_management.Folders().size(); ++i)
	{
		const Folder& folder = *_management.Folders()[i];
		_folderIds.emplace(&folder, i);
		startElement("folder");
		writeAttribute("id", i);
		writeAttribute("name", folder.Name());
		if(!folder.IsRoot())
			writeAttribute("parent", _folderIds.find(&folder.Parent())->second);
		endElement();
	}
}	

void Writer::writeNameAttributes(const NamedObject& obj)
{
	writeAttribute("name", obj.Name());
}

void Writer::writeFolderAttributes(const FolderObject& obj)
{
	writeNameAttributes(obj);
	writeAttribute("parent", _folderIds.find(&obj.Parent())->second);
}

void Writer::writeFixture(const Fixture &fixture)
{
	startElement("fixture");
	writeNameAttributes(fixture);
	writeAttribute("type", fixture.Type().Name());
	writeAttribute("position-x", fixture.Position().X());
	writeAttribute("position-y", fixture.Position().Y());
	writeAttribute("symbol", fixture.Symbol().Name());
	const std::vector<std::unique_ptr<FixtureFunction>>& functions = fixture.Functions();
	for(const std::unique_ptr<FixtureFunction>& ff : functions)
		writeFixtureFunction(*ff);
	endElement();
}

void Writer::writeFixtureFunction(const FixtureFunction& fixtureFunction)
{
	startElement("fixture-function");
	writeAttribute("name", fixtureFunction.Name());
	writeAttribute("type", (int) fixtureFunction.Type());
	writeDmxChannel(fixtureFunction.FirstChannel());
	endElement();
}

void Writer::writeDmxChannel(const DmxChannel &dmxChannel)
{
	startElement("dmx-channel");
	writeAttribute("universe", dmxChannel.Universe());
	writeAttribute("channel", dmxChannel.Channel());
	writeAttribute("default-mix-style", dmxChannel.DefaultMixStyle());
	endElement();
}

void Writer::writeFixtureType(const FixtureType &fixtureType)
{
	startElement("fixture-type");
	writeFolderAttributes(fixtureType);
	writeAttribute("fixture-class", fixtureType.FixtureClass());
	endElement();
}

void Writer::writeControllable(const Controllable &controllable)
{
	if(_controllablesWritten.count(&controllable) == 0)
	{
		const FixtureControl* fixtureControl = dynamic_cast<const FixtureControl *>(&controllable);
		const Chase* chase = dynamic_cast<const Chase *>(&controllable);
		const TimeSequence* tSequence = dynamic_cast<const TimeSequence *>(&controllable);
		const PresetCollection* presetCollection = dynamic_cast<const PresetCollection *>(&controllable);
		const Effect* effect = dynamic_cast<const Effect*>(&controllable);
	
		if(fixtureControl)
			writeFixtureControl(*fixtureControl);
		else if(chase)
			writeChase(*chase);
		else if(tSequence)
			writeTimeSequence(*tSequence);
		else if(presetCollection)
			writePresetCollection(*presetCollection);
		else if(effect)
			writeEffect(*effect);
		else
			throw std::runtime_error("Unknown controllable");
	}
	_controllablesWritten.insert(&controllable);
}

void Writer::writePresetCollection(const class PresetCollection &presetCollection)
{
	const std::vector<std::unique_ptr<PresetValue>>&
		values = presetCollection.PresetValues();
	for(const std::unique_ptr<PresetValue>& pv : values)
		requireControllable(pv->Controllable());
	
	startElement("preset-collection");
	writeFolderAttributes(presetCollection);
	for(const std::unique_ptr<PresetValue>& pv : values)
		writePresetValue(*pv);
	endElement();
}

void Writer::writePresetValue(const PresetValue &presetValue)
{
	requireControllable(presetValue.Controllable());

	startElement("preset-value");
	writeAttribute("controllable-ref", presetValue.Controllable().Name());
	writeAttribute("input-index", presetValue.InputIndex());
	writeAttribute("folder", _folderIds[&presetValue.Controllable().Parent()]);
	writeAttribute("value", presetValue.Value().UInt());
	endElement();
}

void Writer::writeFixtureControl(const FixtureControl &control)
{
	startElement("fixture-control");
	writeFolderAttributes(control);
	writeAttribute("fixture-ref", control.Fixture().Name());
	endElement();
}

void Writer::writeChase(const Chase& chase)
{
	const std::vector<std::pair<Controllable*,size_t>>& list = chase.Sequence().List();
	for(const std::pair<Controllable*, size_t>& input : list)
		requireControllable(*input.first);

	startElement("chase");
	writeFolderAttributes(chase);
	writeTrigger(chase.Trigger());
	writeTransition(chase.Transition());
	writeSequence(chase.Sequence());
	endElement();
}

void Writer::writeTimeSequence(const TimeSequence& timeSequence)
{
	const std::vector<std::pair<Controllable*,size_t>>& list = timeSequence.Sequence().List();
	for(const std::pair<Controllable*, size_t>& input : list)
		requireControllable(*input.first);

	startElement("time-sequence");
	writeFolderAttributes(timeSequence);
	writeAttribute("sustain", timeSequence.Sustain());
	writeAttribute("repeat-count", timeSequence.RepeatCount());
	writeSequence(timeSequence.Sequence());
	for(size_t i=0; i!=timeSequence.Size(); ++i)
	{
		startElement("step");
		const TimeSequence::Step& step = timeSequence.GetStep(i);
		writeTrigger(step.trigger);
		writeTransition(step.transition);
		endElement();
	}
	endElement();
}

void Writer::writeTrigger(const Trigger &trigger)
{
	startElement("trigger");
	writeAttribute("type", trigger.Type());
	writeAttribute("delay-in-ms", trigger.DelayInMs());
	writeAttribute("delay-in-beats", trigger.DelayInBeats());
	writeAttribute("delay-in-syncs", trigger.DelayInSyncs());
	endElement();
}

void Writer::writeTransition(const Transition &transition)
{
	startElement("transition");
	writeAttribute("length-in-ms", transition.LengthInMs());
	writeAttribute("type", transition.Type());
	endElement();
}

void Writer::writeSequence(const Sequence &sequence)
{
	startElement("sequence");
	for(const std::pair<Controllable*, size_t>& input : sequence.List())
	{
		startElement("input-ref");
		writeAttribute("input-index", input.second);
		writeAttribute("folder", _folderIds[&input.first->Parent()]);
		writeAttribute("name", input.first->Name());
		endElement();
	}
	endElement();
}

void Writer::writeEffect(const class Effect& effect)
{
	if(_controllablesWritten.count(&effect) == 0)
	{
		for(const std::pair<Controllable*, size_t>& c : effect.Connections())
			requireControllable(*c.first);
		
		startElement("effect");
		writeFolderAttributes(effect);
		writeAttribute("type", effect.TypeToName(effect.GetType()));
		std::unique_ptr<PropertySet> ps = PropertySet::Make(effect);
		
		// the number and name of the effect controls are implied from the
		// effect type, so do not require to be stored.
		
		for(const Property& p : *ps)
		{
			startElement("property");
			writeAttribute("name", p.Name());
			switch(p.GetType())
			{
				case Property::ControlValue:
					writeAttribute("value", ps->GetControlValue(p));
					break;
				case Property::Duration:
					writeAttribute("value", ps->GetDuration(p));
					break;
				case Property::Boolean:
					writeAttribute("value", ps->GetBool(p));
					break;
				case Property::Integer:
					writeAttribute("value", ps->GetInteger(p));
					break;
			}
			endElement();
		}
		for(const std::pair<Controllable*, size_t>& c : effect.Connections())
		{
			startElement("connection-ref");
			writeAttribute("input-index", c.second);
			writeAttribute("folder", _folderIds[&c.first->Parent()]);
			writeAttribute("name", c.first->Name());
			endElement();
		}
		endElement();
		_controllablesWritten.insert(&effect);
	}
}

void Writer::writeScene(const Scene &scene)
{
	startElement("scene");

	   writeFolderAttributes(scene);
	writeAttribute("audio-file", scene.AudioFile());

	const std::multimap<double, std::unique_ptr<SceneItem>> &items = scene.SceneItems();
	for(const std::pair<const double, std::unique_ptr<SceneItem>>& sceneItem : items)
	{
		writeSceneItem(*sceneItem.second);
	}

	endElement();
}

void Writer::writeSceneItem(const SceneItem &item)
{
	startElement("scene-item");

	writeAttribute("offset", item.OffsetInMS());
	writeAttribute("duration", item.DurationInMS());

	const KeySceneItem *keyItem = dynamic_cast<const KeySceneItem*>(&item);
	const ControlSceneItem *controlItem = dynamic_cast<const ControlSceneItem*>(&item);

	if(keyItem != 0)
		writeKeySceneItem(*keyItem);
	else
		writeControlSceneItem(*controlItem);

	endElement();
}

void Writer::writeKeySceneItem(const KeySceneItem &item)
{
	writeAttribute("type", "key");
	writeAttribute("level", item.Level());
}

void Writer::writeControlSceneItem(const ControlSceneItem &item)
{
	writeAttribute("type", "control");
	writeAttribute("start-value", item.StartValue().UInt());
	writeAttribute("end-value", item.EndValue().UInt());
	writeAttribute("controllable-ref", item.Controllable().Name());
	writeAttribute("folder", _folderIds[&item.Controllable().Parent()]);
}

void Writer::writeGUIState(const GUIState& guiState)
{
	for(const std::unique_ptr<FaderSetupState>& fState : guiState.FaderSetups())
		writeFaderState(*fState);
}

void Writer::writeFaderState(const FaderSetupState& guiState)
{
	startElement("faders");
	writeAttribute("name", guiState.name);
	writeAttribute("active", guiState.isActive);
	writeAttribute("solo", guiState.isSolo);
	writeAttribute("fade-in", guiState.fadeInSpeed);
	writeAttribute("fade-out", guiState.fadeOutSpeed);
	writeAttribute("width", guiState.width);
	writeAttribute("height", guiState.height);
	for(const FaderState& fader : guiState.faders)
	{
		startElement("fader");
		writeAttribute("is-toggle", fader.IsToggleButton());
		if(fader.IsToggleButton())
			writeAttribute("new-toggle-column", fader.NewToggleButtonColumn());
		if(fader.GetPresetValue() != nullptr)
		{
			writeAttribute("input-index", fader.GetPresetValue()->InputIndex());
			writeAttribute("folder", _folderIds[&fader.GetPresetValue()->Controllable().Parent()]);
			writeAttribute("name", fader.GetPresetValue()->Controllable().Name());
		}
		endElement(); // preset
	}
	endElement(); // faders
}
