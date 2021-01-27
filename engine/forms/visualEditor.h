#ifndef __VISUAL_EDITOR__
#define __VISUAL_EDITOR__

#include <set>

#include "innerFrame.h"
#include "pageWindow.h"
#include "utils/events.h"

/**
 * Extends the CInnerFrame to show the object highlight
 */
class CDesignerWindow : public CInnerFrame
{
	int m_x;
	int m_y;

	wxSizer *m_selSizer = NULL;
	wxObject *m_selItem = NULL;
	IControlElement*m_selObj = NULL;
	wxWindow *m_actPanel = NULL;

private:

	void DrawRectangle(wxDC& dc, const wxPoint& point, const wxSize& size, IControlElement* object);

	DECLARE_CLASS(CDesignerWindow)

	// Augh!, this class is needed to paint the highlight in the
	// frame content panel.
	class HighlightPaintHandler : public wxEvtHandler
	{
		wxDECLARE_EVENT_TABLE();

		wxWindow *m_dsgnWin;

	public:
		HighlightPaintHandler(wxWindow *win);
		void OnPaint(wxPaintEvent &event);
	};

public:
	CDesignerWindow(wxWindow *parent, int id, const wxPoint& pos, const wxSize &size = wxDefaultSize,
		long style = 0, const wxString &name = wxT("designer_win"));
	~CDesignerWindow();
	void SetGrid(int x, int y);
	void SetSelectedSizer(wxSizer *sizer) { m_selSizer = sizer; }
	void SetSelectedItem(wxObject *item) { m_selItem = item; }
	void SetSelectedObject(IControlElement* object) { m_selObj = object; }
	void SetSelectedPanel(wxWindow *actPanel) { m_actPanel = actPanel; }
	wxSizer *GetSelectedSizer() { return m_selSizer; }
	wxObject* GetSelectedItem() { return m_selItem; }
	IControlElement* GetSelectedObject() { return m_selObj; }
	wxWindow* GetActivePanel() { return m_actPanel; }
	static wxMenu* GetMenuFromObject(IControlElement* menu);
	void SetFrameWidgets(IControlElement* menubar, wxWindow *toolbar, wxWindow* statusbar);
	void HighlightSelection(wxDC& dc);
	void OnPaint(wxPaintEvent &event);

protected:
	wxDECLARE_EVENT_TABLE();
};

class wxFrameEvent;
class wxFramePropertyEvent;
class wxFrameObjectEvent;

class CVisualEditorContextForm;

#include "elements/frame.h"
#include "elements/sizers.h"
#include "elements/widgets.h"

#include "visualEditorBase.h"
#include "common/cmdProc.h"

#include <wx/artprov.h>
#include <wx/config.h>
#include <wx/cmdproc.h>
#include <wx/docview.h>
#include <wx/splitter.h>
#include <wx/spinbutt.h>
#include <wx/treectrl.h>

class ObjectType;
class CProcUnit;

class CDocument;
class CView;

#define visualEditorDatabase            (CVisualEditorDatabase::Get())
#define visualEditorDatabaseDestroy()  	(CVisualEditorDatabase::Destroy())

class CVisualEditorDatabase
{
	friend class CVisualEditor;
	friend class CVisualEditorObjectTree;

	friend class CFormEditView;

	static CVisualEditorDatabase* s_instance;

private:

	int CountChildrenWithSameType(IControlElement *parent, const wxString &type);
	void SetDefaultLayoutProperties(CValueSizerItem *obj);

	wxString GetClassType(const wxString &className);

public:

	static CVisualEditorDatabase* Get();
	static void Destroy();

public:

	IControlElement *NewObject(const wxString &obj_info, IControlElement *control = NULL);

	/**
	* Fabrica de objetos.
	* A partir del nombre de la clase se crea una nueva instancia de un objeto.
	*/
	CValueFrame *CreateFrame(ticpp::Element* xml_obj, bool isEnabled);

	IControlElement *CreateObject(const wxString &class_name, IControlElement *parent = NULL);
	IControlElement *CreateObject(ticpp::Element* xml_obj, IControlElement *parent = NULL);

	/**
	* Crea un objeto como copia de otro.
	*/
	IControlElement *CopyObject(IControlElement *obj);
};

//////////////////////////////////////////////////////////////////////////////////////////

#include <wx/docview.h>

class CVisualEditorContextForm : public wxPanel
{
	wxDECLARE_DYNAMIC_CLASS(CVisualEditorContextForm);

public:

	class CVisualEditor : public IVisualHost
	{
		friend class CVisualEditorContextForm;
		friend class CVisualEditorObjectTree;

	private:

		CDesignerWindow *m_back;

		// Prevent OnSelected in components
		bool m_stopSelectedEvent;
		// Prevent OnModified in components
		bool m_stopModifiedEvent;

		IControlElement *m_activeControl;

	public:

		friend class ExpandObjectCmd;
		friend class InsertObjectCmd;
		friend class RemoveObjectCmd;
		friend class ModifyPropertyCmd;
		friend class ModifyEventHandlerCmd;
		friend class ShiftChildCmd;
		friend class CutObjectCmd;
		friend class ReparentObjectCmd;

		CVisualEditor(CVisualEditorContextForm *handler, wxWindow *parent);
		virtual ~CVisualEditor() override;

		void OnResizeBackPanel(wxCommandEvent &event);
		void OnClickBackPanel(wxMouseEvent& event);
		void PreventOnSelected(bool prevent = true);
		void PreventOnModified(bool prevent = true);

		bool OnLeftClickFromApp(wxWindow *wnd);
		bool OnRightClickFromApp(wxWindow *wnd, wxMouseEvent &event);

		wxWindow *GetParentBackgroundWindow() { return m_back; }
		wxWindow *GetBackgroundWindow() { return m_back->GetFrameContentPanel(); }

		//override designer host  
		virtual bool IsDesignerHost() override { return true; }
		virtual CValueFrame *GetValueFrame() override;
		virtual void SetValueFrame(class CValueFrame *valueFrame) override;

		//set and create window
		void SetObjectSelect(IControlElement *obj);
		//Setup window 
		void CreateVisualEditor();
		//Update window 
		void UpdateVisualEditor();
		//Clear visualeditor
		void ClearVisualEditor();

	protected:

		wxDECLARE_EVENT_TABLE();
	};

	//////////////////////////////////////////////////////////////////////////////////////////

	class CVisualEditorObjectTree : public wxPanel
	{
		CVisualEditorContextForm *m_formHandler = NULL;

	private:
		typedef std::map< IControlElement *, wxTreeItemId> ObjectItemMap;
		typedef std::map<wxString, int> IconIndexMap;

		ObjectItemMap m_map;

		wxImageList *m_iconList = NULL;
		IconIndexMap m_iconIdx;

		wxTreeCtrl* m_tcObjects = NULL;

		wxTreeItemId m_draggedItem;

		bool m_altKeyIsDown;

		/**
		 * Crea el arbol completamente.
		 */
		void RebuildTree();
		void AddChildren(IControlElement * child, wxTreeItemId &parent, bool is_root = false);
		int GetImageIndex(wxString type);
		void UpdateItem(wxTreeItemId id, IControlElement * obj);
		void RestoreItemStatus(IControlElement * obj);
		void AddItem(IControlElement * item, IControlElement * parent);
		void RemoveItem(IControlElement * item);
		void ClearMap(IControlElement * obj);

		IControlElement * GetObjectFromTreeItem(wxTreeItemId item);

		wxDECLARE_EVENT_TABLE();

	public:

		CVisualEditorObjectTree(CVisualEditorContextForm *handler, wxWindow *parent, int id = wxID_ANY);
		virtual ~CVisualEditorObjectTree() override;

		void Create();

		void OnSelChanged(wxTreeEvent &event);
		void OnRightClick(wxTreeEvent &event);
		void OnBeginDrag(wxTreeEvent &event);
		void OnEndDrag(wxTreeEvent &event);
		void OnExpansionChange(wxTreeEvent &event);

		void OnProjectLoaded(wxFrameEvent &event);
		void OnProjectSaved(wxFrameEvent &event);
		void OnObjectExpanded(wxFrameObjectEvent& event);
		void OnObjectSelected(wxFrameObjectEvent &event);
		void OnObjectCreated(wxFrameObjectEvent &event);
		void OnObjectRemoved(wxFrameObjectEvent &event);
		void OnPropertyModified(wxFramePropertyEvent &event);
		void OnProjectRefresh(wxFrameEvent &event);
		void OnKeyDown(wxTreeEvent &event);
	};

	/**
	 * Gracias a que podemos asociar un objeto a cada item, esta clase nos va
	 * a facilitar obtener el objeto (IControlElement) asociado a un item para
	 * seleccionarlo pinchando en el item.
	 */
	class CVisualEditorObjectTreeItemData : public wxTreeItemData
	{
	private:
		IControlElement * m_object = NULL;
	public:
		CVisualEditorObjectTreeItemData(IControlElement * obj);
		IControlElement * GetObject() { return m_object; }
	};

	/**
	 * Menu popup asociado a cada item del arbol.
	 *
	 * Este objeto ejecuta los comandos incluidos en el menu referentes al objeto
	 * seleccionado.
	 */
	class CVisualEditorItemPopupMenu : public wxMenu
	{
		IControlElement* m_object = NULL;
		CVisualEditorContextForm *m_formHandler = NULL;

		int m_selID;

	public:

		bool HasDeleteObject();
		int GetSelectedID() { return m_selID; }

		CVisualEditorItemPopupMenu(CVisualEditorContextForm *handler, wxWindow *parent, IControlElement * obj);

		void OnUpdateEvent(wxUpdateUIEvent& e);
		void OnMenuEvent(wxCommandEvent & event);

	protected:

		wxDECLARE_EVENT_TABLE();
	};

	//////////////////////////////////////////////////////////////////////////////////////////

	IControlElement* m_selObj = NULL;     // Objeto seleccionado
	IControlElement* m_clipboard = NULL;

	std::vector< wxEvtHandler* > m_handlers;

	bool m_bReadOnly;
	bool m_copyOnPaste; // flag que indica si hay que copiar el objeto al pegar

	// Procesador de comandos Undo/Redo
	CCommandProcessor *m_cmdProc;

	//Elements form 
	CVisualEditor *m_visualEditor;
	CVisualEditorObjectTree *m_objectTree;

	//Document & view 
	CDocument *m_document;
	CView *m_view;

	//access to private object  
	friend class CValueNotebook;
	friend class CValueNotebookPage;

	friend class CValueToolbar;
	friend class CValueToolBarItem;
	friend class CValueToolBarSeparator;

	friend class CValueTableBox;
	friend class CValueTableBoxColumn;

	friend class CVisualDesignerCommandProcessor;
	friend class CVisualCommand;

	friend class CValueFrame;

private:

	CValueFrame *m_valueFrame;

	friend class CVisualEditor;
	friend class CVisualEditorObjectTree;

private:

	wxSplitterWindow *m_splitter = NULL;

public:

	CVisualEditor *GetVisualEditor() { return m_visualEditor; }
	CVisualEditorObjectTree *GetObjectTree() { return m_objectTree; }

	CValueFrame *GetValueFrame() { return m_valueFrame; }
	void SetValueFrame(CValueFrame *valueFrame) { m_valueFrame = valueFrame; }

	bool IsEditable() { return !m_bReadOnly; }
	void SetReadOnly(bool readOnly = true) { m_bReadOnly = readOnly; }

protected:

	//Notify event 
	void NotifyEvent(wxFrameEvent& event, bool forcedelayed = false);

	// Notifican a cada observador el evento correspondiente
	void NotifyProjectLoaded();
	void NotifyProjectSaved();
	void NotifyObjectExpanded(IControlElement* obj);
	void NotifyObjectSelected(IControlElement* obj, bool force = false);
	void NotifyObjectCreated(IControlElement* obj);
	void NotifyObjectRemoved(IControlElement* obj);
	void NotifyPropertyModified(Property* prop);
	void NotifyEventHandlerModified(Event* evtHandler);
	void NotifyProjectRefresh();
	void NotifyCodeGeneration(bool panelOnly = false, bool force = false);

	/**
	 * Resuelve un posible conflicto de nombres.
	 * @note el objeto a comprobar debe estar insertado en proyecto, por tanto
	 *       no es válida para arboles "flotantes".
	 */
	static void ResolveNameConflict(IControlElement* obj);

	/**
	 * Rename all objects that have the same name than any object of a subtree.
	 */
	void ResolveSubtreeNameConflicts(IControlElement* obj, IControlElement* topObj = NULL);

	/**
	 * Rutina auxiliar de ResolveNameConflict
	 */
	static void BuildNameSet(IControlElement* obj, IControlElement* top, std::set<wxString> &name_set);

	//Execute command 
	void Execute(CCommand *cmd);

	/**
	* Search a size in the hierarchy of an object
	*/
	IControlElement *SearchSizerInto(IControlElement *obj);

	void PropagateExpansion(IControlElement* obj, bool expand, bool up);

	/**
	* Eliminar un objeto.
	*/
	void DoRemoveObject(IControlElement* object, bool cutObject, bool force = false);

public:

	CVisualEditorContextForm();
	CVisualEditorContextForm(CDocument *document, CView *view, wxWindow *parent, int id = wxID_ANY);

	// Procedures for register/unregister wxEvtHandlers to be notified of wxOESEvents
	void AddHandler(wxEvtHandler* handler);
	void RemoveHandler(wxEvtHandler* handler);

	void ActivateObject();
	void DeactivateObject();

	// Servicios específicos, no definidos en DataObservable
	void SetClipboardObject(IControlElement* obj) { m_clipboard = obj; }
	IControlElement* GetClipboardObject() { return m_clipboard; }

	//Objects 
	IControlElement *CreateObject(const wxString &name);
	void RemoveObject(IControlElement* obj);
	void CutObject(IControlElement* obj, bool force = false);
	void CopyObject(IControlElement* obj);
	bool PasteObject(IControlElement* parent, IControlElement* objToPaste = NULL);
	void InsertObject(IControlElement* obj, IControlElement* parent);
	void ExpandObject(IControlElement* obj, bool expand);

	void ModifyProperty(Property* prop, const wxString &value);
	void ModifyEventHandler(Event* evt, const wxString &value);

	void CreateWideGui();

	void DetermineObjectToSelect(IControlElement* parent, unsigned int pos);

	// Object will not be selected if it already is selected, unless force = true
	// Returns true if selection changed, false if already selected
	bool SelectObject(IControlElement* obj, bool force = false, bool notify = true);

	void MovePosition(IControlElement *obj, unsigned int toPos);
	void MovePosition(IControlElement *obj, bool right, unsigned int num = 1);

	// Servicios para los observadores
	IControlElement* GetSelectedObject();
	CValueFrame* GetSelectedForm();

	void RefreshEditor() { NotifyProjectRefresh(); }

	void Undo();
	void Redo();

	bool CanUndo() { return m_cmdProc->CanUndo(); }
	bool CanRedo() { return m_cmdProc->CanRedo(); }

	bool CanPasteObject();
	bool CanCopyObject();

	bool IsModified();

	/**
	* Calcula la posición donde deberá ser insertado el objeto.
	*
	* Dado un objeto "padre" y un objeto "seleccionado", esta rutina calcula la
	* posición de inserción de un objeto debajo de "parent" de forma que el objeto
	* quede a continuación del objeto "seleccionado".
	*
	* El algoritmo consiste ir subiendo en el arbol desde el objeto "selected"
	* hasta encontrar un objeto cuyo padre sea el mismo que "parent" en cuyo
	* caso se toma la posición siguiente a ese objeto.
	*
	* @param parent objeto "padre"
	* @param selected objeto "seleccionado".
	* @return posición de insercción (-1 si no se puede insertar).
	*/
	int CalcPositionOfInsertion(IControlElement* selected, IControlElement* parent);

	void ToggleBorderFlag(IControlElement *obj, int border);
	void CreateBoxSizerWithObject(IControlElement *obj);

	bool LoadForm();
	bool SaveForm();
	void RunForm();

	void SetCommandProcessor(CCommandProcessor *cmdProc) { m_cmdProc = cmdProc; }

	~CVisualEditorContextForm();

	// Events
	void OnProjectLoaded(wxFrameEvent &event);
	void OnProjectSaved(wxFrameEvent &event);
	void OnObjectSelected(wxFrameObjectEvent &event);
	void OnObjectCreated(wxFrameObjectEvent &event);
	void OnObjectRemoved(wxFrameObjectEvent &event);
	void OnPropertyModified(wxFramePropertyEvent &event);
	void OnProjectRefresh(wxFrameEvent &event);
	void OnCodeGeneration(wxFrameEventHandlerEvent &event);

	wxDECLARE_EVENT_TABLE();
};

#endif