#ifndef _QUERYUNIT_H__
#define _QUERYUNIT_H__

#include "compiler/value.h"

class CQueryUnit
{
	wxString sQuery; 
	std::map<wxString, CValue> aParams;

public:

	CQueryUnit(); 
	CQueryUnit(const wxString &sQuery);

	void SetQueryText(const wxString &sQuery);
	wxString GetQueryText(); 

	void SetQueryParam(const wxString &sParamName, CValue cParam);
	CValue GetQueryParam(const wxString &sParamName);

	void Execute(); 

protected:

	void Reset();

};

#endif