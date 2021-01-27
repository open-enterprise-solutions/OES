////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : catalog metadata
////////////////////////////////////////////////////////////////////////////

#include "catalog.h"
#include "list/objectList.h"
#include "metadata/metadata.h"
#include "metadata/moduleManager/moduleManager.h"

#define objectModule wxT("objectModule")
#define managerModule wxT("managerModule")

//***********************************************************************
//*                         metadata                                    * 
//***********************************************************************

wxIMPLEMENT_DYNAMIC_CLASS(CMetaObjectCatalogValue, IMetaObjectRefValue);

//********************************************************************************************
//*                                      metadata                                            *
//********************************************************************************************

CMetaObjectCatalogValue::CMetaObjectCatalogValue() : IMetaObjectRefValue(),
//default form 
m_defaultFormObject(wxNOT_FOUND), m_defaultFormList(wxNOT_FOUND), m_defaultFormSelect(wxNOT_FOUND)
{
	PropertyCategory *m_category_form = new PropertyCategory("DefaultForms");
	m_category_form->AddProperty("default_object");
	m_category_form->AddProperty("default_list");
	m_category_form->AddProperty("default_select");
	m_category->AddCategory(m_category_form);

	m_properties["default_object"] = new Property("default_object", PropertyType::PT_OPTION_CUSTOM, this);
	m_properties["default_list"] = new Property("default_list", PropertyType::PT_OPTION_CUSTOM, this);
	m_properties["default_select"] = new Property("default_select", PropertyType::PT_OPTION_CUSTOM, this);

	//create default attributes
	m_attributeCode = new CAttributeObject(wxT("code"), eValueTypes::TYPE_STRING, wxT("CODE"));
	m_attributeCode->SetClsid(g_metaAttributeCLSID);

	//set child/parent
	m_attributeCode->SetParent(this);
	AddChild(m_attributeCode);

	m_attributeName = new CAttributeObject(wxT("name"), eValueTypes::TYPE_STRING, wxT("NAME"));
	m_attributeName->SetClsid(g_metaAttributeCLSID);

	//set child/parent
	m_attributeName->SetParent(this);
	AddChild(m_attributeName);

	m_attributeGroup = new CAttributeObject(wxT("group"), eValueTypes::TYPE_BOOLEAN, wxT("IS_GROUP"));
	m_attributeGroup->SetClsid(g_metaAttributeCLSID);

	//set child/parent
	m_attributeGroup->SetParent(this);
	AddChild(m_attributeGroup);

	//create module
	m_moduleObject = new CMetaModuleObject(objectModule);
	m_moduleObject->SetClsid(g_metaModuleCLSID);

	//set child/parent
	m_moduleObject->SetParent(this);
	AddChild(m_moduleObject);

	m_moduleManager = new CMetaCommonModuleObject(managerModule);
	m_moduleManager->SetClsid(g_metaCommonModuleCLSID);

	//set child/parent
	m_moduleManager->SetParent(this);
	AddChild(m_moduleManager);
}

CMetaObjectCatalogValue::~CMetaObjectCatalogValue()
{
	wxDELETE(m_attributeCode);
	wxDELETE(m_attributeName);
	wxDELETE(m_attributeGroup);

	wxDELETE(m_moduleObject);
	wxDELETE(m_moduleManager);
}

CMetaFormObject *CMetaObjectCatalogValue::GetDefaultFormByID(form_identifier_t id)
{
	if (id == eFormObject
		&& m_defaultFormObject != wxNOT_FOUND) {
		for (auto obj : GetObjectForms()) {
			if (m_defaultFormObject == obj->GetMetaID()) {
				return obj;
			}
		}
	}
	else if (id == eFormList
		&& m_defaultFormList != wxNOT_FOUND) {
		for (auto obj : GetObjectForms()) {
			if (m_defaultFormList == obj->GetMetaID()) {
				return obj;
			}
		}
	}
	else if (id == eFormSelect
		&& m_defaultFormSelect != wxNOT_FOUND) {
		for (auto obj : GetObjectForms()) {
			if (m_defaultFormSelect == obj->GetMetaID()) {
				return obj;
			}
		}
	}

	return NULL;
}

IDataObjectSource *CMetaObjectCatalogValue::CreateObjectData(IMetaFormObject *metaObject)
{
	switch (metaObject->GetTypeForm())
	{
	case eFormObject: return CreateObjectValue(); break;
	case eFormList:
	case eFormSelect: return new CObjectListValue(this, metaObject); break;
	}

	return NULL;
}

#include "appData.h"

IDataObjectValue *CMetaObjectCatalogValue::CreateObjectValue()
{
	IModuleManager *moduleManager = m_metaData->GetModuleManager();
	wxASSERT(moduleManager);

	IDataObjectValue *m_pDataRef = NULL;

	if (appData->IsDesignerMode()) {
		if (!moduleManager->FindCompileModule(m_moduleObject, m_pDataRef))
			return new CObjectCatalogValue(this);
	}
	else {
		m_pDataRef = new CObjectCatalogValue(this);
	}

	return m_pDataRef;
}

IDataObjectValue *CMetaObjectCatalogValue::CreateObjectValue(const Guid &guid)
{
	return new CObjectCatalogValue(this, guid);
}

CValueFrame *CMetaObjectCatalogValue::CreateObjectValue(IMetaFormObject *metaForm)
{
	return metaForm->GenerateFormAndRun(this, CreateObjectData(metaForm));
}

inline bool CompareString(const wxString &lhs, const wxString &rhs)
{
	return lhs.CompareTo(rhs, wxString::ignoreCase) == 0;
}

#include "forms/elements/frame.h"

CValueFrame *CMetaObjectCatalogValue::GetObjectForm(const wxString &formName, IOwnerInfo *ownerControl, const Guid &formGuid)
{
	CMetaFormObject* defList = NULL;

	if (!formName.IsEmpty()) {
		for (auto metaForm : GetObjectForms()) {
			if (CMetaObjectCatalogValue::eFormObject == metaForm->GetTypeForm()
				&& CompareString(formName, metaForm->GetName())) {
				defList = metaForm; break;
			}
		}
		wxASSERT(defList);
	}
	else {
		defList = GetDefaultFormByID(CMetaObjectCatalogValue::eFormObject);
	}

	if (!defList) {
		return new CValueFrame();
	}

	return defList->GenerateFormAndRun(this, CreateObjectValue());
}

CValueFrame *CMetaObjectCatalogValue::GetListForm(const wxString &formName, IOwnerInfo *ownerControl, const Guid &formGuid)
{
	CMetaFormObject* defList = NULL;

	if (!formName.IsEmpty()) {
		for (auto metaForm : GetObjectForms()) {
			if (CMetaObjectCatalogValue::eFormList == metaForm->GetTypeForm()
				&& CompareString(formName, metaForm->GetName())) {
				defList = metaForm; break;
			}
		}
		wxASSERT(defList);
	}
	else {
		defList = GetDefaultFormByID(CMetaObjectCatalogValue::eFormList);
	}

	if (!defList) {
		return new CValueFrame();
	}

	return defList->GenerateFormAndRun(this, new CObjectListValue(this, defList, ownerControl));
}

CValueFrame *CMetaObjectCatalogValue::GetSelectForm(const wxString &formName, IOwnerInfo *ownerControl, const Guid &formGuid)
{
	CMetaFormObject* defList = NULL;

	if (!formName.IsEmpty()) {
		for (auto metaForm : GetObjectForms()) {
			if (CMetaObjectCatalogValue::eFormSelect == metaForm->GetTypeForm()
				&& CompareString(formName, metaForm->GetName())) {
				defList = metaForm; break;
			}
		}
		wxASSERT(defList);
	}
	else {
		defList = GetDefaultFormByID(CMetaObjectCatalogValue::eFormSelect);
	}

	if (!defList) {
		return new CValueFrame();
	}

	return defList->GenerateFormAndRun(this, new CObjectListValue(this, defList, ownerControl));
}

void CMetaObjectCatalogValue::PrepareOption(Property *property, OptionList &optionlist)
{
	if (property->GetName() == wxT("default_object")) {
		optionlist = GetFormObject();
	}
	else if (property->GetName() == wxT("default_list")) {
		optionlist = GetFormList();
	}
	else if (property->GetName() == wxT("default_select")) {
		optionlist = GetFormSelect();
	}
}

OptionList CMetaObjectCatalogValue::GetFormObject()
{
	OptionList optlist;
	optlist.AddOption("<not selected>", wxNOT_FOUND);

	for (auto formObject : GetObjectForms())
	{
		if (eFormObject == formObject->GetTypeForm())
		{
			optlist.AddOption(formObject->GetName(), formObject->GetMetaID());
		}
	}

	return optlist;
}

OptionList CMetaObjectCatalogValue::GetFormList()
{
	OptionList optlist;
	optlist.AddOption("<not selected>", wxNOT_FOUND);

	for (auto formObject : GetObjectForms())
	{
		if (eFormList == formObject->GetTypeForm())
		{
			optlist.AddOption(formObject->GetName(), formObject->GetMetaID());
		}
	}

	return optlist;
}

OptionList CMetaObjectCatalogValue::GetFormSelect()
{
	OptionList optlist;
	optlist.AddOption("<not selected>", wxNOT_FOUND);

	for (auto formObject : GetObjectForms())
	{
		if (eFormSelect == formObject->GetTypeForm())
		{
			optlist.AddOption(formObject->GetName(), formObject->GetMetaID());
		}
	}

	return optlist;
}

std::vector<CAttributeObject *> CMetaObjectCatalogValue::GetObjectAttributes()
{
	std::vector<CAttributeObject *> attributes;

	for (auto metaObj : m_metaObjects)
	{
		if (metaObj->GetClsid() == g_metaAttributeCLSID)
			attributes.push_back(dynamic_cast<CAttributeObject *>(metaObj));
	}

	attributes.push_back(m_attributeCode);
	attributes.push_back(m_attributeName);
	attributes.push_back(m_attributeGroup);

	return attributes;
}

//***************************************************************************
//*                       Save & load metadata                              *
//***************************************************************************

bool CMetaObjectCatalogValue::LoadData(CMemoryReader &dataReader)
{
	//load default attributes:
	m_attributeCode->LoadMeta(dataReader);
	m_attributeName->LoadMeta(dataReader);
	m_attributeGroup->LoadMeta(dataReader);

	//load object module
	m_moduleObject->LoadMeta(dataReader);
	m_moduleManager->LoadMeta(dataReader);

	//load default form 
	m_defaultFormObject = dataReader.r_u32();
	m_defaultFormList = dataReader.r_u32();
	m_defaultFormSelect = dataReader.r_u32();
	return true;
}

bool CMetaObjectCatalogValue::SaveData(CMemoryWriter &dataWritter)
{
	//save default attributes:
	m_attributeCode->SaveMeta(dataWritter);
	m_attributeName->SaveMeta(dataWritter);
	m_attributeGroup->SaveMeta(dataWritter);

	//save object module
	m_moduleObject->SaveMeta(dataWritter);
	m_moduleManager->SaveMeta(dataWritter);

	//save default form 
	dataWritter.w_u32(m_defaultFormObject);
	dataWritter.w_u32(m_defaultFormList);
	dataWritter.w_u32(m_defaultFormSelect);

	//create or update table:
	CreateMetaObjectInDB();
	return true;
}

//***********************************************************************
//*                           read & save property                      *
//***********************************************************************

void CMetaObjectCatalogValue::ReadProperty()
{
	IMetaObjectValue::ReadProperty();

	m_properties["default_object"]->SetValue(m_defaultFormObject);
	m_properties["default_list"]->SetValue(m_defaultFormList);
	m_properties["default_select"]->SetValue(m_defaultFormSelect);
}

void CMetaObjectCatalogValue::SaveProperty()
{
	IMetaObjectValue::SaveProperty();

	m_defaultFormObject = m_properties["default_object"]->GetValueAsInteger();
	m_defaultFormList = m_properties["default_list"]->GetValueAsInteger();
	m_defaultFormSelect = m_properties["default_select"]->GetValueAsInteger();
}

//***********************************************************************
//*                           read & save events                        *
//***********************************************************************

#include "appData.h"

bool CMetaObjectCatalogValue::OnCreateMetaObject()
{
	m_attributeCode->GenerateGuid();
	m_attributeName->GenerateGuid();
	m_attributeGroup->GenerateGuid();

	m_moduleManager->GenerateGuid();
	m_moduleObject->GenerateGuid();;

	return true;
}

bool CMetaObjectCatalogValue::OnLoadMetaObject()
{
	IModuleManager *moduleManager = m_metaData->GetModuleManager();
	wxASSERT(moduleManager);

	m_attributeCode->SetMetadata(m_metaData);
	m_attributeCode->SetFullPath(m_sFullPath);

	m_attributeCode->SetMetaID(m_metaData->GenerateNewID());
	m_metaData->AppendSecondaryMetadata(m_attributeCode);

	if (!m_attributeCode->OnLoadMetaObject())
		return false;

	m_attributeName->SetMetadata(m_metaData);
	m_attributeName->SetFullPath(m_sFullPath);

	m_attributeName->SetMetaID(m_metaData->GenerateNewID());
	m_metaData->AppendSecondaryMetadata(m_attributeName);

	if (!m_attributeName->OnLoadMetaObject())
		return false;

	m_attributeGroup->SetMetadata(m_metaData);
	m_attributeGroup->SetFullPath(m_sFullPath);

	m_attributeGroup->SetMetaID(m_metaData->GenerateNewID());
	m_metaData->AppendSecondaryMetadata(m_attributeGroup);

	if (!m_attributeGroup->OnLoadMetaObject())
		return false;

	m_moduleManager->SetMetadata(m_metaData);
	m_moduleManager->SetFullPath(m_sFullPath);

	m_moduleManager->SetMetaID(m_metaData->GenerateNewID());
	m_metaData->AppendSecondaryMetadata(m_moduleManager);

	if (!moduleManager->AddCommonModule(m_moduleManager, true))
		return false;

	if (!m_moduleManager->OnLoadMetaObject())
		return false;

	m_moduleObject->SetMetadata(m_metaData);
	m_moduleObject->SetFullPath(m_sFullPath);

	m_moduleObject->SetMetaID(m_metaData->GenerateNewID());
	m_metaData->AppendSecondaryMetadata(m_moduleObject);

	if (!m_moduleObject->OnLoadMetaObject())
		return false;

	return true;
}

bool CMetaObjectCatalogValue::OnSaveMetaObject()
{
	if (!m_attributeCode->OnSaveMetaObject())
		return false;

	if (!m_attributeName->OnSaveMetaObject())
		return false;

	if (!m_attributeGroup->OnSaveMetaObject())
		return false;

	if (!m_moduleManager->OnSaveMetaObject())
		return false;

	if (!m_moduleObject->OnSaveMetaObject())
		return false;

	return true;
}

bool CMetaObjectCatalogValue::OnDeleteMetaObject()
{
	IModuleManager *moduleManager = m_metaData->GetModuleManager();
	wxASSERT(moduleManager);

	if (!m_attributeCode->OnDeleteMetaObject())
		return false;

	if (!m_attributeName->OnDeleteMetaObject())
		return false;

	if (!m_attributeGroup->OnDeleteMetaObject())
		return false;

	if (!m_moduleManager->OnDeleteMetaObject())
		return false;

	if (!m_moduleObject->OnDeleteMetaObject())
		return false;

	if (!moduleManager->RemoveCompileModule(m_moduleObject))
		return false;

	m_metaData->RemoveSecondaryMetadata(m_attributeCode);
	m_metaData->RemoveSecondaryMetadata(m_attributeName);
	m_metaData->RemoveSecondaryMetadata(m_attributeGroup);
	m_metaData->RemoveSecondaryMetadata(m_moduleManager);
	m_metaData->RemoveSecondaryMetadata(m_moduleObject);

	return true;
}

bool CMetaObjectCatalogValue::OnRunMetaObject()
{
	IModuleManager *moduleManager = m_metaData->GetModuleManager();
	wxASSERT(moduleManager);

	if (!m_attributeCode->OnRunMetaObject())
		return false;

	if (!m_attributeName->OnRunMetaObject())
		return false;

	if (!m_attributeGroup->OnRunMetaObject())
		return false;

	if (!m_moduleManager->OnRunMetaObject())
		return false;

	if (!m_moduleObject->OnRunMetaObject())
		return false;

	if (appData->IsDesignerMode()) {
		return moduleManager->AddCompileModule(m_moduleObject, CreateObjectValue());
	}

	return true;
}

bool CMetaObjectCatalogValue::OnCloseMetaObject()
{
	if (!m_attributeCode->OnCloseMetaObject())
		return false;

	if (!m_attributeName->OnCloseMetaObject())
		return false;

	if (!m_attributeGroup->OnCloseMetaObject())
		return false;

	if (!m_moduleManager->OnCloseMetaObject())
		return false;

	if (!m_moduleObject->OnCloseMetaObject())
		return false;

	return true;
}

//***********************************************************************
//*                             form events                             *
//***********************************************************************

void CMetaObjectCatalogValue::OnCreateMetaForm(IMetaFormObject *metaForm)
{
	if (metaForm->GetTypeForm() == CMetaObjectCatalogValue::eFormObject
		&& m_defaultFormObject == wxNOT_FOUND)
	{
		m_defaultFormObject = metaForm->GetMetaID(); 
	}
	else if (metaForm->GetTypeForm() == CMetaObjectCatalogValue::eFormList
		&& m_defaultFormList == wxNOT_FOUND)
	{
		m_defaultFormList = metaForm->GetMetaID();
	}
	else if (metaForm->GetTypeForm() == CMetaObjectCatalogValue::eFormSelect
		&& m_defaultFormSelect == wxNOT_FOUND)
	{
		m_defaultFormSelect = metaForm->GetMetaID();
	}
}

void CMetaObjectCatalogValue::OnRemoveMetaForm(IMetaFormObject *metaForm)
{
	if (metaForm->GetTypeForm() == CMetaObjectCatalogValue::eFormObject
		&& m_defaultFormObject == metaForm->GetMetaID())
	{
		m_defaultFormObject = wxNOT_FOUND;
	}
	else if (metaForm->GetTypeForm() == CMetaObjectCatalogValue::eFormList
		&& m_defaultFormList == metaForm->GetMetaID())
	{
		m_defaultFormList = wxNOT_FOUND;
	}
	else if (metaForm->GetTypeForm() == CMetaObjectCatalogValue::eFormSelect
		&& m_defaultFormSelect == metaForm->GetMetaID())
	{
		m_defaultFormSelect = wxNOT_FOUND;
	}
}

//***********************************************************************
//*                       Register in runtime                           *
//***********************************************************************

METADATA_REGISTER(CMetaObjectCatalogValue, "metaCatalog", g_metaCatalogCLSID);