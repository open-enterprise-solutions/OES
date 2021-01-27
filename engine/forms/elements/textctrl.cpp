#include "widgets.h"
#include "frame.h"
#include "metadata/objects/baseObject.h"
#include "compiler/procUnit.h"

#include <wx/popupwin.h>

wxIMPLEMENT_DYNAMIC_CLASS(CValueTextCtrl, CValueWindow)

//****************************************************************************
//*                              Custom control                              *
//****************************************************************************

class CTextCtrl : public wxTextCtrl
{
	std::vector<CValue> m_aListData;

	class CTextCtrlPopupWindow : public wxPopupWindow
	{
		CTextCtrl *m_textOwner;
		wxListBox *m_listBox;

	public:

		CTextCtrlPopupWindow(CTextCtrl *parent) : wxPopupWindow(parent, wxBORDER_SIMPLE),
			m_listBox(new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxBORDER_NONE | wxLB_SINGLE)), m_textOwner(parent)
		{
			this->SetSizeHints(wxDefaultSize, wxDefaultSize);

			wxBoxSizer* bSizerListBox = new wxBoxSizer(wxVERTICAL);
			bSizerListBox->Add(m_listBox, 1, wxALL | wxEXPAND, 5);

			this->SetSizer(bSizerListBox);
			this->Layout();
			this->Centre(wxBOTH);

			m_listBox->SetForegroundColour(m_textOwner->GetForegroundColour());
			m_listBox->SetBackgroundColour(m_textOwner->GetBackgroundColour());
			m_listBox->SetFont(m_textOwner->GetFont());

			// Bind Events
			m_listBox->Bind(wxEVT_COMMAND_LISTBOX_SELECTED, &CTextCtrlPopupWindow::OnListBox, this);
		}
		~CTextCtrlPopupWindow() {}

		void AddString(wxString &item) { m_listBox->AppendString(item); }
		void ClearAll() { m_listBox->Clear(); }

	protected:

		// Virtual event handlers, overide them in your derived class
		virtual void OnListBox(wxCommandEvent& event)
		{
			wxCommandEvent redirectedEvent(event);
			redirectedEvent.SetEventObject(m_textOwner);

			if (!m_textOwner->GetEventHandler()->ProcessEvent(redirectedEvent))
			{
				event.Skip();
			}
		}
	};

	wxButton *m_buttonSelection;
	CTextCtrlPopupWindow *m_winPopup;

	IOwnerInfo *m_ownerInfo;

public:

	CTextCtrl() : wxTextCtrl(), m_winPopup(NULL), m_buttonSelection(NULL)
	{
	}

	CTextCtrl(wxWindow *parent, wxWindowID id,
		const wxString& value = wxEmptyString,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxValidator& validator = wxDefaultValidator,
		const wxString& name = wxTextCtrlNameStr) : wxTextCtrl(parent, id, value, pos, size, style, validator, name)
	{
		m_buttonSelection = new wxButton(this, wxID_ANY, wxT("..."), pos, size, wxBORDER_NONE);
		m_buttonSelection->Hide();

		m_winPopup = new CTextCtrlPopupWindow(this);

		this->Bind(wxEVT_SIZE, &CTextCtrl::OnSizeTextCtrl, this);

		this->Bind(wxEVT_KEY_DOWN, &CTextCtrl::OnKeyEvent, this);
		this->Bind(wxEVT_CHAR, &CTextCtrl::OnCharEvent, this);
		this->Bind(wxEVT_SET_FOCUS, &CTextCtrl::OnFocusEvent, this);
		this->Bind(wxEVT_KILL_FOCUS, &CTextCtrl::OnFocusEvent, this);

		m_buttonSelection->Bind(wxEVT_BUTTON, &CTextCtrl::OnButtonClicked, this);

		// Bind Events
		this->Bind(wxEVT_COMMAND_LISTBOX_SELECTED, &CTextCtrl::OnSelectedValue, this);
	}

	virtual ~CTextCtrl()
	{
		/*if (m_buttonSelection) {
			m_buttonSelection->Destroy();
		}*/

		/*if (m_winPopup)
		{
			m_winPopup->Destroy();
			m_winPopup = NULL;
		}*/
	}

	//buttons:
	void SetButtonSelect(bool select) { m_buttonSelection->Show(select); }
	bool HasButtonSelect() { return m_buttonSelection->IsShown(); }

	//popup:
	bool IsPopupShown() { return m_winPopup->IsShown(); }

	void ShowPopup(IOwnerInfo *owner = NULL)
	{
		if (!m_aListData.size()) { if (IsPopupShown()) HidePopup(); return; }

		m_ownerInfo = owner;

		wxSize ctrlSz = GetSize();

		int screenHeight = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y);
		wxPoint scrPos = GetScreenPosition();

		// Space above and below
		int spaceAbove = scrPos.y;
		int spaceBelow = screenHeight - spaceAbove - ctrlSz.y;

		int maxHeightPopup = spaceBelow;

		if (spaceAbove > spaceBelow) maxHeightPopup = spaceAbove;

		// Width
		int widthPopup = ctrlSz.x;

		if (widthPopup < -1) widthPopup = -1;

		wxSize adjustedSize = wxSize(widthPopup, 250);

		m_winPopup->SetSize(adjustedSize);
		m_winPopup->Move(0, 0);

		//
		// Reposition and resize popup window
		//

		wxSize szp = m_winPopup->GetSize();

		int popupX;
		int popupY = scrPos.y + ctrlSz.y;

		// Default anchor is wxLEFT
		int anchorSide = wxLEFT;

		int rightX = scrPos.x + ctrlSz.x - szp.x;
		int leftX = scrPos.x;

		if (wxTheApp->GetLayoutDirection() == wxLayout_RightToLeft)
			leftX -= ctrlSz.x;

		int screenWidth = wxSystemSettings::GetMetric(wxSYS_SCREEN_X);

		// If there is not enough horizontal space, anchor on the other side.
		// If there is no space even then, place the popup at x 0.
		if (anchorSide == wxRIGHT)
		{
			if (rightX < 0)
			{
				if ((leftX + szp.x) < screenWidth)
					anchorSide = wxLEFT;
				else
					anchorSide = 0;
			}
		}
		else
		{
			if ((leftX + szp.x) >= screenWidth)
			{
				if (rightX >= 0)
					anchorSide = wxRIGHT;
				else
					anchorSide = 0;
			}
		}

		// Select x coordinate according to the anchor side
		if (anchorSide == wxRIGHT)
			popupX = rightX;
		else if (anchorSide == wxLEFT)
			popupX = leftX;
		else
			popupX = 0;

		if (spaceBelow < szp.y)
		{
			popupY = scrPos.y - szp.y;
		}

		m_winPopup->SetBackgroundColour(GetBackgroundColour());

		wxRect popupWinRect(popupX - 2, popupY, szp.x, szp.y);
		m_winPopup->SetSize(popupWinRect);
		m_winPopup->ShowWithEffect(wxSHOW_EFFECT_ROLL_TO_BOTTOM);

		m_winPopup->Refresh();
	}

	void HidePopup()
	{
		if (m_winPopup->Hide()) m_aListData.clear();
	}

	void LoadList(std::vector<CValue> aList)
	{
		m_winPopup->ClearAll();

		for (auto list : aList)
		{
			m_winPopup->AddString(list.GetString());
		}

		m_aListData = aList;
	}

protected:

	void OnSizeTextCtrl(wxSizeEvent& event)
	{
		wxSize size = event.GetSize();

		// Use one two units smaller to match size of the combo's dropbutton.
		// (normally a bigger button is used because it looks better)
		int bt_wid = size.y;
		bt_wid -= 4;
		wxSize bt_sz(bt_wid, bt_wid);

		// Position of button.
		wxPoint bt_pos(size.x - bt_sz.x - 4, -1);
#ifdef __WXMAC__
		bt_pos.y -= 1;
#else
		bt_pos.y += 1;
#endif
		if (IsPopupShown()) HidePopup();

		if (HasButtonSelect())
		{
			m_buttonSelection->SetSize(bt_sz);
			m_buttonSelection->SetPosition(bt_pos);
		}

		event.Skip();
	}
	void OnButtonClicked(wxCommandEvent& event)
	{
		wxCommandEvent redirectedEvent(event);
		redirectedEvent.SetEventObject(this);

		if (!GetEventHandler()->ProcessEvent(redirectedEvent))
		{
			event.Skip();
		}
	}
	void OnKeyEvent(wxKeyEvent& event)
	{
		if (IsPopupShown())
		{
			// pass it to the popped up control
			m_winPopup->GetEventHandler()->ProcessEvent(event);
		}
		else
		{
			event.Skip();
		}
	}
	void OnCharEvent(wxKeyEvent& event)
	{
		if (IsPopupShown())
		{
			// pass it to the popped up control
			m_winPopup->GetEventHandler()->ProcessEvent(event);
		}
		else
		{
			event.Skip();
		}
	}
	void OnFocusEvent(wxFocusEvent& event)
	{
		if (event.GetEventType() == wxEVT_KILL_FOCUS) HidePopup();
		Refresh();
	}

	void OnSelectedValue(wxCommandEvent& event)
	{
		std::vector<CValue>::iterator itSelected = m_aListData.begin();
		std::advance(itSelected, event.GetSelection());

		if (m_ownerInfo)
		{
			m_ownerInfo->SendOwnerValue(*itSelected);
			HidePopup();
		}

		event.Skip();
	}
};

//****************************************************************************
//*                              TextCtrl                                    *
//****************************************************************************

CValueTextCtrl::CValueTextCtrl() : CValueWindow(),
m_selbutton(true)
{
	PropertyCategory *m_categoryText = new PropertyCategory("TextControl");

	//property
	m_categoryText->AddProperty("name");
	m_categoryText->AddProperty("maxlength");
	m_categoryText->AddProperty("value");

	//category 
	m_category->AddCategory(m_categoryText);

	PropertyCategory *m_categoryButton = new PropertyCategory("Button");
	m_categoryButton->AddProperty("button_select");

	//category 
	m_category->AddCategory(m_categoryButton);

	//properties 
	m_properties["name"] = new Property("name", PropertyType::PT_WXNAME, this);
	m_properties["maxlength"] = new Property("maxlength", PropertyType::PT_INT, this);
	m_properties["value"] = new Property("value", PropertyType::PT_WXSTRING, this);
	m_properties["button_select"] = new Property("button_select", PropertyType::PT_BOOL, this);

	//source from object 
	PropertyCategory *m_propery_data = new PropertyCategory("Data");
	m_propery_data->AddProperty("source");
	m_propery_data->AddProperty("type");
	m_category->AddCategory(m_propery_data);

	m_properties["source"] = new Property("source", PropertyType::PT_SOURCE, this);
	m_properties["type"] = new Property("type", PropertyType::PT_TYPE_SELECT, this);
}

wxObject* CValueTextCtrl::Create(wxObject* parent, IVisualHost *visualHost)
{
	CTextCtrl *m_textctrl = new CTextCtrl((wxWindow *)parent, wxID_ANY,
		m_value,
		m_pos,
		m_size,
		m_style | m_window_style);

	return m_textctrl;
}

void CValueTextCtrl::OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost)
{
}

#include "metadata/metadata.h"

void CValueTextCtrl::Update(wxObject* wxobject, IVisualHost *visualHost)
{
	CTextCtrl *m_textctrl = dynamic_cast<CTextCtrl *>(wxobject);

	if (m_textctrl)
	{
		m_textctrl->SetValue(m_value);

		if (!IsNull(_("maxlength")))
		{
			m_textctrl->SetMaxLength(m_maxlength);
		}

		if (!IsNull(_("source")))
		{
			IDataObjectValue *sourceObject = dynamic_cast<IDataObjectValue *>(m_frameOwner->GetSourceObject());
			
			if (sourceObject) {
				//TODO:
				CValue sourcevalue = sourceObject->GetValueByMetaID(m_properties["source"]->GetValueAsInteger());
				m_textctrl->SetValue(sourcevalue.GetString());
			}
		}

		m_textctrl->SetButtonSelect(m_selbutton);

		if (m_textctrl->HasButtonSelect())
		{
			m_textctrl->Bind(wxEVT_BUTTON, &CValueTextCtrl::OnSelectButtonPressed, this);
		}

		m_textctrl->Bind(wxEVT_TEXT, &CValueTextCtrl::OnTextEnter, this);
	}

	UpdateWindow(m_textctrl);
}

void CValueTextCtrl::Cleanup(wxObject* obj, IVisualHost *visualHost)
{
}

//*******************************************************************
//*                            Property                             *
//*******************************************************************

void CValueTextCtrl::ReadProperty()
{
	CValueWindow::ReadProperty();

	m_properties["name"]->SetValue(m_name);
	m_properties["maxlength"]->SetValue(m_maxlength);
	m_properties["value"]->SetValue(m_value);
	m_properties["button_select"]->SetValue(m_selbutton);

	m_properties["source"]->SetValue(m_source);
	m_properties["type"]->SetValue(m_type);
}

void CValueTextCtrl::SaveProperty()
{
	CValueWindow::SaveProperty();

	m_name = m_properties["name"]->GetValueAsString();
	m_maxlength = m_properties["maxlength"]->GetValueAsInteger();
	m_value = m_properties["value"]->GetValueAsString();
	m_selbutton = m_properties["button_select"]->GetValueAsInteger();

	m_source = m_properties["source"]->GetValueAsInteger();
	m_type = m_properties["type"]->GetValueAsInteger();
}

#include "appData.h"
#include "metadata/metadata.h"

void CValueTextCtrl::OnSelectButtonPressed(wxCommandEvent &event)
{
	CTextCtrl *textCtrl = dynamic_cast<CTextCtrl *>(GetWxObject());
	IMetaObjectRefValue *metaRefObject = IMetaObjectRefValue::FindMetaObjectValue(m_type);

	if (metaRefObject)
	{
		if (appData->IsEnterpriseMode())
		{
			//TODO
			if (false) {
				textCtrl->LoadList(metaRefObject->GetListData());
				textCtrl->ShowPopup(this);
			}
			else {
				CValueFrame *valueFrame = metaRefObject->GetSelectForm(wxEmptyString, this);
				valueFrame->ShowForm();
			}
		}
	}
}