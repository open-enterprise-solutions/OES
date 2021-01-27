#ifndef _METAMODULEOBJECT_H__
#define _METAMODULEOBJECT_H__

#include "metaObject.h"

class CMetaModuleObject : public IMetaObject
{
	wxDECLARE_DYNAMIC_CLASS(CMetaModuleObject);

public:

	CMetaModuleObject(const wxString &name = wxEmptyString, const wxString &alias = wxEmptyString, const wxString &comment = wxEmptyString);

	virtual wxString GetClassName() const override { return wxT("module"); }

	//events:
	virtual bool OnCreateMetaObject();
	virtual bool OnLoadMetaObject();
	virtual bool OnSaveMetaObject();
	virtual bool OnDeleteMetaObject();

	//module manager is started or exit 
	virtual bool OnRunMetaObject();
	virtual bool OnCloseMetaObject();

	virtual void SetModuleText(const wxString &moduleText) { m_sModule = moduleText; }
	virtual wxString GetModuleText() { return m_sModule; }

protected:

	virtual bool LoadData(CMemoryReader &reader);
	virtual bool SaveData(CMemoryWriter &writer = CMemoryWriter());

protected:

	wxString m_sModule;
};

class CMetaCommonModuleObject : public CMetaModuleObject
{
	wxDECLARE_DYNAMIC_CLASS(CMetaCommonModuleObject);

	enum
	{
		ID_METATREE_OPEN_MODULE = 19000,
	};

public:

	CMetaCommonModuleObject(const wxString &name = wxEmptyString, const wxString &alias = wxEmptyString, const wxString &comment = wxEmptyString);

	virtual wxString GetClassName() const override { return wxT("commonModule"); }

	//events:
	virtual bool OnCreateMetaObject();
	virtual bool OnLoadMetaObject();
	virtual bool OnSaveMetaObject();
	virtual bool OnDeleteMetaObject();

	virtual bool OnRenameMetaObject(const wxString &sNewName);

	//module manager is started or exit 
	virtual bool OnRunMetaObject();
	virtual bool OnCloseMetaObject();

	//prepare menu for item
	virtual bool PrepareContextMenu(wxMenu *defultMenu);
	virtual void ProcessCommand(unsigned int id);
};

#endif