#include "defaultchase.h"

#include "chase.h"
#include "color.h"
#include "controllable.h"
#include "folder.h"
#include "management.h"
#include "presetcollection.h"
#include "sequence.h"

#include "effects/audioleveleffect.h"
#include "effects/thresholdeffect.h"

#include <algorithm>
#include <random>

void DefaultChase::addColorPresets(Management& management, Controllable& control, PresetCollection& pc, unsigned red, unsigned green, unsigned blue, unsigned master)
{
	for(size_t i=0; i!=control.NInputs(); ++i)
	{
		Color c = control.InputColor(i);
		if(control.InputType(i) == FunctionType::Master && master != 0)
			pc.AddPresetValue(*management.GetPresetValue(control, i)).SetValue(master);
		else if(c == Color::White())
		{
			unsigned white = std::min(red, std::min(green, blue));
			if(white != 0)
				pc.AddPresetValue(*management.GetPresetValue(control, i)).SetValue(white);
		}
		else if(c == Color::Amber())
		{
			unsigned amber = std::min(red, green/2);
			if(amber != 0)
			{
				pc.AddPresetValue(*management.GetPresetValue(control, i)).SetValue(amber);
			}
		}
		else {
			if(c.Red() != 0 && red != 0)
				pc.AddPresetValue(*management.GetPresetValue(control, i)).SetValue(red);
			if(c.Green() != 0 && green != 0)
				pc.AddPresetValue(*management.GetPresetValue(control, i)).SetValue(green);
			if(c.Blue() != 0 && blue != 0)
				pc.AddPresetValue(*management.GetPresetValue(control, i)).SetValue(blue);
		}
	}
}

PresetCollection& DefaultChase::MakeColorPreset(class Management& management, class Folder& destination, const std::vector<class Controllable*>& controllables, const std::vector<class Color>& colors)
{
	PresetCollection& pc = management.AddPresetCollection();
	pc.SetName(destination.GetAvailableName("Colourpreset"));
	destination.Add(pc);
	for(size_t cIndex=0; cIndex!=controllables.size(); ++cIndex)
	{
		size_t colorIndex = cIndex % colors.size();
		unsigned
			red = colors[colorIndex].Red()*((1<<24)-1)/255,
			green = colors[colorIndex].Green()*((1<<24)-1)/255,
			blue = colors[colorIndex].Blue()*((1<<24)-1)/255,
			master = 0;
		if(red != 0 || green != 0 || blue != 0)
			master = (1<<24)-1;
		
		addColorPresets(management, *controllables[cIndex], pc, red, green, blue, master);
	}
	management.AddPreset(pc, 0);
	return pc;
}

Chase& DefaultChase::MakeRunningLight(Management& management, Folder& destination, const std::vector<class Controllable*>& controllables, const std::vector<class Color>& colors, RunType runType)
{
	Chase& chase = management.AddChase();
	chase.SetName(destination.GetAvailableName("Runchase"));
	destination.Add(chase);
	management.AddPreset(chase, 0);
	Sequence& seq = chase.Sequence();
	size_t frames;
	if(runType == InwardRun || runType == OutwardRun)
		frames = (colors.size()+1)/2;
	else
		frames = colors.size();
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
		size_t nFixInPattern = 1;
		if((runType == InwardRun && (frameIndex != frames-1 || colors.size()%2==0) ) ||
			(runType == OutwardRun && (frameIndex != 0 || colors.size()%2==0) ) )
			nFixInPattern = 2;
		
		PresetCollection& pc = management.AddPresetCollection();
		pc.SetName(destination.GetAvailableName(chase.Name() + "_"));
		destination.Add(pc);
		// If there are less colours given than fixtures, the sequence is repeated
		// several times. This loop is for that purpose.
		for(size_t patternIndex = 0; patternIndex < (controllables.size() + colors.size() - 1) / colors.size(); ++patternIndex)
		{
			for(size_t fixInPatIndex = 0; fixInPatIndex != nFixInPattern; ++fixInPatIndex)
			{
				size_t cIndex = 0;
				switch(runType)
				{
					case IncreasingRun:
					case BackAndForthRun:
						cIndex = frameIndex + patternIndex * colors.size();
						break;
					case DecreasingRun:
						cIndex = frames - frameIndex - 1 + patternIndex * colors.size();
						break;
					case RandomRun:
						cIndex = pos[frameIndex] + patternIndex * colors.size();
						break;
					case InwardRun:
						if(fixInPatIndex == 0)
							cIndex = frameIndex + patternIndex * colors.size();
						else
							cIndex = (colors.size() - frameIndex - 1) + patternIndex * colors.size();
						break;
					case OutwardRun:
						if(fixInPatIndex == 0)
							cIndex = frames - frameIndex - 1 + patternIndex * colors.size();
						else
							cIndex = frames + frameIndex + patternIndex * colors.size();
						break;
				}
				if(cIndex < controllables.size())
				{
					size_t colourIndex = cIndex % colors.size();
					unsigned
						red = colors[colourIndex].Red()*((1<<24)-1)/255,
						green = colors[colourIndex].Green()*((1<<24)-1)/255,
						blue = colors[colourIndex].Blue()*((1<<24)-1)/255,
						master = 0;
					if(red != 0 || green != 0 || blue != 0)
						master = (1<<24)-1;
					
					addColorPresets(management, *controllables[cIndex], pc, red, green, blue, master);
				}
			}
		}
		seq.Add(pc, 0);
		management.AddPreset(pc, 0);
	}
	if(runType == BackAndForthRun)
	{
		for(size_t i=2; i<colors.size(); ++i)
			seq.Add(*seq.List()[colors.size()-i].first, 0);
	}
	return chase;
}

Chase& DefaultChase::MakeColorVariation(class Management& management, Folder& destination, const std::vector<class Controllable*>& controllables, const std::vector<class Color>& colors, double variation)
{
	Chase& chase = management.AddChase();
	chase.SetName(destination.GetAvailableName("Colorvar"));
	destination.Add(chase);
	management.AddPreset(chase, 0);
	Sequence& seq = chase.Sequence();
	std::random_device rd;
	std::mt19937 rnd(rd());
	std::normal_distribution<double> distribution(0.0, variation*double((1<<24)-1)/255.0);
	for(size_t chaseIndex=0; chaseIndex!=colors.size(); ++chaseIndex)
	{
		PresetCollection& pc = management.AddPresetCollection();
		pc.SetName(destination.GetAvailableName(chase.Name() + "_"));
		destination.Add(pc);
		unsigned
			red = colors[chaseIndex].Red()*((1<<24)-1)/255,
			green = colors[chaseIndex].Green()*((1<<24)-1)/255,
			blue = colors[chaseIndex].Blue()*((1<<24)-1)/255,
			master = 0;
		if(red != 0 || green != 0 || blue != 0)
			master = (1<<24)-1;
		for(Controllable* c : controllables)
		{
			double
				redVar = round(distribution(rnd)),
				greenVar = round(distribution(rnd)),
				blueVar = round(distribution(rnd));
			unsigned
				rv = std::max<double>(0.0, std::min<double>(double(red) + redVar, (1<<24)-1)),
				gv = std::max<double>(0.0, std::min<double>(double(green) + greenVar, (1<<24)-1)),
				bv = std::max<double>(0.0, std::min<double>(double(blue) + blueVar, (1<<24)-1));
			addColorPresets(management, *c, pc, rv, gv, bv, master);
		}
		seq.Add(pc, 0);
		management.AddPreset(pc, 0);
	}
	return chase;
}

Chase& DefaultChase::MakeColorShift(Management& management, Folder& destination, const std::vector<class Controllable*>& controllables, const std::vector<Color>& colors, ShiftType shiftType)
{
	Chase& chase = management.AddChase();
	chase.SetName(destination.GetAvailableName("Colourshift"));
	destination.Add(chase);
	management.AddPreset(chase, 0);
	Sequence& seq = chase.Sequence();
	size_t frames = controllables.size();
	std::vector<std::vector<size_t>> pos(frames);
	std::random_device rd;
	std::mt19937 mt(rd());
	for(size_t frameIndex=0; frameIndex!=frames; ++frameIndex)
	{
		if(shiftType == RandomShift)
		{
			pos[frameIndex].resize(controllables.size());
			bool duplicate;
			do {
				for(size_t i=0; i!=pos[frameIndex].size(); ++i)
					pos[frameIndex][i] = i;
				std::shuffle(pos[frameIndex].begin(), pos[frameIndex].end(), mt);
				duplicate = false;
				// Check whether previous frames are equal to the new frame
				for(size_t i=0; i!=frameIndex; ++i)
					duplicate = duplicate || pos[i] == pos[frameIndex];
				// Check whether all fixtures are switched to a new position
				// (if all colours are different, this guarantees that the
				//  fixture changes colour)
				if(frameIndex != 0)
				{
					for(size_t i=0; i!=pos[frameIndex].size(); ++i)
						duplicate = duplicate || pos[frameIndex][i] == pos[frameIndex-1][i];
				}
				// For the last frame, also check whether all positions are different compared
				// to the first frame.
				if(frameIndex == frames-1)
				{
					for(size_t i=0; i!=pos[frameIndex].size(); ++i)
						duplicate = duplicate || pos[frameIndex][i] == pos[0][i];
				}
			} while(duplicate);
		}
		
		PresetCollection& pc = management.AddPresetCollection();
		pc.SetName(destination.GetAvailableName(chase.Name() + "_"));
		destination.Add(pc);
		
		for(size_t cIndex=0; cIndex!=controllables.size(); ++cIndex)
		{
			size_t colourIndex;
			switch(shiftType)
			{
				case IncreasingShift:
				case BackAndForthShift:
					colourIndex = (cIndex + frames - frameIndex) % frames;
					break;
				case DecreasingShift:
					colourIndex = (cIndex + frameIndex) % frames;
					break;
				case RandomShift:
					colourIndex = pos[frameIndex][cIndex];
					break;
			}
			unsigned
				red = colors[colourIndex].Red()*((1<<24)-1)/255,
				green = colors[colourIndex].Green()*((1<<24)-1)/255,
				blue = colors[colourIndex].Blue()*((1<<24)-1)/255,
				master = 0;
			if(red != 0 || green != 0 || blue != 0)
				master = (1<<24)-1;
			addColorPresets(management, *controllables[cIndex], pc, red, green, blue, master);
		}
		seq.Add(pc, 0);
		management.AddPreset(pc, 0);
	}
	if(shiftType == BackAndForthShift)
	{
		for(size_t i=2; i<frames; ++i)
			seq.Add(*seq.List()[frames-i].first, 0);
	}
	return chase;
}

Controllable& DefaultChase::MakeVUMeter(Management& management, Folder& destination, const std::vector<class Controllable*>& controllables, const std::vector<Color>& colors, VUMeterDirection direction)
{
	if(colors.size() != controllables.size())
		throw std::runtime_error("Number of colours did not match number of fixtures");
	std::unique_ptr<AudioLevelEffect> audioLevel(new AudioLevelEffect());
	Effect& newAudioLevel = management.AddEffect(std::move(audioLevel), destination);
	for(size_t inp=0; inp!=newAudioLevel.NInputs(); ++inp)
		management.AddPreset(newAudioLevel, inp);
	newAudioLevel.SetName(destination.GetAvailableName("VUMeter"));
	size_t nLevels;
	if(direction == VUInward || direction == VUOutward)
		nLevels = (controllables.size()+1) / 2;
	else
		nLevels = controllables.size();
	for(size_t level=0; level!=nLevels; ++level)
	{
		std::unique_ptr<ThresholdEffect> threshold(new ThresholdEffect());
		threshold->SetLowerStartLimit(((1<<24)-1)*level/nLevels);
		threshold->SetLowerEndLimit(((1<<24)-1)*(level+1)/nLevels);
		Effect& newEffect = management.AddEffect(std::move(threshold), destination);
		for(size_t inp=0; inp!=newEffect.NInputs(); ++inp)
			management.AddPreset(newEffect, inp);
		newEffect.SetName(destination.GetAvailableName(newAudioLevel.Name() + "_Thr"));
		
		size_t nFixInLevel = 1;
		if((direction == VUInward && (level != nLevels-1 || controllables.size()%2==0) ) ||
			(direction == VUOutward && (level != 0 || controllables.size()%2==0) ) )
			nFixInLevel = 2;
		
		PresetCollection& pc = management.AddPresetCollection();
		pc.SetName(destination.GetAvailableName(newAudioLevel.Name() + "_Set"));
		destination.Add(pc);
		for(size_t fixInLevel=0; fixInLevel!=nFixInLevel; ++fixInLevel)
		{
			size_t fixIndex;
			if(fixInLevel == 0)
			{
				switch(direction)
				{
					case VUIncreasing:
					case VUInward: fixIndex = level; break;
					case VUOutward:
					case VUDecreasing: fixIndex = nLevels - level - 1; break;
				}
			}
			else {
				if(direction == VUInward)
					fixIndex = controllables.size() - level - 1;
				else // VUOutward
					fixIndex = nLevels + level;
			}
			unsigned
				red = colors[fixIndex].Red()*((1<<24)-1)/255,
				green = colors[fixIndex].Green()*((1<<24)-1)/255,
				blue = colors[fixIndex].Blue()*((1<<24)-1)/255,
				master = 0;
			if(red != 0 || green != 0 || blue != 0)
				master = (1<<24)-1;
			addColorPresets(management, *controllables[fixIndex], pc, red, green, blue, master);
		}
		management.AddPreset(pc, 0);
		newEffect.AddConnection(pc, 0);
		newAudioLevel.AddConnection(newEffect, 0);
	}
	return newAudioLevel;
}

Chase& DefaultChase::MakeIncreasingChase(Management& management, Folder& destination, const std::vector<class Controllable*>& controllables, const std::vector<class Color>& colors, IncreasingType incType)
{
	if(colors.size() != controllables.size())
		throw std::runtime_error("Number of controllables does not match number of provided colours");
	Chase& chase = management.AddChase();
	chase.SetName(destination.GetAvailableName("Increasing chase"));
	destination.Add(chase);
	management.AddPreset(chase, 0);
	Sequence& seq = chase.Sequence();
	
	size_t nFix = controllables.size();
	for(size_t frameIndex=0; frameIndex!= nFix * 2; ++frameIndex)
	{
		size_t startFixture = 0, endFixture = 0;
		if(frameIndex < controllables.size()) // building up
		{
			switch(incType)
			{
				case IncForward:
				case IncForwardReturn:
					startFixture = 0;
					endFixture = frameIndex;
					break;
				case IncBackward:
				case IncBackwardReturn:
					startFixture = nFix - frameIndex;
					endFixture = nFix;
					break;
			}
		}
		else {
			switch(incType)
			{
				case IncForward:
				case IncBackwardReturn:
					startFixture = frameIndex - nFix;
					endFixture = nFix;
					break;
				case IncBackward:
				case IncForwardReturn:
					startFixture = 0;
					endFixture = (nFix*2 - frameIndex);
					break;
			}
		}
		
		PresetCollection& pc = management.AddPresetCollection();
		pc.SetName(destination.GetAvailableName(chase.Name() + "_"));
		destination.Add(pc);
		
		for(size_t i=startFixture; i!=endFixture; ++i)
		{
			unsigned
				red = colors[i].Red()*((1<<24)-1)/255,
				green = colors[i].Green()*((1<<24)-1)/255,
				blue = colors[i].Blue()*((1<<24)-1)/255,
				master = 0;
			if(red != 0 || green != 0 || blue != 0)
				master = (1<<24)-1;
			addColorPresets(management, *controllables[i], pc, red, green, blue, master);
		}
		seq.Add(pc, 0);
		management.AddPreset(pc, 0);
	}
	return chase;
}

