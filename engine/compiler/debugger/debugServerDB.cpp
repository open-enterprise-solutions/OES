////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : debugger - server part 
////////////////////////////////////////////////////////////////////////////

#include "debugServer.h"
#include "database/databaseLayer.h"
#include "database/databaseErrorCodes.h"
#include "utils/stringutils.h"
#include "appData.h" 

#define DEBUG_POINTS wxString("DEBUG_POINTS")

//db support 
void CDebuggerServer::LoadBreakpoints()
{
	if (m_aBreakpoints.size() > 0)
		m_aBreakpoints.clear();

	//Debug points
	if (!objectDatabase->TableExists(DEBUG_POINTS))
	{
		objectDatabase->RunQuery("CREATE TABLE " + DEBUG_POINTS + " ("
			"moduleName VARCHAR(128) NOT NULL,"
			"moduleLine INTEGER NOT NULL,"
			"PRIMARY KEY(moduleName, moduleLine));");

		objectDatabase->RunQuery("CREATE INDEX " + DEBUG_POINTS + "_INDEX ON " + DEBUG_POINTS + " ("
			"moduleName,"
			"moduleLine);");
	}
	else
	{
		PreparedStatement *m_preparedStatement = objectDatabase->PrepareStatement("SELECT * FROM " + DEBUG_POINTS + "; ");
		wxASSERT(m_preparedStatement);
		DatabaseResultSet *m_resultSetDebug = m_preparedStatement->RunQueryWithResults();
		wxASSERT(m_resultSetDebug);
		while (m_resultSetDebug->Next()) m_aBreakpoints[m_resultSetDebug->GetResultString("moduleName")][m_resultSetDebug->GetResultInt("moduleLine")] = 0;
		objectDatabase->CloseStatement(m_preparedStatement);
		m_resultSetDebug->Close();
	}
}

bool CDebuggerServer::ToggleBreakpointInDB(const wxString &sModuleName, unsigned int line)
{
	bool successful = true;
	PreparedStatement *m_preparedStatement = objectDatabase->PrepareStatement("UPDATE OR INSERT INTO " + DEBUG_POINTS + "(moduleName, moduleLine) VALUES('" + sModuleName + "', " + StringUtils::IntToStr(line) + ") MATCHING (moduleName, moduleLine); ");
	wxASSERT(m_preparedStatement);
	if (m_preparedStatement->RunQuery() == DATABASE_LAYER_QUERY_RESULT_ERROR) { wxASSERT_MSG(false, "error in ToggleBreakpointInDB"); successful = false; }
	objectDatabase->CloseStatement(m_preparedStatement);
	return successful;
}

bool CDebuggerServer::RemoveBreakpointInDB(const wxString &sModuleName, unsigned int line)
{
	bool successful = true;
	PreparedStatement *m_preparedStatement = objectDatabase->PrepareStatement("DELETE FROM " + DEBUG_POINTS + " WHERE moduleName = '" + sModuleName + "' AND moduleLine = " + StringUtils::IntToStr(line) + ";");
	wxASSERT(m_preparedStatement);
	if (m_preparedStatement->RunQuery() == DATABASE_LAYER_QUERY_RESULT_ERROR) { wxASSERT_MSG(false, "error in RemoveBreakpointInDB"); successful = false; }
	if (objectDatabase->RunQuery("DELETE FROM " + DEBUG_POINTS + " WHERE moduleName = '" + sModuleName + "' AND moduleLine = " + StringUtils::IntToStr(line)) == DATABASE_LAYER_QUERY_RESULT_ERROR) { wxASSERT_MSG(false, "error in RemoveBreakpointInDB: "); successful = false; }
	objectDatabase->CloseStatement(m_preparedStatement);
	return successful;
}

bool CDebuggerServer::OffsetBreakpointInDB(const wxString &sModuleName, unsigned int lineFrom, int offset)
{
	bool successful = true;
	PreparedStatement *m_preparedStatement = objectDatabase->PrepareStatement("DELETE FROM " + DEBUG_POINTS + " WHERE moduleName = '" + sModuleName + "' AND moduleLine = " + StringUtils::IntToStr(lineFrom) + ";"
		"UPDATE OR INSERT INTO " + DEBUG_POINTS + "(moduleName, moduleLine) VALUES('" + sModuleName + "', " + StringUtils::IntToStr(lineFrom + offset) + ") MATCHING (moduleName, moduleLine); ");
	wxASSERT(m_preparedStatement);
	if (m_preparedStatement->RunQuery() == DATABASE_LAYER_QUERY_RESULT_ERROR) { wxASSERT_MSG(false, "error in OffsetBreakpointInDB"); successful = false; }
	objectDatabase->CloseStatement(m_preparedStatement);
	return successful;
}

bool CDebuggerServer::RemoveAllBreakPointsInDB()
{
	bool successful = true;
	PreparedStatement *m_preparedStatement = objectDatabase->PrepareStatement("DELETE FROM " + DEBUG_POINTS + ";");
	wxASSERT(m_preparedStatement);
	if (m_preparedStatement->RunQuery() == DATABASE_LAYER_QUERY_RESULT_ERROR) { wxASSERT_MSG(false, "error in RemoveAllBreakPointsInDB"); successful = false; }
	return successful;
}