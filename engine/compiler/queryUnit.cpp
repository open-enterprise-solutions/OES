////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : query unit 
////////////////////////////////////////////////////////////////////////////

#include "queryunit.h"
#include "utils/stringutils.h"
#include "database/databaseLayer.h"
#include "appData.h"

CQueryUnit::CQueryUnit() : sQuery(wxEmptyString) {}

CQueryUnit::CQueryUnit(const wxString &query) : sQuery(query) {}

void CQueryUnit::SetQueryText(const wxString &query)
{
	sQuery = query;
}

wxString CQueryUnit::GetQueryText()
{
	return sQuery;
}

void CQueryUnit::SetQueryParam(const wxString &sParamName, CValue cParam)
{
	aParams[StringUtils::MakeUpper(sParamName)] = cParam;
}

CValue CQueryUnit::GetQueryParam(const wxString &sParamName)
{
	return aParams[StringUtils::MakeUpper(sParamName)];
}

void CQueryUnit::Execute()
{
}

void CQueryUnit::Reset()
{
	sQuery = wxEmptyString;
	aParams.clear();
}
