////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : dataProcessor - metadata
////////////////////////////////////////////////////////////////////////////

#include "dataProcessor.h"
#include "metadata/metadata.h"

#define objectModule wxT("objectModule")
#define managerModule wxT("managerModule")

wxIMPLEMENT_DYNAMIC_CLASS(CMetaObjectDataProcessorValue, IMetaObjectValue)

//********************************************************************************************
//*                                      metadata                                            *
//********************************************************************************************

CMetaObjectDataProcessorValue::CMetaObjectDataProcessorValue(int objMode) : IMetaObjectValue(),
m_defaultFormObject(wxNOT_FOUND), m_objMode(objMode)
{
	PropertyCategory *m_category_form = new PropertyCategory("DefaultForms");
	m_category_form->AddProperty("default_object");
	m_category->AddCategory(m_category_form);

	m_properties["default_object"] = new Property("default_object", PropertyType::PT_OPTION_CUSTOM, this);

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

CMetaObjectDataProcessorValue::~CMetaObjectDataProcessorValue()
{
	wxDELETE(m_moduleObject);
	wxDELETE(m_moduleManager);
}

CMetaFormObject *CMetaObjectDataProcessorValue::GetDefaultFormByID(form_identifier_t id)
{
	if (id == eFormDataProcessor
		&& m_defaultFormObject != wxNOT_FOUND) {
		for (auto obj : GetObjectForms()) {
			if (m_defaultFormObject == obj->GetMetaID()) {
				return obj;
			}
		}
	}

	return NULL;
}

IDataObjectSource *CMetaObjectDataProcessorValue::CreateObjectData(IMetaFormObject *metaObject)
{
	switch (metaObject->GetTypeForm())
	{
	case eFormDataProcessor: return CreateObjectValue(); break;
	}

	return NULL;
}

#include "appData.h"

IDataObjectValue *CMetaObjectDataProcessorValue::CreateObjectValue()
{
	IModuleManager *moduleManager = m_metaData->GetModuleManager();
	wxASSERT(moduleManager);

	IDataObjectValue *pDataRef = NULL;

	if (appData->IsDesignerMode()) {

		if (m_objMode == METAOBJECT_NORMAL)
		{
			if (!moduleManager->FindCompileModule(m_moduleObject, pDataRef))
				return new CObjectDataProcessorValue(this);
		}
		else {
			return moduleManager->GetObjectData();
		}
	}
	else {
		if (m_objMode == METAOBJECT_NORMAL) {
			pDataRef = new CObjectDataProcessorValue(this);
		}
		else {
			return moduleManager->GetObjectData();
		}
	}

	return pDataRef;
}

CValueFrame *CMetaObjectDataProcessorValue::CreateObjectValue(IMetaFormObject *metaForm)
{
	return metaForm->GenerateFormAndRun(this, CreateObjectData(metaForm));
}

inline bool CompareString(const wxString &lhs, const wxString &rhs)
{
	return lhs.CompareTo(rhs, wxString::ignoreCase) == 0;
}

#include "forms/elements/frame.h"

CValueFrame *CMetaObjectDataProcessorValue::GetObjectForm(const wxString &formName, IOwnerInfo *ownerControl, const Guid &formGuid)
{
	CMetaFormObject* defList = NULL;

	if (!formName.IsEmpty()) {
		for (auto metaForm : GetObjectForms()) {
			if (CMetaObjectDataProcessorValue::eFormDataProcessor == metaForm->GetTypeForm()
				&& CompareString(formName, metaForm->GetName())) {
				return defList->GenerateFormAndRun(this, CreateObjectValue());
			}
		}
		wxASSERT(defList);
	}
	else {
		defList = GetDefaultFormByID(CMetaObjectDataProcessorValue::eFormDataProcessor);
		if (defList) {
			return defList->GenerateFormAndRun(this, CreateObjectValue());
		}
	}

	if (!defList) {
		return new CValueFrame();
	}

	return defList->GenerateFormAndRun(this, NULL);
}

void CMetaObjectDataProcessorValue::PrepareOption(Property *property, OptionList &optionlist)
{
	if (property->GetName() == wxT("default_object"))
	{
		optionlist = GetFormObject();
	}
}

OptionList CMetaObjectDataProcessorValue::GetFormObject()
{
	OptionList optlist;
	optlist.AddOption("<not selected>", wxNOT_FOUND);

	for (auto formObject : GetObjectForms())
	{
		if (eFormDataProcessor == formObject->GetTypeForm())
		{
			optlist.AddOption(formObject->GetName(), formObject->GetMetaID());
		}
	}

	return optlist;
}

//***************************************************************************
//*                       Save & load metadata                              *
//***************************************************************************

bool CMetaObjectDataProcessorValue::LoadData(CMemoryReader &dataReader)
{
	//Load object module
	m_moduleObject->LoadMeta(dataReader);
	m_moduleManager->LoadMeta(dataReader);

	//Load default form 
	m_defaultFormObject = dataReader.r_u32();
	return true;
}

bool CMetaObjectDataProcessorValue::SaveData(CMemoryWriter &dataWritter)
{
	//Save object module
	m_moduleObject->SaveMeta(dataWritter);
	m_moduleManager->SaveMeta(dataWritter);

	//Save default form 
	dataWritter.w_u32(m_defaultFormObject);
	return true;
}

//***********************************************************************
//*                           read & save events                        *
//***********************************************************************

#include "appData.h"

bool CMetaObjectDataProcessorValue::OnCreateMetaObject()
{
	if (m_objMode == METAOBJECT_NORMAL) {
		m_moduleManager->GenerateGuid();
		m_moduleManager->SetMetaID(m_metaData->GenerateNewID());
	}

	m_moduleObject->GenerateGuid();
	m_moduleObject->SetMetaID(m_metaData->GenerateNewID());
	return true;
}

bool CMetaObjectDataProcessorValue::OnLoadMetaObject()
{
	IModuleManager *moduleManager = m_metaData->GetModuleManager();
	wxASSERT(moduleManager);

	if (m_objMode == METAOBJECT_NORMAL) {
		m_moduleManager->SetMetadata(m_metaData);
		m_moduleManager->SetFullPath(m_sFullPath);

		m_metaData->AppendSecondaryMetadata(m_moduleManager);

		if (!moduleManager->AddCommonModule(m_moduleManager, true))
			return false;

		if (!m_moduleManager->OnLoadMetaObject())
			return false;

		m_moduleObject->SetMetadata(m_metaData);

		m_moduleObject->SetFullPath(m_sFullPath);
		m_moduleManager->SetMetaID(m_metaData->GenerateNewID());
		m_metaData->AppendSecondaryMetadata(m_moduleObject);

		if (!m_moduleObject->OnLoadMetaObject())
			return false;
	}
	else {
		m_moduleObject->SetMetadata(m_metaData);

		m_moduleObject->SetFullPath(m_sFullPath);
		m_moduleManager->SetMetaID(m_metaData->GenerateNewID());
		m_metaData->AppendSecondaryMetadata(m_moduleObject);

		if (!m_moduleObject->OnLoadMetaObject())
			return false;
	}

	return true;
}

bool CMetaObjectDataProcessorValue::OnSaveMetaObject()
{
	if (m_objMode == METAOBJECT_NORMAL) {
		if (!m_moduleManager->OnSaveMetaObject())
			return false;
	}

	if (!m_moduleObject->OnSaveMetaObject())
		return false;

	return true;
}

bool CMetaObjectDataProcessorValue::OnDeleteMetaObject()
{
	IModuleManager *moduleManager = m_metaData->GetModuleManager();
	wxASSERT(moduleManager);

	if (m_objMode == METAOBJECT_NORMAL) {
		if (!m_moduleManager->OnDeleteMetaObject())
			return false;

		m_metaData->RemoveSecondaryMetadata(m_moduleManager);
	}

	if (!m_moduleObject->OnDeleteMetaObject())
		return false;

	if (!moduleManager->RemoveCompileModule(m_moduleObject))
		return false;

	m_metaData->RemoveSecondaryMetadata(m_moduleObject);
	return true;
}

bool CMetaObjectDataProcessorValue::OnRunMetaObject()
{
	IModuleManager *moduleManager = m_metaData->GetModuleManager();
	wxASSERT(moduleManager);

	if (m_objMode == METAOBJECT_NORMAL)
	{
		if (!m_moduleManager->OnRunMetaObject())
			return false;
	}

	if (!m_moduleObject->OnRunMetaObject())
		return false;

	if (appData->IsDesignerMode()) {
		return moduleManager->AddCompileModule(m_moduleObject, CreateObjectValue());
	}

	return true;
}

bool CMetaObjectDataProcessorValue::OnCloseMetaObject()
{
	if (m_objMode == METAOBJECT_NORMAL)
	{
		if (!m_moduleManager->OnCloseMetaObject())
			return false;
	}

	if (!m_moduleObject->OnCloseMetaObject())
		return false;

	return true;
}

//***********************************************************************
//*                             form events                             *
//***********************************************************************

void CMetaObjectDataProcessorValue::OnCreateMetaForm(IMetaFormObject *metaForm)
{
	if (metaForm->GetTypeForm() == CMetaObjectDataProcessorValue::eFormDataProcessor
		&& m_defaultFormObject == wxNOT_FOUND)
	{
		m_defaultFormObject = metaForm->GetMetaID();
	}
}

void CMetaObjectDataProcessorValue::OnRemoveMetaForm(IMetaFormObject *metaForm)
{
	if (metaForm->GetTypeForm() == CMetaObjectDataProcessorValue::eFormDataProcessor
		&& m_defaultFormObject == metaForm->GetMetaID())
	{
		m_defaultFormObject = wxNOT_FOUND;
	}
}

//***********************************************************************
//*                           read & save property                      *
//***********************************************************************

void CMetaObjectDataProcessorValue::ReadProperty()
{
	IMetaObjectValue::ReadProperty();

	m_properties["default_object"]->SetValue(m_defaultFormObject);
}

void CMetaObjectDataProcessorValue::SaveProperty()
{
	IMetaObjectValue::SaveProperty();

	m_defaultFormObject = m_properties["default_object"]->GetValueAsInteger();
}

//***********************************************************************
//*                       Register in runtime                           *
//***********************************************************************

METADATA_REGISTER(CMetaObjectDataProcessorValue, "metaDataDataProcessor", g_metaDataProcessorCLSID);