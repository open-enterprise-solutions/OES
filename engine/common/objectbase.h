#ifndef _OBJECTBASE_H__
#define _OBJECTBASE_H__

#include "common/formdefs.h"
#include "common/types.h"
#include "utils/fontcontainer.h"

struct Option
{
	wxString m_option;
	wxString m_description;
	double m_dblVal;

	Option(const wxString strOpt, double d) : m_option(strOpt), m_dblVal(d) {}

	operator double() { return m_dblVal; }
};

#include <set>

class OptionList
{
	std::vector< Option > m_options;

public:

	void AddOption(const wxString &option, double d) { m_options.emplace_back(option, d); }
	unsigned int GetOptionCount() { return (unsigned int)m_options.size(); }
	const std::vector< Option >& GetOptions() { return m_options; }
};

///////////////////////////////////////////////////////////////////////////////

class Property
{
protected:

	wxString        m_name;
	PropertyType    m_type;
	wxString		m_description;
	IObjectBase*    m_object; // pointer to the owner object
	wxString        m_value;

protected:

	Property() {}

public:

	Property(const wxString &name, PropertyType type, IObjectBase* obj)
		: m_name(name), m_type(type), m_object(obj)
	{
	}

	bool IsEditable();

	IObjectBase* GetObject() { return m_object; }
	wxString GetName() { return m_name; }
	wxString GetValue() { return m_value; }
	PropertyType GetType() { return m_type; }
	wxString GetDescription() { return m_description; }

	////////////////////

	void SetValue(wxString& val) { m_value = val; }
	void SetValue(const wxChar* val) { m_value = val; }

	////////////////////

	virtual void SetValue(const wxArrayString &str);
	virtual void SetValue(const wxFontContainer &font);
	virtual void SetValue(const wxColour &colour);
	virtual void SetValue(const wxString &str, bool format = false);
	virtual void SetValue(const wxPoint &point);
	virtual void SetValue(const wxSize &size);
	virtual void SetValue(const int integer);
	virtual void SetValue(const long long integer);
	virtual void SetValue(const double val);

	virtual wxFontContainer GetValueAsFont();
	virtual wxColour GetValueAsColour();
	virtual wxPoint  GetValueAsPoint();
	virtual wxSize   GetValueAsSize();
	virtual int      GetValueAsInteger();
	virtual wxString GetValueAsString();
	virtual wxBitmap GetValueAsBitmap();
	virtual wxString GetValueAsText();   // sustituye los ('\n',...) por ("\\n",...)
	virtual wxArrayString GetValueAsArrayString();
	virtual double GetValueAsFloat();

	////////////////////

	virtual OptionList*  GetOptionList() { return NULL; }
	virtual bool IsNull();
};

class PropertyOption : public Property
{
public:

	PropertyOption(const wxString &name, OptionList* opt_list, IObjectBase* obj)
		: Property(name, PropertyType::PT_OPTION, obj), m_opt_list(opt_list)
	{
	};

	void SetOptionList(OptionList* optlist) { m_opt_list = optlist; };
	virtual OptionList*  GetOptionList() override { return m_opt_list; }

	virtual ~PropertyOption() { if (m_opt_list) delete m_opt_list; }

protected:

	OptionList*     m_opt_list;
};

class PropertyBitlist : public Property
{
public:

	PropertyBitlist(const wxString &name, OptionList* opt_list, IObjectBase* obj)
		: Property(name, PropertyType::PT_BITLIST, obj), m_opt_list(opt_list)
	{
	};

	void SetOptionList(OptionList* optlist) { m_opt_list = optlist; };
	virtual OptionList*  GetOptionList() override { return m_opt_list; }

	virtual ~PropertyBitlist() { if (m_opt_list) delete m_opt_list; }

protected:

	OptionList*     m_opt_list;
};

class Event
{
	wxString m_name;
	wxString m_description;

	IObjectBase*m_object; // pointer to the owner object
	wxString m_value;  // handler function name

public:

	Event() {}
	Event(const wxString &eventName,
		const wxString &description, IObjectBase* obj) :
		m_name(eventName), m_description(description),
		m_object(obj)
	{
	}

	void SetValue(const wxString &value) { m_value = value; }
	wxString GetValue() { return m_value; }

	wxString GetName() { return m_name; }
	IObjectBase*GetObject() { return m_object; }
	wxString GetDescription() { return m_description; }
};

class PropertyCategory
{
private:

	wxString m_name;

	std::vector<wxString> m_properties;
	std::set<wxString> m_propertiesVisible;
	std::vector<wxString> m_events;
	std::set<wxString> m_eventsVisible;

	std::vector< PropertyCategory* > m_categories;

public:

	PropertyCategory() {}
	PropertyCategory(const wxString &name) : m_name(name) {}

	void AddProperty(const wxString &name, bool visible = true) { m_properties.push_back(name); if (visible) m_propertiesVisible.insert(name); }
	void ShowProperty(const wxString &name) { m_propertiesVisible.insert(name); }
	void HideProperty(const wxString &name) { m_propertiesVisible.erase(name); }

	bool IsVisibleProperty(const wxString &name) { return m_propertiesVisible.count(name) != 0; }

	void AddEvent(const wxString &name, bool visible = true) { m_events.push_back(name); if (visible) m_eventsVisible.insert(name); }
	void ShowEvent(const wxString &name) { m_eventsVisible.insert(name); }
	void HideEvent(const wxString &name) { m_eventsVisible.erase(name); }

	bool IsVisibleEvent(const wxString &name) { return m_eventsVisible.count(name) != 0; }

	void AddCategory(PropertyCategory* category) { m_categories.push_back(category); }

	wxString GetName() { return m_name; }
	wxString GetPropertyName(unsigned int index)
	{
		if (index < m_properties.size())
		{
			return m_properties[index];
		}
		else
		{
			return wxEmptyString;
		}
	}

	wxString GetEventName(unsigned int index)
	{
		if (index < m_events.size())
		{
			return m_events[index];
		}
		else
		{
			return wxEmptyString;
		}
	}

	PropertyCategory* GetCategory(unsigned int index)
	{
		if (index < m_categories.size())
		{
			return m_categories[index];
		}
		else
		{
			return new PropertyCategory();
		}
	}

	unsigned int GetPropertyCount() { return m_properties.size(); }
	unsigned int GetEventCount() { return m_events.size(); }
	unsigned int GetCategoryCount() { return m_categories.size(); }

};

///////////////////////////////////////////////////////////////////////////////

class IObjectBase
{
protected:

	bool m_enabled = true;

	PropertyMap m_properties;
	EventMap m_events;

	IObjectBase  *m_parent = NULL;
	PropertyCategory *m_category = NULL;

	ObjectBaseVector m_children;

protected:

	// utilites for implementing the tree
	static const int INDENT;  // size of indent
	wxString GetIndentString(int indent); // obtiene la cadena con el indentado

	ObjectBaseVector& GetChildren() { return m_children; };
	PropertyMap&      GetProperties() { return m_properties; };

	// devuelve el puntero "this"
	IObjectBase* GetThis() { return this; }

public:

	virtual ~IObjectBase();

	/**
	* Obtiene el nombre del objeto.
	*
	* @note No confundir con la propiedad nombre que tienen algunos objetos.
	*       Cada objeto tiene un nombre, el cual será el mismo que el usado
	*       como clave en el registro de descriptores.
	*/
	virtual wxString GetClassName() const = 0;

	/// Gets the parent object
	IObjectBase* GetParent() const { return m_parent; }
	IObjectBase* GetNonSizerParent();

	/// Links the object to a parent
	virtual void SetParent(IObjectBase* parent) { m_parent = parent; }

	/**
	* Obtiene la propiedad identificada por el nombre.
	*
	* @note Notar que no existe el método SetProperty, ya que la modificación
	*       se hace a través de la referencia.
	*/
	Property* GetProperty(const wxString &nameParam);
	Event* GetEvent(const wxString &nameParam);

	/**
	* Añade una propiedad al objeto.
	*
	* Este método será usado por el registro de descriptores para crear la
	* instancia del objeto.
	* Los objetos siempre se crearán a través del registro de descriptores.
	*/
	void AddProperty(Property* value);
	void AddEvent(Event* event);

	/**
	* Obtiene el número de propiedades del objeto.
	*/
	unsigned int GetPropertyCount() { return (unsigned int)m_properties.size(); }
	unsigned int GetEventCount() { return m_events.size(); }

	Property* GetProperty(unsigned int idx); // throws ...;
	Event* GetEvent(unsigned int idx); // throws ...;

	/**
	* Devuelve el primer antecesor cuyo tipo coincida con el que se pasa
	* como parámetro.
	*
	* Será útil para encontrar el widget padre.
	*/
	IObjectBase* FindNearAncestor(const wxString &type);
	IObjectBase* FindNearAncestorByBaseClass(const wxString &type);

	/**
	* Añade un hijo al objeto.
	* Esta función es virtual, debido a que puede variar el comportamiento
	* según el tipo de objeto.
	*
	* @return true si se añadió el hijo con éxito y false en caso contrario.
	*/
	virtual bool AddChild(IObjectBase*);
	virtual bool AddChild(unsigned int idx, IObjectBase* obj);

	/**
	* Devuelve la posicion del hijo o GetChildCount() en caso de no encontrarlo
	*/
	unsigned int GetChildPosition(IObjectBase* obj);
	bool ChangeChildPosition(IObjectBase* obj, unsigned int pos);

	/**
	* Elimina un hijo del objeto.
	*/
	void RemoveChild(IObjectBase* obj);
	void RemoveChild(unsigned int idx);
	void RemoveAllChildren() { m_children.clear(); }

	/**
	* Obtiene un hijo del objeto.
	*/
	IObjectBase* GetChild(unsigned int idx);
	IObjectBase* GetChild(unsigned int idx, const wxString& type);

	/**
	* Obtiene el número de hijos del objeto.
	*/
	unsigned int GetChildCount()
	{
		return (unsigned int)m_children.size();
	}

	unsigned int GetPropertyIndex(const wxString &paramName) const
	{
		return std::distance(m_properties.begin(), m_properties.find(paramName.Lower()));
	}

	Property *GetPropertyByIndex(unsigned int idx)
	{
		if (m_properties.size() < idx)
			return NULL;

		auto m_properties_iterator = m_properties.begin();
		std::advance(m_properties_iterator, idx);
		return (*m_properties_iterator).second;
	}

	/**
	* Devuelve el tipo de objeto.
	*
	* Deberá ser redefinida en cada clase derivada.
	*/
	virtual wxString GetObjectTypeName() const = 0;

	/**
	* Devuelve la profundidad  del objeto en el arbol.
	*/
	virtual int GetComponentType() = 0;

	virtual void ReadProperty() = 0;
	virtual void SaveProperty() = 0;

	virtual void PrepareOption(Property *property, OptionList &optionlist) {}

	virtual bool IsEditable() { return m_enabled; }
	virtual void SetReadOnly(bool enabled = true) { m_enabled = enabled; }

	virtual bool IsItem() { return false; }

	/**
	* Property events
	*/
	virtual void OnPropertyCreated() {}

	virtual void OnPropertyCreated(Property *m_property) {}
	virtual void OnPropertySelected(Property *m_property) {}
	virtual void OnPropertyChanged(Property *m_property) {}

	/**
	* Comprueba si el tipo es derivado del que se pasa como parámetro.
	*/

	bool IsSubclassOf(wxString classname);

	PropertyCategory *GetCategory() { return m_category; }

	/////////////////////////
	// Implementación de la interfaz IObject para su uso dentro de los
	// plugins
	bool IsNull(const wxString& pname);
	int GetPropertyAsInteger(const wxString& pname);
	wxFontContainer GetPropertyAsFont(const wxString& pname);
	wxColour GetPropertyAsColour(const wxString& pname);
	wxString GetPropertyAsString(const wxString& pname);
	wxPoint GetPropertyAsPoint(const wxString& pname);
	wxSize GetPropertyAsSize(const wxString& pname);
	wxBitmap GetPropertyAsBitmap(const wxString& pname);
	double GetPropertyAsFloat(const wxString& pname);

	wxArrayInt GetPropertyAsArrayInt(const wxString& pname);
	wxArrayString GetPropertyAsArrayString(const wxString& pname);

	IObjectBase* GetChildPtr(unsigned int idx)
	{
		return GetChild(idx);
	}

	virtual void DeleteRecursive();

	wxDECLARE_ABSTRACT_CLASS(IObjectBase);
};

///////////////////////////////////////////////////////////////////////////////

/**
* Clase que guarda un conjunto de plantillas de código.
*/
class CodeInfo
{
private:
	typedef std::map<wxString, wxString> TemplateMap;
	TemplateMap m_templates;
public:
	wxString GetTemplate(wxString name);
	void AddTemplate(wxString name, wxString _template);
	void Merge(CodeInfo* merger);
};

#endif