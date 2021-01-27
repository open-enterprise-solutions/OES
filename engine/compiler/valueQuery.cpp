////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : query value 
////////////////////////////////////////////////////////////////////////////

#include "valuequery.h"
#include "methods.h"

#include "window/mainFrame.h"
#include "database/databaseLayer.h"

#include "utils/stringutils.h"

//////////////////////////////////////////////////////////////////////
wxIMPLEMENT_DYNAMIC_CLASS(CValueQuery, CValue);

CMethods CValueQuery::m_methods;

CValueQuery::CValueQuery() : CValue(eValueTypes::TYPE_VALUE)
{
}

CValueQuery::~CValueQuery()
{
}

enum
{
	enSetQueryText = 0,
	enExecute,
};

void CValueQuery::PrepareNames() const
{
	SEng aMethods[] =
	{
		{"setQueryText","setQueryText(string) - возвращает true, если найден и false, если не найден"},
		{"execute","execute() - возвращает true, если найден и false, если не найден"},
	};

	int nCountM = sizeof(aMethods) / sizeof(aMethods[0]);
	m_methods.PrepareMethods(aMethods, nCountM);
}

CValue CValueQuery::Method(CMethodParameters &aParams)
{
	CValue ret;

	switch (aParams.GetMethodIndex())
	{
	case enSetQueryText: sQueryText = aParams[0].ToString(); break;
	case enExecute: Execute(); break;
	}

	return ret;
}

///////////////////////////////////////////////////////////////////

void CValueQuery::Execute()
{
	if (sQueryText.IsEmpty()) return;
	PreparedStatement *preparedStatement = objectDatabase->PrepareStatement(sQueryText);
	if (!preparedStatement) return;
	DatabaseResultSet *databaseResulSet = preparedStatement->RunQueryWithResults();
	while (databaseResulSet->Next()) {}
	preparedStatement->Close();
}

//**********************************************************************
//*                       Runtime register                             *
//**********************************************************************

VALUE_REGISTER(CValueQuery, "query", TEXT2CLSID("VL_QUER"));