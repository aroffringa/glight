#include <stdexcept>

#include "writer.h"

#include "libtheatre/chase.h"
#include "libtheatre/controllable.h"
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
		const FixtureFunctionControl *fixtureFunctionControl = dynamic_cast<const FixtureFunctionControl *>(&controllable);
		const Chase *chase = dynamic_cast<const Chase *>(&controllable);
		const PresetCollection *presetCollection = dynamic_cast<const PresetCollection *>(&controllable);
	
		if(fixtureFunctionControl != nullptr)
			writeFixtureFunctionControl(*fixtureFunctionControl);
		else if(chase != nullptr)
			writeChase(*chase);
		else if(presetCollection != nullptr)
			writePresetCollection(*presetCollection);
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
		for(std::vector<PresetCollection*>::const_iterator i=presets.begin();i!=presets.end();++i)
			requireControllable(**i);
	
		startElement("sequence");
		writeAttribute("name", sequence.Name());
		for(std::vector<PresetCollection*>::const_iterator i=presets.begin();i!=presets.end();++i)
		{
			startElement("preset-collection-ref");
			writeAttribute("name", (*i)->Name());
			endElement();
		}
		endElement();
		_sequencesWritten.insert(sequence.Name());
	}
}

void Writer::requireSequence(const Sequence &sequence)
{
	if(_sequencesWritten.count(sequence.Name()) == 0)
		writeSequence(sequence);
}

void Writer::requireControllable(const Controllable &controllable)
{
	if(_controllablesWritten.count(controllable.Name()) == 0)
		writeControllable(controllable);
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
	for(const PresetValue* preset : guiState.presets)
	{
		startElement("fader");
		if(preset != nullptr)
			writeAttribute("preset-id", preset->Id());
		endElement(); // preset
	}
	endElement(); // faders
}
