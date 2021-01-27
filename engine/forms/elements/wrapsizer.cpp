
#include "sizers.h"

wxIMPLEMENT_DYNAMIC_CLASS(CValueWrapSizer, CValueSizer)

//****************************************************************************
//*                             WrapSizer                                    *
//****************************************************************************

CValueWrapSizer::CValueWrapSizer() : CValueSizer()
{
	m_category->AddProperty("name");
	m_category->AddProperty("orient");

	m_properties["name"] = new Property("name", PropertyType::PT_WXNAME, this);
	m_properties["orient"] = new PropertyOption("orient", GetOrient(), this);
}

wxObject* CValueWrapSizer::Create(wxObject* /*parent*/, IVisualHost* /*visualHost*/)
{
	return new wxWrapSizer(m_orient, wxWRAPSIZER_DEFAULT_FLAGS);
}

void CValueWrapSizer::OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost)
{
}

void CValueWrapSizer::Update(wxObject* wxobject, IVisualHost *visualHost)
{
	wxWrapSizer *m_wrapsizer = dynamic_cast<wxWrapSizer *>(wxobject);

	if (m_wrapsizer)
	{
		m_wrapsizer->SetOrientation(m_orient);
		m_wrapsizer->SetMinSize(m_minimum_size);
	}

	UpdateSizer(m_wrapsizer);
}

void CValueWrapSizer::Cleanup(wxObject* obj, IVisualHost *visualHost)
{
}

//**********************************************************************************
//*                           Property                                             *
//**********************************************************************************

void CValueWrapSizer::ReadProperty()
{
	CValueSizer::ReadProperty();

	m_properties["name"]->SetValue(m_name);
	m_properties["orient"]->SetValue(m_orient);
}

void CValueWrapSizer::SaveProperty()
{
	CValueSizer::SaveProperty();

	m_name = m_properties["name"]->GetValueAsString();
	m_orient = (wxOrientation)m_properties["orient"]->GetValueAsInteger();
}