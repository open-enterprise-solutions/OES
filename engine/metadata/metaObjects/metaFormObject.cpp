////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : metaform object
////////////////////////////////////////////////////////////////////////////

#include "metaFormObject.h"
#include "database/databaseLayer.h"
#include "forms/visualEditorView.h"
#include "metadata/metadata.h"
#include "metadata/objects/baseObject.h"
#include "appData.h"

//***********************************************************************
//*                             IMetaFormObject metadata                    *
//***********************************************************************

wxIMPLEMENT_ABSTRACT_CLASS(IMetaFormObject, CMetaModuleObject);

//***********************************************************************
//*                          common value object                        *
//***********************************************************************

IMetaFormObject::IMetaFormObject(const wxString &name, const wxString &alias, const wxString &comment) :
	CMetaModuleObject(name, alias, comment)
{
}

bool IMetaFormObject::LoadData(CMemoryReader &reader)
{
	reader.r_stringZ(m_sForm);
	reader.r_stringZ(m_sModule);

	return true;
}

bool IMetaFormObject::SaveData(CMemoryWriter &writer)
{
	writer.w_stringZ(m_sForm);
	writer.w_stringZ(m_sModule);

	return true;
}

CValueFrame *IMetaFormObject::GenerateForm(IMetaObjectValue *metaValue, IDataObjectSource *owner)
{
	CValueFrame *m_valueFrame = NULL;

	if (m_sForm.IsEmpty())
	{
		m_valueFrame = new CValueFrame();
		m_valueFrame->SetSourceObject(owner);
		m_valueFrame->SetMetaObject(this, metaValue);
		m_valueFrame->SetReadOnly(IsEditable());
		m_valueFrame->ReadProperty();

		return m_valueFrame;
	}

	ticpp::Document doc;
	doc.Parse(m_sForm.ToStdString(), 0, TIXML_ENCODING_UTF8);

	ticpp::Element* root = doc.FirstChildElement();
	ticpp::Element* object = root->FirstChildElement(wxT("object"));

	try
	{
		m_valueFrame = visualEditorDatabase->CreateFrame(object, IsEditable());
	}
	catch (std::exception& ex)
	{
		wxLogError(ex.what());
	}

	if (m_valueFrame)
	{
		m_valueFrame->SetSourceObject(owner);
		m_valueFrame->SetMetaObject(this, metaValue);
		m_valueFrame->SetReadOnly(IsEditable());
		m_valueFrame->ReadProperty();
	}

	return m_valueFrame;
}

CValueFrame *IMetaFormObject::GenerateFormAndRun(IMetaObjectValue *metaValue, IDataObjectSource *owner, bool runModule)
{
	CValueFrame *valueFrame = NULL;
	IModuleManager *moduleManager = m_metaData->GetModuleManager();
	wxASSERT(moduleManager);

	if (!moduleManager->FindCompileModule(this, valueFrame))
	{
		valueFrame = GenerateForm(metaValue, owner);

		if (!valueFrame->InitializeModule(runModule)) {
		}
	}

	return valueFrame;
}

CValueFrame *IMetaFormObject::GenerateFormAndRun(bool runModule)
{
	CValueFrame *valueFrame = NULL;
	IModuleManager *moduleManager = m_metaData->GetModuleManager();
	wxASSERT(moduleManager);

	if (!moduleManager->FindCompileModule(this, valueFrame))
	{
		valueFrame = GenerateForm();
		valueFrame->InitializeModule(runModule);
	}

	return valueFrame;
}

wxIMPLEMENT_DYNAMIC_CLASS(CMetaFormObject, IMetaFormObject)

//***********************************************************************
//*                            Metaform                                 *
//***********************************************************************

CMetaFormObject::CMetaFormObject(const wxString &name, const wxString &alias, const wxString &comment) : IMetaFormObject(name, alias, comment),
m_nTypeFrom(formDefaultID)
{
	PropertyCategory *m_category_form = new PropertyCategory("FormType");
	m_category_form->AddProperty("form_type");
	m_category->AddCategory(m_category_form);

	m_properties["form_type"] = new Property("form_type", PropertyType::PT_OPTION_CUSTOM, this);
}

bool CMetaFormObject::LoadData(CMemoryReader &reader)
{
	m_nTypeFrom = reader.r_u8();
	return IMetaFormObject::LoadData(reader);
}

bool CMetaFormObject::SaveData(CMemoryWriter &writer)
{
	writer.w_u8(m_nTypeFrom);
	return IMetaFormObject::SaveData(writer);
}

void CMetaFormObject::PrepareOption(Property *property, OptionList &optionlist)
{
	if (property->GetName() == wxT("form_type")) {

		IMetaObjectValue *metaObjectValue = wxStaticCast(m_parent, IMetaObjectValue);
		wxASSERT(metaObjectValue);

		optionlist = metaObjectValue->GetFormType();
		optionlist.AddOption(formDefaultName, formDefaultID);
	}
}

//***********************************************************************
//*                           read & save property                      *
//***********************************************************************

void CMetaFormObject::ReadProperty()
{
	CMetaModuleObject::ReadProperty();
	m_properties["form_type"]->SetValue(m_nTypeFrom);
}

void CMetaFormObject::SaveProperty()
{
	CMetaModuleObject::SaveProperty();
	m_nTypeFrom = m_properties["form_type"]->GetValueAsInteger();
}

//***********************************************************************
//*                             event object                            *
//***********************************************************************

#include "window/mainFrame.h"
#include "window/formSelector/formSelector.h"

bool CMetaFormObject::OnCreateMetaObject()
{
	IMetaObjectValue *metaObjectValue = wxStaticCast(m_parent, IMetaObjectValue);
	wxASSERT(metaObjectValue);
	OptionList optList = metaObjectValue->GetFormType();

	CSelectTypeForm *selectTypeForm = new CSelectTypeForm(this);

	for (auto option : optList.GetOptions()) {
		selectTypeForm->AppendTypeForm(option.m_option, option.m_dblVal);
	}

	selectTypeForm->CreateSelector();
	m_nTypeFrom = selectTypeForm->ShowModal();
	metaObjectValue->OnCreateMetaForm(this); 

	return CMetaModuleObject::OnCreateMetaObject();
}

bool CMetaFormObject::OnLoadMetaObject()
{
	return CMetaModuleObject::OnLoadMetaObject();
}

bool CMetaFormObject::OnSaveMetaObject()
{
	return CMetaModuleObject::OnSaveMetaObject();
}

bool CMetaFormObject::OnDeleteMetaObject()
{
	IModuleManager *moduleManager = m_metaData->GetModuleManager();
	wxASSERT(moduleManager);

	if (!moduleManager->RemoveCompileModule(this))
		return false;

	IMetaObjectValue *metaObjectValue = wxStaticCast(m_parent, IMetaObjectValue);
	wxASSERT(metaObjectValue);
	metaObjectValue->OnRemoveMetaForm(this);

	return CMetaModuleObject::OnDeleteMetaObject();
}

bool CMetaFormObject::OnRunMetaObject()
{
	if (appData->IsDesignerMode())
	{
		IModuleManager *moduleManager = m_metaData->GetModuleManager();
		wxASSERT(moduleManager);

		IMetaObjectValue *metaObjectValue = wxStaticCast(m_parent, IMetaObjectValue);
		wxASSERT(metaObjectValue);

		return moduleManager->AddCompileModule(this, metaObjectValue->CreateObjectValue(this));
	}

	return CMetaModuleObject::OnRunMetaObject();
}

bool CMetaFormObject::OnCloseMetaObject()
{
	return CMetaModuleObject::OnCloseMetaObject();
}

//***********************************************************************
//*                           CommonFormObject metadata                 *
//***********************************************************************

wxIMPLEMENT_DYNAMIC_CLASS(CMetaCommonFormObject, IMetaFormObject)

CMetaCommonFormObject::CMetaCommonFormObject(const wxString &name, const wxString &alias, const wxString &comment) : IMetaFormObject(name, alias, comment) {}

//***********************************************************************
//*                             event object                            *
//***********************************************************************

bool CMetaCommonFormObject::OnRunMetaObject()
{
	if (appData->IsDesignerMode())
	{
		IModuleManager *moduleManager = m_metaData->GetModuleManager();
		wxASSERT(moduleManager);
		return moduleManager->AddCompileModule(this, GenerateFormAndRun(true));
	}

	return CMetaModuleObject::OnRunMetaObject();
}

bool CMetaCommonFormObject::OnCloseMetaObject()
{
	return CMetaModuleObject::OnCloseMetaObject();
}

//***********************************************************************
//*                       Register in runtime                           *
//***********************************************************************

METADATA_REGISTER(CMetaFormObject, "metaForm", g_metaFormCLSID);
METADATA_REGISTER(CMetaCommonFormObject, "metaCommonForm", g_metaCommonFormCLSID);