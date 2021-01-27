#ifndef _BASE_CONTROL_H_
#define _BASE_CONTROL_H_

#include <wx/wx.h>
#include <set>

class CProcUnit;

#include "common/ownerInfo.h"
#include "forms/visualEditorBase.h"
#include "utils/tinyxml/ticpp.h"
#include "common/objectbase.h"

class IDataObjectSource;
class IDataObjectValue;
class IDataObjectList;

class CValueFrame;

class CVisualEditor;
class CVisualView;

#include "common/actionInfo.h"

class IControlElement : public CValue,
	public IObjectBase, public IActionSource, public IOwnerInfo
{
	wxDECLARE_ABSTRACT_CLASS(IControlElement);

protected:

	bool m_expanded = true; // is expanded in the object tree, allows for saving to file
	unsigned int m_obj_id;

protected:

	//object of methods 
	CMethods *m_methods;

	//frame owner 
	CValueFrame* m_frameOwner;

public:

	IControlElement();
	virtual ~IControlElement();

	// Gets the parent object
	IControlElement* GetParent() const { return wxDynamicCast(m_parent, IControlElement); }

	/**
	* Obtiene un hijo del objeto.
	*/
	IControlElement* GetChild(unsigned int idx);
	IControlElement* GetChild(unsigned int idx, const wxString& type);

	IControlElement* FindNearAncestor(const wxString &type) { return wxDynamicCast(IObjectBase::FindNearAncestor(type), IControlElement); }
	IControlElement* FindNearAncestorByBaseClass(const wxString &type) { return wxDynamicCast(IObjectBase::FindNearAncestorByBaseClass(type), IControlElement); }

	/**
	* Support generate id
	*/
	virtual void GenerateNewID();

	/**
	* Support get/set object id
	*/
	virtual void SetControlID(unsigned int obj_id) { m_obj_id = obj_id; }
	virtual unsigned int GetControlID() { return m_obj_id; }

	/**
	* Find by control id
	*/
	IControlElement *FindControlByID(unsigned int obj_id);

	/**
	* Support form
	*/
	virtual CValueFrame* GetOwnerForm() const { return m_frameOwner; }

	/**
	* Support default menu
	*/
	virtual void PrepareDefaultMenu(wxMenu *m_menu) {}
	virtual void ExecuteMenu(IVisualHost *visualHost, int id) {}

	/*
	* Set parent
	*/
	virtual void SetParent(IControlElement* parent);

	/**
	* Control generate attribute in valueFrame
	*/
	virtual bool IsComponentObject() { return false; }

	/**
	* Get wxObject from visual view (if exist)
	*/
	wxObject *GetWxObject();

	/**
	* Get type form
	*/
	int GetTypeForm();

	/**
	Sets whether the object is expanded in the object tree or not.
	*/
	void SetExpanded(bool expanded) { m_expanded = expanded; }

	/**
	Gets whether the object is expanded in the object tree or not.
	*/
	bool GetExpanded() { return m_expanded; }

	/**
	* Comprueba si el tipo de objeto pasado es válido como hijo del objeto.
	* Esta rutina es importante, ya que define las restricciónes de ubicación.
	*/

	bool IsContainer() { return (GetObjectTypeName() == wxT("container")); }

	/**
	* Get layout
	*/

	IControlElement* GetLayout();

	/**
	* Set property data
	*/
	virtual void SetPropertyData(Property *property, const CValue &srcValue);

	/**
	* Get property data
	*/
	virtual CValue GetPropertyData(Property *property); 

public:

	/**
	* Create an instance of the wxObject and return a pointer
	*/
	virtual wxObject* Create(wxObject* parent, IVisualHost *visualHost) { return new wxNoObject; };

	/**
	* Allows components to do something after they have been created.
	* For example, Abstract components like NotebookPage and SizerItem can
	* add the actual widget to the Notebook or sizer.
	*
	* @param wxobject The object which was just created.
	* @param wxparent The wxWidgets parent - the wxObject that the created object was added to.
	*/
	virtual void OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost) {};

	/**
	* Allows components to respond when selected in object tree.
	* For example, when a wxNotebook's page is selected, it can switch to that page
	*/
	virtual void OnSelected(wxObject* wxobject) {};

	/**
	* Allows components to do something after they have been updated.
	*/
	virtual void Update(wxObject* wxobject, IVisualHost *visualHost) {};

	/**
	* Allows components to do something after they have been updated.
	* For example, Abstract components like NotebookPage and SizerItem can
	* add the actual widget to the Notebook or sizer.
	*
	* @param wxobject The object which was just updated.
	* @param wxparent The wxWidgets parent - the wxObject that the updated object was added to.
	*/
	virtual void OnUpdated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost) {};

	/**
	 * Cleanup (do the reverse of Create)
	 */
	virtual void Cleanup(wxObject* obj, IVisualHost *visualHost) {};

protected:

	virtual bool ProcessEvent(const wxString &sEventName);

public:

	//support owner 
	virtual void SendOwnerValue(CValue &cVal) override {}

	//read&save propery 
	virtual void ReadProperty() override;
	virtual void SaveProperty() override;

	//support actions 
	virtual CAction GetActions(form_identifier_t formType) { return CAction(); }
	virtual void AddActions(CAction &actions, form_identifier_t formType) {}
	virtual void ExecuteAction(unsigned int action) {}

	//is item 
	virtual bool IsItem() = 0;

public:

	//runtime 
	virtual CProcUnit *GetFormProcUnit() const;

	//methods 
	virtual CMethods* GetPMethods() const { PrepareNames();  return m_methods; }; //получить ссылку на класс помощник разбора имен атрибутов и методов
	virtual void PrepareNames() const;                         //этот метод автоматически вызывается для инициализации имен атрибутов и методов
	virtual CValue Method(CMethodParameters &aParams);       //вызов метода

	//attributes 
	virtual void SetAttribute(CAttributeParameters &aParams, CValue &cVal);        //установка атрибута
	virtual CValue GetAttribute(CAttributeParameters &aParams);                   //значение атрибута
	virtual int FindAttribute(const wxString &sName) const;

	virtual wxString GetTypeString() const;
	virtual wxString GetString() const;

	//check is empty
	virtual inline bool IsEmpty() const override { return false; }

public:

	//generation object 
	void SerializeObject(ticpp::Element* serializedElement);
	void Serialize(ticpp::Document* serializedDocument);
};

#endif // !_BASE_H_
