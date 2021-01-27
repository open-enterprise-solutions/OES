////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : list methods 
////////////////////////////////////////////////////////////////////////////

#include "objectList.h"
#include "compiler/methods.h"

CMethods CObjectListValue::m_methods;

enum
{
};

//****************************************************************************
//*                              Support methods                             *
//****************************************************************************

CMethods* CObjectListValue::GetPMethods() const
{
	CObjectListValue::PrepareNames();
	return &m_methods;
};

void CObjectListValue::PrepareNames() const
{
}

CValue CObjectListValue::Method(CMethodParameters &aParams)
{
	CValue Ret;
	return Ret;
}