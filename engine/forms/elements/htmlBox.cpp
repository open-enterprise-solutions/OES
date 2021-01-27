#include "htmlbox.h"
#include "compiler/methods.h"

//***********************************************************************************
//*                           IMPLEMENT_DYNAMIC_CLASS                               *
//***********************************************************************************

wxIMPLEMENT_DYNAMIC_CLASS(CValueHTMLBox, CValueWindow);

//***********************************************************************************
//*                                 Value Notebook                                  *
//***********************************************************************************

CValueHTMLBox::CValueHTMLBox() : CValueWindow(), m_name("htmlbox")
{
	PropertyCategory *m_categoryNotebook = new PropertyCategory("HTML");
	m_categoryNotebook->AddProperty("name");
	m_category->AddCategory(m_categoryNotebook);

	//properties 
	m_properties["name"] = new Property("name", PropertyType::PT_WXNAME, this);
}

enum
{
	enSetPage = 0,
};

void CValueHTMLBox::PrepareNames() const                         //этот метод автоматически вызывается для инициализации имен атрибутов и методов
{
	IControlElement::PrepareNames();

	std::vector<SEng> aMethods =
	{
		{"setPage", "setPage(string)"}
	};

	m_methods->PrepareMethods(aMethods.data(), aMethods.size());
}

CValue CValueHTMLBox::Method(CMethodParameters &aParams)       //вызов метода
{
	wxHtmlWindow *m_htmlBox = dynamic_cast<wxHtmlWindow *>(GetWxObject());

	switch (aParams.GetMethodIndex())
	{
	case enSetPage: return m_htmlBox ? m_htmlBox->SetPage(aParams[0].ToString()) : !aParams[0].ToString().IsEmpty();
	}

	return IControlElement::Method(aParams);
}

wxObject* CValueHTMLBox::Create(wxObject* parent, IVisualHost *visualHost)
{
	wxHtmlWindow *m_htmlBox = new wxHtmlWindow((wxWindow*)parent, wxID_ANY,
		m_pos,
		m_size);

	wxString dummy_page(
		wxT("<b>wxHtmlWindow</b><br />")
		wxT("This is a dummy page.</body></html>"));

	m_htmlBox->SetPage(dummy_page);

	return m_htmlBox;
}

void CValueHTMLBox::OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost)
{
	wxHtmlWindow *m_htmlBox = dynamic_cast<wxHtmlWindow *>(wxobject);
}

void CValueHTMLBox::OnSelected(wxObject* wxobject)
{
}

void CValueHTMLBox::Update(wxObject* wxobject, IVisualHost *visualHost)
{
	wxHtmlWindow *m_htmlBox = dynamic_cast<wxHtmlWindow *>(wxobject);

	if (m_htmlBox)
	{
	}

	UpdateWindow(m_htmlBox);
}

void CValueHTMLBox::Cleanup(wxObject* obj, IVisualHost *visualHost)
{
}

//**********************************************************************************
//*                                   Property                                     *
//**********************************************************************************

void CValueHTMLBox::ReadProperty()
{
	CValueWindow::ReadProperty();

	m_properties["name"]->SetValue(m_name);
}

void CValueHTMLBox::SaveProperty()
{
	CValueWindow::SaveProperty();

	m_name = m_properties["name"]->GetValueAsString();
}