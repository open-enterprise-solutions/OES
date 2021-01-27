#ifndef _VISUALEDITOR_BASE_H__
#define _VISUALEDITOR_BASE_H__

#include <wx/wx.h>
#include <wx/scrolwin.h>
#include <map>

class IControlElement;
class CVisualEditorContextForm;

class IVisualHost : public wxScrolledCanvas
{
	wxDECLARE_ABSTRACT_CLASS(IVisualHost);

protected:

	//controls
	wxBoxSizer *m_mainBoxSizer;

	std::map<wxObject*, IControlElement *> m_wxObjects;
	std::map<IControlElement*, wxObject* > m_baseObjects;

	CVisualEditorContextForm *m_formHandler;

public:

	IVisualHost() : wxScrolledCanvas(), m_formHandler(NULL), m_mainBoxSizer(NULL) { }
	IVisualHost(wxWindow *parent,
		wxWindowID id,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_FRAME_STYLE,
		const wxString& name = wxFrameNameStr) : wxScrolledCanvas(parent, id, pos, size, style, name), m_mainBoxSizer(NULL) {}

	~IVisualHost() {}

	IControlElement *GetObjectBase(wxObject* wxobject);
	wxObject* GetWxObject(IControlElement* baseobject);

	wxBoxSizer *GetFrameSizer() { return m_mainBoxSizer; }
	CVisualEditorContextForm *GetVisualHostContext() const { return m_formHandler; }

	virtual bool IsDemonstration() { return false; }
	virtual bool IsDesignerHost() { return false; }

	virtual class CValueFrame *GetValueFrame() = 0;
	virtual void SetValueFrame(class CValueFrame *valueFrame) = 0;

	virtual wxWindow *GetParentBackgroundWindow() = 0;
	virtual wxWindow *GetBackgroundWindow() = 0;

protected:

	friend class CValueTableBox;

	friend class CValueNotebook;
	friend class CValueNotebookPage;

	//Insert new control
	void CreateControl(IControlElement *obj, IControlElement *parent = NULL);
	//Update exist control
	void UpdateControl(IControlElement *obj, IControlElement *parent = NULL);
	//Remove control
	void RemoveControl(IControlElement *obj, IControlElement *parent = NULL);
	//Generate component 
	void GenerateControl(IControlElement *obj, wxWindow* wxparent, wxObject* parentObject);
	//Update component
	void RefreshControl(IControlElement *obj, wxWindow* wxparent, wxObject* parentObject);
	// Give components an opportunity to cleanup
	void DeleteRecursive(IControlElement *control, bool force = false);
	//Update virtual size
	void UpdateVirtualSize();

	//*********************************************************
	//*                 Events for visual                     *
	//*********************************************************

	/**
	* Create an instance of the wxObject and return a pointer
	*/
	virtual wxObject* Create(IControlElement *control, wxObject* parent);

	/**
	* Allows components to do something after they have been created.
	* For example, Abstract components like NotebookPage and SizerItem can
	* add the actual widget to the Notebook or sizer.
	*
	* @param wxobject The object which was just created.
	* @param wxparent The wxWidgets parent - the wxObject that the created object was added to.
	*/
	virtual void OnCreated(IControlElement *control, wxObject* obj, wxWindow* wndParent);

	/**
	* Allows components to respond when selected in object tree.
	* For example, when a wxNotebook's page is selected, it can switch to that page
	*/
	virtual void OnSelected(IControlElement *control, wxObject* obj);

	/**
	* Allows components to do something after they have been updated.
	*/
	virtual void Update(IControlElement *control, wxObject* obj);

	/**
	* Allows components to do something after they have been updated.
	* For example, Abstract components like NotebookPage and SizerItem can
	* add the actual widget to the Notebook or sizer.
	*
	* @param wxobject The object which was just updated.
	* @param wxparent The wxWidgets parent - the wxObject that the updated object was added to.
	*/
	virtual void OnUpdated(IControlElement *control, wxObject* obj, wxWindow* wndParent);

	/**
	 * Cleanup (do the reverse of Create)
	 */
	virtual void Cleanup(IControlElement *control, wxObject* obj);
};

extern CVisualEditorContextForm *m_visualHostContext;

#endif 