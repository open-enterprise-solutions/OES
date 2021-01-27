#ifndef _NOTEBOOKS_H__
#define _NOTEBOOKS_H__

#include "window.h"
#include <wx/aui/auibook.h>

class CValueNotebookPage;

class CValueNotebook : public CValueWindow
{
	wxDECLARE_DYNAMIC_CLASS(CValueNotebook);

protected:

	OptionList *GetOrientPage()
	{
		OptionList *m_optionlist = new OptionList();
		m_optionlist->AddOption("Top", wxAUI_NB_TOP);
		//m_optionlist->AddOption("Left", wxAUI_NB_LEFT);
		//m_optionlist->AddOption("Right", wxAUI_NB_RIGHT);
		m_optionlist->AddOption("Bottom", wxAUI_NB_BOTTOM);
		return m_optionlist;
	}

private:

	wxString m_name;
	int m_orientPage;

private:

	CValueNotebookPage *m_activePage;

	std::vector< CValueNotebookPage*> m_aPages;

public:

	CValueNotebook();

	virtual wxObject* Create(wxObject* parent, IVisualHost *visualHost) override;
	virtual void OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost) override;
	virtual void OnUpdated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost) override;
	virtual void OnSelected(wxObject* wxobject) override;
	virtual void Update(wxObject* wxobject, IVisualHost *visualHost) override;
	virtual void Cleanup(wxObject* obj, IVisualHost *visualHost) override;

	virtual wxString GetClassName() const override { return wxT("notebook"); }
	virtual wxString GetObjectTypeName() const override { return wxT("notebook"); }

	//methods 
	virtual void PrepareNames();                          //этот метод автоматически вызывается для инициализации имен атрибутов и методов
	virtual CValue Method(CMethodParameters &aParams);       //вызов метода

	/**
	* Support default menu
	*/
	virtual void PrepareDefaultMenu(wxMenu *m_menu);
	virtual void ExecuteMenu(IVisualHost *visualHost, int id);

	void AddNotebookPage();

	//read&save propery 
	virtual void ReadProperty() override;
	virtual void SaveProperty() override;

protected:

	//Events
	void OnChangedPage(wxAuiNotebookEvent& event);
};

class CValueNotebookPage : public IControlElement
{
	wxDECLARE_DYNAMIC_CLASS(CValueNotebookPage);

protected:

	OptionList *GetOrient()
	{
		OptionList *m_optionlist = new OptionList();
		m_optionlist->AddOption("Vertical", wxVERTICAL);
		m_optionlist->AddOption("Horizontal", wxHORIZONTAL);
		return m_optionlist;
	}

private:

	wxString m_name;
	wxString m_label;
	wxBitmap m_bitmap;
	bool m_visible;
	int m_orient;

public:

	CValueNotebookPage();

	virtual wxObject* Create(wxObject* parent, IVisualHost *visualHost) override;
	virtual void OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost) override;
	virtual void OnUpdated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost) override;
	virtual void OnSelected(wxObject* wxobject) override;
	virtual void Cleanup(wxObject* obj, IVisualHost *visualHost) override;

	virtual wxString GetClassName() const override { return wxT("page"); }
	virtual wxString GetObjectTypeName() const override { return wxT("page"); }

	virtual int GetComponentType() override { return COMPONENT_TYPE_WINDOW; }
	virtual bool IsItem() override { return false; }

	//read&save propery 
	virtual void ReadProperty() override;
	virtual void SaveProperty() override;

	friend class CValueNotebook;
};

#endif 