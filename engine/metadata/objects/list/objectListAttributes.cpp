////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : list attrubutes 
////////////////////////////////////////////////////////////////////////////

#include "objectList.h"

//****************************************************************************
//*                              Override attribute                          *
//****************************************************************************
void CObjectListValue::SetAttribute(CAttributeParameters &aParams, CValue &value)        //��������� ��������
{
}

CValue CObjectListValue::GetAttribute(CAttributeParameters &aParams)                     //�������� ��������
{
	return CValue();
}