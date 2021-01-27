////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko, wxFormBuider-team
//	Description : base object for property objects
////////////////////////////////////////////////////////////////////////////

#include "objectbase.h"
#include "utils/stringutils.h"
#include "utils/typeconv.h"

#include <wx/tokenzr.h>

wxIMPLEMENT_ABSTRACT_CLASS(IObjectBase, wxObject);

bool Property::IsNull()
{
	switch (GetType())
	{
	case PT_BITMAP:
	{
		wxString path;
		unsigned int semicolonIndex = m_value.find_first_of(';');
		if (semicolonIndex != m_value.npos)
		{
			path = m_value.substr(0, semicolonIndex);
		}
		else
		{
			path = m_value;
		}

		return path.empty();
	}
	case PT_WXSIZE:
	{
		return (wxDefaultSize == TypeConv::StringToSize(m_value));
	}
	default:
	{
		return m_value.empty();
	}
	}
}

bool Property::IsEditable()
{
	return m_object->IsEditable();
}

void Property::SetValue(const wxArrayString &str)
{
	m_value = TypeConv::ArrayStringToString(str);
}

void Property::SetValue(const wxFontContainer &font)
{
	m_value = TypeConv::FontToString(font);
}
void Property::SetValue(const wxColour &colour)
{
	m_value = TypeConv::ColourToString(colour);
}
void Property::SetValue(const wxString &str, bool format)
{
	m_value = (format ? TypeConv::TextToString(str) : str);
}

void Property::SetValue(const wxPoint &point)
{
	m_value = TypeConv::PointToString(point);
}

void Property::SetValue(const wxSize &size)
{
	m_value = TypeConv::SizeToString(size);
}

void Property::SetValue(const int integer)
{
	m_value = StringUtils::IntToStr(integer);
}

void Property::SetValue(const long long integer)
{
	wxString result;
	result.Printf(wxT("%d"), integer);
	m_value = result;
}

void Property::SetValue(const double val)
{
	m_value = TypeConv::FloatToString(val);
}

wxFontContainer Property::GetValueAsFont()
{
	return TypeConv::StringToFont(m_value);
}

wxColour Property::GetValueAsColour()
{
	return TypeConv::StringToColour(m_value);
}
wxPoint Property::GetValueAsPoint()
{
	return TypeConv::StringToPoint(m_value);
}
wxSize Property::GetValueAsSize()
{
	return TypeConv::StringToSize(m_value);
}

wxBitmap Property::GetValueAsBitmap()
{
	return TypeConv::StringToBitmap(m_value);
}

int Property::GetValueAsInteger()
{
	int result = 0;

	switch (GetType())
	{
		//case PT_EDIT_OPTION:
		//case PT_OPTION:
		//case PT_TYPE_SELECT:
	case PT_MACRO:
		result = TypeConv::GetMacroValue(m_value);
		break;
		//case PT_BITLIST:
		//	result = TypeConv::BitlistToInt(m_value);
		//	break;
	default:
		result = TypeConv::StringToInt(m_value);
		break;
	}
	return result;
}

wxString Property::GetValueAsString()
{
	return m_value;
}

wxString Property::GetValueAsText()
{
	return TypeConv::StringToText(m_value);
}

wxArrayString Property::GetValueAsArrayString()
{
	return TypeConv::StringToArrayString(m_value);
}

double Property::GetValueAsFloat()
{
	return TypeConv::StringToFloat(m_value);
}

///////////////////////////////////////////////////////////////////////////////

const int IObjectBase::INDENT = 2;

IObjectBase::~IObjectBase()
{
	if (m_category)
		delete m_category;

	// remove the reference in the parent
	if (m_parent)
	{
		IObjectBase* pobj(GetThis());
		m_parent->RemoveChild(pobj);
	}

	for (auto child : m_children) {
		child->SetParent(NULL);
	}

	//LogDebug(wxT("delete IObjectBase"));
}

wxString IObjectBase::GetIndentString(int indent)
{
	int i;
	wxString s;

	for (i = 0; i < indent; i++)
		s += wxT(" ");

	return s;
}

IObjectBase* IObjectBase::GetNonSizerParent()
{
	IObjectBase* current = GetThis();

	while ((current = current->GetParent()) &&
		(current->GetObjectTypeName() == wxT("sizerItem") ||
			current->GetObjectTypeName() == wxT("sizer")));

	return current;
}

Property* IObjectBase::GetProperty(const wxString &nameParam)
{
	PropertyMap::iterator it = m_properties.find(nameParam.Lower());

	if (it != m_properties.end())
		return it->second;

	//LogDebug(wxT("[IObjectBase::GetProperty] Property %s not found!"),name.c_str());
	  // este aserto falla siempre que se crea un sizerItem
	  // assert(false);

	return NULL;
}

Property* IObjectBase::GetProperty(unsigned int idx)
{
	assert(idx < m_properties.size());

	PropertyMap::iterator it = m_properties.begin();
	unsigned int i = 0;
	while (i < idx && it != m_properties.end())
	{
		i++;
		it++;
	}

	if (it != m_properties.end())
		return it->second;

	return NULL;
}

Event* IObjectBase::GetEvent(const wxString &nameParam)
{
	EventMap::iterator it = m_events.find(nameParam);
	if (it != m_events.end())
		return it->second;

	//LogDebug("[IObjectBase::GetEvent] Event " + name + " not found!");
	return new Event();
}

Event* IObjectBase::GetEvent(unsigned int idx)
{
	assert(idx < m_events.size());

	EventMap::iterator it = m_events.begin();
	unsigned int i = 0;
	while (i < idx && it != m_events.end())
	{
		i++;
		it++;
	}

	if (it != m_events.end())
		return it->second;

	return new Event();
}

void IObjectBase::AddProperty(Property* prop)
{
	m_properties.insert(PropertyMap::value_type(prop->GetName(), prop));
}

void IObjectBase::AddEvent(Event* event)
{
	m_events.insert(EventMap::value_type(event->GetName(), event));
}

IObjectBase* IObjectBase::FindNearAncestor(const wxString &type)
{
	IObjectBase* result = NULL;
	IObjectBase* parent = GetParent();
	if (parent)
	{
		if (parent->GetObjectTypeName() == type)
			result = parent;
		else
			result = parent->FindNearAncestor(type);
	}

	return result;
}

IObjectBase* IObjectBase::FindNearAncestorByBaseClass(const wxString &type)
{
	IObjectBase* result = NULL;
	IObjectBase* parent = GetParent();
	if (parent)
	{
		if (parent->GetObjectTypeName() == type)
			result = parent;
		else
			result = parent->FindNearAncestorByBaseClass(type);
	}

	return result;
}

bool IObjectBase::AddChild(IObjectBase* obj)
{
	m_children.push_back(obj);
	return true;
}

bool IObjectBase::AddChild(unsigned int idx, IObjectBase* obj)
{
	m_children.insert(m_children.begin() + idx, obj);
	return true;
}

void IObjectBase::RemoveChild(IObjectBase* obj)
{
	std::vector< IObjectBase* >::iterator it = m_children.begin();
	while (it != m_children.end() && *it != obj)
		it++;

	if (it != m_children.end())
		m_children.erase(it);
}

void IObjectBase::RemoveChild(unsigned int idx)
{
	assert(idx < m_children.size());

	std::vector< IObjectBase* >::iterator it = m_children.begin() + idx;
	m_children.erase(it);
}

IObjectBase* IObjectBase::GetChild(unsigned int idx)
{
	assert(idx < m_children.size());

	return m_children[idx];
}

IObjectBase* IObjectBase::GetChild(unsigned int idx, const wxString& type)
{
	unsigned int cnt = 0;

	for (std::vector< IObjectBase* >::iterator it = m_children.begin(); it != m_children.end(); ++it)
	{
		if ((*it)->GetObjectTypeName() == type && ++cnt == idx) return *it;
	}

	return NULL;
}

unsigned int IObjectBase::GetChildPosition(IObjectBase* obj)
{
	unsigned int pos = 0;
	while (pos < GetChildCount() && m_children[pos] != obj)
		pos++;

	return pos;
}

bool IObjectBase::ChangeChildPosition(IObjectBase* obj, unsigned int pos)
{
	unsigned int obj_pos = GetChildPosition(obj);

	if (obj_pos == GetChildCount() || pos >= GetChildCount())
		return false;

	if (pos == obj_pos)
		return true;

	// Procesamos el cambio de posición
	RemoveChild(obj);
	AddChild(pos, obj);
	return true;
}

///////////////////////////////////////////////////////////////////////////////

bool IObjectBase::IsNull(const wxString& pname)
{
	Property* property = GetProperty(pname);
	if (property)
		return property->IsNull();
	else
		return true;
}

int IObjectBase::GetPropertyAsInteger(const wxString& pname)
{
	Property* property = GetProperty(pname);
	if (property)
		return property->GetValueAsInteger();
	else
		return 0;
}

wxFontContainer IObjectBase::GetPropertyAsFont(const wxString& pname)
{
	Property* property = GetProperty(pname);
	if (property)
		return property->GetValueAsFont();
	else
		return wxFontContainer();
}

wxColour IObjectBase::GetPropertyAsColour(const wxString& pname)
{
	Property* property = GetProperty(pname);
	if (property)
		return property->GetValueAsColour();
	else
		return wxColour();
}

wxString IObjectBase::GetPropertyAsString(const wxString& pname)
{
	Property* property = GetProperty(pname);
	if (property)
		return property->GetValueAsString();
	else
		return wxString();
}

wxPoint  IObjectBase::GetPropertyAsPoint(const wxString& pname)
{
	Property* property = GetProperty(pname);
	if (property)
		return property->GetValueAsPoint();
	else
		return wxPoint();
}

wxSize   IObjectBase::GetPropertyAsSize(const wxString& pname)
{
	Property* property = GetProperty(pname);
	if (property)
		return property->GetValueAsSize();
	else
		return wxDefaultSize;
}

wxBitmap IObjectBase::GetPropertyAsBitmap(const wxString& pname)
{
	Property* property = GetProperty(pname);
	if (property)
		return property->GetValueAsBitmap();
	else
		return wxBitmap();
}
double IObjectBase::GetPropertyAsFloat(const wxString& pname)
{
	Property* property = GetProperty(pname);
	if (property)
		return property->GetValueAsFloat();
	else
		return 0;
}
wxArrayInt IObjectBase::GetPropertyAsArrayInt(const wxString& pname)
{
	wxArrayInt array;
	Property* property = GetProperty(pname);
	if (property)
	{
		IntList il(property->GetValue(), property->GetType() == PT_UINTLIST);
		for (unsigned int i = 0; i < il.GetSize(); i++)
			array.Add(il.GetValue(i));
	}

	return array;
}

wxArrayString IObjectBase::GetPropertyAsArrayString(const wxString& pname)
{
	Property* property = GetProperty(pname);
	if (property)
		return property->GetValueAsArrayString();
	else
		return wxArrayString();
}

bool IObjectBase::IsSubclassOf(wxString classname)
{
	bool found = false;

	if (classname == GetClassName())
	{
		found = true;
	}
	else
	{
		IObjectBase*m_parent = GetParent();

		while (m_parent)
		{
			found = m_parent->IsSubclassOf(classname);

			if (found)
				break;
			else
				m_parent = m_parent->GetParent();
		}
	}

	return found;
}

void IObjectBase::DeleteRecursive()
{
	for (auto objChild : m_children) {
		objChild->SetParent(NULL);
		objChild->DeleteRecursive();
		delete objChild;
	}
	m_children.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

wxString CodeInfo::GetTemplate(wxString name)
{
	wxString result;

	TemplateMap::iterator it = m_templates.find(name);
	if (it != m_templates.end())
		result = it->second;

	return result;
}

void CodeInfo::AddTemplate(wxString name, wxString _template)
{
	m_templates.insert(TemplateMap::value_type(name, _template));
}

void CodeInfo::Merge(CodeInfo* merger)
{
	TemplateMap::iterator mergerTemplate;
	for (mergerTemplate = merger->m_templates.begin(); mergerTemplate != merger->m_templates.end(); ++mergerTemplate)
	{
		std::pair< TemplateMap::iterator, bool > mine = m_templates.insert(TemplateMap::value_type(mergerTemplate->first, mergerTemplate->second));
		if (!mine.second)
		{
			mine.first->second += mergerTemplate->second;
		}
	}
}
