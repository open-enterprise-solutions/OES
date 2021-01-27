////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : base metadata for external files and metaObj
////////////////////////////////////////////////////////////////////////////

#include "metadata.h"

//ID's 
int IMetadata::GenerateNewID()
{
	IMetaObject *commonObject = GetCommonMetaObject();
	wxASSERT(commonObject);

	int meta_id = commonObject->GetMetaID();

	for (auto obj : m_aMetadataCommon)
	{
		if (meta_id < obj->GetMetaID())
			meta_id = obj->GetMetaID();
	}

	for (auto obj : m_aMetadataSecondary)
	{
		if (meta_id < obj->GetMetaID())
			meta_id = obj->GetMetaID();
	}

	return meta_id + 1;
}

IMetaObject *IMetadata::CreateMetaObject(const CLASS_ID &clsid, IMetaObject *parentMetaObj)
{
	wxASSERT(clsid != 0);
	wxString classType = CValue::GetNameObjectFromID(clsid);
	wxASSERT(classType.Length() > 0);
	IMetaObject *m_newMetaObject = CValue::CreateAndConvertObjectRef<IMetaObject *>(classType);
	wxASSERT(m_newMetaObject);

	m_newMetaObject->SetClsid(clsid);
	m_newMetaObject->SetName(GetNewName(clsid, parentMetaObj, m_newMetaObject->GetClassName()));
	m_newMetaObject->SetMetaID(GenerateNewID());

	if (parentMetaObj)
	{
		m_newMetaObject->SetParent(parentMetaObj);
		parentMetaObj->AddChild(m_newMetaObject);
	}

	m_newMetaObject->SetMetadata(this);

	//generate unique guid
	m_newMetaObject->GenerateGuid();

	if (parentMetaObj == GetCommonMetaObject())
		m_aMetadataCommon.push_back(m_newMetaObject);
	else
		m_aMetadataSecondary.push_back(m_newMetaObject);

	//always create metabject
	bool m_bSuccess = m_newMetaObject->OnCreateMetaObject();

	//first initialization
	if (!m_bSuccess || !m_newMetaObject->OnLoadMetaObject())
	{
		if (parentMetaObj)
			parentMetaObj->RemoveChild(m_newMetaObject);

		if (parentMetaObj == GetCommonMetaObject()) {
			auto foundedIt = std::find(m_aMetadataCommon.begin(), m_aMetadataCommon.end(), m_newMetaObject);
			if (foundedIt != m_aMetadataCommon.end()) {
				m_aMetadataCommon.erase(foundedIt);
			}
		}
		else
		{
			auto foundedIt = std::find(m_aMetadataSecondary.begin(), m_aMetadataSecondary.end(), m_newMetaObject);
			if (foundedIt != m_aMetadataSecondary.end()) {
				m_aMetadataSecondary.erase(foundedIt);
			}
		}

		wxDELETE(m_newMetaObject);
		return NULL;
	}

	//and running initialization
	if (!m_bSuccess || !m_newMetaObject->OnRunMetaObject())
	{
		if (parentMetaObj)
			parentMetaObj->RemoveChild(m_newMetaObject);

		if (parentMetaObj == GetCommonMetaObject()) {
			auto foundedIt = std::find(m_aMetadataCommon.begin(), m_aMetadataCommon.end(), m_newMetaObject);
			if (foundedIt != m_aMetadataCommon.end()) {
				m_aMetadataCommon.erase(foundedIt);
			}
		}
		else
		{
			auto foundedIt = std::find(m_aMetadataSecondary.begin(), m_aMetadataSecondary.end(), m_newMetaObject);
			if (foundedIt != m_aMetadataSecondary.end()) {
				m_aMetadataSecondary.erase(foundedIt);
			}
		}

		wxDELETE(m_newMetaObject);
		return NULL;
	}

	Modify(true);

	if (parentMetaObj)
		parentMetaObj->AppendChild(m_newMetaObject);

	m_newMetaObject->ReadProperty();
	m_newMetaObject->IncrRef();

	return m_newMetaObject;
}

wxString IMetadata::GetNewName(const CLASS_ID &clsid, IMetaObject *metaParent, const wxString &prefix)
{
	unsigned int countRec = 1;

	std::vector<IMetaObject *> aObjects;

	wxString currPrefix = prefix.length() > 0 ? prefix : wxT("newItem");
	wxString newName = wxString::Format("%s%d", currPrefix, countRec);

	while (countRec > aObjects.size())
	{
		bool foundedName = false;

		if (metaParent == GetCommonMetaObject())
		{
			for (auto obj : GetMetaObjects(clsid))
			{
				if (newName == obj->GetName()) {
					foundedName = true; break;
				}
			}
		}
		else
		{
			for (auto obj : metaParent->GetObjects(clsid))
			{
				if (newName == obj->GetName()) {
					foundedName = true; break;
				}
			}
		}

		if (!foundedName)
			break;

		newName = wxString::Format("%s%d", currPrefix, ++countRec);
	}

	return newName;
}

IMetaObject *IMetadata::GetMetaObject(meta_identifier_t meta_id)
{
	for (auto obj : m_aMetadataCommon)
	{
		if (meta_id == obj->GetMetaID())
			return obj;
	}

	for (auto obj : m_aMetadataSecondary)
	{
		if (meta_id == obj->GetMetaID())
			return obj;
	}

	return NULL;
}

std::vector<IMetaObject *> IMetadata::GetMetaObjects(const CLASS_ID &clsid) // get from object
{
	std::vector < IMetaObject *> m_objects;

	for (auto metaObject : m_aMetadataCommon)
	{
		if (metaObject && clsid == metaObject->GetClsid())
			m_objects.push_back(metaObject);
	}

	return m_objects;
}

IMetaObject *IMetadata::FindByName(const wxString &fileName)
{
	for (auto obj : m_aMetadataCommon)
	{
		if (fileName == obj->GetDocPath())
			return obj;
	}

	for (auto obj : m_aMetadataSecondary)
	{
		if (fileName == obj->GetDocPath())
			return obj;
	}

	return NULL;
}

bool IMetadata::RenameMetaObject(IMetaObject *pObj, const wxString &sNewName)
{
	if (pObj->OnRenameMetaObject(sNewName)) {
		Modify(true); return true;
	}

	return false;
}

void IMetadata::RemoveMetaObject(IMetaObject *obj, IMetaObject *objParent)
{
	if (!objParent)
		objParent = obj->GetParent();

	for (unsigned int idx = 0; idx < obj->GetChildCount(); idx++)
		RemoveMetaObject(obj->GetChild(idx), obj);

	if (obj->OnDeleteMetaObject()) {
		if (objParent)
			objParent->RemoveChild(obj);
		Modify(true);
		obj->MarkAsDeleted();
	}
}
