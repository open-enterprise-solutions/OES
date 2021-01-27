#ifndef _TABLE_H__
#define _TABLE_H__

#include "window.h"
#include "forms/dvc/dvc.h"

class CValueTableBoxColumn;

#include "common/tableInfo.h"

class CValueTableBox : public CValueWindow
{
	wxDECLARE_DYNAMIC_CLASS(CValueTableBox);

private:

	wxString m_name;
	int m_dataSource;

private:

	IValueTable *m_tableModel;
	IValueTable::IValueTableReturnLine *m_tableCurrentLine;

public:

	CValueTableBox();
	virtual ~CValueTableBox();

	//methods & attributes
	virtual void PrepareNames() const;                         //этот метод автоматически вызывается для инициализации имен атрибутов и методов

	virtual void SetAttribute(CAttributeParameters &aParams, CValue &cVal);        //установка атрибута
	virtual CValue GetAttribute(CAttributeParameters &aParams);                   //значение атрибута

	//control factory 
	virtual wxObject* Create(wxObject* parent, IVisualHost *visualHost) override;
	virtual void OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost) override;
	virtual void Update(wxObject* wxobject, IVisualHost *visualHost) override;
	virtual void OnUpdated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost) override;
	virtual void Cleanup(wxObject* obj, IVisualHost *visualHost) override;

	virtual wxString GetClassName() const override { return wxT("tablebox"); }
	virtual wxString GetObjectTypeName() const override { return wxT("container"); }

	//read&save propery 
	virtual void ReadProperty() override;
	virtual void SaveProperty() override;

	/**
	* Override actions
	*/

	virtual CAction GetActions(form_identifier_t formType);
	virtual void ExecuteAction(unsigned int action);

	/**
	* Support custom option
	*/
	virtual void PrepareOption(Property *property, OptionList &optionlist) override;

	/**
	* Support default menu
	*/
	virtual void PrepareDefaultMenu(wxMenu *m_menu);
	virtual void ExecuteMenu(IVisualHost *visualHost, int id);

	void AddNewColumn();
	void CreateColumns(wxDataViewCtrl *tableCtrl = NULL);

private:

	void AddValue();
	void CopyValue();
	void EditValue();
	void DeleteValue();

protected:

	//events 
	void OnColumnClick(wxDataViewEvent &event);
	void OnColumnReordered(wxDataViewEvent &event);

	void OnSelectionChanged(wxDataViewEvent &event);

	void OnItemActivated(wxDataViewEvent &event);
	void OnItemCollapsed(wxDataViewEvent &event);
	void OnItemExpanded(wxDataViewEvent &event);
	void OnItemCollapsing(wxDataViewEvent &event);
	void OnItemExpanding(wxDataViewEvent &event);
	void OnItemStartEditing(wxDataViewEvent &event);
	void OnItemEditingStarted(wxDataViewEvent &event);
	void OnItemEditingDone(wxDataViewEvent &event);
	void OnItemValueChanged(wxDataViewEvent &event);

#if wxUSE_DRAG_AND_DROP
	void OnItemBeginDrag(wxDataViewEvent &event);
	void OnItemDropPossible(wxDataViewEvent &event);
	void OnItemDrop(wxDataViewEvent &event);
#endif // wxUSE_DRAG_AND_DROP
};

class CValueTableBoxColumn : public IControlElement
{
	wxDECLARE_DYNAMIC_CLASS(CValueTableBoxColumn);

private:

	OptionList *GetAlign()
	{
		OptionList *m_optionlist = new OptionList();
		m_optionlist->AddOption("Left", wxALIGN_LEFT);
		m_optionlist->AddOption("Center", wxALIGN_CENTER);
		m_optionlist->AddOption("Right", wxALIGN_RIGHT);
		return m_optionlist;
	}

private:

	wxString m_name;
	wxString m_title;
	bool m_markup;
	int m_width;
	int m_align;
	wxBitmap m_icon;

	bool m_visible;
	bool m_resizable;
	bool m_sortable;
	bool m_reorderable;

	int m_type;

public:

	int m_model_id;

public:

	CValueTableBoxColumn();

	virtual wxObject* Create(wxObject* parent, IVisualHost *visualHost) override;
	virtual void OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost) override;
	virtual void OnUpdated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost) override;
	virtual void Cleanup(wxObject* obj, IVisualHost *visualHost) override;

	virtual wxString GetClassName() const override { return wxT("tablebox_column"); }
	virtual wxString GetObjectTypeName() const override { return wxT("tablebox_column"); }

	virtual int GetComponentType() override { return COMPONENT_TYPE_ABSTRACT; }
	virtual bool IsItem() override { return false; }

	//read&save propery 
	virtual void ReadProperty() override;
	virtual void SaveProperty() override;

	friend class CValueTableBox;
};

#endif 
