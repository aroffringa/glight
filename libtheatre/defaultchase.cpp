#include "defaultchase.h"

#include "color.h"
#include "fixture.h"
#include "folder.h"
#include "management.h"
#include "presetcollection.h"
#include "sequence.h"

#include "effects/audioleveleffect.h"
#include "effects/thresholdeffect.h"

#include <algorithm>
#include <random>

Sequence& DefaultChase::MakeRunningLight(Management& management, const std::vector<Fixture*>& fixtures, const std::vector<class Color>& colors, RunType runType)
{
	Folder& folder = management.RootFolder(); // TODO
	Sequence& seq = management.AddSequence();
	seq.SetName("Runchase");
	folder.Add(seq);
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
		folder.Add(pc);
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
		management.AddPreset(pc, 0);
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
	for(size_t i=0; i!=f.Functions().size(); ++i)
	{
		const std::unique_ptr<FixtureFunction>& ff = f.Functions()[i];
		if(ff->Type() == FixtureFunction::RedIntensity && red != 0)
		{
			Controllable& c = management.GetControllable(f.Name());
			pc.AddPresetValue(*management.GetPresetValue(c, i)).SetValue(red);
		}
		else if(ff->Type() == FixtureFunction::GreenIntensity && green != 0)
		{
			Controllable& c = management.GetControllable(f.Name());
			pc.AddPresetValue(*management.GetPresetValue(c, i)).SetValue(green);
		}
		else if(ff->Type() == FixtureFunction::BlueIntensity && blue != 0)
		{
			Controllable& c = management.GetControllable(f.Name());
			pc.AddPresetValue(*management.GetPresetValue(c, i)).SetValue(blue);
		}
		else if(ff->Type() == FixtureFunction::Brightness && master != 0)
		{
			Controllable& c = management.GetControllable(f.Name());
			pc.AddPresetValue(*management.GetPresetValue(c, i)).SetValue(master);
		}
	}
}

Sequence& DefaultChase::MakeColorVariation(class Management& management, const std::vector<class Fixture *>& fixtures, const std::vector<class Color>& colors, double variation)
{
	Folder& folder = management.RootFolder(); // TODO
	Sequence& seq = management.AddSequence();
	seq.SetName("Colorseq");
	folder.Add(seq);
	std::random_device rd;
	std::mt19937 rnd(rd());
	std::normal_distribution<double> distribution(0.0, variation*double((1<<24)-1)/255.0);
	for(size_t chaseIndex=0; chaseIndex!=colors.size(); ++chaseIndex)
	{
		PresetCollection& pc = management.AddPresetCollection();
		folder.Add(pc);
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
		management.AddPreset(pc, 0);
	}
	return seq;
}

Controllable& DefaultChase::MakeVUMeter(Management& management, const std::vector<Fixture*>& fixtures, const std::vector<Color>& colors, VUMeterDirection direction)
{
	Folder& folder = management.RootFolder(); // TODO
	if(colors.size() != fixtures.size())
		throw std::runtime_error("Number of colours did not match number of fixtures");
	std::unique_ptr<AudioLevelEffect> audioLevel(new AudioLevelEffect());
	Effect& newAudioLevel = management.AddEffect(std::move(audioLevel), folder);
	for(size_t inp=0; inp!=newAudioLevel.NInputs(); ++inp)
		management.AddPreset(newAudioLevel, inp);
	newAudioLevel.SetName("VUMeter");
	size_t nLevels;
	if(direction == VUInward || direction == VUOutward)
		nLevels = (fixtures.size()+1) / 2;
	else
		nLevels = fixtures.size();
	for(size_t i=0; i!=nLevels; ++i)
	{
		std::unique_ptr<ThresholdEffect> threshold(new ThresholdEffect());
		threshold->SetLowerStartLimit(((1<<24)-1)*i/nLevels);
		threshold->SetLowerEndLimit(((1<<24)-1)*(i+1)/nLevels);
		Effect& newEffect = management.AddEffect(std::move(threshold), folder);
		for(size_t inp=0; inp!=newEffect.NInputs(); ++inp)
			management.AddPreset(newEffect, inp);
		newEffect.SetName("VUM" + std::to_string(i+1) + "_Thr");
		
		size_t nFixInLevel = 1;
		if((direction == VUInward && (i != nLevels-1 || fixtures.size()%2==0) ) ||
			(direction == VUOutward && (i != 0 || fixtures.size()%2==0) ) )
			nFixInLevel = 2;
		
		PresetCollection& pc = management.AddPresetCollection();
		pc.SetName("VUM" + std::to_string(i+1));
		folder.Add(pc);
		for(size_t fixInLevel=0; fixInLevel!=nFixInLevel; ++fixInLevel)
		{
			size_t fixIndex;
			if(fixInLevel == 0)
			{
				switch(direction)
				{
					case VUIncreasing:
					case VUInward: fixIndex = i; break;
					case VUOutward: fixIndex = fixtures.size()/2 - i; break;
					case VUDecreasing: fixIndex = nLevels - i - 1; break;
				}
			}
			else {
				if(direction == VUInward)
					fixIndex = fixtures.size() - i - 1;
				else
					fixIndex = nLevels + i - 1;
			}
			unsigned
				red = colors[fixIndex].Red()*((1<<24)-1)/255,
				green = colors[fixIndex].Green()*((1<<24)-1)/255,
				blue = colors[fixIndex].Blue()*((1<<24)-1)/255,
				master = 0;
			if(red != 0 || green != 0 || blue != 0)
				master = (1<<24)-1;
			addColorPresets(management, *fixtures[fixIndex], pc, red, green, blue, master);
		}
		management.AddPreset(pc, 0);
		newEffect.AddConnection(&pc, 0);
		newAudioLevel.AddConnection(&newEffect, 0);
	}
	return newAudioLevel;
}
