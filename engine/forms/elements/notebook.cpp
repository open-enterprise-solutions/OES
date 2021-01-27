#include "notebook.h"
#include "forms/visualEditor.h"
#include "compiler/methods.h"

//***********************************************************************************
//*                           IMPLEMENT_DYNAMIC_CLASS                               *
//***********************************************************************************

wxIMPLEMENT_DYNAMIC_CLASS(CValueNotebook, CValueWindow);

//***********************************************************************************
//*                                 Special Notebook func                           *
//***********************************************************************************

void CValueNotebook::AddNotebookPage()
{
	IControlElement *m_newNotebookPage = new CValueNotebookPage();
	m_newNotebookPage->ReadProperty();

	m_visualHostContext->InsertObject(m_newNotebookPage, this);
	m_visualHostContext->ResolveSubtreeNameConflicts(m_newNotebookPage, this);

	m_newNotebookPage->SaveProperty();
	m_newNotebookPage->IncrRef();

	m_visualHostContext->RefreshEditor();
}

//***********************************************************************************
//*                                 Value Notebook                                  *
//***********************************************************************************

CValueNotebook::CValueNotebook() : CValueWindow(), m_name("notebook"), m_orientPage(wxAUI_NB_TOP), m_activePage(NULL)
{
	PropertyCategory *m_categoryNotebook = new PropertyCategory("Notebook");
	m_categoryNotebook->AddProperty("name");
	m_categoryNotebook->AddProperty("orient_page");
	m_category->AddCategory(m_categoryNotebook);

	//properties 
	m_properties["name"] = new Property("name", PropertyType::PT_WXNAME, this);
	m_properties["orient_page"] = new PropertyOption("orient_page", GetOrientPage(), this);
}

enum
{
	enPages = 0,
	enActivePage
};

void CValueNotebook::PrepareNames()                          //этот метод автоматически вызывается для инициализации имен атрибутов и методов
{
	IControlElement::PrepareNames();

	std::vector<SEng> aMethods =
	{
		{"pages", "pages()"},
		{"activePage", "activePage()"},
	};

	m_methods->PrepareMethods(aMethods.data(), aMethods.size());
}

#include "compiler/valueMap.h"

CValue CValueNotebook::Method(CMethodParameters &aParams)       //вызов метода
{
	switch (aParams.GetMethodIndex())
	{
	case enPages:
	{
		CValueStructure *m_pageMap = new CValueStructure(true);

		for (unsigned int i = 0; i < GetChildCount(); i++)
		{
			CValueNotebookPage *m_page = dynamic_cast<CValueNotebookPage *>(GetChild(i));
			if (m_page) m_pageMap->Insert(m_page->GetProperty("name"), CValue(m_page));
		}
#pragma message("nouverbe to nouverbe: необходимо доработать!")
		return m_pageMap;
	}
	case enActivePage: return m_activePage;
	}

	return IControlElement::Method(aParams);
}

#include "window/theme/luna_auitabart.h"

wxObject* CValueNotebook::Create(wxObject* parent, IVisualHost *visualHost)
{
	wxAuiNotebook *m_notebook = new wxAuiNotebook((wxWindow*)parent, wxID_ANY,
		m_pos,
		m_size, m_orientPage);

	m_notebook->SetArtProvider(new CLunaTabArt());

	m_notebook->Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGED, &CValueNotebook::OnChangedPage, this);
	m_notebook->Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGED, &CValueNotebook::OnChangedPage, this);

	return m_notebook;
}

void CValueNotebook::OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost)
{
}

void CValueNotebook::OnSelected(wxObject* wxobject)
{
}

void CValueNotebook::Update(wxObject* wxobject, IVisualHost *visualHost)
{
	wxAuiNotebook *m_notebook = dynamic_cast<wxAuiNotebook *>(wxobject);

	if (m_notebook)
	{
	}

	UpdateWindow(m_notebook);
}

void CValueNotebook::OnUpdated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost)
{
	wxAuiNotebook *m_notebook = dynamic_cast<wxAuiNotebook *>(wxobject);
	m_notebook->SetWindowStyle(m_orientPage);

}

void CValueNotebook::Cleanup(wxObject* wxobject, IVisualHost *visualHost)
{
	wxAuiNotebook *m_notebook = dynamic_cast<wxAuiNotebook *>(wxobject);
	m_notebook->Unbind(wxEVT_AUINOTEBOOK_PAGE_CHANGED, &CValueNotebook::OnChangedPage, this);
}

//**********************************************************************************
//*                                   Property                                     *
//**********************************************************************************

void CValueNotebook::ReadProperty()
{
	CValueWindow::ReadProperty();

	m_properties["name"]->SetValue(m_name);
	m_properties["orient_page"]->SetValue(m_orientPage);
}

void CValueNotebook::SaveProperty()
{
	CValueWindow::SaveProperty();

	m_name = m_properties["name"]->GetValueAsString();
	m_orientPage = m_properties["orient_page"]->GetValueAsInteger();
}