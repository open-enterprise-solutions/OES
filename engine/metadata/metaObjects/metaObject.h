#ifndef _METAOBJECT_H__
#define _METAOBJECT_H__

#include "compiler/value.h"
#include "common/objectbase.h"
#include "utils/fs/fs.h"
#include "utils/guid.h"

class CMetaModuleObject;

//*******************************************************************************
//*                          define commom clsid                                *
//*******************************************************************************

//COMMON METADATA
const CLASS_ID g_metaCommonMetadataCLSID = TEXT2CLSID("MD_MTD");

//COMMON OBJECTS
const CLASS_ID g_metaCommonModuleCLSID = TEXT2CLSID("MD_CMOD");
const CLASS_ID g_metaCommonFormCLSID = TEXT2CLSID("MD_CFRM");
const CLASS_ID g_metaCommonTemplateCLSID = TEXT2CLSID("MD_CTMP");

//ADVANCED OBJECTS
const CLASS_ID g_metaAttributeCLSID = TEXT2CLSID("MD_ATTR");
const CLASS_ID g_metaFormCLSID = TEXT2CLSID("MD_FRM");
const CLASS_ID g_metaTemplateCLSID = TEXT2CLSID("MD_TMPL");
const CLASS_ID g_metaModuleCLSID = TEXT2CLSID("MD_MOD");
const CLASS_ID g_metaTableCLSID = TEXT2CLSID("MD_TBL");
const CLASS_ID g_metaEnumCLSID = TEXT2CLSID("MD_ENUM");

//MAIN OBJECTS
const CLASS_ID g_metaConstantCLSID = TEXT2CLSID("MD_CONS");
const CLASS_ID g_metaCatalogCLSID = TEXT2CLSID("MD_CAT");
const CLASS_ID g_metaDocumentCLSID = TEXT2CLSID("MD_DOC");
const CLASS_ID g_metaEnumerationCLSID = TEXT2CLSID("MD_ENM");
const CLASS_ID g_metaDataProcessorCLSID = TEXT2CLSID("MD_DPR");

//*******************************************************************************
//*                             IMetaObject                                     *
//*******************************************************************************

class IMetaObject : public IObjectBase,
	public CValue
{
	wxDECLARE_ABSTRACT_CLASS(IMetaObject);

public:

	CLASS_ID GetClsid() { return m_clsid; }
	void SetClsid(CLASS_ID clsid) { m_clsid = clsid; }

	meta_identifier_t GetMetaID() { return m_metaID; }
	void SetMetaID(meta_identifier_t meta_id) { m_metaID = meta_id; }

	wxString GetName() const { return m_sName; }
	void SetName(const wxString &name) { m_sName = name; }

	wxString GetAlias() const { return m_sAlias; }
	void SetAlias(const wxString &alias) { m_sAlias = alias; }

	wxString GetComment() const { return m_sComment; }
	void SetComment(const wxString &comment) { m_sComment = comment; }

	void SetMetadata(IMetadata *metaData) { m_metaData = metaData; }
	IMetadata *GetMetadata() { return m_metaData; }

	bool IsDeleted() { return m_bDeleted; }
	void MarkAsDeleted() { m_bDeleted = true; }

	void SetFullPath(const wxString &fullPath) { m_sFullPath = fullPath; }
	void GenerateGuid() { wxASSERT(!m_metaGuid.isValid()); m_metaGuid = Guid::newGuid(); }

public:

	IMetaObject(const wxString &name = wxEmptyString, const wxString &alias = wxEmptyString, const wxString &comment = wxEmptyString);
	virtual ~IMetaObject();

	//system override 
	virtual int GetComponentType() override { return COMPONENT_TYPE_METADATA; }
	virtual wxString GetObjectTypeName() const override { return wxT("metadata"); }

	//if object have composite collection
	virtual bool IsRefObject() { return false; }

	virtual wxString GetFileName();
	virtual wxString GetFullName();

	virtual wxString GetModuleName();
	virtual wxString GetDocPath();

	virtual void ReadProperty() override;
	virtual void SaveProperty() override;

	virtual void AppendChild(IMetaObject *childObj) {}
	virtual void RemoveChild(IMetaObject *childObj) {}

	virtual std::vector<IMetaObject *> GetObjects(const CLASS_ID &clsid)
	{
		std::vector<IMetaObject *> m_objects;

		for (auto obj : GetObjects())
		{
			if (clsid == obj->GetClsid())
				m_objects.push_back(obj);
		}

		return m_objects;
	}

	virtual std::vector<IMetaObject *> GetObjects() { return std::vector<IMetaObject *>(); }

	//methods 
	virtual CMethods* GetPMethods() const { PrepareNames();  return m_methods; }; //получить ссылку на класс помощник разбора имен атрибутов и методов
	virtual void PrepareNames() const;                         //этот метод автоматически вызывается для инициализации имен атрибутов и методов
	virtual CValue Method(CMethodParameters &aParams);       //вызов метода

	//attributes 
	virtual void SetAttribute(CAttributeParameters &aParams, CValue &cVal);        //установка атрибута
	virtual CValue GetAttribute(CAttributeParameters &aParams);                   //значение атрибута
	virtual int  FindAttribute(const wxString &sName) const;

	virtual wxString GetTypeString() const { return GetObjectTypeName() << wxT(".") << GetClassName(); }
	virtual wxString GetString() const { return m_sName; }

	//load &save object in metaObject 
	bool LoadMeta(CMemoryReader &dataReader);
	bool SaveMeta(CMemoryWriter &dataWritter = CMemoryWriter());

	//load &save object in DB 
	bool LoadMetaObject(IMetadata *metaData, CMemoryReader &dataReader);
	bool SaveMetaObject(IMetadata *metaData, CMemoryWriter &dataWritter, bool saveToFile = false);
	bool DeleteMetaObject(IMetadata *metaData);

	//events: 
	virtual bool OnCreateMetaObject() { return true; }
	virtual bool OnLoadMetaObject() { return true; }
	virtual bool OnSaveMetaObject() { return true; }
	virtual bool OnDeleteMetaObject() { return true; }
	virtual bool OnRenameMetaObject(const wxString &sNewName) { return true; }

	//module manager is started or exit 
	virtual bool OnRunMetaObject() { return true; }
	virtual bool OnCloseMetaObject() { return true; }

	//prepare menu for item
	virtual bool PrepareContextMenu(wxMenu *defultMenu) { return false; }
	virtual void ProcessCommand(unsigned int id) {}

	// Gets the parent object
	IMetaObject* GetParent() const { return wxDynamicCast(m_parent, IMetaObject); }

	/**
	* Obtiene un hijo del objeto.
	*/
	virtual IMetaObject* GetChild(unsigned int idx) { return dynamic_cast<IMetaObject *>(IObjectBase::GetChild(idx)); }
	virtual IMetaObject* GetChild(unsigned int idx, const wxString& type) { return dynamic_cast<IMetaObject *>(IObjectBase::GetChild(idx, type)); }

	//get module object in compose object 
	virtual CMetaModuleObject *GetModuleObject() { return NULL; }

	//check is empty
	virtual inline bool IsEmpty() const override { return false; }

	/**
	* Property events
	*/
	virtual void OnPropertyCreated(Property *m_property);
	virtual void OnPropertySelected(Property *m_property);
	virtual void OnPropertyChanged(Property *m_property);

	/**
	* Set property data
	*/
	virtual void SetPropertyData(Property *property, const CValue &srcValue);

	/**
	* Get property data
	*/
	virtual CValue GetPropertyData(Property *property);

protected:

	virtual bool LoadData(CMemoryReader &reader) { return true; }
	virtual bool SaveData(CMemoryWriter &writer = CMemoryWriter()) { return true; }
	virtual bool DeleteData() { return true; }

protected:

	bool m_bCanSave;
	bool m_bDeleted;			    //признак удаления

	wxString m_sName;				//имя объекта (идентификатор) 
	wxString m_sAlias;				//синоним объекта
	wxString m_sComment;			//комментарий объекта

	CLASS_ID m_clsid;               //type object name
	meta_identifier_t m_metaID;		 //type id (default is undefined)
	Guid m_metaGuid;

	wxString m_sFullPath;           //for data processor

	IMetadata *m_metaData;
	CMethods *m_methods;
};

#endif