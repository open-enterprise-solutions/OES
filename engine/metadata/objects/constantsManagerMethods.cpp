////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : constants - methods
////////////////////////////////////////////////////////////////////////////

#include "constantsManager.h"
#include "compiler/methods.h"
#include "database/databaseLayer.h"
#include "appData.h"

CMethods CManagerConstants::m_methods;

enum
{
	enSet = 0,
	enGet
};

void CManagerConstants::PrepareNames() const
{
	SEng aMethods[] =
	{
		{"set", "set(value)"},
		{"get", "get()"},
	};

	int nCountM = sizeof(aMethods) / sizeof(aMethods[0]);
	m_methods.PrepareMethods(aMethods, nCountM);
}

#include "metadata/objects/baseObject.h"

CValue CManagerConstants::Method(CMethodParameters &aParams)
{
	CValue ret;

	wxString sTableName = m_metaConst->GetSpecialTableNameDB();
	wxString sFieldName = m_metaConst->GetSpecialFiledNameDB();

	if (objectDatabase->TableExists(sTableName))
	{
		switch (aParams.GetMethodIndex())
		{
		case enSet:
		{
			wxString sQuery = "UPDATE OR INSERT INTO " + sTableName + " ( " + sFieldName + ", RECORD_KEY) VALUES (?, 0x30);";

			PreparedStatement *statement = objectDatabase->PrepareStatement(sQuery);

			switch (m_metaConst->GetTypeObject())
			{
			case eValueTypes::TYPE_BOOLEAN: statement->SetParamBool(1, aParams[0].GetBoolean()); break;
			case eValueTypes::TYPE_NUMBER: statement->SetParamNumber(1, aParams[0].GetNumber()); break;
			case eValueTypes::TYPE_DATE: statement->SetParamDate(1, aParams[0].GetDate()); break;
			case eValueTypes::TYPE_STRING: statement->SetParamString(1, aParams[0].GetString()); break;
			default:
			{
				CValueReference *m_reference = dynamic_cast<CValueReference *>(aParams[0].GetRef());
				if (m_reference) m_reference->SetBinaryData(1, statement);
			}
			}
			statement->RunQuery();
			objectDatabase->CloseStatement(statement); break;
		}
		case enGet:
		{
			DatabaseResultSet *resultSet = objectDatabase->RunQueryWithResults("SELECT FIRST 1 " + sFieldName + " FROM " + sTableName + "");
			while (resultSet->Next())
			{
				switch (m_metaConst->GetTypeObject())
				{
				case eValueTypes::TYPE_BOOLEAN: ret = resultSet->GetResultBool(sFieldName); break;
				case eValueTypes::TYPE_NUMBER: ret = resultSet->GetResultNumber(sFieldName); break;
				case eValueTypes::TYPE_DATE: ret = resultSet->GetResultDate(sFieldName); break;
				case eValueTypes::TYPE_STRING: ret = resultSet->GetResultString(sFieldName); break;
				default:
				{
					wxMemoryBuffer m_buffer;
					resultSet->GetResultBlob(sFieldName, m_buffer);
					if (!m_buffer.IsEmpty()) ret = CValueReference::CreateFromPtr(m_buffer.GetData()); break;
				}
				}
			}
			resultSet->Close(); break;
		}
		}
	}

	return ret;
}