#ifndef _AUTOCOMPLETE_PARSER_H__
#define _AUTOCOMPLETE_PARSER_H__

#include "autoComplectionCtrl.h"

class CParserModule : public CTranslateModule
{
	int m_nCurrentCompile;//текущее положение в массиве лексем
	std::vector<CModuleElementInfo> m_aContentModule;

protected:

	CLexem PreviewGetLexem();
	CLexem GetLexem();
	CLexem GETLexem();
	void GETDelimeter(char c);

	bool IsNextDelimeter(char c);
	bool IsNextKeyWord(int nKey);
	void GETKeyWord(int nKey);
	wxString GETIdentifier(bool realName = false);
	CValue GETConstant();

public:

	CParserModule();
	bool ParseModule(const wxString &sModule);

	//all data
	std::vector<CModuleElementInfo> &GetAllContent() { return m_aContentModule; }
	//variables
	wxArrayString GetVariables(bool bOnlyExport = true);
	//functions & procedures 
	wxArrayString GetFunctions(bool bOnlyExport = true);
	wxArrayString GetProcedures(bool bOnlyExport = true);
};

#endif 