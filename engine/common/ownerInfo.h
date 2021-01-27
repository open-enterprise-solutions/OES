#ifndef _OWNERINFO_H__
#define _OWNERINFO_H__

#include "compiler/value.h"

class IOwnerInfo
{
public:

	virtual void SendOwnerValue(CValue &cValue) = 0;
};

#endif