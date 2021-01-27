#include "gridbox.h"
#include "forms/visualEditor.h"

//***********************************************************************************
//*                           IMPLEMENT_DYNAMIC_CLASS                               *
//***********************************************************************************

wxIMPLEMENT_DYNAMIC_CLASS(CValueGridBox, CValueWindow);

//***********************************************************************************
//*                                 Value Notebook                                  *
//***********************************************************************************

CValueGridBox::CValueGridBox() : CValueWindow(), m_name("gridbox")
{
	PropertyCategory *m_categoryNotebook = new PropertyCategory("Grid");
	m_categoryNotebook->AddProperty("name");
	m_category->AddCategory(m_categoryNotebook);

	//properties 
	m_properties["name"] = new Property("name", PropertyType::PT_WXNAME, this);
}

wxObject* CValueGridBox::Create(wxObject* parent, IVisualHost *visualHost)
{
	CGrid *m_grid = new CGrid((wxWindow*)parent, wxID_ANY, m_pos, m_size);
	return m_grid;
}

void CValueGridBox::OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost)
{
	CGrid *m_grid = dynamic_cast<CGrid *>(wxobject);
}

void CValueGridBox::OnSelected(wxObject* wxobject)
{
}

void CValueGridBox::Update(wxObject* wxobject, IVisualHost *visualHost)
{
	CGrid *m_grid = dynamic_cast<CGrid *>(wxobject);

	if (m_grid)
	{
	}

	UpdateWindow(m_grid);
}

void CValueGridBox::Cleanup(wxObject* wxobject, IVisualHost *visualHost)
{
}

//**********************************************************************************
//*                                   Property                                     *
//**********************************************************************************

void CValueGridBox::ReadProperty()
{
	CValueWindow::ReadProperty();

	m_properties["name"]->SetValue(m_name);
}

void CValueGridBox::SaveProperty()
{
	CValueWindow::SaveProperty();

	m_name = m_properties["name"]->GetValueAsString();
}