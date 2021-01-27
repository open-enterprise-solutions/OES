
#include "sizers.h"

wxIMPLEMENT_DYNAMIC_CLASS(CValueBoxSizer, CValueSizer)

//*******************************************************************
//*                             BoxSizer                            *
//*******************************************************************

CValueBoxSizer::CValueBoxSizer() : CValueSizer(), m_name("boxSizer"), m_orient(wxVERTICAL), m_flags(0)
{
	m_category->AddProperty("name");
	m_category->AddProperty("orient");
	//m_category->AddProperty("flags");

	m_properties["name"] = new Property("name", PropertyType::PT_WXNAME, this);
	m_properties["orient"] = new PropertyOption("orient", GetOrient(), this);
	//m_properties["flags"] = new Property("flags", PropertyType::PT_WXSTRING, this);
}

wxObject* CValueBoxSizer::Create(wxObject* /*parent*/, IVisualHost* /*visualHost*/)
{
	return new wxBoxSizer(m_orient);
}

void CValueBoxSizer::OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost)
{
}

void CValueBoxSizer::Update(wxObject* wxobject, IVisualHost *visualHost)
{
	wxBoxSizer *m_boxsizer = dynamic_cast<wxBoxSizer *>(wxobject);

	if (m_boxsizer)
	{
		m_boxsizer->SetOrientation(m_orient);
		m_boxsizer->SetMinSize(m_minimum_size);
	}

	UpdateSizer(m_boxsizer);
}

void CValueBoxSizer::Cleanup(wxObject* obj, IVisualHost *visualHost)
{
}

//*******************************************************************
//*                           Property                              *
//*******************************************************************

void CValueBoxSizer::ReadProperty()
{
	CValueSizer::ReadProperty();

	m_properties["name"]->SetValue(m_name);
	m_properties["orient"]->SetValue(m_orient);
}

void CValueBoxSizer::SaveProperty()
{
	CValueSizer::SaveProperty();

	m_name = m_properties["name"]->GetValueAsString();
	m_orient = (wxOrientation)m_properties["orient"]->GetValueAsInteger();
}