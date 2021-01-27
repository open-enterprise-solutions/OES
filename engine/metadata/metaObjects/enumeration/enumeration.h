#ifndef _ENUMERATIONOBJECT_H__
#define _ENUMERATIONOBJECT_H__

#include "metadata/metaObjects/metaObject.h"

class CEnumerationObject : public IMetaObject
{
	wxDECLARE_DYNAMIC_CLASS(CEnumerationObject);

public:

	Guid GetGuid() { return m_metaGuid; }

	//load & save metadata from DB 
	virtual bool LoadData(CMemoryReader &reader);
	virtual bool SaveData(CMemoryWriter &writer = CMemoryWriter());

	virtual wxString GetClassName() const override { return wxT("enum"); }
};

#endif