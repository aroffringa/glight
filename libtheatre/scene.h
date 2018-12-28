#ifndef SCENE_H
#define SCENE_H

#include <map>

#include <mutex>

#include "../audioplayer.h"

#include "controlsceneitem.h"
#include "keysceneitem.h"
#include "startable.h"

/**
	@author Andre Offringa
*/
class Scene : public Startable, private SyncListener {
	public:
		Scene(class Management &management);

		~Scene();

		ControlSceneItem *AddControlSceneItem(double offsetInMS, Controllable &controllable)
		{
			ControlSceneItem *item = new ControlSceneItem(controllable);
			item->SetOffsetInMS(offsetInMS);
			_items.insert(std::pair<double, SceneItem*>(offsetInMS, item));
			resetCurrentOffset();
			return item;
		}
		KeySceneItem *AddKeySceneItem(double offsetInMS)
		{
			KeySceneItem *item = new KeySceneItem();
			item->SetOffsetInMS(offsetInMS);
			_items.insert(std::pair<double, SceneItem*>(offsetInMS, item));
			resetCurrentOffset();
			return item;
		}
		void ChangeSceneItemStartTime(SceneItem *item, double newOffsetInMS)
		{
			_items.erase(find(item));
			item->SetOffsetInMS(newOffsetInMS);
			_items.insert(std::pair<double, SceneItem*>(newOffsetInMS, item));
			resetCurrentOffset();
		}
		bool HasEnd(double globalTimeInMS)
		{
			double relTimeInMs = globalTimeInMS - StartTimeInMS();
			skipTo(relTimeInMs);
			return _nextStartedItem == _items.end() && _startedItems.empty();
		}
		const std::multimap<double, std::unique_ptr<SceneItem>>& SceneItems() const
		{
			return _items;
		}
		void Mix(unsigned *channelValues, unsigned universe, const Timing& globalTiming)
		{
			double relTimeInMs = globalTiming.TimeInMS() - StartTimeInMS();
			Timing relTiming(relTimeInMs, globalTiming.BeatValue());
			skipTo(relTimeInMs);

			for(std::vector<SceneItem*>::const_iterator i=_startedItems.begin();i!=_startedItems.end();++i)
			{
				(*i)->Mix(channelValues, universe, relTiming);
			}
		}
		void Remove(SceneItem *item)
		{
			_items.erase(find(item));
		}
		void SetStartOffset(double offsetInMS)
		{
			if(offsetInMS != _startOffset)
			{
				_startOffset = offsetInMS;
				initPlayer();
			}
		}
		void Stop()
		{
			if(_isPlaying)
			{
				_audioPlayer.reset();
				_decoder.reset();
				_isPlaying = false;
				initPlayer();
			} else {
				if(_audioPlayer == 0)
					initPlayer();
			}
		}
		bool HasAudio() const { return _hasAudio; }
		const std::string &AudioFile() const { return _audioFilename; }
		void SetNoAudioFile()
		{
			_hasAudio = false;
			_audioFilename = "";
			Stop();
		}
		void SetAudioFile(const std::string &audioFilename)
		{
			_audioFilename = audioFilename;
			_hasAudio = true;
			initPlayer();
		}
		bool IsPlaying() const { return _isPlaying; }
	protected:
		void initPlayer()
		{
			if(_hasAudio)
			{
				try {
					_audioPlayer.reset();
					_decoder.reset();
					_decoder.reset(new FlacDecoder(_audioFilename));
					_audioPlayer.reset(new AudioPlayer(*_decoder));
					_audioPlayer->SetSyncListener(*this);
					_audioPlayer->Seek(_startOffset);
				} catch(std::exception &e)
				{
					std::cout << "Could not open player for filename " << _audioFilename << std::endl;
					_hasAudio = false;
				}
			}
		}
		void resetCurrentOffset()
		{
			_nextStartedItem = _items.begin();
			_startedItems.clear();
		}
		void onStart()
		{
			resetCurrentOffset();
			Stop();
			setStartTimeInMS(StartTimeInMS() - _startOffset);
			_isPlaying = true;
			if(_hasAudio)
				_audioPlayer->Play();
		}
		void skipTo(double offsetInMS)
		{
			if(_currentOffset > offsetInMS)
				resetCurrentOffset();
			// "Start" all items that have started since the last tick
			while(_nextStartedItem != _items.end() && _nextStartedItem->first <= offsetInMS)
			{
				_startedItems.push_back(_nextStartedItem->second.get());
				++_nextStartedItem;
			}
			// "End" all items which duration has passed.
			for(std::vector<SceneItem*>::iterator i=_startedItems.begin();i!=_startedItems.end();++i)
			{
				if((*i)->OffsetInMS() + (*i)->DurationInMS() < offsetInMS)
				{
					--i;
					std::vector<SceneItem*>::iterator removePointer = i;
					++removePointer;
					_startedItems.erase(removePointer);
				}
			}
			_currentOffset = offsetInMS;
		}
		std::multimap<double, std::unique_ptr<SceneItem>>::iterator find(SceneItem* item)
		{
			for(std::multimap<double, std::unique_ptr<SceneItem>>::iterator i=_items.begin();i!=_items.end();++i)
			{
				if(item == i->second.get())
				{
					return i;
				}
			}
			return _items.end();
		}
	private:
		class Management &_management;
		std::mutex &_mutex;
		std::vector<SceneItem*> _startedItems;
		std::multimap<double, std::unique_ptr<SceneItem>> _items;
		std::multimap<double, std::unique_ptr<SceneItem>>::iterator _nextStartedItem;
		double _currentOffset, _startOffset;
		std::unique_ptr<FlacDecoder> _decoder;
		std::unique_ptr<AudioPlayer> _audioPlayer;
		bool _hasAudio, _isPlaying;
		std::string _audioFilename;

		virtual void OnSyncUpdate(double offsetInMS);
};

#endif
