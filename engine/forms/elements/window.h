#ifndef _WINDOW_BASE_H_
#define _WINDOW_BASE_H_

#include "baseControl.h"

#define FORM_ACTION 1

class CValueWindow : public IControlElement
{
	wxDECLARE_ABSTRACT_CLASS(CValueWindow);

public:

	wxPoint m_pos = wxDefaultPosition;
	wxSize m_size = wxDefaultSize;
	wxSize m_minimum_size = wxDefaultSize;
	wxSize m_maximum_size = wxDefaultSize;
	wxFont m_font;

	wxColour m_fg = RGB(0, 120, 215);
	wxColour m_bg = RGB(240, 240, 240);

	long m_window_style = 0;
	long m_window_extra_style = 0;
	long m_center;

	wxString m_tooltip;
	bool m_context_menu;
	wxString m_context_help;
	bool m_enabled = true;
	bool m_visible = true;

public:

	CValueWindow();

	//attributes 
	virtual void SetAttribute(CAttributeParameters &aParams, CValue &cVal) override; //установка атрибута

	//property
	virtual void ReadProperty() override;
	virtual void SaveProperty() override;

	virtual int GetComponentType() override { return COMPONENT_TYPE_WINDOW; }
	virtual bool IsItem() override { return false; }

protected:

	void UpdateWindow(wxWindow* window);
};

#endif 