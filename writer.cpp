#include <stdexcept>

#include "writer.h"

#include "libtheatre/properties/propertyset.h"

#include "libtheatre/chase.h"
#include "libtheatre/controllable.h"
#include "libtheatre/effect.h"
#include "libtheatre/effectcontrol.h"
#include "libtheatre/fixture.h"
#include "libtheatre/fixturefunction.h"
#include "libtheatre/fixturefunctioncontrol.h"
#include "libtheatre/presetvalue.h"
#include "libtheatre/show.h"
#include "libtheatre/theatre.h"

#include "gui/guistate.h"

Writer::Writer(Management &management) : _management(management), _guiState(nullptr), _encoding("UTF-8")
{
}

void Writer::CheckXmlVersion()
{
	LIBXML_TEST_VERSION;
}

void Writer::Write(const Glib::ustring &filename)
{
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

void Writer::writeGlightShow()
{
	startElement("glight-show");

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

	const std::vector<std::unique_ptr<Sequence>>&
		sequences = _management.Sequences();
	for(const std::unique_ptr<Sequence>& s : sequences)
		writeSequence(*s);

	const std::vector<std::unique_ptr<Effect>>&
		effects = _management.Effects();
	for(const std::unique_ptr<Effect>& e : effects)
		writeEffect(*e);

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

void Writer::writeFixture(const Fixture &fixture)
{
	startElement("fixture");
	writeAttribute("name", fixture.Name());
	writeAttribute("type", fixture.Type().Name());
	const std::vector<std::unique_ptr<FixtureFunction>>& functions = fixture.Functions();
	for(const std::unique_ptr<FixtureFunction>& ff : functions)
		writeFixtureFunction(*ff);
	endElement();
}

void Writer::writeFixtureFunction(const FixtureFunction &fixtureFunction)
{
	startElement("fixture-function");
	writeAttribute("name", fixtureFunction.Name());
	writeAttribute("type", fixtureFunction.Type());
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
	writeAttribute("name", fixtureType.Name());
	writeAttribute("fixture-class", fixtureType.FixtureClass());
	endElement();
}

void Writer::writeControllable(const Controllable &controllable)
{
	if(_controllablesWritten.count(controllable.Name()) == 0)
	{
		const FixtureFunctionControl* fixtureFunctionControl = dynamic_cast<const FixtureFunctionControl *>(&controllable);
		const Chase* chase = dynamic_cast<const Chase *>(&controllable);
		const PresetCollection* presetCollection = dynamic_cast<const PresetCollection *>(&controllable);
		const EffectControl* effectControl = dynamic_cast<const EffectControl*>(&controllable);
	
		if(fixtureFunctionControl != nullptr)
			writeFixtureFunctionControl(*fixtureFunctionControl);
		else if(chase != nullptr)
			writeChase(*chase);
		else if(presetCollection != nullptr)
			writePresetCollection(*presetCollection);
		else if(effectControl != nullptr)
			writeEffect(effectControl->GetEffect());
		else
			throw std::runtime_error("Unknown controllable");
	}
	_controllablesWritten.insert(controllable.Name());
}

void Writer::writePresetCollection(const class PresetCollection &presetCollection)
{
	startElement("preset-collection");
	writeAttribute("name", presetCollection.Name());
	const std::vector<std::unique_ptr<PresetValue>>&
		values = presetCollection.PresetValues();
	for(const std::unique_ptr<PresetValue>& pv : values)
		writePresetValue(*pv);
	endElement();
}

void Writer::writePresetValue(const PresetValue &presetValue)
{
	requireControllable(presetValue.Controllable());

	startElement("preset-value");
	writeAttribute("controllable-ref", presetValue.Controllable().Name());
	writeAttribute("value", presetValue.Value().UInt());
	writeAttribute("id", presetValue.Id());
	endElement();
}

void Writer::writeFixtureFunctionControl(const FixtureFunctionControl &control)
{
	startElement("fixture-function-control");
	writeAttribute("name", control.Name());
	writeAttribute("fixture-function-ref", control.Function().Name());
	endElement();
}

void Writer::writeChase(const Chase &chase)
{
	requireSequence(chase.Sequence());

	startElement("chase");
	writeAttribute("name", chase.Name());
	writeAttribute("sequence-ref", chase.Sequence().Name());
	writeTrigger(chase.Trigger());
	writeTransition(chase.Transition());
	endElement();
}

void Writer::writeTrigger(const Trigger &trigger)
{
	startElement("trigger");
	writeAttribute("delay-in-ms", trigger.DelayInMs());
	writeAttribute("type", trigger.Type());
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
	if(_sequencesWritten.count(sequence.Name()) == 0)
	{
		const std::vector<PresetCollection*> &presets = sequence.Presets();
		for(const PresetCollection* pc : presets)
			requireControllable(*pc);
	
		startElement("sequence");
		writeAttribute("name", sequence.Name());
		for(const PresetCollection* pc : presets)
		{
			startElement("preset-collection-ref");
			writeAttribute("name", pc->Name());
			endElement();
		}
		endElement();
		_sequencesWritten.insert(sequence.Name());
	}
}

void Writer::writeEffect(const class Effect& effect)
{
	if(_effectsWritten.count(effect.Name()) == 0)
	{
		for(const Controllable* c : effect.Connections())
			requireControllable(*c);
			
		startElement("effect");
		writeAttribute("type", effect.TypeToName(effect.GetType()));
		writeAttribute("name", effect.Name());
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
			}
			endElement();
		}
		for(const Controllable* c : effect.Connections())
		{
			startElement("connection-ref");
			writeAttribute("name", c->Name());
			endElement();
		}
		endElement();
		_effectsWritten.insert(effect.Name());
	}
}

void Writer::writeScene(const Scene &scene)
{
	startElement("scene");

	writeAttribute("name", scene.Name());
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
	writeAttribute("width", guiState.width);
	writeAttribute("height", guiState.height);
	for(const FaderState& fader : guiState.faders)
	{
		startElement("fader");
		if(fader.GetPresetValue() != nullptr)
			writeAttribute("preset-id", fader.GetPresetValue()->Id());
		endElement(); // preset
	}
	endElement(); // faders
}
