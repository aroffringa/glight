#include "defaultchase.h"

#include "management.h"

#include "color.h"
#include "fixture.h"
#include "fixturefunctioncontrol.h"
#include "presetcollection.h"
#include "sequence.h"

#include <algorithm>
#include <random>

Sequence& DefaultChase::MakeRunningLight(Management& management, const std::vector<Fixture*>& fixtures, const std::vector<class Color>& colors, RunType runType)
{
	Sequence& seq = management.AddSequence();
	seq.SetName("Runchase");
	size_t frames = colors.size();
	if(runType == InwardRun || runType == OutwardRun)
		frames = (frames+1)/2;
	std::vector<size_t> pos;
	if(runType == RandomRun)
	{
		pos.resize(frames);
		for(size_t i=0; i!=frames; ++i)
			pos[i] = i;
		std::random_device rd;
		std::mt19937 mt(rd());
		std::shuffle(pos.begin(), pos.end(), mt);
	}
	for(size_t frameIndex=0; frameIndex!=frames; ++frameIndex)
	{
		PresetCollection& pc = management.AddPresetCollection();
		pc.SetName("Runchase" + std::to_string(frameIndex+1));
		unsigned
			red = colors[frameIndex].Red()*((1<<24)-1)/255,
			green = colors[frameIndex].Green()*((1<<24)-1)/255,
			blue = colors[frameIndex].Blue()*((1<<24)-1)/255,
			master = 0;
		if(red != 0 || green != 0 || blue != 0)
			master = (1<<24)-1;
		for(size_t i = 0; i < (fixtures.size() + colors.size() - 1) / colors.size(); ++i)
		{
			size_t fixIndex = 0;
			switch(runType)
			{
				case IncreasingRun:
				case BackAndForthRun:
				case InwardRun:
					fixIndex = frameIndex + i * colors.size();
					break;
				case DecreasingRun:
				case OutwardRun:
					fixIndex = frames - frameIndex - 1 + i * colors.size();
					break;
				case RandomRun:
					fixIndex = pos[frameIndex] + i * colors.size();
					break;
			}
			if(fixIndex < fixtures.size())
			{
				Fixture* f = fixtures[fixIndex];
				addColorPresets(management, *f, pc, red, green, blue, master);
			}
		}
		seq.AddPreset(&pc);
		management.AddPreset(pc);
	}
	if(runType == BackAndForthRun)
	{
		for(size_t i=2; i<colors.size(); ++i)
			seq.AddPreset(seq.Presets()[colors.size()-i]);
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
