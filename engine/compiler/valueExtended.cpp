////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : extended module
////////////////////////////////////////////////////////////////////////////

#include "value.h"
#include "appData.h"
#include "database/databaseLayer.h"
#include "translateModule.h"

#include "definition.h"

inline eValueTypes GetTypeFromFieldDB(const wxString &sField)
{
	char c = sField[sField.Length() - 1];

	switch (c)
	{
	case 'B': return eValueTypes::TYPE_BOOLEAN;
	case 'N': return eValueTypes::TYPE_NUMBER;
	case 'D': return eValueTypes::TYPE_DATE;
	case 'S': return eValueTypes::TYPE_STRING;
	case 'R': return eValueTypes::TYPE_REFFER;
	};

	return eValueTypes::TYPE_EMPTY;
}

void CValue::SetBinaryData(int nPosition, PreparedStatement *preparedStatment)
{
	switch (m_typeClass)
	{
	case eValueTypes::TYPE_BOOLEAN: preparedStatment->SetParamBool(nPosition, GetBoolean()); break;
	case eValueTypes::TYPE_NUMBER: preparedStatment->SetParamNumber(nPosition, GetNumber()); break;
	case eValueTypes::TYPE_STRING: preparedStatment->SetParamString(nPosition, GetString()); break;
	case eValueTypes::TYPE_DATE: preparedStatment->SetParamDate(nPosition, GetDate()); break;

	case eValueTypes::TYPE_REFFER: if (m_pRef) m_pRef->SetBinaryData(nPosition, preparedStatment); break;
	};
}

void CValue::GetBinaryData(int nPosition, DatabaseResultSet *databaseResultSet)
{
	ResultSetMetaData *resultSetMetadata = databaseResultSet->GetMetaData();
	wxString sColName = resultSetMetadata->GetColumnName(nPosition);

	switch (GetTypeFromFieldDB(sColName))
	{
	case eValueTypes::TYPE_BOOLEAN: m_bData = databaseResultSet->GetResultBool(nPosition); break;
	case eValueTypes::TYPE_NUMBER: m_fData = databaseResultSet->GetResultNumber(nPosition); break;
	case eValueTypes::TYPE_STRING: m_sData = databaseResultSet->GetResultString(nPosition); break;
	case eValueTypes::TYPE_DATE: m_dData = databaseResultSet->GetResultDate(nPosition).GetValue().GetValue(); break;

	case eValueTypes::TYPE_REFFER: if (m_pRef) m_pRef->GetBinaryData(nPosition, databaseResultSet); break;
	}
}

void CValue::CopyValue() { if (m_typeClass == eValueTypes::TYPE_REFFER && m_pRef) m_pRef->CopyValue(); }

//*************************************************************************************************************************************
//*                                                           CValueNoRet                                                             *
//*************************************************************************************************************************************

void CValue::CheckValue() { if (m_typeClass == eValueTypes::TYPE_REFFER && m_pRef) m_pRef->CheckValue(); }

void CValueNoRet::CheckValue() { CTranslateError::Error(ERROR_USE_PROCEDURE_AS_FUNCTION, wxEmptyString, sProcedure.wc_str()); }

//*************************************************************************************************************************************
//*                                                         CAttributeParameters                                                      *
//*************************************************************************************************************************************

CAttributeParameters::CAttributeParameters(int iName, const wxString &sName) : iAttributeName(iName), sAttributeName(sName) {}

//*************************************************************************************************************************************
//*                                                         CMethodParameters                                                         *
//*************************************************************************************************************************************

void CMethodParameters::CheckValue(unsigned int idx) { if (!appData->IsDesignerMode()) { if (m_varCount < idx + 1) CTranslateError::Error(ERROR_FEW_PARAMS, sMethodName.wc_str()); } }
CMethodParameters::CMethodParameters(CValue **params, unsigned int varCount, int iName, const wxString &sName) : aParams(params), m_varCount(varCount), iMethodName(iName), sMethodName(sName) {}
void CMethodParameters::CheckParams() { if (!appData->IsDesignerMode()) { if (m_setParams.size() < m_varCount) CTranslateError::Error(ERROR_MANY_PARAMS, sMethodName.wc_str()); } }
CValue *CMethodParameters::GetAt(unsigned int idx) { CheckValue(idx); if (m_varCount < idx + 1) { return NULL; } m_setParams.insert(idx); return aParams[idx]; }
CValue CMethodParameters::operator[](unsigned int idx) { CheckValue(idx); if (m_varCount < idx + 1) { return eValueTypes::TYPE_EMPTY; } m_setParams.insert(idx); return *aParams[idx]; }

//*************************************************************************************************************************************
//*                                                         ValueRefData                                                              *
//*************************************************************************************************************************************

void CValue::DecrRef()
{
	wxASSERT_MSG(m_refCount > 0, "invalid ref data count");

	if (--m_refCount == 0)
		delete this;
}