////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : enumeration metadata
////////////////////////////////////////////////////////////////////////////

#include "enumeration.h"
#include "metadata/metadata.h"
#include "list/objectList.h"

#define managerModule wxT("managerModule")

wxIMPLEMENT_DYNAMIC_CLASS(CMetaObjectEnumerationValue, IMetaObjectRefValue)

//********************************************************************************************
//*                                      metadata                                            *
//********************************************************************************************

CMetaObjectEnumerationValue::CMetaObjectEnumerationValue() : IMetaObjectRefValue(),
m_defaultFormList(wxNOT_FOUND), m_defaultFormSelect(wxNOT_FOUND)
{
	PropertyCategory *m_category_form = new PropertyCategory("DefaultForms");
	m_category_form->AddProperty("default_list");
	m_category_form->AddProperty("default_select");
	m_category->AddCategory(m_category_form);

	m_properties["default_list"] = new Property("default_list", PropertyType::PT_OPTION_CUSTOM, this);
	m_properties["default_select"] = new Property("default_select", PropertyType::PT_OPTION_CUSTOM, this);

	m_moduleManager = new CMetaCommonModuleObject(managerModule);
	m_moduleManager->SetClsid(g_metaCommonModuleCLSID);

	//set child/parent
	m_moduleManager->SetParent(this);
	AddChild(m_moduleManager);
}

CMetaObjectEnumerationValue::~CMetaObjectEnumerationValue()
{
	wxDELETE(m_moduleManager);
}

CMetaFormObject *CMetaObjectEnumerationValue::GetDefaultFormByID(form_identifier_t id)
{
	if (id == eFormList
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

IDataObjectSource *CMetaObjectEnumerationValue::CreateObjectData(IMetaFormObject *metaObject)
{
	switch (metaObject->GetTypeForm())
	{
	case eFormList:
	case eFormSelect: return new CObjectListValue(this, metaObject); break;
	}

	return NULL;
}

CValueFrame *CMetaObjectEnumerationValue::CreateObjectValue(IMetaFormObject *metaForm)
{
	return metaForm->GenerateFormAndRun(this, CreateObjectData(metaForm));
}

inline bool CompareString(const wxString &lhs, const wxString &rhs)
{
	return lhs.CompareTo(rhs, wxString::ignoreCase) == 0;
}

#include "forms/elements/frame.h"

CValueFrame *CMetaObjectEnumerationValue::GetObjectForm(const wxString & formName, IOwnerInfo * ownerControl, const Guid & formGuid)
{
	return NULL;
}

CValueFrame *CMetaObjectEnumerationValue::GetListForm(const wxString &formName, IOwnerInfo *ownerControl, const Guid &formGuid)
{
	CMetaFormObject* defList = NULL;

	if (!formName.IsEmpty()) {
		for (auto metaForm : GetObjectForms()) {
			if (CMetaObjectEnumerationValue::eFormList == metaForm->GetTypeForm()
				&& CompareString(formName, metaForm->GetName())) {
				defList = metaForm; break;
			}
		}
		wxASSERT(defList);
	}
	else {
		defList = GetDefaultFormByID(CMetaObjectEnumerationValue::eFormList);
	}

	if (!defList) {
		return new CValueFrame();
	}

	return defList->GenerateFormAndRun(this, new CObjectListValue(this, defList, ownerControl));
}

CValueFrame *CMetaObjectEnumerationValue::GetSelectForm(const wxString &formName, IOwnerInfo *ownerControl, const Guid &formGuid)
{
	CMetaFormObject* defList = NULL;

	if (!formName.IsEmpty()) {
		for (auto metaForm : GetObjectForms()) {
			if (CMetaObjectEnumerationValue::eFormSelect == metaForm->GetTypeForm()
				&& CompareString(formName, metaForm->GetName())) {
				defList = metaForm; break;
			}
		}
		wxASSERT(defList);
	}
	else {
		defList = GetDefaultFormByID(CMetaObjectEnumerationValue::eFormSelect);
	}

	if (!defList) {
		return new CValueFrame();
	}

	return defList->GenerateFormAndRun(this, new CObjectListValue(this, defList, ownerControl));
}

void CMetaObjectEnumerationValue::PrepareOption(Property *property, OptionList &optionlist)
{
	if (property->GetName() == wxT("default_list"))
	{
		optionlist = GetFormList();
	}
	else if (property->GetName() == wxT("default_select"))
	{
		optionlist = GetFormSelect();
	}
}

OptionList CMetaObjectEnumerationValue::GetFormList()
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

OptionList CMetaObjectEnumerationValue::GetFormSelect()
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

//***************************************************************************
//*                       Save & load metadata                              *
//***************************************************************************

bool CMetaObjectEnumerationValue::LoadData(CMemoryReader &dataReader)
{
	//Load object module
	m_moduleManager->LoadMeta(dataReader);

	//save default form 
	m_defaultFormList = dataReader.r_u32();
	m_defaultFormSelect = dataReader.r_u32();

	return true;
}

bool CMetaObjectEnumerationValue::SaveData(CMemoryWriter &dataWritter)
{
	//Save object module
	m_moduleManager->SaveMeta(dataWritter);

	//save default form 
	dataWritter.w_u32(m_defaultFormList);
	dataWritter.w_u32(m_defaultFormSelect);

	//create or update table:
	return CreateMetaObjectInDB();
}

//***********************************************************************
//*                           read & save events                        *
//***********************************************************************

bool CMetaObjectEnumerationValue::OnCreateMetaObject()
{
	m_moduleManager->GenerateGuid();
	return true;
}

bool CMetaObjectEnumerationValue::OnLoadMetaObject()
{
	IModuleManager *moduleManager = m_metaData->GetModuleManager();
	wxASSERT(moduleManager);

	m_moduleManager->SetMetadata(m_metaData);
	m_moduleManager->SetFullPath(m_sFullPath);
	m_moduleManager->SetMetaID(m_metaData->GenerateNewID());
	m_metaData->AppendSecondaryMetadata(m_moduleManager);

	if (!moduleManager->AddCommonModule(m_moduleManager, true))
		return false;

	if (!m_moduleManager->OnLoadMetaObject())
		return false;

	return true;
}

bool CMetaObjectEnumerationValue::OnSaveMetaObject()
{
	if (!m_moduleManager->OnSaveMetaObject())
		return false;

	return true;
}

bool CMetaObjectEnumerationValue::OnDeleteMetaObject()
{
	if (!m_moduleManager->OnDeleteMetaObject())
		return false;

	m_metaData->RemoveSecondaryMetadata(m_moduleManager);
	return true;
}

bool CMetaObjectEnumerationValue::OnRunMetaObject()
{
	if (!m_moduleManager->OnRunMetaObject())
		return false;

	return true;
}

bool CMetaObjectEnumerationValue::OnCloseMetaObject()
{
	if (!m_moduleManager->OnCloseMetaObject())
		return false;

	return true;
}

//***********************************************************************
//*                             form events                             *
//***********************************************************************

void CMetaObjectEnumerationValue::OnCreateMetaForm(IMetaFormObject *metaForm)
{
	if (metaForm->GetTypeForm() == CMetaObjectEnumerationValue::eFormList
		&& m_defaultFormList == wxNOT_FOUND)
	{
		m_defaultFormList = metaForm->GetMetaID();
	}
	else if (metaForm->GetTypeForm() == CMetaObjectEnumerationValue::eFormSelect
		&& m_defaultFormSelect == wxNOT_FOUND)
	{
		m_defaultFormSelect = metaForm->GetMetaID();
	}
}

void CMetaObjectEnumerationValue::OnRemoveMetaForm(IMetaFormObject *metaForm)
{
	if (metaForm->GetTypeForm() == CMetaObjectEnumerationValue::eFormList
		&& m_defaultFormList == metaForm->GetMetaID())
	{
		m_defaultFormList = wxNOT_FOUND;
	}
	else if (metaForm->GetTypeForm() == CMetaObjectEnumerationValue::eFormSelect
		&& m_defaultFormSelect == metaForm->GetMetaID())
	{
		m_defaultFormSelect = wxNOT_FOUND;
	}
}

//***********************************************************************
//*                           read & save property                      *
//***********************************************************************

void CMetaObjectEnumerationValue::ReadProperty()
{
	IMetaObjectValue::ReadProperty();

	m_properties["default_list"]->SetValue(m_defaultFormList);
	m_properties["default_select"]->SetValue(m_defaultFormSelect);
}

void CMetaObjectEnumerationValue::SaveProperty()
{
	IMetaObjectValue::SaveProperty();

	m_defaultFormList = m_properties["default_list"]->GetValueAsInteger();
	m_defaultFormSelect = m_properties["default_select"]->GetValueAsInteger();
}

//***********************************************************************
//*                       Register in runtime                           *
//***********************************************************************

METADATA_REGISTER(CMetaObjectEnumerationValue, "metaEnumeration", g_metaEnumerationCLSID);