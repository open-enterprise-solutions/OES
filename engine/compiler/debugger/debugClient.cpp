////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : debugger - client part
////////////////////////////////////////////////////////////////////////////

#include "debugClient.h"
#include "compiler/procUnit.h"
#include "database/databaseLayer.h"
#include "metadata/metadata.h"
#include "window/mainFrame.h"
#include "utils/fs/fs.h"
#if defined(_USE_NET_COMPRESSOR)
#include "utils/fs/lz/lzhuf.h"
#endif
#include "utils/stringutils.h"
#include "appData.h"

#include <wx/sysopt.h>
#include <wx/evtloop.h>

CDebuggerClient* CDebuggerClient::s_instance = NULL;

CDebuggerClient* CDebuggerClient::Get()
{
	if (!s_instance) s_instance = new CDebuggerClient(); return s_instance;
}

void CDebuggerClient::Initialize()
{
	if (wxSystemOptions::GetOption("debug.enable") == wxT("true"))
		CDebuggerClient::Get()->Connect();
}

void CDebuggerClient::Destroy()
{
	if (s_instance)
	{
		if (wxSystemOptions::GetOption("debug.enable") == wxT("true"))
			CDebuggerClient::Get()->Disconnect();

		s_instance->Delete();
		wxDELETE(s_instance);
	}
}

CDebuggerClient::CDebuggerClient() : wxThread(wxTHREAD_JOINABLE),
m_bUseDebug(false), m_bDoLoop(false), m_bDebugLoop(false), m_bDebugStopLine(false), m_nCurrentNumberStopContext(0),
m_pRunContext(NULL), m_socketClient(new wxSocketClient(wxSOCKET_WAITALL | wxSOCKET_BLOCK))
{
}

bool CDebuggerClient::Connect(const wxString &hostName, unsigned short port)
{
	if (m_socketClient && m_socketClient->IsConnected())
		m_socketClient->Close();

	wxIPV4address address;
	address.Hostname(hostName);
	address.Service(port);

	if (m_socketClient->Connect(address))
		return wxThread::Run() == wxThreadError::wxTHREAD_NO_ERROR;

	return false;
}

bool CDebuggerClient::Disconnect()
{
	if (m_socketClient && m_socketClient->IsConnected())
		return m_socketClient->Close();

	return false;
}


#include "compiler/valueOLE.h"

void CDebuggerClient::DoDebugLoop(const wxString &filePath, const wxString &moduleName, int line, CRunContext *pSetRunContext)
{
	m_pRunContext = pSetRunContext;

	if (!m_socketClient
		|| !m_socketClient->IsConnected())
	{
		m_bUseDebug = false;
		m_bDebugLoop = false;
		m_pRunContext = NULL;
		return;
	}

	m_nCurrentNumberStopContext = 0;

	CMemoryWriter m_commandChannelEnterLoop;

	m_commandChannelEnterLoop.w_u16(CommandId_EnterLoop);
	m_commandChannelEnterLoop.w_stringZ(filePath);
	m_commandChannelEnterLoop.w_stringZ(moduleName);
	m_commandChannelEnterLoop.w_s32(line);

	SendCommand(m_commandChannelEnterLoop.pointer(), m_commandChannelEnterLoop.size());

	//send expressions from user 
	SendExpressions();

	//send local variable
	SendLocalVariables();

	//send stack data to designer
	SendStack();

	//start debug loop
	m_bDebugLoop = true;
	m_bDebugStopLine = false;

	//create stream for this loop
	CValueOLE::CreateStreamForDispatch();

	//start debug loop
	while (m_bDebugLoop)
	{
		// нет конфигуратора
		if (!m_socketClient
			|| !m_socketClient->IsConnected())
		{
			m_bUseDebug = false;
			m_bDebugLoop = false;
			break;
		}

		//отвалился поток 
		if (!wxThread::IsRunning())
		{
			m_bUseDebug = false;
			m_bDebugLoop = false;

			CDebuggerClient::Disconnect();
			break;
		}

		wxMilliSleep(5);
	}

	if (mainFrame->IsFocusable())
	{
		mainFrame->Iconize(false); // restore the window if minimized
		mainFrame->SetFocus();     // focus on my window
		mainFrame->Raise();        // bring window to front
	}

	CMemoryWriter m_commandChannelLeaveLoop;

	m_commandChannelLeaveLoop.w_u16(CommandId_LeaveLoop);
	m_commandChannelLeaveLoop.w_stringZ(filePath);
	m_commandChannelLeaveLoop.w_stringZ(moduleName);
	m_commandChannelLeaveLoop.w_s32(line);

	SendCommand(m_commandChannelLeaveLoop.pointer(), m_commandChannelLeaveLoop.size());

	m_pRunContext = NULL;
}

#include "compiler/definition.h"

void CDebuggerClient::EnterDebugger(CRunContext *pContext, CByte &CurCode, int &nPrevLine)
{
	if (m_bUseDebug)
	{
		if (CurCode.m_nOper != OPER_FUNC && CurCode.m_nOper != OPER_END
			&& CurCode.m_nOper != OPER_SET && CurCode.m_nOper != OPER_SETCONST && CurCode.m_nOper != OPER_SET_TYPE
			&& CurCode.m_nOper != OPER_TRY && CurCode.m_nOper != OPER_ENDTRY)
		{
			if (CurCode.m_nNumberLine != nPrevLine)
			{
				int offsetPoint = 0; m_bDoLoop = false;

				if (m_bDebugStopLine &&
					CurCode.m_nNumberLine >= 0)//шагнуть в 
				{
					std::map<unsigned int, int> aOffsetPointList = m_aOffsetPoints[CurCode.m_sDocPath];
					std::map<unsigned int, int>::iterator foundedOffsetList = aOffsetPointList.find(CurCode.m_nNumberLine);

					m_bDebugStopLine = false;
					m_bDoLoop = true;

					if (foundedOffsetList != aOffsetPointList.end()) offsetPoint = foundedOffsetList->second;
				}
				else if (m_nCurrentNumberStopContext &&
					m_nCurrentNumberStopContext >= CProcUnit::GetCountRunContext() &&
					CurCode.m_nNumberLine >= 0) // шагнуть через
				{
					std::map<unsigned int, int> aOffsetPointList = m_aOffsetPoints[CurCode.m_sDocPath];
					std::map<unsigned int, int>::iterator foundedOffsetList = aOffsetPointList.find(CurCode.m_nNumberLine);

					m_nCurrentNumberStopContext = CProcUnit::GetCountRunContext();
					m_bDoLoop = true;

					if (foundedOffsetList != aOffsetPointList.end()) offsetPoint = foundedOffsetList->second;
				}
				else//произвольная точка останова
				{
					if (CurCode.m_nNumberLine >= 0)
					{
						std::map<unsigned int, int> aDebugPointList = m_aBreakpoints[CurCode.m_sDocPath];
						std::map<unsigned int, int>::iterator foundedDebugPoint = aDebugPointList.find(CurCode.m_nNumberLine);

						if (foundedDebugPoint != aDebugPointList.end()) { offsetPoint = foundedDebugPoint->second; m_bDoLoop = true; }
					}
				}

				if (m_bDoLoop) DoDebugLoop(CurCode.m_sFileName, CurCode.m_sDocPath, CurCode.m_nNumberLine + offsetPoint + 1, pContext);
			}

			nPrevLine = CurCode.m_nNumberLine;
		}
	}
}

void CDebuggerClient::WaitDebugger()
{
	if (!m_socketClient || !m_socketClient->IsConnected()) return;

	CMemoryWriter commandChannel;
	commandChannel.w_u16(CommandId_GetBreakPoints);
	SendCommand(commandChannel.pointer(), commandChannel.size());

	while (true)
	{
		if (!m_socketClient || !m_socketClient->IsConnected()) break;
		if (m_bUseDebug) break;
	}
}

void CDebuggerClient::InitializeBreakpoints(const wxString &sModuleName, unsigned int from, unsigned int to)
{
	std::map<unsigned int, int> &moduleOffsets = m_aOffsetPoints[sModuleName];
	moduleOffsets.clear(); for (unsigned int i = from; i < to; i++) moduleOffsets[i] = 0;
}

void CDebuggerClient::SendErrorToDesigner(const wxString &moduleName, unsigned int line, const wxString &errorMessage)
{
	if (!m_socketClient || !m_socketClient->IsConnected()) return;
	if (!m_bUseDebug) return;

	CMemoryWriter m_commandChannel;

	m_commandChannel.w_u16(CommandId_MessageFromEnterprise);
	m_commandChannel.w_stringZ(moduleName); // module name
	m_commandChannel.w_u32(line); // line code 
	m_commandChannel.w_stringZ(errorMessage); // error message 

	SendCommand(m_commandChannel.pointer(), m_commandChannel.size());
}

void CDebuggerClient::OnKill()
{
	wxThread::Pause();

	if (m_socketClient)
	{
		m_socketClient->Destroy();
		m_socketClient = NULL;
	}

#ifdef __WXMSW__
	::CoUninitialize();
#endif // !_WXMSW

	m_pRunContext = NULL;
	m_bUseDebug = m_bDebugLoop = false;
}

wxThread::ExitCode CDebuggerClient::Entry()
{
#ifdef __WXMSW__
	HRESULT hr = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hr)) { wxLogSysError(hr, _("Failed to create an instance in thread!")); }
#endif // !_WXMSW

	ExitCode retCode = 0;

	try
	{
		EntryClient();
	}
	catch (...)
	{
		retCode = (ExitCode)1;
	}

#ifdef __WXMSW__
	if (SUCCEEDED(hr)) ::CoUninitialize();
#endif // !_WXMSW

	if (m_socketClient)
	{
		m_socketClient->Destroy();
		m_socketClient = NULL;
	}

	m_pRunContext = NULL;
	m_bUseDebug = m_bDebugLoop = false;

	return retCode;
}

void CDebuggerClient::EntryClient()
{
	unsigned int length = 0;

	while (!TestDestroy() &&
		(m_socketClient && (m_socketClient->IsConnected() || !m_socketClient->Error())))
	{
		if (m_socketClient && m_socketClient->WaitForRead())
		{
			m_socketClient->ReadMsg(&length, sizeof(unsigned int));

			if (m_socketClient && m_socketClient->WaitForRead())
			{
				wxMemoryBuffer m_bufferData(length);
				m_socketClient->ReadMsg(m_bufferData.GetData(), length);

				if (length > 0)
				{
					CValueOLE::GetInterfaceAndReleaseStream();

#if defined(_USE_NET_COMPRESSOR)
					BYTE* dest = NULL; unsigned int dest_sz = 0;
					_decompressLZ(&dest, &dest_sz, m_bufferData.GetData(), m_bufferData.GetBufSize());
					RecvCommand(dest, dest_sz); free(dest);
#else
					RecvCommand(m_bufferData.GetData(), m_bufferData.GetBufSize());
#endif 
					length = 0;
				}
			}
		}
	}
}

void CDebuggerClient::SendExpressions()
{
	if (!m_aExpressions.size()) 
		return;

	CMemoryWriter m_commandChannel;

	//header 
	m_commandChannel.w_u16(CommandId_SetExpressions);
	m_commandChannel.w_u32(m_aExpressions.size());

	for (auto expression : m_aExpressions)
	{
		bool bError = false;
		CValue vResult = CProcUnit::Evaluate(expression.second, m_pRunContext, false, &bError);

		//header 
#if defined(_USE_64_BIT_POINT_IN_DEBUG)
		m_commandChannel.w_u64(expression.first);
#else 
		m_commandChannel.w_u32(expression.first);
#endif 
		//variable
		m_commandChannel.w_stringZ(expression.second);
		m_commandChannel.w_stringZ(vResult.GetString());
		if (!bError) m_commandChannel.w_stringZ(vResult.GetTypeString());
		else m_commandChannel.w_stringZ(wxEmptyString);
		//array
		m_commandChannel.w_u32(vResult.GetNAttributes());
	}

	SendCommand(m_commandChannel.pointer(), m_commandChannel.size());
}

void CDebuggerClient::SendLocalVariables()
{
	CMemoryWriter m_commandChannel;
	m_commandChannel.w_u16(CommandId_SetLocalVariables);

	CCompileContext *m_compileContext = m_pRunContext->m_compileContext;
	wxASSERT(m_compileContext);

	m_commandChannel.w_u32(m_compileContext->m_cVariables.size());

	for (auto variable : m_compileContext->m_cVariables)
	{
		CVariable m_currentVariable = variable.second; CValue *locRefValue = m_pRunContext->m_pRefLocVars[m_currentVariable.m_nNumber];
		//send temp var 
		m_commandChannel.w_u8(m_currentVariable.m_bTempVar);
		//send attribute body
		m_commandChannel.w_stringZ(m_currentVariable.m_sRealName);
		m_commandChannel.w_stringZ(locRefValue->GetString());
		m_commandChannel.w_stringZ(locRefValue->GetTypeString());
		//send attribute count 
		m_commandChannel.w_u32(locRefValue->GetNAttributes());
	}

	SendCommand(m_commandChannel.pointer(), m_commandChannel.size());
}

void CDebuggerClient::SendStack()
{
	CMemoryWriter m_commandChannel;

	m_commandChannel.w_u16(CommandId_SetStack);
	m_commandChannel.w_u32(CProcUnit::GetCountRunContext());

	for (unsigned int i = CProcUnit::GetCountRunContext(); i > 0; i--) //передаем снизу вверх
	{
		CRunContext *m_pRunContext = CProcUnit::GetRunContext(i - 1);
		CByteCode *pByteCode = m_pRunContext->GetByteCode();

		wxASSERT(m_pRunContext && pByteCode);
		CCompileContext *m_compileContext = m_pRunContext->m_compileContext;
		wxASSERT(m_compileContext);
		CCompileModule *m_module = m_compileContext->m_compileModule;
		wxASSERT(m_module);
		if (m_module->m_bExpressionOnly) continue;

		if (pByteCode)
		{
			unsigned int nCurLine = m_pRunContext->m_nCurLine;

			if (nCurLine >= 0 && nCurLine <= pByteCode->m_aCodeList.size())
			{
				wxString sFullName = pByteCode->m_aCodeList[nCurLine].m_sModuleName;

				sFullName += wxT(".");

				if (m_compileContext->m_functionContext)
				{
					sFullName += m_compileContext->m_functionContext->m_sRealName;
					sFullName += wxT("(");

					for (unsigned int j = 0; j < m_compileContext->m_functionContext->m_aParamList.size(); j++)
					{
						wxString sValue = m_pRunContext->m_pRefLocVars[m_compileContext->m_cVariables[StringUtils::MakeUpper(m_compileContext->m_functionContext->m_aParamList[j].m_sName)].m_nNumber]->GetString();

						if (j != m_compileContext->m_functionContext->m_aParamList.size() - 1) sFullName += m_compileContext->m_functionContext->m_aParamList[j].m_sName + wxT(" = ") + sValue + wxT(", ");
						else sFullName += m_compileContext->m_functionContext->m_aParamList[j].m_sName + wxT(" = ") + sValue;
					}

					sFullName += wxT(")");
				}
				else
				{
					sFullName += wxT("<initializer>");
				}

				m_commandChannel.w_stringZ(sFullName);
				m_commandChannel.w_u32(pByteCode->m_aCodeList[nCurLine].m_nNumberLine + 1);
			}
		}
	}

	SendCommand(m_commandChannel.pointer(), m_commandChannel.size());
}

void CDebuggerClient::RecvCommand(void *pointer, unsigned int length)
{
	CMemoryReader commandReader(pointer, length);

	u16 m_commandFromServer = commandReader.r_u16();

	if (m_commandFromServer == CommandId_SetBreakPoints)
	{
		unsigned int countBreakpoints = commandReader.r_u32();

		//parse breakpoints 
		for (unsigned int i = 0; i < countBreakpoints; i++)
		{
			unsigned int countBreakPoints = commandReader.r_u32();
			wxString sModuleName; commandReader.r_stringZ(sModuleName);

			for (unsigned int j = 0; j < countBreakPoints; j++)
			{
				unsigned int line = commandReader.r_u32(); int offsetLine = commandReader.r_s32();
				m_aBreakpoints[sModuleName][line] = offsetLine;
			}
		}

		unsigned int countOffsetLines = commandReader.r_u32();

		//parse line offsets
		for (unsigned int i = 0; i < countOffsetLines; i++)
		{
			unsigned int countBreakPoints = commandReader.r_u32();
			wxString sModuleName; commandReader.r_stringZ(sModuleName);

			for (unsigned int j = 0; j < countBreakPoints; j++)
			{
				unsigned int line = commandReader.r_u32(); int offsetLine = commandReader.r_s32();
				m_aOffsetPoints[sModuleName][line] = offsetLine;
			}
		}

		m_bUseDebug = true;
	}
	else if (m_commandFromServer == CommandId_ToggleBreakpoint)
	{
		wxString sModuleName; commandReader.r_stringZ(sModuleName);
		unsigned int line = commandReader.r_u32(); int offset = commandReader.r_s32();

		std::map<unsigned int, int> &moduleBreakpoints = m_aBreakpoints[sModuleName];
		std::map<unsigned int, int>::iterator it = moduleBreakpoints.find(line);

		if (it == moduleBreakpoints.end()) moduleBreakpoints[line] = offset;
		m_bUseDebug = m_aBreakpoints.size() > 0;
	}
	else if (m_commandFromServer == CommandId_RemoveBreakpoint)
	{
		wxString sModuleName; commandReader.r_stringZ(sModuleName);
		unsigned int line = commandReader.r_u32();

		std::map<unsigned int, int> &moduleBreakpoints = m_aBreakpoints[sModuleName];
		std::map<unsigned int, int>::iterator it = moduleBreakpoints.find(line);

		if (it != moduleBreakpoints.end()) { moduleBreakpoints.erase(it); if (!moduleBreakpoints.size()) { m_aBreakpoints.erase(sModuleName); } }
		m_bUseDebug = m_aBreakpoints.size() > 0;
	}
	else if (m_commandFromServer == CommandId_AddExpression)
	{
		wxString sExpression; commandReader.r_stringZ(sExpression);
#if defined(_USE_64_BIT_POINT_IN_DEBUG)
		unsigned long long id = commandReader.r_u64();
#else 
		unsigned int id = commandReader.r_u32();
#endif 
		CMemoryWriter m_commandChannel; bool bError = false;

		m_commandChannel.w_u16(CommandId_SetExpressions);
		m_commandChannel.w_u32(1); // first elements 

		if (m_bDebugLoop)
		{
			CValue vResult = CProcUnit::Evaluate(sExpression, m_pRunContext, false, &bError);

			//header 
#if defined(_USE_64_BIT_POINT_IN_DEBUG)
			m_commandChannel.w_u64(id);
#else
			m_commandChannel.w_u32(id);
#endif 
			//variable
			m_commandChannel.w_stringZ(sExpression);
			m_commandChannel.w_stringZ(vResult.GetString());
			if (!bError) m_commandChannel.w_stringZ(vResult.GetTypeString());
			else m_commandChannel.w_stringZ(wxEmptyString);
			//count of elemetns 
			m_commandChannel.w_u32(vResult.GetNAttributes());
			//send expression 
			SendCommand(m_commandChannel.pointer(), m_commandChannel.size());
			//set expression in map 
			m_aExpressions.insert_or_assign(id, sExpression);
		}
		else
		{
			//header 
#if defined(_USE_64_BIT_POINT_IN_DEBUG)
			m_commandChannel.w_u64(id);
#else
			m_commandChannel.w_u32(id);
#endif 
			//variable
			m_commandChannel.w_stringZ(sExpression);
			m_commandChannel.w_stringZ(wxEmptyString);
			if (!bError) m_commandChannel.w_stringZ(wxEmptyString);
			else m_commandChannel.w_stringZ(wxEmptyString);
			//count of elemetns 
			m_commandChannel.w_u32(0);
			//send expression 
			SendCommand(m_commandChannel.pointer(), m_commandChannel.size());
			//set expression in map 
			m_aExpressions.insert_or_assign(id, sExpression);
		}
	}
	else if (m_commandFromServer == CommandId_ExpandExpression)
	{
		wxString sExpression; bool bError = false;
		commandReader.r_stringZ(sExpression);

		if (m_bDebugLoop)
		{
			CValue vResult = CProcUnit::Evaluate(sExpression, m_pRunContext, false, &bError);

#if defined(_USE_64_BIT_POINT_IN_DEBUG)
			unsigned long long id = commandReader.r_u64();
#else 
			unsigned int id = commandReader.r_u32();
#endif
			if (!bError)
			{
				CMemoryWriter m_commandChannel;
				m_commandChannel.w_u16(CommandId_ExpandExpression);
#if defined(_USE_64_BIT_POINT_IN_DEBUG)
				m_commandChannel.w_u64(id);
#else 
				m_commandChannel.w_u32(id);
#endif 
				//count of attribute  
				m_commandChannel.w_u32(vResult.GetNAttributes());

				//send varables 
				for (unsigned int i = 0; i < vResult.GetNAttributes(); i++)
				{
					wxString sName = vResult.GetAttributeName(i); int nFindAttribute = vResult.FindAttribute(sName);

					if (nFindAttribute != wxNOT_FOUND)
					{
						CAttributeParameters aParams(nFindAttribute, sName);
						CValue vAttribute = vResult.GetAttribute(aParams);
						//send attribute body
						m_commandChannel.w_stringZ(sName);
						m_commandChannel.w_stringZ(vAttribute.GetString());
						if (!bError) m_commandChannel.w_stringZ(vAttribute.GetTypeString());
						else m_commandChannel.w_stringZ(wxEmptyString);
						//count of attribute   
						m_commandChannel.w_u32(vAttribute.GetNAttributes());
					}
					else
					{
						//send attribute body
						m_commandChannel.w_stringZ(sName);
						m_commandChannel.w_stringZ(wxEmptyString);
						m_commandChannel.w_stringZ(wxEmptyString);
						//count of attribute   
						m_commandChannel.w_u32(0);
					}
				}

				SendCommand(m_commandChannel.pointer(), m_commandChannel.size());
			}
		}
	}
	else if (m_commandFromServer == CommandId_RemoveExpression)
	{
#if defined(_USE_64_BIT_POINT_IN_DEBUG)
		m_aExpressions.erase(commandReader.r_u64());
#else 
		m_aExpressions.erase(commandReader.r_u32());
#endif 
	}
	else if (m_commandFromServer == CommandId_EvalToolTip)
	{
		wxString sModuleName, sExpression; bool bError = false;

		commandReader.r_stringZ(sModuleName);
		commandReader.r_stringZ(sExpression);

		if (m_bDebugLoop)
		{
			CValue vResult = CProcUnit::Evaluate(sExpression, m_pRunContext, false, &bError);

			if (vResult.GetType() != eValueTypes::TYPE_EMPTY)
			{
				CMemoryWriter m_commandChannel;

				m_commandChannel.w_u16(CommandId_EvalToolTip);
				m_commandChannel.w_stringZ(sModuleName);
				m_commandChannel.w_stringZ(sExpression);
				m_commandChannel.w_stringZ(vResult.GetString());

				if (m_bDebugLoop && !bError)
					SendCommand(m_commandChannel.pointer(), m_commandChannel.size());
			}
		}
	}
	else if (m_commandFromServer == CommandId_EvalAutocomplete)
	{
		wxString sExpression, sKeyWord; bool bError = false;

#if defined(_USE_64_BIT_POINT_IN_DEBUG)
		u64 pointer = commandReader.r_u64();
#else 
		u32 pointer = commandReader.r_u32();
#endif 
		commandReader.r_stringZ(sExpression);
		commandReader.r_stringZ(sKeyWord);

		s32 currPos = commandReader.r_s32();

		if (m_bDebugLoop)
		{
			CValue vResult = CProcUnit::Evaluate(sExpression, m_pRunContext, false, &bError);

			if (!bError)
			{
				CMemoryWriter m_commandChannel;
				m_commandChannel.w_u16(CommandId_EvalAutocomplete);
#if defined(_USE_64_BIT_POINT_IN_DEBUG)
				m_commandChannel.w_u64(pointer);
#else 
				m_commandChannel.w_u32(pointer);
#endif 
				m_commandChannel.w_stringZ(sExpression);
				m_commandChannel.w_stringZ(sKeyWord);
				m_commandChannel.w_s32(currPos);

				m_commandChannel.w_u32(vResult.GetNAttributes());

				//send varables 
				for (unsigned int i = 0; i < vResult.GetNAttributes(); i++)
				{
					wxString sAttributeName = vResult.GetAttributeName(i);
					m_commandChannel.w_stringZ(sAttributeName);
				}

				m_commandChannel.w_u32(vResult.GetNMethods());

				//send functions 
				for (unsigned int i = 0; i < vResult.GetNMethods(); i++)
				{
					wxString sMethodName = vResult.GetMethodName(i);
					wxString sMethodDescription = vResult.GetMethodDescription(i);

					//send attribute body
					m_commandChannel.w_stringZ(sMethodName);
					m_commandChannel.w_stringZ(sMethodDescription);
				}

				SendCommand(m_commandChannel.pointer(), m_commandChannel.size());
			}
		}
	}
	else if (m_commandFromServer == CommandId_PatchInsertLine ||
		m_commandFromServer == CommandId_PatchDeleteLine)
	{
		wxString sModuleName; commandReader.r_stringZ(sModuleName); unsigned int line = commandReader.r_u32(); int offsetLine = commandReader.r_s32();
		std::map<unsigned int, int> &moduleBreakpoints = m_aBreakpoints[sModuleName];
		for (auto it = moduleBreakpoints.begin(); it != moduleBreakpoints.end(); it++) { if (((it->first + it->second) >= line)) it->second += offsetLine; }
		std::map<unsigned int, int> &moduleOffsets = m_aOffsetPoints[sModuleName];
		for (auto it = moduleOffsets.begin(); it != moduleOffsets.end(); it++) { if (((it->first + it->second) >= line)) it->second += offsetLine; }
	}
	else if (m_commandFromServer == CommandId_PatchComplete)
	{
	}
	else if (m_commandFromServer == CommandId_Continue)
	{
		m_bDebugLoop = m_bDoLoop = false;
	}
	else if (m_commandFromServer == CommandId_StepInto)
	{
		if (m_bDebugLoop)
		{
			m_bDebugStopLine = true;
			m_bDebugLoop = m_bDoLoop = false;
		}
	}
	else if (m_commandFromServer == CommandId_StepOver)
	{
		if (m_bDebugLoop)
		{
			m_nCurrentNumberStopContext = CProcUnit::GetCountRunContext();
			m_bDebugLoop = m_bDoLoop = false;
		}
	}
	else if (m_commandFromServer == CommandId_Pause)
	{
		m_bDebugStopLine = true;
	}
	else if (m_commandFromServer == CommandId_Detach)
	{
		m_bUseDebug = m_bDebugLoop = m_bDoLoop = false;
	}
	else if (m_commandFromServer == CommandId_Destroy)
	{
		if (objectDatabase->Close())
		{
#ifdef __WXMSW__
			::CoUninitialize();
#endif // !_WXMSW
			if (m_socketClient) m_socketClient->Destroy();
			std::exit(EXIT_SUCCESS);
		}
	}
	else if (m_commandFromServer == CommandId_DeleteAllBreakpoints)
	{
		m_aBreakpoints.clear();
	}
}

void CDebuggerClient::SendCommand(void *pointer, unsigned int length)
{
#if defined(_USE_NET_COMPRESSOR)
	BYTE* dest = NULL; unsigned int dest_sz = 0;
	_compressLZ(&dest, &dest_sz, pointer, length);
	if (m_socketClient)
	{
		m_socketClient->WriteMsg(&dest_sz, sizeof(unsigned int));
		m_socketClient->WriteMsg(dest, dest_sz);
	}
	free(dest);
#else
	if (m_socketClient)
	{
		m_socketClient->WriteMsg(&length, sizeof(unsigned int));
		m_socketClient->WriteMsg(pointer, length);
	}
#endif
}

CDebuggerClient::~CDebuggerClient()
{
	if (wxThread::IsRunning()) { wxThread::Kill(); }
	if (m_socketClient) m_socketClient->Destroy();
}