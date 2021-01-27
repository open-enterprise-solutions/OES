#ifndef _CATALOG_H__
#define _CATALOG_H__

#include "baseObject.h"

//********************************************************************************************
//*                                     Defines                                              *
//********************************************************************************************

class CObjectCatalogValue;

//********************************************************************************************
//*                                  Factory & metadata                                      *
//********************************************************************************************

class CMetaObjectCatalogValue : public IMetaObjectRefValue
{
	wxDECLARE_DYNAMIC_CLASS(CMetaObjectCatalogValue);

	enum
	{
		ID_METATREE_OPEN_MODULE = 19000,
		ID_METATREE_OPEN_MANAGER = 19001,
	};

	CMetaModuleObject *m_moduleObject;
	CMetaCommonModuleObject *m_moduleManager;

	enum
	{
		eFormObject = 1,
		eFormList,
		eFormSelect
	};

	virtual OptionList GetFormType() override
	{
		OptionList optionlist;
		optionlist.AddOption("formObject", eFormObject);
		optionlist.AddOption("formList", eFormList);
		optionlist.AddOption("formSelect", eFormSelect);
		return optionlist;
	}

private:

	//default form 
	int m_defaultFormObject;
	int m_defaultFormList;
	int m_defaultFormSelect;

	//default attributes 
	CAttributeObject *m_attributeCode;
	CAttributeObject *m_attributeName;
	CAttributeObject *m_attributeGroup;

private:

	OptionList GetFormObject();
	OptionList GetFormList();
	OptionList GetFormSelect();

public:

	//default constructor 
	CMetaObjectCatalogValue();
	virtual ~CMetaObjectCatalogValue();

	virtual wxString GetClassName() const { return wxT("catalog"); }

	//DB
	virtual bool CreateMetaObjectInDB();
	virtual bool RemoveMetaObjectInDB();

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

	//override base objects 
	virtual std::vector<CAttributeObject *> GetObjectAttributes() override;

	//create associate value 
	virtual CMetaFormObject *GetDefaultFormByID(form_identifier_t id);

	//create object data with metaForm
	virtual IDataObjectSource *CreateObjectData(IMetaFormObject *metaObject);

	//create empty object
	virtual IDataObjectValue *CreateObjectValue();
	virtual IDataObjectValue *CreateObjectValue(const Guid &guid);

	//create form with data 
	virtual CValueFrame *CreateObjectValue(IMetaFormObject *metaForm);

	//support form 
	virtual CValueFrame *GetObjectForm(const wxString &formName = wxEmptyString, IOwnerInfo *ownerControl = NULL, const Guid &formGuid = Guid::newGuid());
	virtual CValueFrame *GetListForm(const wxString &formName = wxEmptyString, IOwnerInfo *ownerControl = NULL, const Guid &formGuid = Guid::newGuid());
	virtual CValueFrame *GetSelectForm(const wxString &formName = wxEmptyString, IOwnerInfo *ownerControl = NULL, const Guid &formGuid = Guid::newGuid());

	//support actions
	virtual CAction GetActions(form_identifier_t formType);
	virtual void AddActions(CAction &actions, form_identifier_t formType);

	//special functions for DB 
	virtual wxString GetSpecialTableNameSpaceDB();

	//descriptions...
	virtual wxString GetDescription(const Guid &objGuid);
	virtual wxString GetDescription(const std::map<wxString, CValue> &valData);
	virtual wxString GetDescription(const std::map<wxString, CValue> &valData, const Guid &objGuid);

	//get module object in compose object 
	virtual CMetaModuleObject *GetModuleObject() { return m_moduleObject; }
	virtual CMetaCommonModuleObject *GetModuleManager() { return m_moduleManager; }

	//prepare menu for item
	virtual bool PrepareContextMenu(wxMenu *defultMenu);
	virtual void ProcessCommand(unsigned int id);

	//read and write property 
	virtual void ReadProperty() override;
	virtual	void SaveProperty() override;

protected:

	//load & save metadata from DB 
	virtual bool LoadData(CMemoryReader &reader);
	virtual bool SaveData(CMemoryWriter &writer = CMemoryWriter());

	virtual void PrepareOption(Property *property, OptionList &optionlist) override;

	friend class CObjectCatalogValue;
};

//********************************************************************************************
//*                                      Object                                              *
//********************************************************************************************

#define thisObject wxT("thisObject")

class CObjectCatalogValue : public IDataObjectRefValue
{
	bool InitializeModule();

private:

	CObjectCatalogValue *m_catOwner;
	CObjectCatalogValue *m_catParent;

public:

	CObjectCatalogValue(const CObjectCatalogValue &source);
	CObjectCatalogValue(CMetaObjectCatalogValue *metaObj);
	CObjectCatalogValue(CMetaObjectCatalogValue *metaObj, const Guid &guid);

	//****************************************************************************
	//*                              Support id's                                *
	//****************************************************************************

	virtual void SetValueByMetaID(meta_identifier_t metaid, CValue &value);
	virtual CValue GetValueByMetaID(meta_identifier_t metaid);

	virtual IDataObjectValue *CopyObjectValue() { return new CObjectCatalogValue(*this); }

	//save modify 
	virtual bool SaveModify() { return WriteObject(); }

	//default methods
	virtual void FillObject(CValue vFillObject);
	virtual CValue CopyObject();
	virtual bool WriteObject();
	virtual bool DeleteObject();

	//****************************************************************************
	//*                              Support methods                             *
	//****************************************************************************

	virtual CMethods* GetPMethods() const;
	virtual void PrepareNames() const;
	virtual CValue Method(CMethodParameters &aParams);

	//****************************************************************************
	//*                              Override attribute                          *
	//****************************************************************************
	virtual void SetAttribute(CAttributeParameters &aParams, CValue &cVal);
	virtual CValue GetAttribute(CAttributeParameters &aParams);

	//support show 
	virtual void ShowFrameValue(const wxString &formName = wxEmptyString, IOwnerInfo *owner = NULL);
	virtual CValueFrame *GetFrameValue(const wxString &formName = wxEmptyString, IOwnerInfo *owner = NULL);

	//support actions
	virtual void ExecuteAction(unsigned int action);
};

#endif