#ifndef _BASE_OBJECT_H__
#define _BASE_OBJECT_H__

#include "common/ownerInfo.h"
#include "reference/reference.h"
#include "metadata/metaObjectsDefines.h"
#include "compiler/valueGuid.h"

//********************************************************************************************
//*                                     Defines                                              *
//********************************************************************************************

class IDataObjectSource;

class IDataObjectValue;
class IDataObjectList;

class IValueTabularSection;

//special names 
#define GUID_NAME wxT("UUID")
#define GUID_REF wxT("UUIDREF")

//********************************************************************************************
//*                                  Factory & metadata                                      *
//********************************************************************************************

#include "common/actionInfo.h"

class IMetaObjectValue : public IMetaObject,
	public IActionSource
{
	wxDECLARE_ABSTRACT_CLASS(IMetaObjectValue);

protected:

	std::vector<IMetaObject *> m_metaObjects;

private:

	virtual OptionList GetFormType() = 0;

public:

	IMetaObjectValue();
	virtual ~IMetaObjectValue();

	//support actions 
	virtual CAction GetActions(form_identifier_t formType) { return CAction(); }
	virtual void AddActions(CAction &actions, form_identifier_t formType) {}
	virtual void ExecuteAction(unsigned int action) {}

	//create & remove table in DB 
	virtual bool CreateMetaObjectInDB() = 0;
	virtual bool RemoveMetaObjectInDB() = 0;

	//meta events
	virtual bool OnLoadMetaObject();
	virtual bool OnSaveMetaObject();

	//form events 
	virtual void OnCreateMetaForm(IMetaFormObject *metaForm) {}
	virtual void OnRemoveMetaForm(IMetaFormObject *metaForm) {}

	//base objects 
	virtual std::vector<IMetaObject *> GetObjects(const CLASS_ID &clsid) { return IMetaObject::GetObjects(clsid); }
	virtual std::vector<IMetaObject *> GetObjects() { return m_metaObjects; }

	//get attributes, form etc.. 
	virtual std::vector<CAttributeObject *> GetObjectAttributes();
	virtual std::vector<CTableObject *> GetObjectTables();
	virtual std::vector<CMetaFormObject *> GetObjectForms();
	virtual std::vector<CMetaGridObject *> GetObjectTemplates();

	//other objects
	virtual std::vector<CMetaModuleObject *> GetObjectModules();
	virtual std::vector<CEnumerationObject *> GetObjectEnums();

	//find in current metaObject
	IMetaObject *FindMetaObjectByID(meta_identifier_t metaid);
	static IMetaObjectValue *FindMetaObjectValue(meta_identifier_t metaid);

	//get module object in compose object 
	virtual CMetaModuleObject *GetModuleObject() { return NULL; }
	virtual CMetaCommonModuleObject *GetModuleManager() { return NULL; }

	//Get metaObject by def id
	virtual CMetaFormObject *GetDefaultFormByID(form_identifier_t id) = 0;

	//create associate value 
	virtual IDataObjectValue *CreateObjectValue() = 0; //create object 

	//create form with data 
	virtual CValueFrame *CreateObjectValue(IMetaFormObject *metaForm) = 0; //create with form

	//create in this metaObject 
	virtual void AppendChild(IMetaObject *parentObj) { m_metaObjects.push_back(parentObj); }
	virtual void RemoveChild(IMetaObject *parentObj) {
		auto itFounded = std::find(m_metaObjects.begin(), m_metaObjects.end(), parentObj);
		if (itFounded != m_metaObjects.end()) m_metaObjects.erase(itFounded);
	}

	//support form 
	virtual CValueFrame *GetObjectForm(const wxString &formName = wxEmptyString, IOwnerInfo *ownerControl = NULL, const Guid &formGuid = Guid::newGuid()) = 0;

	//special functions for DB 
	virtual wxString GetSpecialTableNameDB();
	virtual wxString GetSpecialTableNameSpaceDB() = 0;
	virtual wxString GetSpecialFiledNameDB(CAttributeObject *pAttribute);
	virtual wxString GetSpecialFiledNameSpaceDB();

	virtual int GetTypeIDFromTableDB(const wxString &sTable);
	virtual int GetTypeIDFromFieldDB(const wxString &sField);
	virtual eValueTypes GetTypeFromFieldDB(const wxString &sField);

	virtual bool DeleteData();

protected:

	friend class CMetaFormObject;
	friend class IDataObjectValue;
};

//metaObject with reference 
class IMetaObjectRefValue : public IMetaObjectValue
{
	wxDECLARE_ABSTRACT_CLASS(IMetaObjectRefValue);

public:

	IMetaObjectRefValue() : IMetaObjectValue() {}

	//find in current metaObject
	IMetaObject *FindMetaObjectByID(meta_identifier_t metaid);
	static IMetaObjectRefValue *FindMetaObjectValue(meta_identifier_t metaid);

	virtual IDataObjectValue *CreateObjectValue() = 0;  //create object 
	virtual IDataObjectValue *CreateObjectValue(const Guid &guid) = 0; //create object and read by guid 

	virtual CValueReference *FindObjectValue(const Guid &guid); //find by guid and ret reference 

	//ref object object
	virtual bool IsRefObject() { return true; }

	//if object is enum return true 
	virtual bool isEnumeration() { return false; }

	//for popup, search menu 
	virtual std::vector<CValue> GetListData(int minValue = 0);

	//support form 
	virtual CValueFrame *GetListForm(const wxString &formName = wxEmptyString, IOwnerInfo *ownerControl = NULL, const Guid &formGuid = Guid::newGuid()) = 0;
	virtual CValueFrame *GetSelectForm(const wxString &formName = wxEmptyString, IOwnerInfo *ownerControl = NULL, const Guid &formGuid = Guid::newGuid()) = 0;

	//descriptions...
	virtual wxString GetDescription(const Guid &objGuid) = 0;
	virtual wxString GetDescription(const std::map<wxString, CValue> &valData) = 0;
	virtual wxString GetDescription(const std::map<wxString, CValue> &valData, const Guid &objGuid) = 0;
};

//********************************************************************************************
//*                                      Base data                                           *
//********************************************************************************************

class IDataObjectSource : public IActionSource
{
public:

	IDataObjectSource();
	virtual ~IDataObjectSource();

	//override default type object 
	virtual CValueFrame *GetFrame() { return m_valueFrameObject; }
	virtual bool SaveModify() { return true; }

	//get metadata from object 
	virtual IMetaObjectValue *GetMetaObject() const = 0;

	//get unique identifier 
	virtual Guid GetGuid() const = 0;

	//standart override 
	virtual CMethods* GetPMethods() const = 0;
	virtual void PrepareNames() const = 0;
	virtual CValue Method(CMethodParameters &aParams) = 0;

	virtual void SetAttribute(CAttributeParameters &aParams, CValue &cVal) = 0;
	virtual CValue GetAttribute(CAttributeParameters &aParams) = 0;

	virtual inline bool IsEmpty() const = 0;

	//support actions 
	virtual CAction GetActions(form_identifier_t formType) { return CAction(); }
	virtual void AddActions(CAction &actions, form_identifier_t formType) {}
	virtual void ExecuteAction(unsigned int action) {}

	//counter
	virtual void IncrRef() = 0;
	virtual void DecrRef() = 0;

	//operator 
	virtual operator CValue() const = 0;

protected:

	CValueFrame *m_valueFrameObject;
};

//********************************************************************************************
//*                                      Object                                              *
//********************************************************************************************

#include "common/actionInfo.h"
#include "common/moduleInfo.h"
#include "common/tableInfo.h"

class IDataObjectValue : public CValue,
	public IDataObjectSource, public IModuleInfo
{
	wxDECLARE_ABSTRACT_CLASS(IDataObjectValue);

public:

	//override copy constructor
	IDataObjectValue();
	IDataObjectValue(const IDataObjectValue &source);
	virtual ~IDataObjectValue();

	virtual bool IsObject() { return true; }
	virtual IDataObjectValue *CopyObjectValue() = 0;

	//standart override 
	virtual CMethods* GetPMethods() const { return CValue::GetPMethods(); };
	virtual void PrepareNames() const { CValue::PrepareNames(); };
	virtual CValue Method(CMethodParameters &aParams) { return CValue::Method(aParams); };

	virtual void SetAttribute(CAttributeParameters &aParams, CValue &cVal) { CValue::SetAttribute(aParams, cVal); };
	virtual CValue GetAttribute(CAttributeParameters &aParams) { return CValue::GetAttribute(aParams); };

	//check is empty
	virtual inline bool IsEmpty() const override { return CValue::IsEmpty(); }

	//counter
	virtual void IncrRef() { CValue::IncrRef(); }
	virtual void DecrRef() { CValue::DecrRef(); }

	//get metadata from object 
	virtual IMetaObjectValue *GetMetaObject() const = 0;

	//get unique identifier 
	virtual Guid GetGuid() const = 0;

	//support show 
	virtual void ShowFrameValue(const wxString &formName = wxEmptyString, IOwnerInfo *owner = NULL) = 0;
	virtual CValueFrame *GetFrameValue(const wxString &formName = wxEmptyString, IOwnerInfo *owner = NULL) = 0;

	//default showing
	virtual void ShowValue() override { ShowFrameValue(); }

	//save modify 
	virtual bool SaveModify() override { return true; }

	virtual wxString GetTypeString() const;
	virtual wxString GetString() const;

	//operator 
	virtual operator CValue() const { return this; };

	//special default methods 
	virtual void SetValueByMetaID(meta_identifier_t metaid, CValue &value) = 0;
	virtual CValue GetValueByMetaID(meta_identifier_t metaid) = 0;

protected:

	friend class IMetaObjectValue;

	std::vector<IValueTabularSection *> m_aTableParts;

	std::map<wxString, CValue> m_aObjectValues;
	std::map<wxString, unsigned int> m_aObjectTypes;

	CMethods *m_methods;
};

//Object with reference type 
class IDataObjectRefValue : public IDataObjectValue
{
	wxDECLARE_ABSTRACT_CLASS(IDataObjectRefValue);

public:

	IDataObjectRefValue() : IDataObjectValue() {}
	IDataObjectRefValue(IMetaObjectRefValue *metaObjValue);
	IDataObjectRefValue(IMetaObjectRefValue *metaObjValue, const Guid &guid);
	virtual ~IDataObjectRefValue();

	virtual wxString GetTypeString() const;
	virtual wxString GetString() const;

	//check is empty
	virtual inline bool IsEmpty() const override { return !m_objGuid.isValid(); }

	//get metadata from object 
	virtual IMetaObjectValue *GetMetaObject() const { return m_metaObject; };

	//get unique identifier 
	virtual Guid GetGuid() const { return m_objGuid; }

	//copy new object
	virtual IDataObjectValue *CopyObjectValue() = 0;

	//is new object?
	virtual bool IsNewObject() const { return m_bNewObject; }

	//get reference
	virtual CValueReference *GetReference();

protected:

	virtual bool ReadInDB();
	virtual bool SaveInDB();
	virtual bool DeleteInDB();

protected:

	void PrepareEmptyObject();

protected:

	friend class CValueTabularRefSection;

	bool m_bNewObject;

	Guid m_objGuid;
	IMetaObjectRefValue *m_metaObject;

	reference_t* m_reference_impl;
};

//********************************************************************************************
//*                                        List & select                                     *
//********************************************************************************************

class IDataObjectList : public IValueTable,
	public IDataObjectSource
{
public:

	virtual bool AutoCreateColumns() { return false; }
	virtual bool EditableRows() { return false; }

	IDataObjectList(IMetaObjectRefValue *metaObjValue, IMetaFormObject *formObject, IOwnerInfo *ownerControl = NULL);
	virtual ~IDataObjectList();

	//support actions 
	virtual CAction GetActions(form_identifier_t formType) { return CAction(); }
	virtual void AddActions(CAction &actions, form_identifier_t formType) {}
	virtual void ExecuteAction(unsigned int action) {}

	//events:
	virtual void AddValue();
	virtual void CopyValue();
	virtual void EditValue();
	virtual void DeleteValue();

	virtual void SelectValue();

	//update model
	virtual void UpdateModel() = 0;

	void SetColumnInfo(std::map<meta_identifier_t, unsigned int> &columninfo) { m_aColumnInfo = columninfo; }
	void SetCurrentLine(long currentLine) { m_currentLine = currentLine; }

	//get unique identifier 
	virtual Guid GetGuid() const { return m_objGuid; };

	//check is empty
	virtual inline bool IsEmpty() const override { return false; }

	//get metadata from object 
	virtual IMetaObjectValue *GetMetaObject() const { return m_metaObject; };

	//counter
	virtual void IncrRef() { CValue::IncrRef(); }
	virtual void DecrRef() { CValue::DecrRef(); }

	//operator 
	virtual operator CValue() const { return this; };

protected:

	Guid m_objGuid; 

	long m_currentLine;
	form_identifier_t m_nFormType;

	std::map<meta_identifier_t, unsigned int> m_aColumnInfo;
	
	std::vector<std::map<unsigned int, CValue>> m_aTableValues;
	std::vector<Guid> m_aTableGuidValues;

	IMetaObjectRefValue *m_metaObject;
	IOwnerInfo *m_ownerInfo;
};

#endif 