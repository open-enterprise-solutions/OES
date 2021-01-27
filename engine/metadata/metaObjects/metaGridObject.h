#ifndef _METAGRIDOBJECT_H__
#define _METAGRIDOBJECT_H__

#include "metaObject.h"

class CMetaGridObject : public IMetaObject
{
	wxDECLARE_DYNAMIC_CLASS(CMetaGridObject);

	enum
	{
		ID_METATREE_OPEN_TEMPLATE = 19000,
	};

public:

	virtual wxString GetClassName() const override { return wxT("template"); }

	//module manager is started or exit 
	virtual bool OnRunMetaObject();
	virtual bool OnCloseMetaObject();

	//prepare menu for item
	virtual bool PrepareContextMenu(wxMenu *defultMenu);
	virtual void ProcessCommand(unsigned int id);

protected:

	virtual bool LoadData(CMemoryReader &reader);
	virtual bool SaveData(CMemoryWriter &writer = CMemoryWriter());
};

class CMetaCommonGridObject : public CMetaGridObject
{
	wxDECLARE_DYNAMIC_CLASS(CMetaCommonGridObject);

public:

	virtual wxString GetClassName() const override { return wxT("commonTemplates"); }
};

#endif 