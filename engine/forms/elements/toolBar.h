#ifndef _TOOLBAR_H__
#define _TOOLBAR_H__

#include "window.h"
#include "window/controls/toolBarCtrl.h"

class CValueToolBarItem;
class CValueToolBarSeparator;

class CValueToolbar : public CValueWindow
{
	wxDECLARE_DYNAMIC_CLASS(CValueToolbar);

public:

	wxString m_name;
	wxSize m_bitmapsize;
	wxSize m_margins;
	int m_packing;
	int m_separation;

	int m_actionSource;

public:

	CValueToolbar();

	virtual wxObject* Create(wxObject* parent, IVisualHost *visualHost) override;
	virtual void OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost) override;
	virtual void Update(wxObject* wxobject, IVisualHost *visualHost) override;
	virtual void OnUpdated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost) override;
	virtual void Cleanup(wxObject* obj, IVisualHost *visualHost) override;

	virtual wxString GetClassName() const override { return wxT("toolbar"); }
	virtual wxString GetObjectTypeName() const override { return wxT("toolbar"); }

	//read&save propery 
	virtual void ReadProperty() override;
	virtual void SaveProperty() override;

	/**
	* Support default menu
	*/
	virtual void PrepareDefaultMenu(wxMenu *m_menu);
	virtual void ExecuteMenu(IVisualHost *visualHost, int id);

	/**
	* Support custom option
	*/
	virtual void PrepareOption(Property *property, OptionList &optionlist) override;

	//specific function 
	void AddToolItem();
	void AddToolSeparator();

protected:

	//events 
	void OnTool(wxCommandEvent &event);
	void OnRightDown(wxMouseEvent &event);
};

class CValueToolBarItem : public IControlElement
{
	wxDECLARE_DYNAMIC_CLASS(CValueToolBarItem);

private:

	wxString m_name;
	wxString m_label;
	wxBitmap m_bitmap;
	bool m_context_menu;
	wxString m_tooltip;

	bool m_enabled;

public:

	CValueToolBarItem();

	virtual void OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost) override;
	virtual void OnUpdated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost) override;
	virtual void Cleanup(wxObject* obj, IVisualHost *visualHost) override;

	virtual wxString GetClassName() const override { return wxT("tool"); }
	virtual wxString GetObjectTypeName() const override { return wxT("tool"); }

	virtual int GetComponentType() override { return COMPONENT_TYPE_ABSTRACT; }
	virtual bool IsItem() override { return false; }

	/**
	* Attach/detach
	*/
	virtual void Attach(void *pObj) override;
	virtual void Detach() override;

	/**
	* Support custom option
	*/
	virtual void PrepareOption(Property *property, OptionList &optionlist) override;

	//read&save propery 
	virtual void ReadProperty() override;
	virtual void SaveProperty() override;

	friend class CValueToolbar;
};

class CValueToolBarSeparator : public IControlElement
{
	wxDECLARE_DYNAMIC_CLASS(CValueToolBarSeparator);

private:

	wxString m_name;

public:

	CValueToolBarSeparator();
	virtual void OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost) override;
	virtual void OnUpdated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost) override;
	virtual void Cleanup(wxObject* obj, IVisualHost *visualHost) override;

	virtual wxString GetClassName() const override { return wxT("toolSeparator"); }
	virtual wxString GetObjectTypeName() const override { return wxT("tool"); }

	virtual int GetComponentType() override { return COMPONENT_TYPE_ABSTRACT; }
	virtual bool IsItem() override { return false; }

	/**
	* Attach/detach
	*/
	virtual void Attach(void *pObj) override;
	virtual void Detach() override;

	//read&save propery 
	virtual void ReadProperty() override;
	virtual void SaveProperty() override;

	friend class CValueToolbar;
};

#endif

