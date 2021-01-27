#ifndef _SIZER_H_
#define _SIZER_H_

#include "baseControl.h"

class CValueSizer : public IControlElement
{
	wxDECLARE_ABSTRACT_CLASS(CValueSizer);

protected:

	OptionList *GetOrient()
	{
		OptionList *m_optionlist = new OptionList();
		m_optionlist->AddOption("Vertical", wxVERTICAL);
		m_optionlist->AddOption("Horizontal", wxHORIZONTAL);
		return m_optionlist;
	}

public:

	wxSize m_minimum_size;

public:

	CValueSizer() : m_minimum_size(wxDefaultSize)
	{
		m_category = new PropertyCategory("Sizer");
		m_category->AddProperty("minimum_size");

		m_properties["minimum_size"] = new Property("minimum_size", PropertyType::PT_WXSIZE, this);
	}

	virtual int GetComponentType() override { return COMPONENT_TYPE_SIZER; }
	virtual bool IsItem() override { return false; }

	virtual void ReadProperty() override;
	virtual void SaveProperty() override;

protected:

	void UpdateSizer(wxSizer* sizer);
};

//////////////////////////////////////////////////////////////////////////////

class CValueSizerItem : public CValueSizer
{
	wxDECLARE_DYNAMIC_CLASS(CValueSizerItem);

private:

	OptionList *GetDefaultOptionBorder();
	OptionList *GetDefaultOptionState();

public:

	int m_proportion;
	long m_flag_border;
	long m_flag_state;
	int m_border;

public:

	CValueSizerItem();

	virtual void OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost) override;
	virtual void OnUpdated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost) override;

	virtual wxString GetClassName() const override { return wxT("sizerItem"); }
	virtual wxString GetObjectTypeName() const override { return wxT("sizer"); }

	virtual int GetComponentType() override { return COMPONENT_TYPE_SIZERITEM; }
	virtual bool IsItem() override { return true; }

	virtual void ReadProperty() override;
	virtual void SaveProperty() override;

	friend class CVisualEditorDatabase;
};

//////////////////////////////////////////////////////////////////////////////

class CValueBoxSizer : public CValueSizer
{
	wxDECLARE_DYNAMIC_CLASS(CValueBoxSizer);

public:

	wxString m_name;
	wxOrientation m_orient;
	long m_flags;

public:

	CValueBoxSizer();

	virtual wxObject* Create(wxObject* /*parent*/, IVisualHost* /*visualHost*/) override;
	virtual void OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost) override;
	virtual void Update(wxObject* wxobject, IVisualHost *visualHost) override;
	virtual void Cleanup(wxObject* obj, IVisualHost *visualHost) override;

	virtual wxString GetClassName() const override { return wxT("boxsizer"); }
	virtual wxString GetObjectTypeName() const override { return wxT("sizer"); }

	virtual void ReadProperty() override;
	virtual void SaveProperty() override;

	/**
	* Set property data
	*/
	virtual void SetPropertyData(Property *property, const CValue &srcValue);

	/**
	* Get property data
	*/
	virtual CValue GetPropertyData(Property *property);
};

#include <wx/wrapsizer.h>

class CValueWrapSizer : public CValueSizer
{
	wxDECLARE_DYNAMIC_CLASS(CValueWrapSizer);

protected:

	wxString m_name = "wrapSizer";
	wxOrientation m_orient = wxHORIZONTAL;

public:

	CValueWrapSizer();

	virtual wxObject* Create(wxObject* /*parent*/, IVisualHost* /*visualHost*/) override;
	virtual void OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost) override;
	virtual void Update(wxObject* wxobject, IVisualHost *visualHost) override;
	virtual void Cleanup(wxObject* obj, IVisualHost *visualHost) override;

	virtual wxString GetClassName() const override { return wxT("wrapsizer"); }
	virtual wxString GetObjectTypeName() const override { return wxT("sizer"); }

	virtual void ReadProperty() override;
	virtual void SaveProperty() override;

	/**
	* Set property data
	*/
	virtual void SetPropertyData(Property *property, const CValue &srcValue);

	/**
	* Get property data
	*/
	virtual CValue GetPropertyData(Property *property);
};

class CValueStaticBoxSizer : public CValueSizer
{
	wxDECLARE_DYNAMIC_CLASS(CValueStaticBoxSizer);

protected:

	wxString m_name = "staticboxSizer";
	wxOrientation m_orient = wxHORIZONTAL;

	wxString m_label;
	wxString m_id;
	bool m_parent;

	wxFont m_font;
	wxColour m_fg = RGB(0, 120, 215);
	wxColour m_bg = RGB(240, 240, 240);

	wxString m_tooltip;
	bool m_context_menu;
	wxString m_context_help;
	bool m_enabled = true;
	bool m_visible = true;

public:

	CValueStaticBoxSizer();

	virtual wxObject* Create(wxObject* parent, IVisualHost *visualHost) override;
	virtual void OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost) override;
	virtual void Update(wxObject* wxobject, IVisualHost *visualHost) override;
	virtual void Cleanup(wxObject* obj, IVisualHost *visualHost) override;

	virtual wxString GetClassName() const override { return wxT("staticboxsizer"); }
	virtual wxString GetObjectTypeName() const override { return wxT("sizer"); }

	virtual void ReadProperty() override;
	virtual void SaveProperty() override;

	/**
	* Set property data
	*/
	virtual void SetPropertyData(Property *property, const CValue &srcValue);

	/**
	* Get property data
	*/
	virtual CValue GetPropertyData(Property *property);
};

class CValueGridSizer : public CValueSizer
{
	wxDECLARE_DYNAMIC_CLASS(CValueGridSizer);

protected:

	wxString m_name = wxT("gridSizer");

	int m_row = 0;
	int m_cols = 2;

public:

	CValueGridSizer();

	virtual wxObject* Create(wxObject* /*parent*/, IVisualHost* /*visualHost*/) override;
	virtual void OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost) override;
	virtual void Update(wxObject* wxobject, IVisualHost *visualHost) override;
	virtual void Cleanup(wxObject* obj, IVisualHost *visualHost) override;

	virtual wxString GetClassName() const override { return wxT("gridsizer"); }
	virtual wxString GetObjectTypeName() const override { return wxT("sizer"); }

	virtual void ReadProperty() override;
	virtual void SaveProperty() override;
};

#endif 