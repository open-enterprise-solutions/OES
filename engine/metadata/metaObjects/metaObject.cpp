////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : metaobject
////////////////////////////////////////////////////////////////////////////

#include "metaObject.h"
#include "appData.h"
#include "compiler/methods.h"
#include "metadata/metadata.h"
#include "database/databaseLayer.h"
#include "database/databaseErrorCodes.h"
#include "utils/stringutils.h"

wxIMPLEMENT_ABSTRACT_CLASS(IMetaObject, CValue)

//*****************************************************************************************
//*                                  MetaObject                                           *
//*****************************************************************************************

IMetaObject::IMetaObject(const wxString &name, const wxString &alias, const wxString &comment) : CValue(eValueTypes::TYPE_VALUE, true), IObjectBase(), m_methods(new CMethods()), m_metaData(NULL),
m_sName(name), m_sAlias(alias), m_sComment(comment), m_bCanSave(true), m_bDeleted(false), m_metaID(0)
{
	m_category = new PropertyCategory("Property");

	PropertyCategory *m_common = new PropertyCategory("Common");
	m_common->AddProperty("name");
	m_common->AddProperty("alias");
	m_common->AddProperty("comment");

	m_category->AddCategory(m_common);

	m_properties["name"] = new Property("name", PropertyType::PT_WXNAME, this);
	m_properties["alias"] = new Property("alias", PropertyType::PT_WXSTRING, this);
	m_properties["comment"] = new Property("comment", PropertyType::PT_WXSTRING, this);
}

IMetaObject::~IMetaObject()
{
	wxDELETE(m_methods);
}

void IMetaObject::ReadProperty()
{
	m_properties["name"]->SetValue(m_sName);
	m_properties["alias"]->SetValue(m_sAlias);
	m_properties["comment"]->SetValue(m_sComment);
}

void IMetaObject::SaveProperty()
{
	m_sName = m_properties["name"]->GetValueAsString();
	m_sAlias = m_properties["alias"]->GetValueAsString();
	m_sComment = m_properties["comment"]->GetValueAsString();
}

bool IMetaObject::LoadMeta(CMemoryReader &dataReader)
{
	//Save meta version 
	dataReader.r_u32(); //reserved 

	//Load unique guid 
	wxString strGuid;
	dataReader.r_stringZ(strGuid);
	m_metaGuid = strGuid;

	//Load standart fields
	dataReader.r_stringZ(m_sName);
	dataReader.r_stringZ(m_sAlias);
	dataReader.r_stringZ(m_sComment);

	return LoadData(dataReader);
}

bool IMetaObject::SaveMeta(CMemoryWriter &dataWritter)
{
	//Save meta version 
	dataWritter.w_u32(OES_VERSION_NUMBER); //reserved 

	//Save unique guid
	dataWritter.w_stringZ(m_metaGuid);

	//Save standart fields
	dataWritter.w_stringZ(m_sName);
	dataWritter.w_stringZ(m_sAlias);
	dataWritter.w_stringZ(m_sComment);

	return SaveData(dataWritter);
}

bool IMetaObject::LoadMetaObject(IMetadata *metaData, CMemoryReader &dataReader)
{
	if (!LoadMeta(dataReader))
		return false;

	m_metaData = metaData;

	if (!OnLoadMetaObject())
		return false;

	return true;
}

bool IMetaObject::SaveMetaObject(IMetadata *metaData, CMemoryWriter &dataWritter, bool saveToFile)
{
	if (!SaveMeta(dataWritter))
		return false;

	if (m_metaData != metaData)
		return false;

	if (!saveToFile && !OnSaveMetaObject())
		return false;

	return true;
}

bool IMetaObject::DeleteMetaObject(IMetadata *metaData)
{
	if (!OnDeleteMetaObject())
		return false;

	if (m_metaData != metaData)
		return false;

	if (!DeleteData())
		return false;

	return true;
}

wxString IMetaObject::GetModuleName()
{
	return GetClassName() + wxT(": ") + m_sName;
}

wxString IMetaObject::GetFullName()
{
	wxString m_sFullName = GetClassName() + '.' + GetName();
	IMetaObject *m_metaParent = GetParent();

	while (m_metaParent)
	{
		if (g_metaCommonMetadataCLSID == m_metaParent->GetClsid())
			break;

		wxString m_sModuleName = m_metaParent->GetName();
		m_sFullName = m_sModuleName + '.' + m_sFullName;
		m_metaParent = m_metaParent->GetParent();
	}

	return m_sFullName;
}

wxString IMetaObject::GetFileName()
{
	return m_sFullPath;
}

wxString IMetaObject::GetDocPath()
{
	return m_metaGuid.str();
}