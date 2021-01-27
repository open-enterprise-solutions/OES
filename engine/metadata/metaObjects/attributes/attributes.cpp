////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : meta-attribues
////////////////////////////////////////////////////////////////////////////

#include "attributes.h"
#include "metadata/metadata.h"
#include "utils/stringutils.h"

wxIMPLEMENT_DYNAMIC_CLASS(CAttributeObject, IMetaObject)

//***********************************************************************
//*                         Attributes                                  * 
//***********************************************************************

CAttributeObject::CAttributeObject() : IMetaObject(), m_typeDescription(eValueTypes::TYPE_STRING, 10), 
m_bIsDefaultAttribute(false)
{
	//types of category 
	PropertyCategory *m_categoryType = new PropertyCategory("Type");
	m_categoryType->AddProperty("type");
	m_categoryType->AddProperty("precision");
	m_categoryType->AddProperty("scale");
	m_categoryType->AddProperty("date_time");
	m_categoryType->AddProperty("length");
	m_category->AddCategory(m_categoryType);

	m_properties["type"] = new Property("type", PropertyType::PT_TYPE_SELECT, this);

	//number
	m_properties["precision"] = new Property("precision", PropertyType::PT_UINT, this);
	m_properties["scale"] = new Property("scale", PropertyType::PT_UINT, this);

	//date
	m_properties["date_time"] = new Property("date_time", PropertyType::PT_UINT, this);

	//string 
	m_properties["length"] = new Property("length", PropertyType::PT_UINT, this);
}

CAttributeObject::CAttributeObject(const wxString &name, unsigned int typeObject, const wxString &SQLFieldName) : IMetaObject(name), m_typeDescription(typeObject), sSQLFieldName(SQLFieldName),
m_bIsDefaultAttribute(true)
{
	//types of category 
	PropertyCategory *m_categoryType = new PropertyCategory("Type");
	m_categoryType->AddProperty("type");
	m_categoryType->AddProperty("precision");
	m_categoryType->AddProperty("scale");
	m_categoryType->AddProperty("date_time");
	m_categoryType->AddProperty("length");
	m_category->AddCategory(m_categoryType);

	m_properties["type"] = new Property("type", PropertyType::PT_TYPE_SELECT, this);

	//number
	m_properties["precision"] = new Property("precision", PropertyType::PT_UINT, this);
	m_properties["scale"] = new Property("scale", PropertyType::PT_UINT, this);

	//date
	m_properties["date_time"] = new Property("date_time", PropertyType::PT_UINT, this);

	//string 
	m_properties["length"] = new Property("length", PropertyType::PT_UINT, this);
}

CAttributeObject::CAttributeObject(const wxString &name, const  wxString &alias, const wxString &comment, unsigned int typeObject, const wxString &SQLFieldName) : IMetaObject(name, alias, comment), m_typeDescription(typeObject, 10), sSQLFieldName(SQLFieldName),
m_bIsDefaultAttribute(true)
{
	//types of category 
	PropertyCategory *m_categoryType = new PropertyCategory("Type");
	m_categoryType->AddProperty("type");
	m_categoryType->AddProperty("precision");
	m_categoryType->AddProperty("scale");
	m_categoryType->AddProperty("date_time");
	m_categoryType->AddProperty("length");
	m_category->AddCategory(m_categoryType);

	m_properties["type"] = new Property("type", PropertyType::PT_TYPE_SELECT, this);

	//number
	m_properties["precision"] = new Property("precision", PropertyType::PT_UINT, this);
	m_properties["scale"] = new Property("scale", PropertyType::PT_UINT, this);

	//date
	m_properties["date_time"] = new Property("date_time", PropertyType::PT_UINT, this);

	//string 
	m_properties["length"] = new Property("length", PropertyType::PT_UINT, this);
}

wxString CAttributeObject::GetSuffixObject()
{
	wxString sSuffix;

	switch (GetTypeObject())
	{
	case eValueTypes::TYPE_BOOLEAN: sSuffix = wxT("b"); break;
	case eValueTypes::TYPE_NUMBER: sSuffix = wxT("n"); break;
	case eValueTypes::TYPE_DATE: sSuffix = wxT("d"); break;
	case eValueTypes::TYPE_STRING: sSuffix = wxT("s"); break;
	default: sSuffix = wxT("r"); break;
	}

	return sSuffix;
}

wxString CAttributeObject::GetSQLTypeObject()
{
	wxString sSQLType;

	switch (GetTypeObject())
	{
	case eValueTypes::TYPE_BOOLEAN: sSQLType = wxT("SMALLINT"); break;
	case eValueTypes::TYPE_NUMBER:
	{
		if (m_typeDescription.m_scale > 0) sSQLType = wxT("NUMERIC(") + StringUtils::IntToStr(m_typeDescription.m_precision) + wxT(",") + StringUtils::IntToStr(m_typeDescription.m_scale) + wxT(")");
		else sSQLType = wxT("NUMERIC(") + StringUtils::IntToStr(m_typeDescription.m_precision) + wxT(")");
		break;
	}
	case eValueTypes::TYPE_DATE: sSQLType = wxT("TIMESTAMP"); break;
	case eValueTypes::TYPE_STRING: sSQLType = wxT("VARCHAR(") + StringUtils::IntToStr(m_typeDescription.m_lenght) + ")"; break;
	default: sSQLType = wxT("BLOB"); break;
	}

	return sSQLType;
}

wxString CAttributeObject::GetSQLFieldName()
{
	return sSQLFieldName;
}

bool CAttributeObject::LoadData(CMemoryReader &reader)
{
	reader.r(&m_typeDescription, sizeof(typedescription_t));
	return true;
}

bool CAttributeObject::SaveData(CMemoryWriter &writer)
{
	writer.w(&m_typeDescription, sizeof(typedescription_t));
	return true;
}

//***********************************************************************
//*                          Read&save property                         *
//***********************************************************************

void CAttributeObject::ReadProperty()
{
	IMetaObject::ReadProperty();

	m_properties["type"]->SetValue(m_typeDescription.m_typeObject);

	switch (m_typeDescription.m_typeObject)
	{
	case eValueTypes::TYPE_NUMBER:
		m_properties["precision"]->SetValue(m_typeDescription.m_precision);
		m_properties["scale"]->SetValue(m_typeDescription.m_scale);
		break;
	case eValueTypes::TYPE_DATE:
		m_properties["date_time"]->SetValue((int)m_typeDescription.m_dateTime);
		break;
	case eValueTypes::TYPE_STRING:
		m_properties["length"]->SetValue(m_typeDescription.m_lenght);
		break;
	}
}

void CAttributeObject::SaveProperty()
{
	IMetaObject::SaveProperty();
	m_typeDescription.m_typeObject = m_properties["type"]->GetValueAsInteger();

	switch (m_typeDescription.m_typeObject)
	{
	case eValueTypes::TYPE_NUMBER:
		m_typeDescription.m_precision = m_properties["precision"]->GetValueAsInteger();
		m_typeDescription.m_scale = m_properties["scale"]->GetValueAsInteger();
		break;
	case eValueTypes::TYPE_DATE:
		m_typeDescription.m_dateTime = (eDateVariant)m_properties["date_time"]->GetValueAsInteger();
		break;
	case eValueTypes::TYPE_STRING:
		m_typeDescription.m_lenght = m_properties["length"]->GetValueAsInteger();
		break;
	}
}

//***********************************************************************
//*                       Register in runtime                           *
//***********************************************************************

METADATA_REGISTER(CAttributeObject, "metaAttribute", TEXT2CLSID("MD_ATTR"));