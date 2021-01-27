#ifndef _DVC_H__
#define _DVC_H__

#include <wx/dataview.h>
#include <wx/datectrl.h>
#include <wx/renderer.h>

// ----------------------------------------------------------------------------
// CValueViewRenderer
// ----------------------------------------------------------------------------

enum eRendererType
{
	enToggle,
	enRadio,
	enDate,
	enText
};

class CValueViewRenderer : public wxDataViewCustomRenderer
{
	eRendererType m_rendererType;

public:
	// This renderer can be either activatable or editable, for demonstration
	// purposes. In real programs, you should select whether the user should be
	// able to activate or edit the cell and it doesn't make sense to switch
	// between the two -- but this is just an example, so it doesn't stop us.
	explicit CValueViewRenderer(eRendererType rendrerType = eRendererType::enText)
		: wxDataViewCustomRenderer(wxT("string"), wxDATAVIEW_CELL_EDITABLE, wxALIGN_LEFT), m_rendererType(rendrerType)
	{
	}

	virtual bool Render(wxRect rect, wxDC *dc, int state) override
	{
		int flags = 0;

		if (m_rendererType == eRendererType::enToggle
			&& m_valueVariant.GetBool())
			flags |= wxCONTROL_CHECKED;

		/*if (GetMode() != wxDATAVIEW_CELL_ACTIVATABLE ||
			!(GetOwner()->GetOwner()->IsEnabled() && GetEnabled()))
			flags |= wxCONTROL_DISABLED;*/

		wxRendererNative& renderer = wxRendererNative::Get();
		wxWindow* const win = GetOwner()->GetOwner();

		if (m_rendererType == eRendererType::enToggle)
		{
			// Ensure that the check boxes always have at least the minimal required
			// size, otherwise DrawCheckBox() doesn't really work well. If this size is
			// greater than the cell size, the checkbox will be truncated but this is a
			// lesser evil.
			//wxSize size = rect.GetSize();
			//size.IncTo(GetSize());
			//rect.SetSize(size);

			renderer.DrawCheckBox(win, *dc, rect, flags);
		}
		else if (m_rendererType == eRendererType::enRadio)
		{
			// Ensure that the check boxes always have at least the minimal required
			// size, otherwise DrawCheckBox() doesn't really work well. If this size is
			// greater than the cell size, the checkbox will be truncated but this is a
			// lesser evil.
			//wxSize size = rect.GetSize();
			//size.IncTo(GetSize());
			//rect.SetSize(size);

			renderer.DrawRadioBitmap(win, *dc, rect, flags);
		}
		else
		{
			RenderText(m_valueVariant,
				0, // no offset
				rect,
				dc,
				state);
		}

		return true;
	}

	virtual bool ActivateCell(const wxRect& cell,
		wxDataViewModel *model,
		const wxDataViewItem & item,
		unsigned int col,
		const wxMouseEvent *mouseEvent) override
	{
		if (m_rendererType == eRendererType::enToggle
			|| m_rendererType == eRendererType::enRadio)
		{
			if (mouseEvent)
			{
				// Only react to clicks directly on the checkbox, not elsewhere in the
				// same cell.
				if (!wxRect(GetSize()).Contains(mouseEvent->GetPosition()))
					return false;
			}

			model->ChangeValue(!m_valueVariant.GetBool(), item, col);
			return true;
		}
		else
		{
			return false;
		}
	}

	virtual wxSize GetSize() const override
	{
		if (m_rendererType == eRendererType::enToggle
			|| m_rendererType == eRendererType::enRadio)
		{
			return wxRendererNative::Get().GetCheckBoxSize(GetView());
		}
		else
		{
			if (!m_valueVariant.IsNull())
			{
				return GetTextExtent(m_valueVariant);
			}
			else
			{
				return GetView()->FromDIP(wxSize(wxDVC_DEFAULT_RENDERER_SIZE,
					wxDVC_DEFAULT_RENDERER_SIZE));
			}
		}
	}

	virtual bool SetValue(const wxVariant &value) override
	{
		if (m_rendererType == eRendererType::enToggle
			|| m_rendererType == eRendererType::enRadio)
		{
			m_valueVariant = value.GetBool();
		}
		else
		{
			m_valueVariant = value.GetString();
		}

		return true;
	}

	virtual bool GetValue(wxVariant &WXUNUSED(value)) const override
	{
		return true;
	}

#if wxUSE_ACCESSIBILITY
	virtual wxString GetAccessibleDescription() const override
	{
		if (m_rendererType == eRendererType::enToggle
			|| m_rendererType == eRendererType::enRadio)
		{
			/* TRANSLATORS: Checkbox state name */
			return m_valueVariant.GetBool() ? _("checked")
				/* TRANSLATORS: Checkbox state name */
				: _("unchecked");
		}
		else
		{
			return m_valueVariant;
		}
	}
#endif // wxUSE_ACCESSIBILITY

	virtual bool HasEditorCtrl() const override { return true; }

	virtual wxWindow* CreateEditorCtrl(wxWindow* parent,
		wxRect labelRect,
		const wxVariant& value) override
	{
		wxWindow *m_wndCtrl = NULL;

		switch (m_rendererType)
		{
		case enToggle:
		{
			wxCheckBox* m_checkBox = new wxCheckBox(parent, wxID_ANY, wxEmptyString,
				labelRect.GetPosition(),
				labelRect.GetSize());
			m_wndCtrl = m_checkBox;
			break;
		}
		case enRadio:
		{
			wxRadioButton* m_checkBox = new wxRadioButton(parent, wxID_ANY, wxEmptyString,
				labelRect.GetPosition(),
				labelRect.GetSize());
			m_wndCtrl = m_checkBox;
			break;
		}
		case enDate:
		{
			wxDatePickerCtrl* m_dateCtrl = new wxDatePickerCtrl(parent, wxID_ANY, value,
				labelRect.GetPosition(),
				labelRect.GetSize(),
				wxTE_PROCESS_ENTER);
			m_wndCtrl = m_dateCtrl;
			break;
		}
		case enText:
		{
			wxTextCtrl* m_textCtrl = new wxTextCtrl(parent, wxID_ANY, value,
				labelRect.GetPosition(),
				labelRect.GetSize(),
				wxTE_PROCESS_ENTER);

			m_textCtrl->SetInsertionPointEnd();
			m_wndCtrl = m_textCtrl;
			break;
		}
		}

		return m_wndCtrl;
	}

	virtual bool GetValueFromEditorCtrl(wxWindow* ctrl, wxVariant& value) override
	{
		if (m_rendererType == eRendererType::enToggle)
		{
			wxCheckBox* checkCtrl = wxDynamicCast(ctrl, wxCheckBox);
			if (!checkCtrl)
				return false;
			value = checkCtrl->GetValue();
			return true;
		}
		else if (m_rendererType == eRendererType::enRadio)
		{
			wxRadioButton* radioCtrl = wxDynamicCast(ctrl, wxRadioButton);
			if (!radioCtrl)
				return false;
			value = radioCtrl->GetValue();
			return true;
		}
		else if (m_rendererType == eRendererType::enDate)
		{
			wxDatePickerCtrl* dateCtrl = wxDynamicCast(ctrl, wxDatePickerCtrl);
			if (!dateCtrl)
				return false;
			value = dateCtrl->GetValue();
			return true;
		}
		else if (m_rendererType == eRendererType::enText)
		{
			wxTextCtrl* textCtrl = wxDynamicCast(ctrl, wxTextCtrl);
			if (!textCtrl)
				return false;
			value = textCtrl->GetValue();
			return true;
		}
		return false;
	}

	void SetRendererType(eRendererType type) { m_valueVariant.Clear(); m_rendererType = type; }
	eRendererType GetRendererType() { return m_rendererType; }

private:

	wxVariant m_valueVariant;
};

// ----------------------------------------------------------------------------
// CDataViewColumnObject
// ----------------------------------------------------------------------------

class CDataViewColumnObject : public wxObject,
	public wxDataViewColumn
{
	unsigned int m_obj_id;

public:

	CDataViewColumnObject(const wxString& title,
		unsigned int model_column,
		int width = wxDVC_DEFAULT_WIDTH,
		wxAlignment align = wxALIGN_CENTER,
		int flags = wxDATAVIEW_COL_RESIZABLE)
		: wxDataViewColumn(title, new CValueViewRenderer, model_column, width, align, flags), m_obj_id(0)
	{
	}

	CDataViewColumnObject(const wxBitmap& bitmap,
		unsigned int model_column,
		int width = wxDVC_DEFAULT_WIDTH,
		wxAlignment align = wxALIGN_CENTER,
		int flags = wxDATAVIEW_COL_RESIZABLE)
		: wxDataViewColumn(bitmap, new CValueViewRenderer, model_column, width, align, flags), m_obj_id(0)
	{
	}

	CValueViewRenderer* GetRenderer() const { return dynamic_cast<CValueViewRenderer *>(m_renderer); }

	void SetControlID(unsigned int obj_id) { wxASSERT(m_obj_id == 0); m_obj_id = obj_id; }
	unsigned int GetControlID() { return m_obj_id; }

	void SetColModel(unsigned int col_model) { 
		m_model_column = col_model; 
	}
};

#endif // !_DVC_H__
