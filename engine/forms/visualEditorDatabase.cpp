////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko, wxFormBuilder
//	Description : visual editor 
////////////////////////////////////////////////////////////////////////////

#include "visualEditor.h"

#include "common/objectbase.h"
#include "utils/typeconv.h"

#include "utils/stringutils.h"

/////////////////////////////////////////////////////////////////////////////////

#define OBJINFO_TAG "objectinfo"
#define CODEGEN_TAG "codegen"
#define TEMPLATE_TAG "template"
#define NAME_TAG "name"
#define DESCRIPTION_TAG "help"
#define CUSTOM_EDITOR_TAG "editor"
#define SYSTEM_TAG "system"
#define SYSTEM_SIZER_TAG "system_sizer"
#define PROPERTY_TAG "property"
#define PROPERTY_SIZER_TAG "property_sizer"
#define CHILD_TAG "child"
#define EVENT_TAG "event"
#define EVENT_SIZER_TAG "event_sizer"
#define EVENT_CLASS_TAG "class"
#define CATEGORY_TAG "category"
#define OBJECT_TAG "object"
#define CLASS_TAG "class"
#define PACKAGE_TAG "package"
#define PKGDESC_TAG "desc"
#define PRGLANG_TAG "language"
#define ICON_TAG "icon"
#define SMALL_ICON_TAG "smallIcon"
#define EXPANDED_TAG "expanded"

CVisualEditorDatabase* CVisualEditorDatabase::s_instance = NULL;

CVisualEditorDatabase* CVisualEditorDatabase::Get()
{
	if (!s_instance)
		s_instance = new CVisualEditorDatabase();

	return s_instance;
}

void CVisualEditorDatabase::Destroy()
{
	wxDELETE(s_instance);
}

/////////////////////////////////////////////////////////////////////////////////

IControlElement *CVisualEditorDatabase::NewObject(const wxString &objInfo, IControlElement *controlParent)
{
	IControlElement *m_objBase = CValue::CreateAndConvertObjectRef< IControlElement *>(objInfo);
	wxASSERT(m_objBase);
	m_objBase->ReadProperty();

	if (controlParent)
	{
		controlParent->AddChild(controlParent);
		m_objBase->SetParent(controlParent);
	}

	if (objInfo != wxT("frame"))
		m_objBase->IncrRef();

	return m_objBase;
}

wxString CVisualEditorDatabase::GetClassType(const wxString &className)
{
	IControlValueAbstract *objectSingle = dynamic_cast<IControlValueAbstract *>(CValue::GetAvailableObject(className));
	wxASSERT(objectSingle);
	return objectSingle->GetControlType();
}

CValueFrame *CVisualEditorDatabase::CreateFrame(ticpp::Element* xml_obj, bool isEnabled)
{
	try
	{
		std::string className;
		xml_obj->GetAttribute(CLASS_TAG, &className, false);

		CValueFrame *newobject = wxStaticCast(CreateObject(className), CValueFrame);
		wxASSERT(newobject);
		newobject->SetReadOnly(isEnabled);

		// It is possible the CreateObject returns an "item" containing the object, e.g. SizerItem or SplitterItem
		// If that is the case, reassign "object" to the actual object
		IControlElement * object = newobject;
		if (object && object->GetChildCount() > 0)
		{
			object = object->GetChild(0);
		}

		IControlElement *sizeritembase = newobject && newobject->GetChildCount() > 0 ? newobject : NULL;

		if (object)
		{
			// Get the state of expansion in the object tree
			bool expanded;
			xml_obj->GetAttributeOrDefault(EXPANDED_TAG, &expanded, true);
			object->SetExpanded(expanded);

			// Load the system data 
			ticpp::Element* xml_prop = xml_obj->FirstChildElement(SYSTEM_TAG, false);
			while (xml_prop)
			{
				std::string sys_name, sys_value;

				xml_prop->GetAttribute(NAME_TAG, &sys_name, false);
				xml_prop->GetText(&sys_value, false);

				unsigned int m_obj_id = wxAtol(sys_value);
				object->SetControlID(m_obj_id);

				xml_prop = xml_prop->NextSiblingElement(SYSTEM_TAG, false);
			}

			// Load the properties
			xml_prop = xml_obj->FirstChildElement(PROPERTY_TAG, false);
			while (xml_prop)
			{
				std::string prop_name;
				xml_prop->GetAttribute(NAME_TAG, &prop_name, false);
				Property *prop = object->GetProperty(_WXSTR(prop_name));

				if (prop) // does the property exist
				{
					// load the value
					prop->SetValue(_WXSTR(xml_prop->GetText(false)));
				}
				else
				{
					std::string value = xml_prop->GetText(false);
					if (!value.empty())
					{
						wxLogError(wxT("The property named \"%s\" of class \"%s\" is not supported by this version of Enterprise.\n")
							wxT("If your project file was just converted from an older version, then the conversion was not complete.\n")
							wxT("Otherwise, this project is from a newer version of Enterprise.\n\n")
							wxT("The property's value is: %s\n")
							wxT("If you save this project, YOU WILL LOSE DATA"), _WXSTR(prop_name).c_str(), _WXSTR(className).c_str(), _WXSTR(value).c_str());
					}
				}

				xml_prop = xml_prop->NextSiblingElement(PROPERTY_TAG, false);
			}

			xml_prop = xml_obj->FirstChildElement(SYSTEM_SIZER_TAG, false);
			while (xml_prop)
			{
				std::string sys_name, sys_value;

				xml_prop->GetAttribute(NAME_TAG, &sys_name, false);
				xml_prop->GetText(&sys_value, false);

				unsigned int m_obj_id = wxAtol(sys_value);
				sizeritembase->SetControlID(m_obj_id);

				xml_prop = xml_prop->NextSiblingElement(SYSTEM_SIZER_TAG, false);
			}

			xml_prop = xml_obj->FirstChildElement(PROPERTY_SIZER_TAG, false);

			while (xml_prop)
			{
				std::string prop_name;
				xml_prop->GetAttribute(NAME_TAG, &prop_name, false);
				Property *prop = sizeritembase->GetProperty(_WXSTR(prop_name));

				if (prop) // does the property exist
				{
					// load the value
					prop->SetValue(_WXSTR(xml_prop->GetText(false)));
				}
				else
				{
					std::string value = xml_prop->GetText(false);
					if (!value.empty())
					{
						wxLogError(wxT("The property named \"%s\" of class \"%s\" is not supported by this version of Enterprise.\n")
							wxT("If your project file was just converted from an older version, then the conversion was not complete.\n")
							wxT("Otherwise, this project is from a newer version of Enterprise.\n\n")
							wxT("The property's value is: %s\n")
							wxT("If you save this project, YOU WILL LOSE DATA"), _WXSTR(prop_name).c_str(), _WXSTR(className).c_str(), _WXSTR(value).c_str());
					}
				}

				xml_prop = xml_prop->NextSiblingElement(PROPERTY_SIZER_TAG, false);
			}

			// load the event handlers
			ticpp::Element* xml_event = xml_obj->FirstChildElement(EVENT_TAG, false);
			while (xml_event)
			{
				std::string event_name;
				xml_event->GetAttribute(NAME_TAG, &event_name, false);
				Event *event = object->GetEvent(_WXSTR(event_name));
				if (event)
				{
					event->SetValue(_WXSTR(xml_event->GetText(false)));
				}

				xml_event = xml_event->NextSiblingElement(EVENT_TAG, false);
			}

			xml_event = xml_obj->FirstChildElement(EVENT_SIZER_TAG, false);
			while (xml_event)
			{
				std::string event_name;
				xml_event->GetAttribute(NAME_TAG, &event_name, false);
				Event *event = sizeritembase->GetEvent(_WXSTR(event_name));
				if (event)
				{
					event->SetValue(_WXSTR(xml_event->GetText(false)));
				}

				xml_event = xml_event->NextSiblingElement(EVENT_SIZER_TAG, false);
			}

			//save property in memory 
			if (sizeritembase) sizeritembase->SaveProperty();
			object->SaveProperty();

			// create the children
			ticpp::Element* child = xml_obj->FirstChildElement(OBJECT_TAG, false);
			while (child)
			{
				CreateObject(child, object);
				child = child->NextSiblingElement(OBJECT_TAG, false);
			}
		}

		return newobject;
	}
	catch (ticpp::Exception&)
	{
		return NULL;
	}
}

IControlElement *CVisualEditorDatabase::CreateObject(const wxString &className, IControlElement *parent)
{
	IControlElement *object = NULL;
	wxString classType = GetClassType(className);

	if (parent)
	{
		bool bSizer = false;

		if (classType == wxT("form")) bSizer = true;
		else if (classType == wxT("sizer")) bSizer = parent->GetObjectTypeName() == wxT("sizer") || parent->GetObjectTypeName() == wxT("form") ? false : true;

		//FIXME! Esto es un parche para evitar crear los tipos menubar,statusbar y
		//toolbar en un form que no sea wxFrame.
		//Hay que modificar el conjunto de tipos para permitir tener varios tipos
		//de forms (como childType de project), pero hay mucho código no válido
		//para forms que no sean de tipo "form". Dicho de otra manera, hay
		//código que dependen del nombre del tipo, cosa que hay que evitar.
		if (parent->GetObjectTypeName() == wxT("form") && parent->GetClassName() != wxT("frame") &&
			(classType == wxT("statusbar") ||
				classType == wxT("menubar") ||
				classType == wxT("ribbonbar") ||
				classType == wxT("toolbar")))

			return NULL; // tipo no válido

		// No menu dropdown for wxToolBar until wx 2.9 :(
		if (parent->GetObjectTypeName() == wxT("tool"))
		{
			IControlElement *gParent = parent->GetParent();

			if (
				(gParent->GetClassName() == wxT("toolbar")) &&
				(className == wxT("menu"))
				)
				return NULL; // not a valid type
		}

		if (parent->GetComponentType() == COMPONENT_TYPE_FRAME ||
			parent->GetComponentType() == COMPONENT_TYPE_SIZER ||
			parent->GetComponentType() == COMPONENT_TYPE_SIZERITEM)
		{
			CValueSizerItem *sizerItem = wxDynamicCast(NewObject("SizerItem"), CValueSizerItem);
			IControlElement *obj = NewObject(className);

			if (parent) {
				sizerItem->SetReadOnly(parent->IsEditable());
			}

			//set frame owner for this situation 
			sizerItem->m_frameOwner = parent->GetOwnerForm();

			// la siguiente condición debe cumplirse siempre
			// ya que un item debe siempre contener a otro objeto
			if (obj)
			{
				//set enabled item
				obj->SetReadOnly(sizerItem->IsEditable());

				// enlazamos item y obj
				sizerItem->AddChild(obj);
				obj->SetParent(sizerItem);

				// sizerItem es un tipo de objeto reservado, para que el uso sea
				// más práctico se asignan unos valores por defecto en función
				// del tipo de objeto creado
				if (sizerItem->IsSubclassOf(wxT("sizerItem")))
					SetDefaultLayoutProperties(sizerItem);

				object = sizerItem;
			}
		}
		else if (parent->GetObjectTypeName() == wxT("toolbar"))
		{
			if (classType == wxT("tool") || classType == wxT("widget"))
			{
				object = NewObject(className);
				//set enabled item
				object->SetReadOnly(parent->IsEditable());
			}
		}
		else if (parent->GetObjectTypeName() == wxT("notebook"))
		{
			if (classType == wxT("page"))
			{
				object = NewObject(className);
				//set enabled item
				object->SetReadOnly(parent->IsEditable());
			}
		}
		else if (parent->GetObjectTypeName() == wxT("container")
			&& parent->GetClassName() == wxT("tablebox"))
		{
			if (classType == wxT("tablebox_column"))
			{
				object = NewObject(className);
				//set enabled item
				object->SetReadOnly(parent->IsEditable());
			}
		}
		else if (parent->GetObjectTypeName() == wxT("page"))
		{
			CValueSizerItem *sizerItem = wxDynamicCast(NewObject("SizerItem"), CValueSizerItem);
			IControlElement *obj = NewObject(className);

			if (parent) {
				sizerItem->SetReadOnly(parent->IsEditable());
			}

			//set frame owner for this situation 
			sizerItem->m_frameOwner = parent->GetOwnerForm();

			// la siguiente condición debe cumplirse siempre
			// ya que un item debe siempre contener a otro objeto
			if (obj)
			{
				//set enabled item
				obj->SetReadOnly(sizerItem->IsEditable());

				// enlazamos item y obj
				sizerItem->AddChild(obj);
				obj->SetParent(sizerItem);

				// sizerItem es un tipo de objeto reservado, para que el uso sea
				// más práctico se asignan unos valores por defecto en función
				// del tipo de objeto creado
				if (sizerItem->IsSubclassOf(wxT("sizerItem")))
					SetDefaultLayoutProperties(sizerItem);

				object = sizerItem;
			}

		}
	}
	else // parent == NULL;
	{
		object = NewObject(className);
	}

	return object;
}

IControlElement *CVisualEditorDatabase::CreateObject(ticpp::Element* xml_obj, IControlElement *parent)
{
	try
	{
		std::string className;
		xml_obj->GetAttribute(CLASS_TAG, &className, false);

		IControlElement *newobject = CreateObject(className, parent);

		// It is possible the CreateObject returns an "item" containing the object, e.g. SizerItem or SplitterItem
		// If that is the case, reassign "object" to the actual object
		IControlElement * object = newobject;
		if (object && object->GetChildCount() > 0)
		{
			object = object->GetChild(0);
		}

		IControlElement *sizeritembase = newobject && newobject->GetChildCount() > 0 ? newobject : NULL;

		if (object)
		{
			// Get the state of expansion in the object tree
			bool expanded;
			xml_obj->GetAttributeOrDefault(EXPANDED_TAG, &expanded, true);
			object->SetExpanded(expanded);

			// Load the system data 
			ticpp::Element* xml_prop = xml_obj->FirstChildElement(SYSTEM_TAG, false);
			while (xml_prop)
			{
				std::string sys_name, sys_value;

				xml_prop->GetAttribute(NAME_TAG, &sys_name, false);
				xml_prop->GetText(&sys_value, false);

				unsigned int m_obj_id = wxAtol(sys_value);
				object->SetControlID(m_obj_id);

				xml_prop = xml_prop->NextSiblingElement(SYSTEM_TAG, false);
			}

			// Load the properties
			xml_prop = xml_obj->FirstChildElement(PROPERTY_TAG, false);
			while (xml_prop)
			{
				std::string prop_name;
				xml_prop->GetAttribute(NAME_TAG, &prop_name, false);
				Property *prop = object->GetProperty(_WXSTR(prop_name));

				if (prop) // does the property exist
				{
					// load the value
					prop->SetValue(_WXSTR(xml_prop->GetText(false)));
				}
				else
				{
					std::string value = xml_prop->GetText(false);
					if (!value.empty())
					{
						wxLogError(wxT("The property named \"%s\" of class \"%s\" is not supported by this version of Enterprise.\n")
							wxT("If your project file was just converted from an older version, then the conversion was not complete.\n")
							wxT("Otherwise, this project is from a newer version of Enterprise.\n\n")
							wxT("The property's value is: %s\n")
							wxT("If you save this project, YOU WILL LOSE DATA"), _WXSTR(prop_name).c_str(), _WXSTR(className).c_str(), _WXSTR(value).c_str());
					}
				}

				xml_prop = xml_prop->NextSiblingElement(PROPERTY_TAG, false);
			}

			xml_prop = xml_obj->FirstChildElement(SYSTEM_SIZER_TAG, false);
			while (xml_prop)
			{
				std::string sys_name, sys_value;

				xml_prop->GetAttribute(NAME_TAG, &sys_name, false);
				xml_prop->GetText(&sys_value, false);

				unsigned int m_obj_id = wxAtol(sys_value);
				sizeritembase->SetControlID(m_obj_id);

				xml_prop = xml_prop->NextSiblingElement(SYSTEM_SIZER_TAG, false);
			}

			xml_prop = xml_obj->FirstChildElement(PROPERTY_SIZER_TAG, false);

			while (xml_prop)
			{
				std::string prop_name;
				xml_prop->GetAttribute(NAME_TAG, &prop_name, false);
				Property *prop = sizeritembase->GetProperty(_WXSTR(prop_name));

				if (prop) // does the property exist
				{
					// load the value
					prop->SetValue(_WXSTR(xml_prop->GetText(false)));
				}
				else
				{
					std::string value = xml_prop->GetText(false);
					if (!value.empty())
					{
						wxLogError(wxT("The property named \"%s\" of class \"%s\" is not supported by this version of Enterprise.\n")
							wxT("If your project file was just converted from an older version, then the conversion was not complete.\n")
							wxT("Otherwise, this project is from a newer version of Enterprise.\n\n")
							wxT("The property's value is: %s\n")
							wxT("If you save this project, YOU WILL LOSE DATA"), _WXSTR(prop_name).c_str(), _WXSTR(className).c_str(), _WXSTR(value).c_str());
					}
				}

				xml_prop = xml_prop->NextSiblingElement(PROPERTY_SIZER_TAG, false);
			}

			// load the event handlers
			ticpp::Element* xml_event = xml_obj->FirstChildElement(EVENT_TAG, false);
			while (xml_event)
			{
				std::string event_name;
				xml_event->GetAttribute(NAME_TAG, &event_name, false);
				Event *event = object->GetEvent(_WXSTR(event_name));
				if (event)
				{
					event->SetValue(_WXSTR(xml_event->GetText(false)));
				}

				xml_event = xml_event->NextSiblingElement(EVENT_TAG, false);
			}

			xml_event = xml_obj->FirstChildElement(EVENT_SIZER_TAG, false);
			while (xml_event)
			{
				std::string event_name;
				xml_event->GetAttribute(NAME_TAG, &event_name, false);
				Event *event = sizeritembase->GetEvent(_WXSTR(event_name));
				if (event)
				{
					event->SetValue(_WXSTR(xml_event->GetText(false)));
				}

				xml_event = xml_event->NextSiblingElement(EVENT_SIZER_TAG, false);
			}

			if (parent)
			{
				// set up parent/child relationship
				parent->AddChild(newobject);
				newobject->SetParent(parent);
			}

			//save property in memory 
			if (sizeritembase) sizeritembase->SaveProperty();
			object->SaveProperty();

			// create the children
			ticpp::Element* child = xml_obj->FirstChildElement(OBJECT_TAG, false);
			while (child)
			{
				CreateObject(child, object);
				child = child->NextSiblingElement(OBJECT_TAG, false);
			}
		}

		return newobject;
	}
	catch (ticpp::Exception&)
	{
		return NULL;
	}
}

IControlElement * CVisualEditorDatabase::CopyObject(IControlElement *obj)
{
	assert(obj);
	IControlElement *copyObj = NewObject(obj->GetClassName()); // creamos la copia
	assert(copyObj);

	// copiamos las propiedades
	unsigned int i;
	unsigned int count = obj->GetPropertyCount();
	for (i = 0; i < count; i++)
	{
		Property *objProp = obj->GetProperty(i);
		assert(objProp);

		if (objProp->GetName() == wxT("name")) continue;

		Property *copyProp = copyObj->GetProperty(objProp->GetName());
		assert(copyProp);

		wxString propValue = objProp->GetValue();
		copyProp->SetValue(propValue);
	}

	// ...and the event handlers
	count = obj->GetEventCount();
	for (i = 0; i < count; i++)
	{
		Event *event = obj->GetEvent(i);
		Event *copyEvent = copyObj->GetEvent(event->GetName());
		copyEvent->SetValue(event->GetValue());
	}

	copyObj->SetParent(obj->GetParent());
	copyObj->GenerateNewID();

	// creamos recursivamente los hijos
	count = obj->GetChildCount();
	for (i = 0; i < count; i++)
	{
		IControlElement *childCopy = CopyObject(obj->GetChild(i));
		copyObj->AddChild(childCopy);

		childCopy->SetParent(copyObj);
		childCopy->SaveProperty();
	}

	copyObj->SaveProperty();

	return copyObj;
}

////////////////////////////////////////////////////////////////////////////

int CVisualEditorDatabase::CountChildrenWithSameType(IControlElement *parent, const wxString &type)
{
	unsigned int count = 0;
	unsigned int numChildren = parent->GetChildCount();
	for (unsigned int i = 0; i < numChildren; i++)
	{
		if (type == parent->GetChild(i)->GetObjectTypeName())
			count++;
	}

	return count;
}

void CVisualEditorDatabase::SetDefaultLayoutProperties(CValueSizerItem *sizerItem)
{
	IControlElement *child = sizerItem->GetChild(0);
	wxString obj_type = child->GetObjectTypeName();

	if (obj_type == wxT("sizer"))
	{
		sizerItem->m_flag_border = wxALL;
		sizerItem->m_flag_state = wxEXPAND;
	}
	else if (child->GetClassName() == wxT("splitter") ||
		child->GetClassName() == wxT("spacer"))
	{
		sizerItem->m_proportion = 1;
		sizerItem->m_flag_state = wxEXPAND;
	}
	else if (child->GetClassName() == wxT("staticline"))
	{
		sizerItem->m_flag_border = wxALL;
		sizerItem->m_flag_state = wxEXPAND;
	}
	else if (child->GetClassName() == wxT("toolbar"))
	{
		sizerItem->m_flag_border = wxDOWN;
		sizerItem->m_flag_state = wxEXPAND;
	}
	else if (child->GetClassName() == wxT("notebook"))
	{
		sizerItem->m_proportion = 1;
		sizerItem->m_flag_border = wxALL;
		sizerItem->m_flag_state = wxEXPAND;
	}
	else if (obj_type == wxT("widget") ||
		obj_type == wxT("statusbar"))
	{
		sizerItem->m_proportion = 0;
		sizerItem->m_flag_border = wxALL;
		sizerItem->m_flag_state = wxSHRINK;
	}
	else if (obj_type == wxT("container"))
	{
		sizerItem->m_proportion = 1;
		sizerItem->m_flag_border = wxALL;
		sizerItem->m_flag_state = wxEXPAND;
	}

	sizerItem->ReadProperty();
}

/////////////////////////////////////////////////////////////////////////////////

#include "elements/frame.h"
#include "elements/sizers.h"
#include "elements/widgets.h"
#include "elements/toolbar.h"
#include "elements/notebook.h"

#include "elements/tableBox.h"
#include "elements/gridbox.h"
#include "elements/htmlbox.h"
#include "elements/chartBox.h"

#include "elements/icons/icons.h"

CONTROL_VALUE_REGISTER(CValueFrame, "frame", "form", NULL, TEXT2CLSID("CT_FRME"));
//sizer 
CONTROL_VALUE_REGISTER(CValueSizerItem, "sizerItem", "sizer", NULL, TEXT2CLSID("CT_SIZR"));
CONTROL_VALUE_REGISTER(CValueBoxSizer, "boxsizer", "sizer", sizer_2_xpm, TEXT2CLSID("CT_BSZR"));
CONTROL_VALUE_REGISTER(CValueWrapSizer, "wrapsizer", "sizer", wrap_sizer_xpm, TEXT2CLSID("CT_WSZR"));
CONTROL_VALUE_REGISTER(CValueStaticBoxSizer, "staticboxsizer", "sizer", staticbox_sizer_xpm, TEXT2CLSID("CT_SSZER"));
CONTROL_VALUE_REGISTER(CValueGridSizer, "gridsizer", "sizer", grid_sizer_xpm, TEXT2CLSID("CT_GSZR"));
//widgets  
CONTROL_VALUE_REGISTER(CValueButton, "button", "widget", button_xpm, TEXT2CLSID("CT_BUTN"));
CONTROL_VALUE_REGISTER(CValueStaticText, "statictext", "widget", static_text_xpm, TEXT2CLSID("CT_STTX"));
CONTROL_VALUE_REGISTER(CValueTextCtrl, "textctrl", "widget", text_ctrl_xpm, TEXT2CLSID("CT_TXTC"));
CONTROL_VALUE_REGISTER(CValueComboBox, "combobox", "widget", combo_box_xpm, TEXT2CLSID("CT_CMBB"));
CONTROL_VALUE_REGISTER(CValueChoice, "choice", "widget", choice_xpm, TEXT2CLSID("CT_CHOI"));
CONTROL_VALUE_REGISTER(CValueListBox, "listbox", "widget", list_box_xpm, TEXT2CLSID("CT_LSTB"));
CONTROL_VALUE_REGISTER(CValueCheckbox, "checkbox", "widget", checkbox_xpm, TEXT2CLSID("CT_CHKB"));
CONTROL_VALUE_REGISTER(CValueRadioButton, "radiobutton", "widget", radio_button_xpm, TEXT2CLSID("CT_RDBT"));
CONTROL_VALUE_REGISTER(CValueStaticLine, "staticline", "widget", static_line_xpm, TEXT2CLSID("CT_STLI"));
CONTROL_VALUE_REGISTER(CValueSlider, "slider", "widget", slider_xpm, TEXT2CLSID("CT_SLID"));
CONTROL_VALUE_REGISTER(CValueGauge, "gauge", "widget", gauge_xpm, TEXT2CLSID("CT_GAUG"));
//toolbar
CONTROL_VALUE_REGISTER(CValueToolbar, "toolbar", "toolbar", toolbar_xpm, TEXT2CLSID("CT_TLBR"));
CONTROL_VALUE_REGISTER(CValueToolBarItem, "tool", "tool", NULL, TEXT2CLSID("CT_TLIT"));
CONTROL_VALUE_REGISTER(CValueToolBarSeparator, "toolSeparator", "tool", NULL, TEXT2CLSID("CT_TLIS"));
//notebook
CONTROL_VALUE_REGISTER(CValueNotebook, "notebook", "notebook", notebook_xpm, TEXT2CLSID("CT_NTBK"));
CONTROL_VALUE_REGISTER(CValueNotebookPage, "page", "page", NULL, TEXT2CLSID("CT_NTPG"));
//tablebox
CONTROL_VALUE_REGISTER(CValueTableBox, "tablebox", "tablebox", list_ctrl_xpm, TEXT2CLSID("CT_TABL"));
CONTROL_VALUE_REGISTER(CValueTableBoxColumn, "tablebox_column", "tablebox_column", NULL, TEXT2CLSID("CT_TBLC"));
//grid 
CONTROL_VALUE_REGISTER(CValueGridBox, "gridbox", "gridbox", grid_xpm, TEXT2CLSID("CT_GRID"));
//html 
CONTROL_VALUE_REGISTER(CValueHTMLBox, "htmlbox", "htmlbox", htmlwin_xpm, TEXT2CLSID("CT_HTML"));
//chart 
CONTROL_VALUE_REGISTER(CValueChartBox, "chartbox", "chartbox", static_bitmap_xpm, TEXT2CLSID("CT_CHRB"));
