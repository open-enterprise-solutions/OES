#ifndef _DATAPROCESSOR_H__
#define _DATAPROCESSOR_H__

#include "baseObject.h"

enum
{
	METAOBJECT_NORMAL = 1,
	METAOBJECT_EXTERNAL
};

class CMetaObjectDataProcessorValue : public IMetaObjectValue
{
	wxDECLARE_DYNAMIC_CLASS(CMetaObjectDataProcessorValue);

	enum
	{
		ID_METATREE_OPEN_MODULE = 19000,
		ID_METATREE_OPEN_MANAGER = 19001,
	};

	CMetaModuleObject *m_moduleObject;
	CMetaCommonModuleObject *m_moduleManager;

	enum
	{
		eFormDataProcessor = 1,
	};

	virtual OptionList GetFormType() override
	{
		OptionList optionlist;
		optionlist.AddOption("formDataProcessor", eFormDataProcessor);
		return optionlist;
	}

	//default form 
	int m_defaultFormObject;
	//external or default dataProcessor
	int m_objMode;

private:

	OptionList GetFormObject();

public:

	CMetaObjectDataProcessorValue(int objMode = METAOBJECT_NORMAL);
	virtual ~CMetaObjectDataProcessorValue();

	virtual wxString GetClassName() const { return wxT("dataProcessor"); }

	//DB
	virtual bool CreateMetaObjectInDB() { return true; }
	virtual bool RemoveMetaObjectInDB() { return true; }

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

	//create associate value 
	virtual CMetaFormObject *GetDefaultFormByID(form_identifier_t id);

	//create object data with metaForm
	virtual IDataObjectSource *CreateObjectData(IMetaFormObject *metaObject);

	//create empty object
	virtual IDataObjectValue *CreateObjectValue();

	//create form with data 
	virtual CValueFrame *CreateObjectValue(IMetaFormObject *metaForm);

	//suppot form
	virtual CValueFrame *GetObjectForm(const wxString &formName = wxEmptyString, IOwnerInfo *ownerControl = NULL, const Guid &formGuid = Guid::newGuid());

	//support actions
	virtual CAction GetActions(form_identifier_t formType);
	virtual void AddActions(CAction &actions, form_identifier_t formType);

	//special functions for DB 
	virtual wxString GetSpecialTableNameSpaceDB() { return wxEmptyString; }

	//get module object in compose object 
	virtual CMetaModuleObject *GetModuleObject() { return m_moduleObject; }
	virtual CMetaCommonModuleObject *GetModuleManager() { return m_moduleManager; }

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
	friend class CObjectDataProcessorValue;
};

//********************************************************************************************
//*                                      Object                                              *
//********************************************************************************************

#define thisObject wxT("thisObject")

class CObjectDataProcessorValue : public IDataObjectValue
{
	void PrepareEmptyObject();
	bool InitializeModule();

public:

	CObjectDataProcessorValue(const CObjectDataProcessorValue &source);
	CObjectDataProcessorValue(CMetaObjectDataProcessorValue *metaObj);
	virtual ~CObjectDataProcessorValue(); 

	//special default methods 
	virtual void SetValueByMetaID(meta_identifier_t metaid, CValue &value);
	virtual CValue GetValueByMetaID(meta_identifier_t metaid);

	//copy new object
	virtual IDataObjectValue *CopyObjectValue() { return new CObjectDataProcessorValue(*this); }

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

	//get metadata from object 
	virtual IMetaObjectValue *GetMetaObject() const { return m_metaObject; };

	//get unique identifier 
	virtual Guid GetGuid() const { return m_objGuid; }

	//check is empty
	virtual inline bool IsEmpty() const override { return false; }

	//support actions
	virtual void ExecuteAction(unsigned int action);

protected:

	Guid m_objGuid;
	CMetaObjectDataProcessorValue *m_metaObject;

	friend class CExternalModuleManager;
};

#endif