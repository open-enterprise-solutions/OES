#ifndef _MANAGER_INFO_H__
#define _MANAGER_INFO_H__

class IMetaManagerInfo
{
public:

	virtual class CMetaCommonModuleObject *GetModuleManager() = 0;
};

#endif 