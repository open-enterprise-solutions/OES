#ifndef _METAOBJECT_METADATA_H__
#define _METAOBJECT_METADATA_H__

#include "metaObject.h"

//*****************************************************************************************
//*                                  metadata object                                      *
//*****************************************************************************************

#define st_configuration wxT("ñonfiguration")

class CMetaObject : public IMetaObject
{
	wxDECLARE_DYNAMIC_CLASS(CMetaObject);

	enum
	{
		ID_METATREE_OPEN_INIT_MODULE = 19000,
	};

public:

	CMetaObject();
	virtual ~CMetaObject();

	//if object have composite collection
	virtual bool IsRefObject() { return true; }

	virtual wxString GetFullName() { return st_configuration; }
	virtual wxString GetModuleName() { return st_configuration; }

	//get object class 
	virtual wxString GetClassName() const override { return wxT("metadata"); }

	//events
	virtual bool OnCreateMetaObject();
	virtual bool OnLoadMetaObject();
	virtual bool OnSaveMetaObject();
	virtual bool OnDeleteMetaObject();

	//module manager is started or exit 
	virtual bool OnRunMetaObject();
	virtual bool OnCloseMetaObject();

	//prepare menu for item
	virtual bool PrepareContextMenu(wxMenu *defultMenu);
	virtual void ProcessCommand(unsigned int id);

public:

	virtual CMetaModuleObject *GetModuleObject() { return m_commonModule; }

protected:


	//load & save metadata from DB 
	virtual bool LoadData(CMemoryReader &reader);
	virtual bool SaveData(CMemoryWriter &writer = CMemoryWriter());

private:

	CMetaModuleObject *m_commonModule;
};

#endif 