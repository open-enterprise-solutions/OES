#ifndef _ENUMERATION_H__
#define _ENUMERATION_H__

#include "baseObject.h"

class CEnumerationObject;

class CMetaObjectEnumerationValue : public IMetaObjectRefValue
{
	wxDECLARE_DYNAMIC_CLASS(CMetaObjectEnumerationValue);

	enum
	{
		ID_METATREE_OPEN_MANAGER = 19000,
	};

	CMetaCommonModuleObject *m_moduleManager;

	enum
	{
		eFormList = 1,
		eFormSelect
	};

	virtual OptionList GetFormType() override
	{
		OptionList optionlist;

		optionlist.AddOption("formList", eFormList);
		optionlist.AddOption("formSelect", eFormSelect);

		return optionlist;
	}

private:

	//default form 
	int m_defaultFormList;
	int m_defaultFormSelect;

private:

	OptionList GetFormList();
	OptionList GetFormSelect();

public:

	CMetaObjectEnumerationValue();
	virtual ~CMetaObjectEnumerationValue();

	virtual wxString GetClassName() const { return wxT("enumeration"); }

	//create & remove table in DB 
	virtual bool CreateMetaObjectInDB();
	virtual bool RemoveMetaObjectInDB();

	//metaevents
	virtual bool OnCreateMetaObject();
	virtual bool OnLoadMetaObject();
	virtual bool OnSaveMetaObject();
	virtual bool OnDeleteMetaObject();

	//module manager is started or exit 
	virtual bool OnRunMetaObject();
	virtual bool OnCloseMetaObject();

	//form events 
	virtual void OnCreateMetaForm(IMetaFormObject *metaForm);
	virtual void OnRemoveMetaForm(IMetaFormObject *metaForm);

	//if object is enum return true 
	virtual bool isEnumeration() { return true; }

	//create associate value 
	virtual CMetaFormObject *GetDefaultFormByID(form_identifier_t id);

	//create object data with metaForm
	virtual IDataObjectSource *CreateObjectData(IMetaFormObject *metaObject);

	//create empty object
	virtual IDataObjectValue *CreateObjectValue() { return NULL; }
	virtual IDataObjectValue *CreateObjectValue(const Guid &guid) { return NULL; }

	//create form with data 
	virtual CValueFrame *CreateObjectValue(IMetaFormObject *metaForm); 

	//support form 
	virtual CValueFrame *GetObjectForm(const wxString &formName = wxEmptyString, IOwnerInfo *ownerControl = NULL, const Guid &formGuid = Guid::newGuid());
	virtual CValueFrame *GetListForm(const wxString &formName = wxEmptyString, IOwnerInfo *ownerControl = NULL, const Guid &formGuid = Guid::newGuid());
	virtual CValueFrame *GetSelectForm(const wxString &formName = wxEmptyString, IOwnerInfo *ownerControl = NULL, const Guid &formGuid = Guid::newGuid());

	//special functions for DB 
	virtual wxString GetSpecialTableNameSpaceDB();

	//get module object in compose object 
	virtual CMetaModuleObject *GetModuleObject() { return NULL; }
	virtual CMetaCommonModuleObject *GetModuleManager() { return m_moduleManager; }

	//descriptions...
	virtual wxString GetDescription(const Guid &objGuid);
	virtual wxString GetDescription(const std::map<wxString, CValue> &valData);
	virtual wxString GetDescription(const std::map<wxString, CValue> &valData, const Guid &objGuid);

	//prepare menu for item
	virtual bool PrepareContextMenu(wxMenu *defultMenu);
	virtual void ProcessCommand(unsigned int id);

	//read & save property
	virtual void ReadProperty() override;
	virtual void SaveProperty() override;

protected:

	//load & save metadata from DB 
	virtual bool LoadData(CMemoryReader &reader);
	virtual bool SaveData(CMemoryWriter &writer = CMemoryWriter());

	virtual void PrepareOption(Property *property, OptionList &optionlist) override;
};

#endif