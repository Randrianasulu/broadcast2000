#include <string.h>
#include "vdevicebase.h"

VDeviceBase::VDeviceBase(VideoDevice *device)
{
	this->device = device;
}

VDeviceBase::~VDeviceBase()
{
}

