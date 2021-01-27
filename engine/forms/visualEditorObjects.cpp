////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko, wxFormBuilder
//	Description : visual editor 
////////////////////////////////////////////////////////////////////////////

#include "visualEditor.h"

#include "window/mainFrame.h"
#include "window/property/objinspect.h"

#include "common/objectbase.h"
#include "utils/stringutils.h"
#include "utils/typeconv.h"

///////////////////////////////////////////////////////////////////////////////
// Comandos
///////////////////////////////////////////////////////////////////////////////

class BaseVisualCmd : public CCommand
{
public:

	BaseVisualCmd(CVisualEditorContextForm *visualData = NULL) : m_visualData(visualData) {}

	virtual void Execute() { CCommand::Execute(); }
	virtual void Restore() { CCommand::Restore(); }

protected:

	CVisualEditorContextForm *m_visualData;
};

/** Command for expanding an object in the object tree */

class ExpandObjectCmd : public BaseVisualCmd
{
	IControlElement* m_object = NULL;
	bool m_expand;

public:

	ExpandObjectCmd(CVisualEditorContextForm *data, IControlElement* object, bool expand);

protected:

	virtual void DoExecute() override;
	virtual void DoRestore() override;
};

/**
* Comando para insertar un objeto en el arbol.
*/

class InsertObjectCmd : public BaseVisualCmd
{
	IControlElement* m_parent = NULL;
	IControlElement* m_object = NULL;
	int m_pos;
	IControlElement* m_oldSelected;

public:

	InsertObjectCmd(CVisualEditorContextForm *data, IControlElement* object, IControlElement* parent, int pos = -1);

protected:

	virtual void DoExecute() override;
	virtual void DoRestore() override;
};

/**
* Comando para borrar un objeto.
*/

class RemoveObjectCmd : public BaseVisualCmd,
	public wxEvtHandler
{
	IControlElement* m_parent = NULL;
	IControlElement* m_object = NULL;
	int m_oldPos;
	IControlElement* m_oldSelected = NULL;

public:

	RemoveObjectCmd(CVisualEditorContextForm *data, IControlElement* object);
	~RemoveObjectCmd();

protected:

	void OnObjectRemoved(wxFrameObjectEvent &event);

	virtual void DoExecute() override;
	virtual void DoRestore() override;
};

/**
* Comando para modificar una propiedad.
*/

class ModifyPropertyCmd : public BaseVisualCmd
{
	Property* m_property;
	wxString m_oldValue, m_newValue;

public:

	ModifyPropertyCmd(CVisualEditorContextForm *data, Property* prop, const wxString &value);

protected:
	virtual void DoExecute() override;
	virtual void DoRestore() override;
};

/**
* BaseVisualCmd for modifying an event
*/

class ModifyEventHandlerCmd : public BaseVisualCmd
{
	Event* m_event = NULL;
	wxString m_oldValue, m_newValue;

public:
	ModifyEventHandlerCmd(CVisualEditorContextForm *data, Event* event, const wxString &value);

protected:
	virtual void DoExecute() override;
	virtual void DoRestore() override;
};

/**
* Comando para mover de posicion un objeto.
*/

class ShiftChildCmd : public BaseVisualCmd
{
	IControlElement* m_object = NULL;
	int m_oldPos, m_newPos;

public:
	ShiftChildCmd(CVisualEditorContextForm *data, IControlElement* object, int pos);

protected:
	virtual void DoExecute() override;
	virtual void DoRestore() override;
};

/**
* CutObjectCmd ademas de eliminar el objeto del arbol se asegura
* de eliminar la referencia "clipboard" deshacer el cambio.
*/

class CutObjectCmd : public BaseVisualCmd,
	public wxEvtHandler

{
	// necesario para consultar/modificar el objeto "clipboard"
	IControlElement* m_parent = NULL;
	IControlElement* m_object = NULL;
	int m_oldPos;
	IControlElement* m_oldSelected = NULL;

	bool m_needEvent;

public:

	CutObjectCmd(CVisualEditorContextForm *data, IControlElement* object, bool force);
	~CutObjectCmd();

protected:

	void OnObjectRemoved(wxFrameObjectEvent &event);

	virtual void DoExecute() override;
	virtual void DoRestore() override;
};

///////////////////////////////////////////////////////////////////////////////
// Implementacion de los Comandos
///////////////////////////////////////////////////////////////////////////////

ExpandObjectCmd::ExpandObjectCmd(CVisualEditorContextForm *data, IControlElement* object, bool expand) : BaseVisualCmd(data),
m_object(object), m_expand(expand)
{
}

void ExpandObjectCmd::DoExecute()
{
	m_object->SetExpanded(m_expand);
}

void ExpandObjectCmd::DoRestore()
{
	m_object->SetExpanded(!m_expand);
}

InsertObjectCmd::InsertObjectCmd(CVisualEditorContextForm *data, IControlElement* object, IControlElement* parent, int pos) : BaseVisualCmd(data),
m_parent(parent), m_object(object), m_pos(pos)
{
	m_oldSelected = data->GetSelectedObject();
}

void InsertObjectCmd::DoExecute()
{
	if (m_parent)
	{
		m_parent->AddChild(m_object);
		m_object->SetParent(m_parent);
	}

	m_object->GenerateNewID();

	if (m_pos >= 0)
	{
		m_parent->ChangeChildPosition(m_object, m_pos);
	}

	IControlElement* obj = m_object;
	while (obj && obj->IsItem())
	{
		if (obj->GetChildCount() > 0)
			obj = obj->GetChild(0);
		else return;
	}

	if (obj && obj != m_object) { obj->GenerateNewID(); obj->SaveProperty(); }
	else if (obj) { obj->SaveProperty(); }

	//create control in visual editor
	CVisualEditorContextForm::CVisualEditor *m_visulEditor = m_visualData->GetVisualEditor();
	m_visulEditor->CreateControl(m_object);

	//select object
	m_visualData->SelectObject(obj, false, false);
}

void InsertObjectCmd::DoRestore()
{
	//remove control in visual editor
	CVisualEditorContextForm::CVisualEditor *m_visualEditor = m_visualData->GetVisualEditor();
	m_visualEditor->RemoveControl(m_object);

	m_parent->RemoveChild(m_object);
	m_object->SetParent(NULL);

	m_visualData->SelectObject(m_oldSelected);
}

//-----------------------------------------------------------------------------

RemoveObjectCmd::RemoveObjectCmd(CVisualEditorContextForm *data, IControlElement* object) : BaseVisualCmd(data)
{
	m_object = object;
	m_parent = object->GetParent();
	m_oldPos = m_parent->GetChildPosition(object);
	m_oldSelected = data->GetSelectedObject();

	wxEvtHandler::Connect(wxEVT_OBJECT_REMOVED, wxFrameObjectEventHandler(RemoveObjectCmd::OnObjectRemoved), NULL, this);
}

RemoveObjectCmd::~RemoveObjectCmd()
{
	wxEvtHandler::Disconnect(wxEVT_OBJECT_REMOVED, wxFrameObjectEventHandler(RemoveObjectCmd::OnObjectRemoved), NULL, this);
}

void RemoveObjectCmd::OnObjectRemoved(wxFrameObjectEvent &event)
{
	//remove control in visual editor
	CVisualEditorContextForm::CVisualEditor *m_visulEditor = m_visualData->GetVisualEditor();

	m_parent->AddChild(m_object);
	m_object->SetParent(m_parent);

	m_visulEditor->RemoveControl(m_object);

	m_parent->RemoveChild(m_object);
	m_object->SetParent(NULL);
}

void RemoveObjectCmd::DoExecute()
{
	m_parent->RemoveChild(m_object);
	m_object->SetParent(NULL);

	m_visualData->DetermineObjectToSelect(m_parent, m_oldPos);

	wxFrameObjectEvent event(wxEVT_OBJECT_REMOVED, m_object);
	wxEvtHandler::AddPendingEvent(event);
}

void RemoveObjectCmd::DoRestore()
{
	m_parent->AddChild(m_object);
	m_object->SetParent(m_parent);

	// restauramos la posicion
	m_parent->ChangeChildPosition(m_object, m_oldPos);
	m_visualData->SelectObject(m_oldSelected, true, false);

	//create control in visual editor
	CVisualEditorContextForm::CVisualEditor *m_visulEditor = m_visualData->GetVisualEditor();
	m_visulEditor->CreateControl(m_object);
}

//-----------------------------------------------------------------------------

ModifyPropertyCmd::ModifyPropertyCmd(CVisualEditorContextForm *data, Property* prop, const wxString &value) : BaseVisualCmd(data),
m_property(prop), m_newValue(value)
{
	m_oldValue = prop->GetValue();
}

void ModifyPropertyCmd::DoExecute()
{
	CVisualEditorContextForm::CVisualEditor *m_visulEditor = m_visualData->GetVisualEditor();
	// Get the IControlElement from the event
	IControlElement* m_object = dynamic_cast<IControlElement *>(m_property->GetObject());
	m_property->SetValue(m_newValue);
	m_object->SaveProperty();

	if (m_object->GetClassName() == wxT("frame")) 
		m_visulEditor->UpdateVisualEditor();
	else m_visulEditor->UpdateControl(m_object);
}

void ModifyPropertyCmd::DoRestore()
{
	CVisualEditorContextForm::CVisualEditor *m_visulEditor = m_visualData->GetVisualEditor();
	// Get the IControlElement from the event
	IControlElement* m_object = dynamic_cast<IControlElement *>(m_property->GetObject());

	m_property->SetValue(m_oldValue);
	m_object->SaveProperty();

	if (m_object->GetClassName() == wxT("frame")) m_visulEditor->UpdateVisualEditor();
	else m_visulEditor->UpdateControl(m_object);
}

//-----------------------------------------------------------------------------

ModifyEventHandlerCmd::ModifyEventHandlerCmd(CVisualEditorContextForm *data, Event* event, const wxString &value) : BaseVisualCmd(data),
m_event(event), m_newValue(value)
{
	m_oldValue = event->GetValue();
}

void ModifyEventHandlerCmd::DoExecute()
{
	m_event->SetValue(m_newValue);
}

void ModifyEventHandlerCmd::DoRestore()
{
	m_event->SetValue(m_oldValue);
}

//-----------------------------------------------------------------------------

ShiftChildCmd::ShiftChildCmd(CVisualEditorContextForm *data, IControlElement* object, int pos) : BaseVisualCmd(data)
{
	m_object = object;
	IControlElement* parent = object->GetParent();

	assert(parent);

	m_oldPos = parent->GetChildPosition(object);
	m_newPos = pos;
}

void ShiftChildCmd::DoExecute()
{
	CVisualEditorContextForm::CVisualEditor *m_visulEditor = m_visualData->GetVisualEditor();

	if (m_oldPos != m_newPos)
	{
		IControlElement* parent(m_object->GetParent());
		parent->ChangeChildPosition(m_object, m_newPos);

		m_visulEditor->UpdateControl(m_object);
	}
}

void ShiftChildCmd::DoRestore()
{
	CVisualEditorContextForm::CVisualEditor *m_visulEditor = m_visualData->GetVisualEditor();

	if (m_oldPos != m_newPos)
	{
		IControlElement* parent(m_object->GetParent());
		parent->ChangeChildPosition(m_object, m_oldPos);

		m_visulEditor->UpdateControl(m_object);
	}
}

//-----------------------------------------------------------------------------

CutObjectCmd::CutObjectCmd(CVisualEditorContextForm *data, IControlElement* object, bool force) : BaseVisualCmd(data), m_needEvent(!force)
{
	m_object = object;
	m_parent = object->GetParent();
	m_oldPos = m_parent->GetChildPosition(object);
	m_oldSelected = data->GetSelectedObject();

	if (m_needEvent) { wxEvtHandler::Connect(wxEVT_OBJECT_REMOVED, wxFrameObjectEventHandler(CutObjectCmd::OnObjectRemoved), NULL, this); }
}

CutObjectCmd::~CutObjectCmd()
{
	if (m_needEvent) { wxEvtHandler::Disconnect(wxEVT_OBJECT_REMOVED, wxFrameObjectEventHandler(CutObjectCmd::OnObjectRemoved), NULL, this); }
}

void CutObjectCmd::OnObjectRemoved(wxFrameObjectEvent &event)
{
	//remove control in visual editor
	CVisualEditorContextForm::CVisualEditor *m_visulEditor = m_visualData->GetVisualEditor();

	m_parent->AddChild(m_object);
	m_object->SetParent(m_parent);

	m_visulEditor->RemoveControl(m_object);

	m_parent->RemoveChild(m_object);
	m_object->SetParent(NULL);
}

void CutObjectCmd::DoExecute()
{
	m_visualData->SetClipboardObject(m_object);

	if (!m_needEvent)
	{
		//remove control in visual editor
		CVisualEditorContextForm::CVisualEditor *m_visulEditor = m_visualData->GetVisualEditor();
		m_visulEditor->RemoveControl(m_object);
	}

	//remove control in visual editor
	m_parent->RemoveChild(m_object);
	m_object->SetParent(NULL);

	m_visualData->DetermineObjectToSelect(m_parent, m_oldPos);

	if (m_needEvent)
	{
		wxFrameObjectEvent event(wxEVT_OBJECT_REMOVED, m_object);
		wxEvtHandler::AddPendingEvent(event);
	}
}

void CutObjectCmd::DoRestore()
{
	// reubicamos el objeto donde estaba
	m_parent->AddChild(m_object);
	m_object->SetParent(m_parent);

	m_parent->ChangeChildPosition(m_object, m_oldPos);

	// restauramos el clipboard
	m_visualData->SetClipboardObject(NULL);
	m_visualData->SelectObject(m_oldSelected, true, false);

	//create control in visual editor
	CVisualEditorContextForm::CVisualEditor *m_visulEditor = m_visualData->GetVisualEditor();
	m_visulEditor->CreateControl(m_object, m_parent);
}

//-----------------------------------------------------------------------------

IControlElement *CVisualEditorContextForm::CreateObject(const wxString &name)
{
	IControlElement* obj = NULL;

	try
	{
		//LogDebug("[ApplicationData::CreateObject] New " + name );
		IControlElement* old_selected = GetSelectedObject();
		IControlElement* parent = old_selected;

		if (parent)
		{
			bool created = false;

			// Para que sea mas practico, si el objeto no se puede crear debajo
			// del objeto seleccionado vamos a intentarlo en el padre del seleccionado
			// y seguiremos subiendo hasta que ya no podamos crear el objeto.

			while (parent && !created)
			{
				// ademas, el objeto se insertara a continuacion del objeto seleccionado
				obj = visualEditorDatabase->CreateObject(_STDSTR(name), parent);

				if (obj)
				{
					int pos = CalcPositionOfInsertion(GetSelectedObject(), parent);

					Execute(new InsertObjectCmd(this, obj, parent, pos));
					created = true;
					ResolveNameConflict(obj);
				}
				else
				{
					// lo vamos a seguir intentando con el padre, pero cuidado, el padre
					// no puede ser un item!
					parent = parent->GetParent();

					while (parent && parent->IsItem())
						parent = parent->GetParent();
				}
			}
		}

		// Seleccionamos el objeto, si este es un item entonces se selecciona
		// el objeto contenido. ?Tiene sentido tener un item debajo de un item?
		while (obj && obj->IsItem())
			obj = (obj->GetChildCount() > 0 ? obj->GetChild(0) : NULL);

		NotifyObjectCreated(obj);

		if (obj)
		{
			SelectObject(obj, true, true);
		}
		else
		{
			SelectObject(old_selected, true, true);
		}
	}
	catch (const std::exception& ex)
	{
		wxLogError(ex.what());
	}

	return obj;
}

void CVisualEditorContextForm::InsertObject(IControlElement* obj, IControlElement* parent)
{
	Execute(new InsertObjectCmd(this, obj, parent));
	NotifyObjectCreated(obj);
}

void CVisualEditorContextForm::CopyObject(IControlElement* obj)
{
	m_copyOnPaste = true;

	// Make a copy of the object on the clipboard, otherwise
	// modifications to the object after the copy will also
	// be made on the clipboard.
	IControlElement *objParent = obj->GetParent();

	if (objParent && objParent->IsItem())
		m_clipboard = visualEditorDatabase->CopyObject(objParent);
	else
		m_clipboard = visualEditorDatabase->CopyObject(obj);
}

bool CVisualEditorContextForm::PasteObject(IControlElement* parent, IControlElement* objToPaste)
{
	try
	{
		IControlElement* clipboard = NULL;
		if (objToPaste)
		{
			clipboard = objToPaste;
		}
		else if (m_clipboard)
		{
			if (m_copyOnPaste)
			{
				clipboard = visualEditorDatabase->CopyObject(m_clipboard);
			}
			else
			{
				clipboard = m_clipboard;
			}
		}

		if (!clipboard) return false;

		// Remove parent/child relationship from clipboard object
		IControlElement* clipParent = clipboard->GetParent();
		if (clipParent)
		{
			clipParent->RemoveChild(clipboard);
			clipboard->SetParent(NULL);
		}

		// Vamos a hacer un pequeno truco, intentaremos crear un objeto nuevo
		// del mismo tipo que el guardado en m_clipboard debajo de parent.
		// El objeto devuelto quiza no sea de la misma clase que m_clipboard debido
		// a que este incluido dentro de un "item".
		// Por tanto, si el objeto devuelto es no-nulo, entonces vamos a descender
		// en el arbol hasta que el objeto sea de la misma clase que m_clipboard,
		// momento en que cambiaremos dicho objeto por m_clipboard.
		//
		// Ejemplo:
		//
		//  m_clipboard :: wxButton
		//  parent      :: wxBoxSizer
		//
		//  obj = CreateObject(m_clipboard->GetObjectInfo()->GetClassName(), parent)
		//
		//  obj :: sizerItem
		//              /
		//           wxButton   <- Cambiamos este por m_clipboard

		IControlElement* obj = visualEditorDatabase->CreateObject(_STDSTR(clipboard->GetClassName()), parent);

		// If the object is already contained in an item, we may need to get the object out of the first
		// item before pasting
		if (!obj)
		{
			IControlElement* tempItem = clipboard;
			while (tempItem->IsItem())
			{
				tempItem = tempItem->GetChild(0);
				if (!tempItem)
				{
					break;
				}

				obj = visualEditorDatabase->CreateObject(_STDSTR(tempItem->GetClassName()), parent);
				if (obj)
				{
					clipboard = tempItem;
					break;
				}
			}
		}

		int pos = -1;

		if (!obj)
		{
			// si no se ha podido crear el objeto vamos a intentar crearlo colgado
			// del padre de "parent" y ademas vamos a insertarlo en la posicion
			// siguiente a "parent"
			IControlElement* selected = parent;
			parent = selected->GetParent();

			while (parent && parent->IsItem())
			{
				selected = parent;
				parent = selected->GetParent();
			}

			if (parent)
			{
				obj = visualEditorDatabase->CreateObject(_STDSTR(clipboard->GetClassName()), parent);
				if (obj)
				{
					pos = CalcPositionOfInsertion(selected, parent);
				}
			}
		}

		if (!obj)
			return false;

		IControlElement* aux = obj;

		while (aux && aux != clipboard)
			aux = (aux->GetChildCount() > 0 ? aux->GetChild(0) : NULL);

		if (aux && aux != obj)
		{
			// sustituimos aux por clipboard
			IControlElement* auxParent = aux->GetParent();
			auxParent->RemoveChild(aux);
			aux->SetParent(NULL);

			auxParent->AddChild(clipboard);
			clipboard->SetParent(auxParent);
		}
		else
			obj = clipboard;

		// y finalmente insertamos en el arbol
		Execute(new InsertObjectCmd(this, obj, parent, pos));

		if (!m_copyOnPaste)
			m_clipboard = NULL;

		ResolveSubtreeNameConflicts(obj);
		NotifyObjectCreated(obj);

		// vamos a mantener seleccionado el nuevo objeto creado
		// pero hay que tener en cuenta que es muy probable que el objeto creado
		// sea un "item"
		while (obj && obj->IsItem())
		{
			assert(obj->GetChildCount() > 0);
			obj = obj->GetChild(0);
		}

		SelectObject(obj, true, true);
	}
	catch (const std::exception& ex)
	{
		wxLogError(ex.what());
		return false;
	}

	return true;
}

IControlElement *CVisualEditorContextForm::SearchSizerInto(IControlElement *obj)
{
	IControlElement *theSizer = NULL;

	if (obj->IsSubclassOf(wxT("boxsizer")) || obj->IsSubclassOf(wxT("wrapsizer")) ||
		obj->IsSubclassOf(wxT("staticboxsizer")) || obj->IsSubclassOf(wxT("gridsizer")))
	{
		theSizer = obj;
	}
	else
	{
		for (unsigned int i = 0; !theSizer && i < obj->GetChildCount(); i++)
			theSizer = SearchSizerInto(obj->GetChild(i));
	}

	return theSizer;

}

void CVisualEditorContextForm::ExpandObject(IControlElement* obj, bool expand)
{
	Execute(new ExpandObjectCmd(this, obj, expand));

	// collapse also all children ...
	PropagateExpansion(obj, expand, !expand);
	NotifyObjectExpanded(obj);
}

void CVisualEditorContextForm::RemoveObject(IControlElement* obj)
{
	DoRemoveObject(obj, false);
}

void CVisualEditorContextForm::CutObject(IControlElement* obj, bool force)
{
	DoRemoveObject(obj, true, force);
}

bool CVisualEditorContextForm::SelectObject(IControlElement* obj, bool force, bool notify)
{
	if ((obj == objectInspector->GetSelectedObject()) && !force) 
		return false;

	m_visualEditor->SetObjectSelect(obj); m_selObj = obj;

	if (notify) { NotifyObjectSelected(obj, force); }

	objectInspector->SelectObject(obj, this);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////

void CVisualEditorContextForm::MovePosition(IControlElement *obj, unsigned int toPos)
{
	Execute(new ShiftChildCmd(this, obj, toPos));
	NotifyProjectRefresh();
	SelectObject(obj, true);
}

void CVisualEditorContextForm::MovePosition(IControlElement *obj, bool right, unsigned int num)
{
	IControlElement *noItemObj = obj;
	IControlElement *parent = obj->GetParent();

	if (parent)
	{
		// Si el objeto está incluido dentro de un item hay que desplazar
		// el item

		while (parent && parent->IsItem())
		{
			obj = parent;
			parent = obj->GetParent();
		}

		unsigned int pos = parent->GetChildPosition(obj);

		// nos aseguramos de que los límites son correctos
		unsigned int children_count = parent->GetChildCount();

		if ((right && num + pos < children_count) ||
			(!right && (num <= pos)))
		{
			pos = (right ? pos + num : pos - num);

			Execute(new ShiftChildCmd(this, obj, pos));
			NotifyProjectRefresh();
			SelectObject(noItemObj, true);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////

void CVisualEditorContextForm::ModifyProperty(Property* prop, const wxString &oldValue)
{
	IObjectBase* object = prop->GetObject();

	if (oldValue != prop->GetValue())
	{
		Execute(new ModifyPropertyCmd(this, prop, prop->GetValue()));
		NotifyPropertyModified(prop);
	}
}

void CVisualEditorContextForm::ModifyEventHandler(Event* evt, const wxString &value)
{
	IObjectBase* object = evt->GetObject();

	if (value != evt->GetValue())
	{
		Execute(new ModifyEventHandlerCmd(this, evt, value));
		NotifyEventHandlerModified(evt);
	}
}

/////////////////////////////////////////////////////////////////////////////////////

void CVisualEditorContextForm::PropagateExpansion(IControlElement* obj, bool expand, bool up)
{
	if (obj)
	{
		if (up)
		{
			IControlElement* child = NULL;

			for (unsigned int i = 0; i < obj->GetChildCount(); i++)
			{
				child = obj->GetChild(i);

				Execute(new ExpandObjectCmd(this, child, expand));
				PropagateExpansion(child, expand, up);
			}
		}
		else
		{
			PropagateExpansion(obj->GetParent(), expand, up);
			Execute(new ExpandObjectCmd(this, obj, expand));
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////

void CVisualEditorContextForm::ResolveNameConflict(IControlElement* obj)
{
	while (obj && obj->IsItem())
	{
		if (obj->GetChildCount() > 0)
			obj = obj->GetChild(0);
		else
			return;
	}

	Property* nameProp = obj->GetProperty(wxT("name"));

	if (!nameProp)
		return;

	// Save the original name for use later.
	wxString originalName = nameProp->GetValue();

	// el nombre no puede estar repetido dentro del mismo form
	/*IControlElement* top = obj->FindNearAncestor( wxT( "form" ) );*/
	IControlElement* top = obj->GetOwnerForm();

	wxASSERT(top);

	// construimos el conjunto de nombres
	std::set<wxString> name_set;

	BuildNameSet(obj, top, name_set);

	// comprobamos si hay conflicto
	std::set<wxString>::iterator it = name_set.find(originalName);

	int i = 0;

	wxString name = originalName; // The name that gets incremented.

	while (it != name_set.end())
	{
		i++;
		name = wxString::Format(wxT("%s%i"), originalName.c_str(), i);
		it = name_set.find(name);
	}

	nameProp->SetValue(name);
	obj->SaveProperty();
}

void CVisualEditorContextForm::ResolveSubtreeNameConflicts(IControlElement* obj, IControlElement* topObj)
{
	if (!topObj)
	{
		/*topObj = obj->FindNearAncestor( wxT( "form" ) );*/
		topObj = obj->GetOwnerForm();

		if (!topObj) topObj = m_valueFrame; // object is the project
	}

	// Ignore item objects
	while (obj && obj->IsItem())
	{
		if (obj->GetChildCount() > 0)
			obj = obj->GetChild(0);
		else
			return; // error
	}

	// Resolve a possible name conflict
	ResolveNameConflict(obj);

	// Recurse through all children
	for (unsigned int i = 0; i < obj->GetChildCount(); i++)
		ResolveSubtreeNameConflicts(obj->GetChild(i), topObj);
}

void CVisualEditorContextForm::BuildNameSet(IControlElement* obj, IControlElement* top, std::set< wxString >& name_set)
{
	if (obj != top)
	{
		Property* nameProp = top->GetProperty(wxT("name"));
		if (nameProp) name_set.emplace(nameProp->GetValue());
	}

	for (unsigned int i = 0; i < top->GetChildCount(); i++) BuildNameSet(obj, top->GetChild(i), name_set);
}

/////////////////////////////////////////////////////////////////////////////////////

void CVisualEditorContextForm::DoRemoveObject(IControlElement* obj, bool cutObject, bool force)
{
	// Note:
	//  When removing an object it is important that the "item" objects
	// are not left behind
	IControlElement* parent = obj->GetParent();
	IControlElement* deleted_obj = obj;

	if (parent)
	{
		// Get the top item
		while (parent && parent->IsItem())
		{
			obj = parent;
			parent = obj->GetParent();
		}

		NotifyObjectRemoved(deleted_obj);

		if (cutObject)
		{
			m_copyOnPaste = false;
			Execute(new CutObjectCmd(this, obj, force));
		}
		else
		{
			Execute(new RemoveObjectCmd(this, obj));
		}

		SelectObject(GetSelectedObject(), true, true);
	}
	else
	{
		if (obj->GetObjectTypeName() != wxT("form"))
			assert(false);
	}
}

void CVisualEditorContextForm::DetermineObjectToSelect(IControlElement* parent, unsigned int pos)
{
	// get position of next control or last control
	IControlElement* objToSelect = NULL;
	unsigned int count = parent->GetChildCount();
	if (0 == count)
	{
		objToSelect = parent;
	}
	else
	{
		pos = (pos < count ? pos : count - 1);
		objToSelect = parent->GetChild(pos);
	}

	while (objToSelect && objToSelect->IsItem())
	{
		objToSelect = objToSelect->GetChild(0);
	}

	SelectObject(objToSelect);
}

/////////////////////////////////////////////////////////////////////////////////////

void CVisualEditorContextForm::Undo()
{
	m_cmdProc->Undo();
	m_document->Modify(!m_cmdProc->IsAtSavePoint());
	NotifyProjectRefresh();
	NotifyObjectSelected(GetSelectedObject());
}

void CVisualEditorContextForm::Redo()
{
	m_cmdProc->Redo();
	m_document->Modify(!m_cmdProc->IsAtSavePoint());
	NotifyProjectRefresh();
	NotifyObjectSelected(GetSelectedObject());
}

bool CVisualEditorContextForm::CanPasteObject()
{
	IControlElement* obj = GetSelectedObject();

	if (obj && obj->GetObjectTypeName() != wxT("form"))
		return (m_clipboard != NULL);

	return false;
}

bool CVisualEditorContextForm::CanCopyObject()
{
	IControlElement* obj = GetSelectedObject();

	if (obj && obj->GetObjectTypeName() != wxT("form"))
		return true;

	return false;
}

bool CVisualEditorContextForm::IsModified() { return m_document->IsModified(); }

IControlElement* CVisualEditorContextForm::GetSelectedObject() { return m_selObj; }

CValueFrame* CVisualEditorContextForm::GetSelectedForm() { return m_valueFrame; }

int CVisualEditorContextForm::CalcPositionOfInsertion(IControlElement*selected, IControlElement* parent)
{
	int pos = wxNOT_FOUND;

	if (parent && selected)
	{
		IControlElement* parentSelected = selected->GetParent();

		while (parentSelected && parentSelected != parent)
		{
			selected = parentSelected;
			parentSelected = selected->GetParent();
		}

		if (parentSelected && parentSelected == parent)
			pos = parent->GetChildPosition(selected) + 1;
	}

	return pos;
}

/////////////////////////////////////////////////////////////////////////////////////

void CVisualEditorContextForm::ToggleBorderFlag(IControlElement *obj, int border)
{
	if (!obj) return;

	IControlElement *parent = obj->GetParent();
	if (!parent) return;

	if (!parent->IsSubclassOf(wxT("sizerItem"))) return;

	Property *propFlag = parent->GetProperty(wxT("flag"));
	if (!propFlag) return;

	wxString value = propFlag->GetValueAsString();

	value = TypeConv::ClearFlag(wxT("wxALL"), value);
	value = TypeConv::ClearFlag(wxT("wxTOP"), value);
	value = TypeConv::ClearFlag(wxT("wxBOTTOM"), value);
	value = TypeConv::ClearFlag(wxT("wxRIGHT"), value);
	value = TypeConv::ClearFlag(wxT("wxLEFT"), value);

	int intVal = propFlag->GetValueAsInteger();
	intVal ^= border;

	if ((intVal & wxALL) == wxALL)
	{
		value = TypeConv::SetFlag(wxT("wxALL"), value);
	}
	else
	{
		if ((intVal & wxTOP) != 0) value = TypeConv::SetFlag(wxT("wxTOP"), value);
		if ((intVal & wxBOTTOM) != 0) value = TypeConv::SetFlag(wxT("wxBOTTOM"), value);
		if ((intVal & wxRIGHT) != 0) value = TypeConv::SetFlag(wxT("wxRIGHT"), value);
		if ((intVal & wxLEFT) != 0) value = TypeConv::SetFlag(wxT("wxLEFT"), value);
	}

	ModifyProperty(propFlag, value);
}

void CVisualEditorContextForm::CreateBoxSizerWithObject(IControlElement* obj)
{
	IControlElement* parent = obj->GetParent();
	if (!parent) return;

	IControlElement* grandParent = parent->GetParent();
	if (!grandParent) return;

	int childPos = -1;
	if (parent->IsSubclassOf(wxT("sizerItem")))
	{
		childPos = (int)grandParent->GetChildPosition(parent);
		parent = grandParent;
	}

	// Must first cut the old object in case it is the only allowable object
	IControlElement* clipboard = m_clipboard;

	CutObject(obj);

	// Create the wxBoxSizer
	IControlElement* newSizer = visualEditorDatabase->CreateObject(wxT("boxSizer"), parent);

	if (newSizer)
	{
		Execute(new InsertObjectCmd(this, newSizer, parent, childPos));

		if (newSizer->GetObjectTypeName() == wxT("sizerItem"))
			newSizer = newSizer->GetChild(0);

		PasteObject(newSizer);
		m_clipboard = clipboard;

		NotifyProjectRefresh();
	}
	else
	{
		Undo();
		m_clipboard = clipboard;
	}
}
