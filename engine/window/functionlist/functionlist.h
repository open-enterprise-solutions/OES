#if !defined(_FUNCTIONLIST_H__)
#define _FUNCTIONLIST_H__

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/dialog.h>
#include <wx/listctrl.h>

class CDocument; 
class CAutocomplectionCtrl;

typedef struct
{
	wxListCtrl *m_listCtrl;
	bool m_sortOrder;
} sortInfo_t;

struct CFunctionList : public wxDialog
{
	wxArrayString aListName;
	wxArrayInt aListImage;

	wxCheckBox *m_Sort;
	wxButton *m_Cancel;
	wxButton *m_OK;

	wxListCtrl *m_listProcedures;

	CDocument *m_docModule;
	CAutocomplectionCtrl *m_codeEditor;

private:

	sortInfo_t sortInfo; 

public:

	CFunctionList(CDocument *moduleDoc, CAutocomplectionCtrl* parent);   // standard constructor

	void OnButtonOk(wxCommandEvent &event); 
	void OnButtonCancel(wxCommandEvent &event);
	void OnCheckBoxSort(wxCommandEvent &event);
	void OnItemSelected(wxListEvent &event);

protected:

	// Events 
	wxDECLARE_EVENT_TABLE();
};

#endif // !defined(_FUNCTIONLIST_H__)
