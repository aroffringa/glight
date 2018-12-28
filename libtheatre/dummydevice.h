#ifndef DUMMYDEVICE_H
#define DUMMYDEVICE_H

#include "dmxdevice.h"

#include <unistd.h>

/**
	@author Andre Offringa
*/
class DummyDevice : public DmxDevice {
	public:
		DummyDevice() : _isOpen(false)
		{
		}

		virtual ~DummyDevice()
		{
		}

		virtual void Open()
		{
			_isOpen = true;
		}
		
		virtual void SetValues(unsigned char *newValues, size_t size)
		{
		}
		
		virtual void GetValues(unsigned char *destination, size_t size)
		{
			for(size_t i=0;i<size;++i)
				destination[i] = 0;
		}

		virtual void WaitForNextSync()
		{
			usleep(1000);
		}

		virtual void Abort()
		{
			_isOpen = false;
		}

		virtual bool IsOpen()
		{
			return _isOpen;
		}

	private:
		bool _isOpen;
};

#endif //DUMMYDEVICE_H
