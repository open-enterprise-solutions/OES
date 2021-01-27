////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : frame control
////////////////////////////////////////////////////////////////////////////

#include "frame.h"

wxIMPLEMENT_DYNAMIC_CLASS(CValueFrame, IControlElement);

//****************************************************************************
//*                              Frame                                       *
//****************************************************************************

CValueFrame::CValueFrame() : IControlElement(), IModuleInfo(),
m_sourceObject(NULL),
m_metaObject(NULL),
m_valueFrameDocument(NULL),
m_orient(wxVERTICAL),
m_formModified(false)
{
	m_category = new PropertyCategory("formProperty");

	PropertyCategory *m_categoryFrame = new PropertyCategory("Frame");
	m_categoryFrame->AddProperty("title");
	m_categoryFrame->AddProperty("fg");
	m_categoryFrame->AddProperty("bg");

	m_category->AddCategory(m_categoryFrame);

	m_properties["name"] = new Property("name", PropertyType::PT_WXNAME, this);
	m_properties["title"] = new Property("title", PropertyType::PT_WXSTRING, this);

	m_properties["fg"] = new Property("fg", PropertyType::PT_WXCOLOUR, this);
	m_properties["bg"] = new Property("bg", PropertyType::PT_WXCOLOUR, this);

	m_categoryFrame->AddEvent("beforeOpen");
	m_categoryFrame->AddEvent("onOpen");
	m_categoryFrame->AddEvent("onReopen");
	m_categoryFrame->AddEvent("beforeClose");
	m_categoryFrame->AddEvent("onClose");

	m_events["beforeOpen"] = new Event("beforeOpen", _("Calls on before open form. May be cancelled."), this);
	m_events["onOpen"] = new Event("onOpen", "", this);
	m_events["onReopen"] = new Event("onReopen", "", this);
	m_events["beforeClose"] = new Event("beforeClose", "", this);
	m_events["onClose"] = new Event("onClose", "", this);

	PropertyCategory *m_category_sizer = new PropertyCategory("Sizer");
	m_category_sizer->AddProperty("orient");
	m_properties["orient"] = new PropertyOption("orient", GetOrient(), this);
	m_category->AddCategory(m_category_sizer);

	//init frame controls
	m_frameControls = new CValueFrameControls(this);
	m_frameControls->IncrRef();

	//set owner 
	m_frameOwner = this;
}

void CValueFrame::Update(wxObject* wxobject, IVisualHost *visualHost)
{
	UpdateForm();
}

void CValueFrame::OnUpdated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost)
{
	//lay out parent window 
	wxWindow *m_wndParent = visualHost->GetParent();
	if (m_wndParent) m_wndParent->Layout();
}

void CValueFrame::SetPropertyData(Property *property, const CValue &srcValue)
{
	IControlElement::SetPropertyData(property, srcValue);
}

#include "forms/special/enums/valueOrient.h"

CValue CValueFrame::GetPropertyData(Property *property)
{
	if (property->GetName() == wxT("orient"))
	{
		return new CValueEnumOrient(m_orient);
	}

	return IControlElement::GetPropertyData(property);
}

//**********************************************************************************
//*                                   Property                                     *
//**********************************************************************************

void CValueFrame::ReadProperty()
{
	m_properties["name"]->SetValue(m_name);
	m_properties["title"]->SetValue(m_title);
	m_properties["orient"]->SetValue(m_orient);

	m_properties["fg"]->SetValue(m_fg);
	m_properties["bg"]->SetValue(m_bg);
}

void CValueFrame::SaveProperty()
{
	m_name = m_properties["name"]->GetValueAsString();
	m_title = m_properties["title"]->GetValueAsString();
	m_orient = (wxOrientation)m_properties["orient"]->GetValueAsInteger();

	m_fg = m_properties["fg"]->GetValueAsColour();
	m_bg = m_properties["bg"]->GetValueAsColour();
}

#include "metadata/objects/baseObject.h"

void CValueFrame::ClearRecursive(IControlElement *control)
{
	for (unsigned int i = 0; i < control->GetChildCount(); i++)
	{
		IControlElement *controlChild = dynamic_cast<IControlElement *>(control->GetChild(i));
		ClearRecursive(controlChild);
		if (controlChild) controlChild->DecrRef();
	}
}

CValueFrame::~CValueFrame()
{
	if (m_frameControls)
		m_frameControls->DecrRef();

	for (unsigned int i = 0; i < GetChildCount(); i++)
	{
		IControlElement *controlChild = dynamic_cast<IControlElement *>(GetChild(i));
		ClearRecursive(controlChild);
		if (controlChild) controlChild->DecrRef();
	}

	if (m_sourceObject) 
		m_sourceObject->DecrRef(); 
}