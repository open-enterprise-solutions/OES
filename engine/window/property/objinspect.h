#ifndef __OBJ_INSPECT__
#define __OBJ_INSPECT__

#include "common/objectbase.h"

#include <wx/aui/auibook.h>
#include <wx/propgrid/manager.h>

#if !wxUSE_PROPGRID
#error "wxUSE_PROPGRID must be set to 1 in your wxWidgets library."
#endif

class wxFrameEventHandlerEvent;
class wxFramePropertyEvent;
class wxFrameObjectEvent;
class wxFrameEvent;

enum {
	wxOES_OI_DEFAULT_STYLE,
	wxOES_OI_MULTIPAGE_STYLE,
	wxOES_OI_SINGLE_PAGE_STYLE
};

class CObjectInspector;

#define objectInspector           (CObjectInspector::Get())
#define objectInspectorDestroy()  (CObjectInspector::Destroy())

class CObjectInspector : public wxPanel
{
	static CObjectInspector *s_instance;

private:

	typedef std::map< wxPGProperty*, Property*> ObjInspectorPropertyMap;
	typedef std::map< wxPGProperty*, Event*> ObjInspectorEventMap;

	ObjInspectorPropertyMap m_propMap;
	ObjInspectorEventMap m_eventMap;

	IObjectBase *m_currentSel;
	wxEvtHandler *m_evthandler;

	//save the current selected property
	wxString m_strSelPropItem;
	wxString m_pageName;

	wxAuiNotebook* m_nb;

	wxPropertyGridManager* m_pg;
	wxPropertyGridManager* m_eg;

	int m_style;

	int StringToBits(const wxString& strVal, wxPGChoices& constants);

	typedef std::map< wxString, bool > ExpandMap;
	ExpandMap m_isExpanded;

	template < class ValueT >
	void CreateCategory(const wxString& name, IObjectBase* obj, std::map< wxString, ValueT >& itemMap, bool addingEvents)
	{
		// Get Category
		PropertyCategory* category = obj->GetCategory();
		if (!category) return;

		// Prevent page creation if there are no properties
		if (category->GetCategoryCount() == 0 && (addingEvents ? category->GetEventCount() : category->GetPropertyCount()) == 0) return;

		wxString pageName;

		if (m_style == wxOES_OI_MULTIPAGE_STYLE) pageName = name;
		else pageName = wxT("default");

		wxPropertyGridManager* pg = (addingEvents ? m_eg : m_pg);
		int pageIndex = pg->GetPageByName(pageName);
		
		if (pageIndex == wxNOT_FOUND)
		{
			//pg->AddPage( pageName, obj_info->GetSmallIconFile() );
			pg->AddPage(pageName);
		}

		const wxString& catName = category->GetName();
		wxPGProperty* id = pg->Append(new wxPropertyCategory(catName));

		AddItems(name, obj, category, itemMap);

		ExpandMap::iterator it = m_isExpanded.find(catName);
		
		if (it != m_isExpanded.end())
		{
			if (it->second)
			{
				pg->Expand(id);
			}
			else
			{
				pg->Collapse(id);
			}
		}

		pg->SetPropertyAttributeAll(wxPG_BOOL_USE_CHECKBOX, (long)1);
	}

	void AddItems(const wxString& name, IObjectBase* obj, PropertyCategory* category, PropertyMap& map);
	void AddItems(const wxString& name, IObjectBase* obj, PropertyCategory* category, EventMap& map);

	wxPGProperty* GetProperty(Property* prop);

	void Create(bool force = false);

	void OnPropertyGridChanging(wxPropertyGridEvent& event);
	void OnPropertyGridChanged(wxPropertyGridEvent& event);
	void OnPropertyGridDblClick(wxPropertyGridEvent& event);
	void OnPropertyGridExpand(wxPropertyGridEvent& event);
	void OnPropertyGridItemSelected(wxPropertyGridEvent& event);

	void OnEventGridChanged(wxPropertyGridEvent& event);
	void OnEventGridDblClick(wxPropertyGridEvent& event);
	void OnEventGridExpand(wxPropertyGridEvent& event);

	void OnBitmapPropertyChanged(wxCommandEvent& event);

	bool ModifyProperty(Property* prop, const wxString& str);
	void RestoreLastSelectedPropItem();

	CObjectInspector(wxWindow *parent, int id, int style = wxOES_OI_DEFAULT_STYLE);

public:

	~CObjectInspector();

	static CObjectInspector* Get();
	static void Destroy();

	void OnObjectSelected(wxFrameObjectEvent& event);
	void OnProjectRefresh(wxFrameEvent& event);
	void OnPropertyModified(wxFramePropertyEvent& event);
	void OnEventHandlerModified(wxFrameEventHandlerEvent& event);


	// Servicios para los observadores
	void SelectObject(IObjectBase *selobj, bool force = true);
	void SelectObject(IObjectBase *selobj, wxEvtHandler *m_handler, bool force = true);
	IObjectBase* GetSelectedObject();

	void ShowProperty();
	void RefreshProperty();

	void AutoGenerateId(IObjectBase* objectChanged, Property* propChanged, const wxString &reason);
	wxPropertyGridManager* CreatePropertyGridManager(wxWindow *parent, wxWindowID id);
	void SavePosition();

	void ClearProperty();

	DECLARE_EVENT_TABLE()
};

#endif //__OBJ_INSPECT__