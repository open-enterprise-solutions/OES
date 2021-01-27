////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : document metadata
////////////////////////////////////////////////////////////////////////////

#include "document.h"
#include "list/objectList.h"
#include "metadata/metadata.h"
#include "metadata/moduleManager/moduleManager.h"

#define objectModule wxT("objectModule")
#define managerModule wxT("managerModule")

wxIMPLEMENT_DYNAMIC_CLASS(CMetaObjectDocumentValue, IMetaObjectRefValue);

//********************************************************************************************
//*                                      metadata                                            *
//********************************************************************************************

CMetaObjectDocumentValue::CMetaObjectDocumentValue() : IMetaObjectRefValue(), 
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
	m_attributeNumber = new CAttributeObject(wxT("number"), eValueTypes::TYPE_STRING, wxT("NUMBER"));
	m_attributeNumber->SetClsid(g_metaAttributeCLSID); 

	//set child/parent
	m_attributeNumber->SetParent(this);
	AddChild(m_attributeNumber);

	m_attributeDate = new CAttributeObject(wxT("date"), eValueTypes::TYPE_DATE, wxT("DATE"));
	m_attributeDate->SetClsid(g_metaAttributeCLSID);

	//set child/parent
	m_attributeDate->SetParent(this);
	AddChild(m_attributeDate);

	m_attributePosted = new CAttributeObject(wxT("posted"), eValueTypes::TYPE_BOOLEAN, wxT("POSTED"));
	m_attributePosted->SetClsid(g_metaAttributeCLSID);

	//set child/parent
	m_attributePosted->SetParent(this);
	AddChild(m_attributePosted);

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

CMetaObjectDocumentValue::~CMetaObjectDocumentValue()
{
	wxDELETE(m_attributeNumber);
	wxDELETE(m_attributeDate);
	wxDELETE(m_attributePosted);

	wxDELETE(m_moduleObject);
	wxDELETE(m_moduleManager);
}

CMetaFormObject *CMetaObjectDocumentValue::GetDefaultFormByID(form_identifier_t id)
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

IDataObjectSource *CMetaObjectDocumentValue::CreateObjectData(IMetaFormObject *metaObject)
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

IDataObjectValue *CMetaObjectDocumentValue::CreateObjectValue()
{
	IModuleManager *moduleManager = m_metaData->GetModuleManager();
	wxASSERT(moduleManager);

	IDataObjectValue *m_pDataRef = NULL;

	if (appData->IsDesignerMode()) {
		if (!moduleManager->FindCompileModule(m_moduleObject, m_pDataRef))
			return new CObjectDocumentValue(this);
	}
	else {
		m_pDataRef = new CObjectDocumentValue(this);
	}

	return m_pDataRef;
}

IDataObjectValue *CMetaObjectDocumentValue::CreateObjectValue(const Guid &guid)
{
	return new CObjectDocumentValue(this, guid);
}

CValueFrame *CMetaObjectDocumentValue::CreateObjectValue(IMetaFormObject *metaForm)
{
	return metaForm->GenerateFormAndRun(this, CreateObjectData(metaForm));
}

inline bool CompareString(const wxString &lhs, const wxString &rhs)
{
	return lhs.CompareTo(rhs, wxString::ignoreCase) == 0;
}

#include "forms/elements/frame.h"

CValueFrame *CMetaObjectDocumentValue::GetObjectForm(const wxString &formName, IOwnerInfo *ownerControl, const Guid &formGuid)
{
	CMetaFormObject* defList = NULL;

	if (!formName.IsEmpty()) {
		for (auto metaForm : GetObjectForms()) {
			if (CMetaObjectDocumentValue::eFormObject == metaForm->GetTypeForm()
				&& CompareString(formName, metaForm->GetName())) {
				defList = metaForm; break;
			}
		}
		wxASSERT(defList);
	}
	else {
		defList = GetDefaultFormByID(CMetaObjectDocumentValue::eFormObject);
	}

	if (!defList) {
		return new CValueFrame();
	}

	return defList->GenerateFormAndRun(this, CreateObjectValue());
}

CValueFrame *CMetaObjectDocumentValue::GetListForm(const wxString &formName, IOwnerInfo *ownerControl, const Guid &formGuid)
{
	CMetaFormObject* defList = NULL;

	if (!formName.IsEmpty()) {
		for (auto metaForm : GetObjectForms()) {
			if (CMetaObjectDocumentValue::eFormList == metaForm->GetTypeForm()
				&& CompareString(formName, metaForm->GetName())) {
				defList = metaForm; break;
			}
		}
		wxASSERT(defList);
	}
	else {
		defList = GetDefaultFormByID(CMetaObjectDocumentValue::eFormList);
	}

	if (!defList) {
		return new CValueFrame();
	}

	return defList->GenerateFormAndRun(this, new CObjectListValue(this, defList, ownerControl));
}

CValueFrame *CMetaObjectDocumentValue::GetSelectForm(const wxString &formName, IOwnerInfo *ownerControl, const Guid &formGuid)
{
	CMetaFormObject* defList = NULL;

	if (!formName.IsEmpty()) {
		for (auto metaForm : GetObjectForms()) {
			if (CMetaObjectDocumentValue::eFormSelect == metaForm->GetTypeForm()
				&& CompareString(formName, metaForm->GetName())) {
				defList = metaForm; break;
			}
		}
		wxASSERT(defList);
	}
	else {
		defList = GetDefaultFormByID(CMetaObjectDocumentValue::eFormSelect);
	}

	if (!defList) {
		return new CValueFrame();
	}

	return defList->GenerateFormAndRun(this, new CObjectListValue(this, defList, ownerControl));
}

void CMetaObjectDocumentValue::PrepareOption(Property *property, OptionList &optionlist)
{
	if (property->GetName() == wxT("default_object"))
	{
		optionlist = GetFormObject();
	}
	else if (property->GetName() == wxT("default_list"))
	{
		optionlist = GetFormList();
	}
	else if (property->GetName() == wxT("default_select"))
	{
		optionlist = GetFormSelect();
	}
}

OptionList CMetaObjectDocumentValue::GetFormObject()
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

OptionList CMetaObjectDocumentValue::GetFormList()
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

OptionList CMetaObjectDocumentValue::GetFormSelect()
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

std::vector<CAttributeObject *> CMetaObjectDocumentValue::GetObjectAttributes()
{
	std::vector<CAttributeObject *> attributes;

	for (auto metaObj : m_metaObjects)
	{
		if (metaObj->GetClsid() == g_metaAttributeCLSID)
			attributes.push_back(dynamic_cast<CAttributeObject *>(metaObj));
	}

	attributes.push_back(m_attributeNumber);
	attributes.push_back(m_attributeDate);
	attributes.push_back(m_attributePosted);

	return attributes;
}

//***************************************************************************
//*                       Save & load metadata                              *
//***************************************************************************

bool CMetaObjectDocumentValue::LoadData(CMemoryReader &dataReader)
{
	//load default attributes:
	m_attributeNumber->LoadMeta(dataReader);
	m_attributeDate->LoadMeta(dataReader);
	m_attributePosted->LoadMeta(dataReader);

	//load object module
	m_moduleObject->LoadMeta(dataReader);
	m_moduleManager->LoadMeta(dataReader);

	m_defaultFormObject = dataReader.r_u32();
	m_defaultFormList = dataReader.r_u32();
	m_defaultFormSelect = dataReader.r_u32();

	return true;
}

bool CMetaObjectDocumentValue::SaveData(CMemoryWriter &dataWritter)
{
	//save default attributes:
	m_attributeNumber->SaveMeta(dataWritter);
	m_attributeDate->SaveMeta(dataWritter);
	m_attributePosted->SaveMeta(dataWritter);

	//Save object module
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
//*                           read & save events                        *
//***********************************************************************

#include "appData.h"

bool CMetaObjectDocumentValue::OnCreateMetaObject()
{
	m_attributeNumber->GenerateGuid();
	m_attributeDate->GenerateGuid();
	m_attributePosted->GenerateGuid();
	m_moduleManager->GenerateGuid();
	m_moduleObject->GenerateGuid();

	return true;
}

bool CMetaObjectDocumentValue::OnLoadMetaObject()
{
	IModuleManager *moduleManager = m_metaData->GetModuleManager();
	wxASSERT(moduleManager);

	m_attributeNumber->SetMetadata(m_metaData);
	m_attributeNumber->SetFullPath(m_sFullPath);

	m_attributeNumber->SetMetaID(m_metaData->GenerateNewID());
	m_metaData->AppendSecondaryMetadata(m_attributeNumber);

	if (!m_attributeNumber->OnLoadMetaObject())
		return false; 

	m_attributeDate->SetMetadata(m_metaData);
	m_attributeDate->SetFullPath(m_sFullPath);

	m_attributeDate->SetMetaID(m_metaData->GenerateNewID());
	m_metaData->AppendSecondaryMetadata(m_attributeDate);

	if (!m_attributeDate->OnLoadMetaObject())
		return false;

	m_attributePosted->SetMetadata(m_metaData);
	m_attributePosted->SetFullPath(m_sFullPath);

	m_attributePosted->SetMetaID(m_metaData->GenerateNewID());
	m_metaData->AppendSecondaryMetadata(m_attributePosted);

	if (!m_attributePosted->OnLoadMetaObject())
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

bool CMetaObjectDocumentValue::OnSaveMetaObject()
{
	if (!m_attributeNumber->OnSaveMetaObject())
		return false;

	if (!m_attributeDate->OnSaveMetaObject())
		return false;

	if (!m_attributePosted->OnSaveMetaObject())
		return false;

	if (!m_moduleManager->OnSaveMetaObject())
		return false;

	if (!m_moduleObject->OnSaveMetaObject())
		return false;

	return true;
}

bool CMetaObjectDocumentValue::OnDeleteMetaObject()
{
	IModuleManager *moduleManager = m_metaData->GetModuleManager();
	wxASSERT(moduleManager);

	if (!m_attributeNumber->OnDeleteMetaObject())
		return false;

	if (!m_attributeDate->OnDeleteMetaObject())
		return false;

	if (!m_attributePosted->OnDeleteMetaObject())
		return false;

	if (!m_moduleManager->OnDeleteMetaObject())
		return false;

	if (!m_moduleObject->OnDeleteMetaObject())
		return false;

	if (!moduleManager->RemoveCompileModule(m_moduleObject))
		return false;
	
	m_metaData->RemoveSecondaryMetadata(m_attributeNumber);
	m_metaData->RemoveSecondaryMetadata(m_attributeDate);
	m_metaData->RemoveSecondaryMetadata(m_attributePosted);
	m_metaData->RemoveSecondaryMetadata(m_moduleManager);
	m_metaData->RemoveSecondaryMetadata(m_moduleObject);

	return true;
}

bool CMetaObjectDocumentValue::OnRunMetaObject()
{
	IModuleManager *moduleManager = m_metaData->GetModuleManager();
	wxASSERT(moduleManager);

	if (!m_attributePosted->OnRunMetaObject())
		return false;

	if (!m_attributePosted->OnRunMetaObject())
		return false;

	if (!m_attributePosted->OnRunMetaObject())
		return false;

	if (!m_moduleManager->OnRunMetaObject())
		return false;

	if (!m_moduleObject->OnRunMetaObject())
		return false;

	if (appData->IsDesignerMode())
		return moduleManager->AddCompileModule(m_moduleObject, CreateObjectValue());

	return true;
}

bool CMetaObjectDocumentValue::OnCloseMetaObject()
{
	if (!m_attributePosted->OnCloseMetaObject())
		return false;

	if (!m_attributePosted->OnCloseMetaObject())
		return false;

	if (!m_attributePosted->OnCloseMetaObject())
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

void CMetaObjectDocumentValue::OnCreateMetaForm(IMetaFormObject *metaForm)
{
	if (metaForm->GetTypeForm() == CMetaObjectDocumentValue::eFormObject
		&& m_defaultFormObject == wxNOT_FOUND)
	{
		m_defaultFormObject = metaForm->GetMetaID();
	}
	else if (metaForm->GetTypeForm() == CMetaObjectDocumentValue::eFormList
		&& m_defaultFormList == wxNOT_FOUND)
	{
		m_defaultFormList = metaForm->GetMetaID();
	}
	else if (metaForm->GetTypeForm() == CMetaObjectDocumentValue::eFormSelect
		&& m_defaultFormSelect == wxNOT_FOUND)
	{
		m_defaultFormSelect = metaForm->GetMetaID();
	}
}

void CMetaObjectDocumentValue::OnRemoveMetaForm(IMetaFormObject *metaForm)
{
	if (metaForm->GetTypeForm() == CMetaObjectDocumentValue::eFormObject
		&& m_defaultFormObject == metaForm->GetMetaID())
	{
		m_defaultFormObject = wxNOT_FOUND;
	}
	else if (metaForm->GetTypeForm() == CMetaObjectDocumentValue::eFormList
		&& m_defaultFormList == metaForm->GetMetaID())
	{
		m_defaultFormList = wxNOT_FOUND;
	}
	else if (metaForm->GetTypeForm() == CMetaObjectDocumentValue::eFormSelect
		&& m_defaultFormSelect == metaForm->GetMetaID())
	{
		m_defaultFormSelect = wxNOT_FOUND;
	}
}

//***********************************************************************
//*                           read & save property                      *
//***********************************************************************

void CMetaObjectDocumentValue::ReadProperty()
{
	IMetaObjectValue::ReadProperty();

	m_properties["default_object"]->SetValue(m_defaultFormObject);
	m_properties["default_list"]->SetValue(m_defaultFormList);
	m_properties["default_select"]->SetValue(m_defaultFormSelect);
}

void CMetaObjectDocumentValue::SaveProperty()
{
	IMetaObjectValue::SaveProperty();

	m_defaultFormObject = m_properties["default_object"]->GetValueAsInteger();
	m_defaultFormList = m_properties["default_list"]->GetValueAsInteger();
	m_defaultFormSelect = m_properties["default_select"]->GetValueAsInteger();
}

//***********************************************************************
//*                       Register in runtime                           *
//***********************************************************************

METADATA_REGISTER(CMetaObjectDocumentValue, "metaDocument", g_metaDocumentCLSID);