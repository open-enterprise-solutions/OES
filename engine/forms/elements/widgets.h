#ifndef _COMMON_H_
#define _COMMON_H_

#include "window.h"

/////////////////////////////////////////////////////////////////////////////////////
//                                 COMMON ELEMENTS                                 //
/////////////////////////////////////////////////////////////////////////////////////
#include <wx/button.h>

class CValueButton : public CValueWindow
{
	wxDECLARE_DYNAMIC_CLASS(CValueButton);

protected:

	wxString m_name = "button";
	long m_style = 0;
	wxString m_label = "MyButton";
	bool m_markup = false;
	bool m_default = false;

	wxSize m_position;
	wxSize m_margins;

	//private:
	//	wxButton *m_button;

public:

	CValueButton();

	virtual wxObject* Create(wxObject* parent, IVisualHost *visualHost) override;
	virtual void OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost)override;
	virtual void Update(wxObject* wxobject, IVisualHost *visualHost) override;
	virtual void Cleanup(wxObject* obj, IVisualHost *visualHost) override;

	virtual wxString GetClassName() const override { return wxT("button"); }
	virtual wxString GetObjectTypeName() const override { return wxT("widget"); }

	virtual void ReadProperty() override;
	virtual void SaveProperty() override;

protected:
	//events 
	void OnButtonPressed(wxCommandEvent &event);
};

#include <wx/stattext.h>

class CValueStaticText : public CValueWindow
{
	wxDECLARE_DYNAMIC_CLASS(CValueStaticText);
protected:

	wxString m_name = "staticText";
	long m_style = 0;
	wxString m_label = "MyLabel";
	bool m_markup = false;
	int m_wrap = 0;

	//private:
	//	wxStaticText *m_static_text;

public:

	CValueStaticText();

	virtual wxObject* Create(wxObject* parent, IVisualHost *visualHost) override;
	virtual void OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost)override;
	virtual void Update(wxObject* wxobject, IVisualHost *visualHost) override;
	virtual void Cleanup(wxObject* obj, IVisualHost *visualHost) override;

	virtual wxString GetClassName() const override { return wxT("statictext"); }
	virtual wxString GetObjectTypeName() const override { return wxT("widget"); }

	virtual void ReadProperty() override;
	virtual void SaveProperty() override;
};

#include <wx/textctrl.h>

class CValueTextCtrl : public CValueWindow
{
	wxDECLARE_DYNAMIC_CLASS(CValueTextCtrl);

protected:

	wxString m_name = "textCtrl";
	long m_style = 0;
	wxString m_value = "";
	int m_maxlength = 0;

	bool m_selbutton;

	int m_source;
	int m_type;

public:

	CValueTextCtrl();

	virtual wxObject* Create(wxObject* parent, IVisualHost *visualHost) override;
	virtual void OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost) override;
	virtual void Update(wxObject* wxobject, IVisualHost *visualHost) override;
	virtual void Cleanup(wxObject* obj, IVisualHost *visualHost) override;

	virtual wxString GetClassName() const override { return wxT("textctrl"); }
	virtual wxString GetObjectTypeName() const override { return wxT("widget"); }

	//support owner 
	virtual void SendOwnerValue(CValue &Value) override;

	virtual void ReadProperty() override;
	virtual void SaveProperty() override;

protected:

	//Events:
	void OnTextEnter(wxCommandEvent &event);
	void OnSelectButtonPressed(wxCommandEvent &event);
};

#include <wx/combobox.h>

class CValueComboBox : public CValueWindow
{
	wxDECLARE_DYNAMIC_CLASS(CValueComboBox);
protected:

	wxString m_name = "combobox";
	long m_style = 0;
	wxString m_value = "Combo!";
	wxArrayString m_choices;
	int m_selection;

public:

	CValueComboBox();

	virtual wxObject* Create(wxObject* parent, IVisualHost *visualHost) override;
	virtual void OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost) override;
	virtual void Update(wxObject* wxobject, IVisualHost *visualHost) override;
	virtual void Cleanup(wxObject* obj, IVisualHost *visualHost) override;

	virtual wxString GetClassName() const override { return wxT("combobox"); }
	virtual wxString GetObjectTypeName() const override { return wxT("widget"); }

	virtual void ReadProperty() override;
	virtual void SaveProperty() override;
};

#include <wx/choice.h>

class CValueChoice : public CValueWindow
{
	wxDECLARE_DYNAMIC_CLASS(CValueChoice);

protected:

	wxString m_name = "choice";
	long m_style = 0;
	wxArrayString m_choices;
	int m_selection;

public:

	CValueChoice();

	virtual wxObject* Create(wxObject* parent, IVisualHost *visualHost) override;
	virtual void OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost) override;
	virtual void Update(wxObject* wxobject, IVisualHost *visualHost) override;
	virtual void Cleanup(wxObject* obj, IVisualHost *visualHost) override;

	virtual wxString GetClassName() const override { return wxT("choice"); }
	virtual wxString GetObjectTypeName() const override { return wxT("widget"); }

	virtual void ReadProperty() override;
	virtual void SaveProperty() override;
};

#include <wx/listbox.h>

class CValueListBox : public CValueWindow
{
	wxDECLARE_DYNAMIC_CLASS(CValueListBox);

protected:

	wxString m_name = "listbox";
	long m_style = 0;
	wxArrayString m_choices;

public:

	CValueListBox();

	virtual wxObject* Create(wxObject* parent, IVisualHost *visualHost) override;
	virtual void OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost) override;
	virtual void Update(wxObject* wxobject, IVisualHost *visualHost) override;
	virtual void Cleanup(wxObject* obj, IVisualHost *visualHost) override;

	virtual wxString GetClassName() const override { return wxT("listbox"); }
	virtual wxString GetObjectTypeName() const override { return wxT("widget"); }

	virtual void ReadProperty() override;
	virtual void SaveProperty() override;
};

#include <wx/checkbox.h>

class CValueCheckbox : public CValueWindow
{
	wxDECLARE_DYNAMIC_CLASS(CValueCheckbox);

protected:

	wxString m_name = "checkBox";
	long m_style = 0;
	wxString m_label = "Check Me!";
	bool m_checked = false;

public:

	CValueCheckbox();

	virtual wxObject* Create(wxObject* parent, IVisualHost *visualHost) override;
	virtual void OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost) override;
	virtual void Update(wxObject* wxobject, IVisualHost *visualHost) override;
	virtual void Cleanup(wxObject* obj, IVisualHost *visualHost) override;

	virtual wxString GetClassName() const override { return wxT("checkbox"); }
	virtual wxString GetObjectTypeName() const override { return wxT("widget"); }

	virtual void ReadProperty() override;
	virtual void SaveProperty() override;

protected:

	//events 
	void OnClickedCheckbox(wxCommandEvent& event);
};

#include <wx/radiobut.h>

class CValueRadioButton : public CValueWindow
{
	wxDECLARE_DYNAMIC_CLASS(CValueRadioButton);
protected:

	wxString m_name = "radioBtn";
	long m_style = 0;
	wxString m_label = "RadioBtn";
	bool m_selected;

public:

	CValueRadioButton();

	virtual wxObject* Create(wxObject* parent, IVisualHost *visualHost) override;
	virtual void OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost) override;
	virtual void Update(wxObject* wxobject, IVisualHost *visualHost) override;
	virtual void Cleanup(wxObject* obj, IVisualHost *visualHost) override;

	virtual wxString GetClassName() const override { return wxT("radiobutton"); }
	virtual wxString GetObjectTypeName() const override { return wxT("widget"); }

protected:

	virtual void ReadProperty() override;
	virtual void SaveProperty() override;
};

#include <wx/statline.h>

class CValueStaticLine : public CValueWindow
{
	wxDECLARE_DYNAMIC_CLASS(CValueStaticLine);
protected:

	wxString m_name = "staticline";
	long m_style = wxLI_HORIZONTAL;

public:

	CValueStaticLine();

	virtual wxObject* Create(wxObject* parent, IVisualHost *visualHost) override;
	virtual void OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost) override;
	virtual void Update(wxObject* wxobject, IVisualHost *visualHost) override;
	virtual void Cleanup(wxObject* obj, IVisualHost *visualHost) override;

	virtual wxString GetClassName() const override { return wxT("staticline"); }
	virtual wxString GetObjectTypeName() const override { return wxT("widget"); }

	virtual void ReadProperty() override;
	virtual void SaveProperty() override;
};

#include <wx/slider.h>

class CValueSlider : public CValueWindow
{
	wxDECLARE_DYNAMIC_CLASS(CValueSlider);
protected:

	wxString m_name = "slider";
	long m_style = wxSL_HORIZONTAL;

	int m_value = 50;
	int m_minValue = 0;
	int m_maxValue = 100;

public:

	CValueSlider();

	virtual wxObject* Create(wxObject* parent, IVisualHost *visualHost) override;
	virtual void OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost) override;
	virtual void Update(wxObject* wxobject, IVisualHost *visualHost) override;
	virtual void Cleanup(wxObject* obj, IVisualHost *visualHost) override;

	virtual wxString GetClassName() const override { return wxT("slider"); }
	virtual wxString GetObjectTypeName() const override { return wxT("widget"); }

	virtual void ReadProperty() override;
	virtual void SaveProperty() override;
};

#include <wx/gauge.h>

class CValueGauge : public CValueWindow
{
	wxDECLARE_DYNAMIC_CLASS(CValueGauge);
protected:

	wxString m_name = "gauge";
	long m_style = wxGA_HORIZONTAL;

	int m_range;
	int m_value;

public:

	CValueGauge();

	virtual wxObject* Create(wxObject* parent, IVisualHost *visualHost) override;
	virtual void OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost) override;
	virtual void Update(wxObject* wxobject, IVisualHost *visualHost) override;
	virtual void Cleanup(wxObject* obj, IVisualHost *visualHost) override;

	virtual wxString GetClassName() const override { return wxT("gauge"); }
	virtual wxString GetObjectTypeName() const override { return wxT("widget"); }

	virtual void ReadProperty() override;
	virtual void SaveProperty() override;
};

#endif