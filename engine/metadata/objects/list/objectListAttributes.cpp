////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : list attrubutes 
////////////////////////////////////////////////////////////////////////////

#include "objectList.h"

//****************************************************************************
//*                              Override attribute                          *
//****************************************************************************
void CObjectListValue::SetAttribute(CAttributeParameters &aParams, CValue &value)        //установка атрибута
{
}

CValue CObjectListValue::GetAttribute(CAttributeParameters &aParams)                     //значение атрибута
{
	return CValue();
}