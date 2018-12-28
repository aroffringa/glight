#ifndef READER_H
#define READER_H

#include <stdexcept>
#include <string>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include <glibmm/ustring.h>

/**
	@author Andre Offringa
*/
class Reader{
	public:
		Reader(class Management &management);
		~Reader();

		void Read(const Glib::ustring &filename);
	private:
		std::string getStringAttribute(xmlNode *node, const char *name) const
		{
			xmlChar *typeCh = xmlGetProp(node, BAD_CAST name);
			if(typeCh == 0)
				throw std::runtime_error(std::string("Could not find attribute ") + name);
			std::string typeStr((const char*) typeCh);
			xmlFree(typeCh);
			return typeStr;
		}
		int getIntAttribute(xmlNode *node, const char *name) const;
		double getDoubleAttribute(xmlNode *node, const char *name) const;
		std::string name(xmlNode *node) { return std::string((const char *) node->name); }

		void parseGlightShow(xmlNode *node);
		void parseGroup(xmlNode *node);
		void parseTheatre(xmlNode *node);
		void parseTheatreItem(xmlNode *node);
		void parseControl(xmlNode *node);
		void parseControlItem(xmlNode *node);
		void parseFixtureType(xmlNode *node);
		void parseFixture(xmlNode *node);
		void parseFixtureFunctionControl(xmlNode *node);
		void parsePresetCollection(xmlNode *node);
		void parseSequence(xmlNode *node);
		void parseChase(xmlNode *node);
		void parsePresetValue(xmlNode *node);

		void parseFixtureFunction(xmlNode *node, class Fixture &parentFixture);
		void parseDmxChannel(xmlNode *node, class DmxChannel &dmxChannel);
		void parseTrigger(xmlNode *node, class Trigger &trigger);
		void parseTransition(xmlNode *node, class Transition &transition);

		void parseShow(xmlNode *node);
		void parseShowItem(xmlNode *node);
		void parseScene(xmlNode *node);
		void parseSceneItem(xmlNode *node, class Scene &scene);

		class KeySceneItem &parseKeySceneItem(xmlNode *node, class Scene &scene);
		class ControlSceneItem &parseControlSceneItem(xmlNode *node, class Scene &scene);

		xmlDocPtr _xmlDocument;
		class Management &_management;
		class Theatre &_theatre;
};

#endif
