
#include "sizers.h"

wxIMPLEMENT_DYNAMIC_CLASS(CValueGridSizer, CValueSizer)

//****************************************************************************
//*                             GridSizer                                    *
//****************************************************************************

CValueGridSizer::CValueGridSizer() : CValueSizer()
{
	m_category->AddProperty("name");
	m_category->AddProperty("row");
	m_category->AddProperty("cols");

	m_properties["name"] = new Property("name", PropertyType::PT_WXNAME, this);
	m_properties["row"] = new Property("row", PropertyType::PT_INT, this);
	m_properties["cols"] = new Property("cols", PropertyType::PT_INT, this);
}

wxObject* CValueGridSizer::Create(wxObject* /*parent*/, IVisualHost* /*visualHost*/)
{
	return new wxGridSizer(m_row, m_cols, 0, 0);
}

void CValueGridSizer::OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost)
{
}

void CValueGridSizer::Update(wxObject* wxobject, IVisualHost *visualHost)
{
	wxGridSizer *m_gridsizer = dynamic_cast<wxGridSizer *>(wxobject);

	if (m_gridsizer)
	{
		m_gridsizer->SetRows(m_row);
		m_gridsizer->SetCols(m_cols);

		m_gridsizer->SetMinSize(m_minimum_size);
	}

	UpdateSizer(m_gridsizer);
}

void CValueGridSizer::Cleanup(wxObject* obj, IVisualHost *visualHost)
{
}

//**********************************************************************************
//*                           Property                                             *
//**********************************************************************************

void CValueGridSizer::ReadProperty()
{
	CValueSizer::ReadProperty();

	m_properties["name"]->SetValue(m_name);
	m_properties["row"]->SetValue(m_row);
	m_properties["cols"]->SetValue(m_cols);
}

void CValueGridSizer::SaveProperty()
{
	CValueSizer::SaveProperty();

	m_name = m_properties["name"]->GetValueAsString();
	m_row = m_properties["row"]->GetValueAsInteger();
	m_cols = m_properties["cols"]->GetValueAsInteger();
}