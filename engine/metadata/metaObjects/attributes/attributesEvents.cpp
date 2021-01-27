////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : meta-attribues
////////////////////////////////////////////////////////////////////////////

#include "attributes.h"
#include "window/property/objinspect.h"

void CAttributeObject::OnPropertyCreated()
{
	PropertyCategory *m_categoryType = m_category->GetCategory(1);

	switch (m_typeDescription.m_typeObject)
	{
	case eValueTypes::TYPE_NUMBER:
		m_categoryType->ShowProperty("precision");
		m_categoryType->ShowProperty("scale");
		m_categoryType->HideProperty("date_time");
		m_categoryType->HideProperty("length");
		break;
	case eValueTypes::TYPE_DATE:
		m_categoryType->HideProperty("precision");
		m_categoryType->HideProperty("scale");
		m_categoryType->ShowProperty("date_time");
		m_categoryType->HideProperty("length");
		break;
	case eValueTypes::TYPE_STRING:
		m_categoryType->HideProperty("precision");
		m_categoryType->HideProperty("scale");
		m_categoryType->HideProperty("date_time");
		m_categoryType->ShowProperty("length");
		break;
	default:
		m_categoryType->HideProperty("precision");
		m_categoryType->HideProperty("scale");
		m_categoryType->HideProperty("date_time");
		m_categoryType->HideProperty("length");
	}
}

void CAttributeObject::OnPropertyCreated(Property *m_property)
{
}

void CAttributeObject::OnPropertySelected(Property *m_property)
{
}

void CAttributeObject::OnPropertyChanged(Property *m_property)
{
	if (m_property->GetName() == wxT("type")) objectInspector->RefreshProperty();
}