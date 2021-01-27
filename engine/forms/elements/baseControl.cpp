////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : base control
////////////////////////////////////////////////////////////////////////////

#include "baseControl.h"
#include "frame.h"
#include "compiler/procUnit.h"
#include "compiler/methods.h"
#include "utils/tinyxml/ticpp.h"
#include "utils/typeconv.h"

wxIMPLEMENT_ABSTRACT_CLASS(IControlElement, CValue)

//*************************************************************************
//*                          ControlElement                               *
//*************************************************************************

IControlElement::IControlElement() : CValue(eValueTypes::TYPE_VALUE), m_methods(new CMethods()), m_frameOwner(NULL), m_obj_id(0)
{
	m_category = NULL;
}

IControlElement::~IControlElement()
{
	wxDELETE(m_methods);
}

IControlElement *IControlElement::GetChild(unsigned int idx)
{
	return dynamic_cast<IControlElement *>(IObjectBase::GetChild(idx));
}

IControlElement *IControlElement::GetChild(unsigned int idx, const wxString & type)
{
	return dynamic_cast<IControlElement *>(IObjectBase::GetChild(idx, type));
}

IControlElement* IControlElement::GetLayout()
{
	IControlElement* result = NULL;

	if (GetParent() && GetParent()->GetObjectTypeName() == wxT("sizerItem"))
		result = GetParent();

	return result;
}

void IControlElement::SerializeObject(ticpp::Element* serializedElement)
{
	if (GetClassName() != (wxT("sizerItem")))
	{
		ticpp::Element element("object");
		element.SetAttribute("class", _STDSTR(GetClassName()));
		element.SetAttribute("expanded", GetExpanded());

		//add default id from obj 
		ticpp::Element system_element("system");
		system_element.SetAttribute("name", "id");
		system_element.SetText(m_obj_id);
		element.LinkEndChild(&system_element);

		for (unsigned int i = 0; i < GetPropertyCount(); i++)
		{
			Property *prop = GetProperty(i);
			ticpp::Element prop_element("property");
			prop_element.SetAttribute("name", _STDSTR(prop->GetName()));
			prop_element.SetText(_STDSTR(prop->GetValue()));
			element.LinkEndChild(&prop_element);
		}

		for (unsigned int i = 0; i < GetEventCount(); i++)
		{
			Event *event = GetEvent(i);
			ticpp::Element event_element("event");
			event_element.SetAttribute("name", _STDSTR(event->GetName()));
			event_element.SetText(_STDSTR(event->GetValue()));
			element.LinkEndChild(&event_element);
		}

		//add in end property & event values from sizerItem
		if (m_parent && m_parent->GetClassName() == (wxT("sizerItem")))
		{
			ticpp::Element system_element_sizer("system_sizer");
			system_element_sizer.SetAttribute("name", "id");
			system_element_sizer.SetText(GetParent()->GetControlID());
			element.LinkEndChild(&system_element_sizer);

			for (unsigned int i = 0; i < m_parent->GetPropertyCount(); i++)
			{
				Property *prop = m_parent->GetProperty(i);
				ticpp::Element prop_element("property_sizer");
				prop_element.SetAttribute("name", _STDSTR(prop->GetName()));
				prop_element.SetText(_STDSTR(prop->GetValue()));
				element.LinkEndChild(&prop_element);
			}

			for (unsigned int i = 0; i < m_parent->GetEventCount(); i++)
			{
				Event *event = m_parent->GetEvent(i);
				ticpp::Element event_element("event_sizer");
				event_element.SetAttribute("name", _STDSTR(event->GetName()));
				event_element.SetText(_STDSTR(event->GetValue()));
				element.LinkEndChild(&event_element);
			}
		}

		for (unsigned int i = 0; i < GetChildCount(); i++)
		{
			IControlElement *child = dynamic_cast<IControlElement *>(GetChild(i));
			ticpp::Element child_element;
			child->SerializeObject(&child_element);
			element.LinkEndChild(&child_element);
		}

		*serializedElement = element;
	}
	else
	{
		for (unsigned int i = 0; i < GetChildCount(); i++)
		{
			IControlElement *child = dynamic_cast<IControlElement *>(GetChild(i));
			child->SerializeObject(serializedElement);
		}
	}
}

void IControlElement::Serialize(ticpp::Document* serializedDocument)
{
	ticpp::Document document("document");

#if wxUSE_UNICODE
	ticpp::Declaration dec("1.0", "UTF-8", "yes");
#else
	ticpp::Declaration dec("1.0", "ISO-8859-13", "yes");
#endif

	document.LinkEndChild(&dec);

	ticpp::Element root("form_data");

	ticpp::Element element;
	SerializeObject(&element);

	root.LinkEndChild(&element);
	document.LinkEndChild(&root);

	*serializedDocument = document;
}

wxString IControlElement::GetTypeString() const
{
	return GetObjectTypeName() << wxT(".") << GetClassName();
}

wxString IControlElement::GetString() const
{
	return GetTypeString();
}

//*******************************************************************
//*                          Runtime                                *
//*******************************************************************

bool IControlElement::ProcessEvent(const wxString &sEventName)
{
	if (sEventName.Length() > 0 &&
		m_events.find(sEventName) != m_events.end())
	{
		wxString m_sEventValue = m_events[sEventName]->GetValue();
		CProcUnit *m_formProcUnit = GetFormProcUnit();

		CValue m_bCancel = false;

		ReadProperty();

		if (m_formProcUnit && m_sEventValue.Length() > 0)
		{
			try
			{
				CValue m_controlElement = this;
				m_formProcUnit->CallFunction(m_sEventValue, m_controlElement, m_bCancel);
			}
			catch (...)
			{
				return false;
			}
		}

		return m_bCancel.GetBoolean();
	}

	return false;
}

#include "forms/visualEditorView.h"

void IControlElement::SetParent(IControlElement *parent)
{
	if (parent &&
		parent->m_frameOwner &&
		!m_frameOwner)
	{
		CValueFrame *frameOwner = parent->m_frameOwner;

		if (GetClassName() != wxT("sizerItem"))
			frameOwner->m_aControls.push_back(this);
	}
	else if (!parent &&
		m_frameOwner)
	{
		if (GetClassName() != wxT("sizerItem"))
			m_frameOwner->m_aControls.erase(std::find(m_frameOwner->m_aControls.begin(), m_frameOwner->m_aControls.end(), this));
	}

	m_frameOwner = parent ? parent->m_frameOwner : NULL;

	IObjectBase::SetParent(parent);
}

wxObject *IControlElement::GetWxObject()
{
	CValueFrame *m_valueFrame = dynamic_cast<CValueFrame *>(GetOwnerForm());
	if (!m_valueFrame) return NULL;

	//if run designer form search in own visualHost 
	if (m_visualHostContext)
	{
		CVisualEditorContextForm::CVisualEditor *m_visualEditor = m_visualHostContext->GetVisualEditor();
		return m_visualEditor->GetWxObject(this);
	}

	CVisualDocument *m_visualDoc = m_valueFrame->GetVisualDocument();
	if (!m_visualDoc) return NULL;

	CVisualView *m_visualView = m_visualDoc->GetVisualView();
	if (!m_visualView) return NULL;

	return m_visualView->GetWxObject(this);
}

#include "metadata/metaObjects/metaFormObject.h"

int IControlElement::GetTypeForm()
{
	if (!m_frameOwner) {
		wxASSERT(m_frameOwner);
		return 0;
	}

	IMetaFormObject *metaFormObj = m_frameOwner->GetFormMetaObject();
	wxASSERT(metaFormObj);
	return metaFormObj->GetTypeForm();
}

CProcUnit *IControlElement::GetFormProcUnit() const
{
	if (!m_frameOwner) {
		wxASSERT(m_frameOwner);
		return NULL;
	}

	return m_frameOwner->GetProcUnit();
}

//*******************************************************************
//*                           Attributes                            *
//*******************************************************************

#include "compiler/methods.h"

void IControlElement::SetAttribute(CAttributeParameters &aParams, CValue &cVal)
{
	wxString sAlias = m_methods->GetAttributeAlias(aParams.GetAttributeIndex());

	if (sAlias == wxT("attribute"))
	{
		Property *m_property = GetPropertyByIndex(aParams.GetAttributeIndex());
		if (m_property) SetPropertyData(m_property, cVal);
	}
	else if (sAlias == wxT("sizerItem"))
	{
		//if we have sizerItem then call him savepropery 
		IControlElement *m_sizeritem = GetParent();
		if (m_sizeritem && m_sizeritem->GetClassName() == wxT("sizerItem"))
		{
			Property *m_property = m_sizeritem->GetPropertyByIndex(aParams.GetAttributeIndex());
			if (m_property) SetPropertyData(m_property, cVal);
		}
	}

	SaveProperty();

	CValueFrame *m_valueFrame = dynamic_cast<CValueFrame *>(GetOwnerForm());
	if (!m_valueFrame) return;
	CVisualDocument *m_visualDoc = m_valueFrame->GetVisualDocument();
	if (!m_visualDoc) return;
	CVisualView *m_visualView = m_visualDoc->GetVisualView();
	if (!m_visualView) return;

	wxObject *m_object = m_visualView->GetWxObject(this);
	if (m_object)
	{
		wxWindow *m_parentWnd = NULL;
		Update(m_object, m_visualView);
		IControlElement* nextParent = GetParent();
		while (!m_parentWnd && nextParent)
		{
			if (nextParent->GetComponentType() == COMPONENT_TYPE_WINDOW) { m_parentWnd = dynamic_cast<wxWindow *>(m_visualView->GetWxObject(nextParent)); break; }
			nextParent = nextParent->GetParent();
		}
		if (!m_parentWnd) m_parentWnd = m_visualView->GetBackgroundWindow();
		OnUpdated(m_object, m_parentWnd, m_visualView);
	}
}

CValue IControlElement::GetAttribute(CAttributeParameters &aParams)
{
	wxString sAlias = m_methods->GetAttributeAlias(aParams.GetAttributeIndex());

	if (sAlias == wxT("sizerItem"))
	{
		//if we have sizerItem then call him savepropery 
		IControlElement *m_sizeritem = GetParent();
		if (m_sizeritem && m_sizeritem->GetClassName() == wxT("sizerItem"))
		{
			unsigned int idx = m_methods->GetAttributePosition(aParams.GetAttributeIndex());
			Property *m_property = m_sizeritem->GetPropertyByIndex(idx);
			if (m_property) return GetPropertyData(m_property);
		}
	}
	else
	{
		Property *m_property = GetPropertyByIndex(aParams.GetAttributeIndex());
		if (m_property) return GetPropertyData(m_property);
	}

	return CValue();
}

int IControlElement::FindAttribute(const wxString &sName) const
{
	return GetPropertyIndex(sName) + 1;
}

//*******************************************************************
//*                           Property                              *
//*******************************************************************

void IControlElement::ReadProperty()
{
}

void IControlElement::SaveProperty()
{
}