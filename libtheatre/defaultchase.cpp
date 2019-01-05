#include "defaultchase.h"

#include "management.h"

#include "color.h"
#include "fixture.h"
#include "fixturefunctioncontrol.h"
#include "presetcollection.h"
#include "sequence.h"

#include <random>

Sequence& DefaultChase::MakeRunningLight(Management& management, const std::vector<Fixture*>& fixtures, const std::vector<class Color>& colors)
{
	Sequence& seq = management.AddSequence();
	seq.SetName("Runchase");
	for(size_t chaseIndex=0; chaseIndex!=colors.size(); ++chaseIndex)
	{
		PresetCollection& pc = management.AddPresetCollection();
		pc.SetName("Runchase" + std::to_string(chaseIndex+1));
		unsigned
			red = colors[chaseIndex].Red()*((1<<24)-1)/255,
			green = colors[chaseIndex].Green()*((1<<24)-1)/255,
			blue = colors[chaseIndex].Blue()*((1<<24)-1)/255,
			master = 0;
		if(red != 0 || green != 0 || blue != 0)
			master = (1<<24)-1;
		for(size_t fixIndex = chaseIndex; fixIndex < fixtures.size(); fixIndex += colors.size())
		{
			Fixture* f = fixtures[fixIndex];
			addColorPresets(management, *f, pc, red, green, blue, master);
		}
		seq.AddPreset(&pc);
		management.AddPreset(pc);
	}
	return seq;
}

void DefaultChase::addColorPresets(Management& management, Fixture& f, PresetCollection& pc, unsigned red, unsigned green, unsigned blue, unsigned master)
{
	for(const std::unique_ptr<FixtureFunction>& ff : f.Functions())
	{
		if(ff->Type() == FixtureFunction::RedIntensity && red != 0)
		{
			Controllable& c = management.GetControllable(ff->Name());
			pc.AddPresetValue(*management.GetPresetValue(c)).SetValue(red);
		}
		else if(ff->Type() == FixtureFunction::GreenIntensity && green != 0)
		{
			Controllable& c = management.GetControllable(ff->Name());
			pc.AddPresetValue(*management.GetPresetValue(c)).SetValue(green);
		}
		else if(ff->Type() == FixtureFunction::BlueIntensity && blue != 0)
		{
			Controllable& c = management.GetControllable(ff->Name());
			pc.AddPresetValue(*management.GetPresetValue(c)).SetValue(blue);
		}
		else if(ff->Type() == FixtureFunction::Brightness && master != 0)
		{
			Controllable& c = management.GetControllable(ff->Name());
			pc.AddPresetValue(*management.GetPresetValue(c)).SetValue(master);
		}
	}
}
#include <iostream>
Sequence& DefaultChase::MakeColorVariation(class Management& management, const std::vector<class Fixture *>& fixtures, const std::vector<class Color>& colors, double variation)
{
	Sequence& seq = management.AddSequence();
	seq.SetName("Colorseq");
	std::random_device rd;
	std::mt19937 rnd(rd());
	std::normal_distribution<double> distribution(0.0, variation*double((1<<24)-1)/255.0);
	for(size_t chaseIndex=0; chaseIndex!=colors.size(); ++chaseIndex)
	{
		PresetCollection& pc = management.AddPresetCollection();
		pc.SetName("Colorseq" + std::to_string(chaseIndex+1));
		unsigned
			red = colors[chaseIndex].Red()*((1<<24)-1)/255,
			green = colors[chaseIndex].Green()*((1<<24)-1)/255,
			blue = colors[chaseIndex].Blue()*((1<<24)-1)/255,
			master = 0;
		if(red != 0 || green != 0 || blue != 0)
			master = (1<<24)-1;
		for(Fixture* f : fixtures)
		{
			double
				redVar = round(distribution(rnd)),
				greenVar = round(distribution(rnd)),
				blueVar = round(distribution(rnd));
			unsigned
				rv = std::max<double>(0.0, std::min<double>(double(red) + redVar, (1<<24)-1)),
				gv = std::max<double>(0.0, std::min<double>(double(green) + greenVar, (1<<24)-1)),
				bv = std::max<double>(0.0, std::min<double>(double(blue) + blueVar, (1<<24)-1));
			addColorPresets(management, *f, pc, rv, gv, bv, master);
		}
		seq.AddPreset(&pc);
		management.AddPreset(pc);
	}
	return seq;
}
