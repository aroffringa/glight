#include "reader.h"

#include "libtheatre/properties/propertyset.h"

#include "libtheatre/chase.h"
#include "libtheatre/controllable.h"
#include "libtheatre/effect.h"
#include "libtheatre/fixture.h"
#include "libtheatre/fixturefunction.h"
#include "libtheatre/fixturefunctioncontrol.h"
#include "libtheatre/folder.h"
#include "libtheatre/presetvalue.h"
#include "libtheatre/scene.h"
#include "libtheatre/show.h"
#include "libtheatre/theatre.h"

#include "gui/guistate.h"

Reader::Reader(Management &management) : _management(management), _theatre(management.Theatre()), _guiState(nullptr)
{
}

void Reader::Read(const Glib::ustring &filename)
{
	_xmlDocument = xmlReadFile(filename.c_str(), NULL, 0);
	if (_xmlDocument == NULL)
		throw std::runtime_error("Failed to parse file");

	xmlNode *rootElement = xmlDocGetRootElement(_xmlDocument);

	for (xmlNode *curNode=rootElement; curNode!=NULL; curNode=curNode->next)
	{
		if(curNode->type == XML_ELEMENT_NODE)
		{
			parseGlightShow(curNode);
		}
	}

	xmlFreeDoc(_xmlDocument);
}

int Reader::getIntAttribute(xmlNode *node, const char *name) const
{
	return atoi(getStringAttribute(node, name).c_str());
}

double Reader::getDoubleAttribute(xmlNode *node, const char *name) const
{
	return atof(getStringAttribute(node, name).c_str());
}

void Reader::parseGlightShow(xmlNode *node)
{
	if(name(node) != "glight-show")
		throw std::runtime_error("File is not a compatible glight show save file");

	for (xmlNode *curNode=node->children; curNode!=NULL; curNode=curNode->next)
	{
		if(curNode->type == XML_ELEMENT_NODE)
			parseGroup(curNode);
	}
}

void Reader::parseGroup(xmlNode *node)
{
	if(name(node) == "folders")
		parseFolders(node);
	else if(name(node) == "theatre")
		parseTheatre(node);
	else if(name(node) == "control")
		parseControl(node);
	else if(name(node) == "show")
		parseShow(node);
	else if(name(node) == "gui")
	{
		if(_guiState == nullptr)
		{
			GUIState scratchState;
			parseGUI(node, scratchState);
		}
		else
			parseGUI(node, *_guiState);
	}
	else throw std::runtime_error(std::string("Invalid node: ") + name(node));
}

void Reader::parseNameAttr(xmlNode* node, class NamedObject& object)
{
	size_t parent = getIntAttribute(node, "parent");
	_management.Folders()[parent]->Add(object);
	object.SetName(getStringAttribute(node, "name"));
}

void Reader::parseFolders(xmlNode *node)
{
	for (xmlNode *curNode=node->children; curNode!=NULL; curNode=curNode->next)
	{
		if(curNode->type == XML_ELEMENT_NODE)
		{
			if(name(curNode) == "folder")
			{
				if(hasAttribute(curNode, "parent"))
				{
					size_t parent = getIntAttribute(curNode, "parent");
					Folder& folder = _management.AddFolder(*_management.Folders()[parent]);
					folder.SetName(getStringAttribute(curNode, "name"));
				}
				else {
					_management.RootFolder().SetName(getStringAttribute(curNode, "name"));
				}
			}
			else throw std::runtime_error(std::string("Invalid node while expecting a folder : ") + name(curNode));
		}
	}
}

void Reader::parseTheatre(xmlNode *node)
{
	for (xmlNode *curNode=node->children; curNode!=NULL; curNode=curNode->next)
	{
		if(curNode->type == XML_ELEMENT_NODE)
			parseTheatreItem(curNode);
	}
}

void Reader::parseControl(xmlNode *node)
{
	for (xmlNode *curNode=node->children; curNode!=NULL; curNode=curNode->next)
	{
		if(curNode->type == XML_ELEMENT_NODE)
			parseControlItem(curNode);
	}
}

void Reader::parseTheatreItem(xmlNode *node)
{
	const std::string n = name(node);
	if(n == "fixture-type")
		parseFixtureType(node);
	else if(n == "fixture")
		parseFixture(node);
	else throw std::runtime_error(std::string("Invalid node while expecting a theatre item : ") + n);
}

void Reader::parseControlItem(xmlNode *node)
{
	const std::string n = name(node);
	if(n == "fixture-function-control")
		parseFixtureFunctionControl(node);
	else if(n == "preset-collection")
		parsePresetCollection(node);
	else if(n == "sequence")
		parseSequence(node);
	else if(n == "chase")
		parseChase(node);
	else if(n == "preset-value")
		parsePresetValue(node);
	else if(n == "effect")
		parseEffect(node);
	else throw std::runtime_error(std::string("Invalid node while expecting a control item: ") + n);
}

void Reader::parseFixtureType(xmlNode *node)
{
	FixtureType &type =
		_theatre.AddFixtureType((enum FixtureType::FixtureClass) getIntAttribute(node, "fixture-class"));
	parseNameAttr(node, type);
}

void Reader::parseFixture(xmlNode *node)
{
	FixtureType &type =
		_theatre.GetFixtureType(getStringAttribute(node, "type"));
	Fixture &fixture =
		_theatre.AddFixture(type);
	parseNameAttr(node, fixture);
	fixture.ClearFunctions();

	for (xmlNode *curNode=node->children; curNode!=NULL; curNode=curNode->next)
	{
		if(curNode->type == XML_ELEMENT_NODE)
		{
			if(name(curNode) == "fixture-function")
				parseFixtureFunction(curNode, fixture);
			else
				throw std::runtime_error("Invalid node in fixture");
		}
	}
}

void Reader::parseFixtureFunction(xmlNode *node, Fixture &parentFixture)
{
	FixtureFunction &function =
		parentFixture.AddFunction((enum FixtureFunction::FunctionType) getIntAttribute(node, "type"));
	parseNameAttr(node, function);
	for (xmlNode *curNode=node->children; curNode!=NULL; curNode=curNode->next)
	{
		if(curNode->type == XML_ELEMENT_NODE)
		{
			if(name(curNode) == "dmx-channel")
			{
				DmxChannel channel;
				parseDmxChannel(curNode, channel);
				function.SetChannel(channel);
			}
			else
				throw std::runtime_error("Invalid node in fixture");
		}
	}
}

void Reader::parseDmxChannel(xmlNode *node, class DmxChannel &dmxChannel)
{
	dmxChannel.SetUniverse(getIntAttribute(node, "universe"));
	dmxChannel.SetChannel(getIntAttribute(node, "channel"));
	dmxChannel.SetDefaultMixStyle((ControlValue::MixStyle) getIntAttribute(node, "default-mix-style"));
}

void Reader::parseFixtureFunctionControl(xmlNode *node)
{
	FixtureFunction &function =
		_theatre.GetFixtureFunction(getStringAttribute(node, "fixture-function-ref"));
	FixtureFunctionControl &control = _management.AddFixtureFunctionControl(function);
	parseNameAttr(node, control);
}

void Reader::parsePresetCollection(xmlNode *node)
{
	PresetCollection &collection =
		_management.AddPresetCollection();
	parseNameAttr(node, collection);
	for (xmlNode *curNode=node->children; curNode!=NULL; curNode=curNode->next)
	{
		if(curNode->type == XML_ELEMENT_NODE)
		{
			if(name(curNode) == "preset-value")
			{
				size_t folderId = getIntAttribute(curNode, "folder");
				Controllable& controllable = static_cast<Controllable&>(
					_management.Folders()[folderId]->GetChild(getStringAttribute(curNode, "controllable-ref")));

				PresetValue &value = collection.AddPresetValue(getIntAttribute(curNode, "id"), controllable);
				value.SetValue(ControlValue(getIntAttribute(curNode, "value")));
			}
			else
				throw std::runtime_error("Bad node in preset collection");
		}
	}
}

void Reader::parseSequence(xmlNode *node)
{
	Sequence &sequence =
		_management.AddSequence();
	parseNameAttr(node, sequence);
	for (xmlNode *curNode=node->children; curNode!=NULL; curNode=curNode->next)
	{
		if(curNode->type == XML_ELEMENT_NODE)
		{
			if(name(curNode) == "preset-collection-ref")
			{
				size_t folderId = getIntAttribute(curNode, "folder");
				PresetCollection& pc = dynamic_cast<PresetCollection&>(
					_management.Folders()[folderId]->GetChild(getStringAttribute(curNode, "name")));
				sequence.AddPreset(&pc);
			}
			else
				throw std::runtime_error("Bad node in sequence");
		}
	}
}

void Reader::parseChase(xmlNode *node)
{
	Sequence &sequence =
		_management.GetSequence(getStringAttribute(node, "sequence-ref"));
	Chase &chase = _management.AddChase(sequence);
	parseNameAttr(node, chase);
	for (xmlNode *curNode=node->children; curNode!=NULL; curNode=curNode->next)
	{
		if(curNode->type == XML_ELEMENT_NODE)
		{
			if(name(curNode) == "trigger")
				parseTrigger(curNode, chase.Trigger());
			else if(name(curNode) == "transition")
				parseTransition(curNode, chase.Transition());
			else throw std::runtime_error("Bad node in chase");
		}
	}
}

void Reader::parsePresetValue(xmlNode *node)
{
	Controllable &controllable =
		_management.GetControllable(getStringAttribute(node, "controllable-ref"));
	PresetValue &value = _management.AddPreset(getIntAttribute(node, "id"), controllable);
	value.SetValue(ControlValue(getIntAttribute(node, "value")));
}

void Reader::parseEffect(xmlNode* node)
{
	Effect::Type type = Effect::NameToType(getStringAttribute(node, "type"));
	std::unique_ptr<Effect> effect = Effect::Make(type);
	parseNameAttr(node, *effect);
	Effect* effectPtr = &_management.AddEffect(std::move(effect));
	std::unique_ptr<PropertySet> ps = PropertySet::Make(*effectPtr);
	for (xmlNode *curNode=node->children; curNode!=NULL; curNode=curNode->next)
	{
		if(curNode->type == XML_ELEMENT_NODE)
		{
			if(name(curNode) == "property")
			{
				std::string propName = getStringAttribute(curNode, "name");
				Property& p = ps->GetProperty(propName);
				switch(p.GetType())
				{
				case Property::ControlValue:
					ps->SetControlValue(p, getIntAttribute(curNode, "value"));
					break;
				case Property::Duration:
					ps->SetDuration(p, getDoubleAttribute(curNode, "value"));
					break;
				}
			}
			else if(name(curNode) == "connection-ref")
			{
				std::string cName = getStringAttribute(curNode, "name");
				effectPtr->AddConnection(&_management.GetControllable(cName));
			}
			else throw std::runtime_error("Bad element in effect");
		}
	}
}

void Reader::parseTrigger(xmlNode *node, class Trigger &trigger)
{
	trigger.SetType((enum Trigger::Type) getIntAttribute(node, "type"));
	trigger.SetDelayInMs(getDoubleAttribute(node, "delay-in-ms"));
}

void Reader::parseTransition(xmlNode *node, class Transition &transition)
{
	transition.SetType((enum Transition::Type) getIntAttribute(node, "type"));
	transition.SetLengthInMs(getDoubleAttribute(node, "length-in-ms"));
}

void Reader::parseShow(xmlNode *node)
{
	for (xmlNode *curNode=node->children; curNode!=NULL; curNode=curNode->next)
	{
		if(curNode->type == XML_ELEMENT_NODE)
			parseShowItem(curNode);
	}
}

void Reader::parseShowItem(xmlNode *node)
{
	const std::string n = name(node);
	if(n == "scene")
		parseScene(node);
	else throw std::runtime_error(std::string("Invalid node while expecting a show item: ") + n);
}

void Reader::parseScene(xmlNode *node)
{
	Scene *scene = _management.Show().AddScene();
	parseNameAttr(node, *scene);
	scene->SetAudioFile(getStringAttribute(node, "audio-file"));

	for (xmlNode *curNode=node->children; curNode!=NULL; curNode=curNode->next)
	{
		if(curNode->type == XML_ELEMENT_NODE)
		{
			if(name(curNode) == "scene-item")
				parseSceneItem(curNode, *scene);
			else
				throw std::runtime_error(std::string("Invalid node while expecting a scene item: ") + name(curNode));
		}
	}
}

void Reader::parseSceneItem(xmlNode *node, Scene &scene)
{
	std::string type = getStringAttribute(node, "type");
	SceneItem *item;
	if(type == "key")
		item = &parseKeySceneItem(node, scene);
	else if(type == "control")
		item = &parseControlSceneItem(node, scene);
	else
		throw std::runtime_error(std::string("Invalid type for scene item: ") + type);
	item->SetDurationInMS(getDoubleAttribute(node, "duration"));
}

KeySceneItem &Reader::parseKeySceneItem(xmlNode *node, Scene &scene)
{
	KeySceneItem *item = scene.AddKeySceneItem(getDoubleAttribute(node, "offset"));
	item->SetLevel((enum KeySceneItem::Level) getIntAttribute(node, "level"));
	return *item;
}

ControlSceneItem &Reader::parseControlSceneItem(xmlNode *node, Scene &scene)
{
	Controllable &controllable =
		_management.GetControllable(getStringAttribute(node, "controllable-ref"));
	ControlSceneItem *item = scene.AddControlSceneItem(getDoubleAttribute(node, "offset"), controllable);
	item->StartValue().Set(getIntAttribute(node, "start-value"));
	item->EndValue().Set(getIntAttribute(node, "end-value"));
	return *item;
}

void Reader::parseGUI(xmlNode* node, GUIState& guiState)
{
	for (xmlNode *curNode=node->children; curNode!=NULL; curNode=curNode->next)
	{
		if(curNode->type == XML_ELEMENT_NODE)
			parseGUIItem(curNode, guiState);
	}
}

void Reader::parseGUIItem(xmlNode* node, GUIState& guiState)
{
	const std::string n = name(node);
	if(n == "faders")
		parseGUIFaders(node, guiState);
	else
		throw std::runtime_error("Invalid GUI element: " + n);
}

void Reader::parseGUIFaders(xmlNode* node, GUIState& guiState)
{
	guiState.FaderSetups().emplace_back(new FaderSetupState());
	FaderSetupState& fader = *guiState.FaderSetups().back().get();
	fader.name = getStringAttribute(node, "name");
	fader.isActive = getBoolAttribute(node, "active");
	fader.isSolo = getBoolAttribute(node, "solo");
	fader.width = getIntAttribute(node, "width");
	fader.height = getIntAttribute(node, "height");
	for (xmlNode *curNode=node->children; curNode!=NULL; curNode=curNode->next)
	{
		if(curNode->type == XML_ELEMENT_NODE)
		{
			const std::string n = name(curNode);
			if(n == "fader")
				parseGUIPresetRef(curNode, fader);
			else
				throw std::runtime_error("Invalid GUI fader element: " + n);
		}
	}
}

void Reader::parseGUIPresetRef(xmlNode* node, FaderSetupState& fader)
{
	if(hasAttribute(node, "preset-id"))
	{
		fader.faders.emplace_back(_management.GetPresetValue(getIntAttribute(node, "preset-id")));
	}
	else
		fader.faders.emplace_back(nullptr);
}
