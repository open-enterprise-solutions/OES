////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : common factory module 
////////////////////////////////////////////////////////////////////////////

#include "value.h"
#include "functions.h"

static std::vector<IObjectValueAbstract *> *m_aFactoryObjects = NULL;

inline bool CompareString(const wxString &lhs, const wxString &rhs)
{
	return lhs.CompareTo(rhs, wxString::ignoreCase) == 0;
}

//*******************************************************************************
//*                       Guard factory objects                                 *
//*******************************************************************************

inline void AllocateFactoryObjects()
{
	m_aFactoryObjects = new std::vector<IObjectValueAbstract *>;
}

inline void DestroyFactoryObjects()
{
	wxDELETE(m_aFactoryObjects);
}

//*******************************************************************************
//*                      Support dynamic object                                 *
//*******************************************************************************

#include "appData.h"
#include "enumFactory.h"

CValue *CValue::CreateObjectRef(const wxString &className, CValue **aParams)
{
	auto itFounded = std::find_if(m_aFactoryObjects->begin(), m_aFactoryObjects->end(), [className](IObjectValueAbstract *singleObject) {
		return CompareString(className, singleObject->GetClassName());
	});

	if (itFounded == m_aFactoryObjects->end())
		CTranslateError::Error(_("Error creating object '%s'"), className.wc_str());

	IObjectValueAbstract *singleObject = *itFounded;
	wxASSERT(singleObject);

	if (singleObject->GetObjectType() != eObjectType::eObjectType_enum)
	{
		CValue* m_newObject = singleObject->CreateObject();
		wxASSERT(m_newObject);

		if (singleObject->GetObjectType() == eObjectType::eObjectType_object)
		{
			if (aParams)
			{
				if (!m_newObject->Init(aParams))
				{
					if (!appData->IsDesignerMode())
					{
						wxDELETE(m_newObject);
						CTranslateError::Error(_("Error initializing object '%s'"), className.wc_str());
					}
				}
			}
			else
			{
				if (!m_newObject->Init())
				{
					if (!appData->IsDesignerMode())
					{
						wxDELETE(m_newObject);
						CTranslateError::Error(_("Error initializing object '%s'"), className.wc_str());
					}
				}
			}
		}

		return m_newObject;
	}
	else
	{
		return enumFactory->GetEnumeration(className);
	}
}

void CValue::RegisterObject(const wxString &className, IObjectValueAbstract *singleObject)
{
	if (!m_aFactoryObjects) AllocateFactoryObjects();

	if (CValue::IsRegisterObject(className))
		CTranslateError::Error(_("Object '%s' is exist"), className.wc_str());

	//first initialization
	if (singleObject->GetObjectType() == eObjectType::eObjectType_object)
	{
		CValue* m_newObject = singleObject->CreateObject();
		m_newObject->PrepareNames();
		delete m_newObject;
	}
	else if (singleObject->GetObjectType() == eObjectType::eObjectType_enum)
	{
		enumFactory->AppendEnumeration(className, singleObject->CreateObject());
	}

	m_aFactoryObjects->push_back(singleObject);
}

void CValue::UnRegisterObject(const wxString &className)
{
	auto itFounded = std::find_if(m_aFactoryObjects->begin(), m_aFactoryObjects->end(), [className](IObjectValueAbstract *singleObject) {
		return CompareString(className, singleObject->GetClassName());
	});

	if (itFounded == m_aFactoryObjects->end())
		CTranslateError::Error(_("Object '%s' is not exist"), className.wc_str());

	IObjectValueAbstract *singleObject = *itFounded;
	wxASSERT(singleObject);

	if (singleObject->GetObjectType() == eObjectType::eObjectType_enum)
	{
		enumFactory->RemoveEnumeration(className);
	}

	m_aFactoryObjects->erase(itFounded);

	if (m_aFactoryObjects->size() == 0)
		DestroyFactoryObjects();
}

bool CValue::IsRegisterObject(const wxString &className)
{
	auto itFounded = std::find_if(m_aFactoryObjects->begin(), m_aFactoryObjects->end(), [className](IObjectValueAbstract *singleObject) {
		return CompareString(className, singleObject->GetClassName());
	});

	return itFounded != m_aFactoryObjects->end();
}

bool CValue::IsRegisterObject(const wxString &className, eObjectType objectType)
{
	auto itFounded = std::find_if(m_aFactoryObjects->begin(), m_aFactoryObjects->end(), [className, objectType](IObjectValueAbstract *singleObject) {
		return CompareString(className, singleObject->GetClassName())
			&& (objectType == singleObject->GetObjectType());
	});

	return itFounded != m_aFactoryObjects->end();
}

CLASS_ID CValue::GetTypeIDByRef(const wxClassInfo *classInfo)
{
	auto itFounded = std::find_if(m_aFactoryObjects->begin(), m_aFactoryObjects->end(), [classInfo](IObjectValueAbstract *singleObject) {
		return classInfo == singleObject->GetClassInfo();
	});

	if (itFounded == m_aFactoryObjects->end())
		return eValueTypes::TYPE_VALUE;

	IObjectValueAbstract *singleObject = *itFounded;
	wxASSERT(singleObject);
	return singleObject->GetTypeID();
}

CLASS_ID CValue::GetTypeIDByRef(const ITypeValue *objectRef)
{
	if (objectRef->m_typeClass != eValueTypes::TYPE_VALUE &&
		objectRef->m_typeClass != eValueTypes::TYPE_ENUM &&
		objectRef->m_typeClass != eValueTypes::TYPE_OLE)
	{
		return objectRef->GetTypeID();
	}

	wxClassInfo *classInfo = objectRef->GetClassInfo();
	wxASSERT(classInfo);
	return GetTypeIDByRef(classInfo);
}

CLASS_ID CValue::GetIDObjectFromString(const wxString &className)
{
	auto itFounded = std::find_if(m_aFactoryObjects->begin(), m_aFactoryObjects->end(), [className](IObjectValueAbstract *singleObject) {
		return CompareString(className, singleObject->GetClassName());
	});

	if (itFounded == m_aFactoryObjects->end())
		CTranslateError::Error(_("Object '%s' is not exist"), className.wc_str());

	IObjectValueAbstract *singleObject = *itFounded;
	wxASSERT(singleObject);
	return singleObject->GetTypeID();
}

bool CValue::CompareObjectName(const wxString &className, eValueTypes valueType)
{
	return CompareString(className, GetNameObjectFromVT(valueType));
}

wxString CValue::GetNameObjectFromID(const CLASS_ID &clsid, bool upper)
{
	auto itFounded = std::find_if(m_aFactoryObjects->begin(), m_aFactoryObjects->end(), [clsid](IObjectValueAbstract *singleObject) {
		return clsid == singleObject->GetTypeID();
	});

	if (itFounded == m_aFactoryObjects->end())
		CTranslateError::Error(_("Object with id '%s' is not exist"), clsid);

	IObjectValueAbstract *singleObject = *itFounded;
	wxASSERT(singleObject);
	return upper ? singleObject->GetClassName().Upper() : singleObject->GetClassName();
}

wxString CValue::GetNameObjectFromVT(eValueTypes valueType, bool upper)
{
	if (valueType > eValueTypes::TYPE_REFFER)
		return wxEmptyString;

	auto itFounded = std::find_if(m_aFactoryObjects->begin(), m_aFactoryObjects->end(), [valueType](IObjectValueAbstract *singleObject) {
		ISimpleObjectValueSingle *simpleSingleObject = dynamic_cast<ISimpleObjectValueSingle *>(singleObject);
		if (simpleSingleObject) {
			return valueType == simpleSingleObject->GetValueType();
		}
		return false;
	});

	IObjectValueAbstract *singleObject = *itFounded;
	wxASSERT(singleObject);
	return upper ? singleObject->GetClassName().Upper() : singleObject->GetClassName();
}

eValueTypes CValue::GetVTByID(const CLASS_ID &clsid)
{
	auto itFounded = std::find_if(m_aFactoryObjects->begin(), m_aFactoryObjects->end(), [clsid](IObjectValueAbstract *singleObject) {
		return clsid == singleObject->GetTypeID();
	});

	if (itFounded == m_aFactoryObjects->end())
		return eValueTypes::TYPE_EMPTY; 

	ISimpleObjectValueSingle *singleObject = dynamic_cast<ISimpleObjectValueSingle *>(*itFounded);

	if (!singleObject)
		return eValueTypes::TYPE_EMPTY;

	return singleObject->GetValueType();
}

IObjectValueAbstract *CValue::GetAvailableObject(const wxString &className)
{
	auto itFounded = std::find_if(m_aFactoryObjects->begin(), m_aFactoryObjects->end(), [className](IObjectValueAbstract *singleObject) {
		return CompareString(className, singleObject->GetClassName());
	});

	if (itFounded == m_aFactoryObjects->end())
		CTranslateError::Error(_("Object '%s' is not exist"), className.wc_str());

	return *itFounded;
}

wxArrayString CValue::GetAvailableObjects(eObjectType objectType)
{
	wxArrayString m_strings;

	for (auto singleObject : *m_aFactoryObjects)
	{
		if (singleObject->GetObjectType() == objectType)
			m_strings.push_back(singleObject->GetClassName());
	}

	m_strings.Sort();
	return m_strings;
}