#ifndef WRITER_H
#define WRITER_H

#include <set>
#include <stdexcept>
#include <string>

#include <glibmm/ustring.h>

#include <libxml/xmlwriter.h>

#include "libtheatre/management.h"

class WriterException : public std::runtime_error {
	public:
		WriterException(const std::string &msg) : std::runtime_error(msg) { }
};

/**
	@author Andre Offringa
*/
class Writer {
	public:
		Writer(class Management &management);
		~Writer() { }
		
		void SetGUIState(class GUIState& guiState) { _guiState = &guiState; }

		void Write(const Glib::ustring &filename);
		static void CheckXmlVersion();

	private:
		void writeGlightShow();

		void writeDmxChannel(const class DmxChannel &dmxChannel);
		void writeFixtureType(const class FixtureType &fixtureType);
		void writeFixture(const class Fixture &fixture);
		void writeFixtureFunction(const class FixtureFunction &fixtureFunction);
		void writePresetCollection(const class PresetCollection &presetCollection);
		void writePresetValue(const class PresetValue &presetValue);
		void writeControllable(const class Controllable &controllable);
		void writeFixtureFunctionControl(const class FixtureFunctionControl &control);
		void writeChase(const class Chase &control);
		void writeTransition(const class Transition &transition);
		void writeTrigger(const class Trigger &trigger);
		void writeSequence(const class Sequence& sequence);
		void writeEffect(const class Effect& effect);
		
		void writeScene(const class Scene &scene);
		void writeSceneItem(const class SceneItem &item);
		void writeKeySceneItem(const class KeySceneItem &item);
		void writeControlSceneItem(const class ControlSceneItem &item);
		
		void writeGUIState(const class GUIState& guiState);
		void writeFaderState(const class FaderSetupState& guiState);

		void startElement(const char *elementName);
		void endElement();
		void writeElement(const char *elementName, const char *elementValue);
		void writeElement(const char *elementName, const std::string &elementValue)
		{ writeElement(elementName, elementValue.c_str()); }
		void writeAttribute(const char *attributeName, const char *attributeValue);
		void writeAttribute(const char *attributeName, const std::string &attributeValue)
		{ writeAttribute(attributeName, attributeValue.c_str()); }
		void writeAttribute(const char *attributeName, int attributeValue);

		class Management& _management;
		class GUIState* _guiState;
		
		xmlTextWriterPtr _writer;
		const char *_encoding;

		void requireSequence(const class Sequence& sequence)
		{ writeSequence(sequence); }

		void requireControllable(const class Controllable& controllable)
		{ writeControllable(controllable); }

		std::set<std::string> _sequencesWritten;
		std::set<std::string> _controllablesWritten;
		std::set<std::string> _effectsWritten;
};

#endif
