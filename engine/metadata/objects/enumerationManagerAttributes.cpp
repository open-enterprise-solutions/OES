////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : enumeration manager - attributes
////////////////////////////////////////////////////////////////////////////

#include "enumerationManager.h"
#include "reference/reference.h"
#include "appData.h"

//****************************************************************************
//*                              Override attribute                          *
//****************************************************************************
void CManagerEnumeration::SetAttribute(CAttributeParameters &aParams, CValue &cValue) {}       //установка атрибута

#include "compiler/methods.h"

CValue CManagerEnumeration::GetAttribute(CAttributeParameters &aParams)                     //значение атрибута
{
	auto enums = m_metaObject->GetObjectEnums();
	CEnumerationObject *m_enum = enums.at(aParams.GetAttributeIndex());

	return new CValueReference(m_metaObject, m_enum->GetGuid());
}