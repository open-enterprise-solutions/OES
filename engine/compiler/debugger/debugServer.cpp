////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : debugger - server part
////////////////////////////////////////////////////////////////////////////

#include "debugServer.h"
#include "debugEvent.h"
#include "utils/fs/fs.h"
#if defined(_USE_NET_COMPRESSOR)
#include "utils/fs/lz/lzhuf.h"
#endif 
#include "window/mainFrame.h"
#include "window/stack/stackWindow.h"
#include "window/watch/watchwindow.h"
#include "window/output/outputWindow.h"
#include "window/autocomplete/autoComplectionCtrl.h"
#include "window/metatree/metatreeWnd.h"
#include "utils/stringutils.h"

enum eSocketTypes
{
	wxID_SOCKET_SERVER = 1
};

wxBEGIN_EVENT_TABLE(CDebuggerServer, wxEvtHandler)
EVT_SOCKET(eSocketTypes::wxID_SOCKET_SERVER, CDebuggerServer::OnSocketServerEvent)
//special debugger event 
EVT_DEBUG(CDebuggerServer::OnDebugEvent)
EVT_DEBUG_TOOLTIP_EVENT(CDebuggerServer::OnDebugToolTipEvent)
EVT_DEBUG_AUTOCOMPLETE_EVENT(CDebuggerServer::OnDebugAutoCompleteEvent)
wxEND_EVENT_TABLE()

CDebuggerServer* CDebuggerServer::s_instance = NULL;

CDebuggerServer* CDebuggerServer::Get()
{
	wxASSERT(appData->IsDesignerMode());
	if (!s_instance) s_instance = new CDebuggerServer();
	return s_instance;
}

void CDebuggerServer::Destroy()
{
	wxDELETE(s_instance);
}

void CDebuggerServer::Initialize()
{
	s_instance->CreateServer();
}

void CDebuggerServer::CreateServer(const wxString &hostName, unsigned short port)
{
	if (m_socketServer) m_socketServer->Destroy();

	LoadBreakpoints();

	wxIPV4address addr;
	addr.Hostname(hostName);
	addr.Service(port);

	m_socketServer = new wxSocketServer(addr);

	m_socketServer->SetEventHandler(*this, eSocketTypes::wxID_SOCKET_SERVER);
	m_socketServer->SetNotify(wxSOCKET_CONNECTION_FLAG | wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
	m_socketServer->Notify(true);
}

CDebuggerServer::CDebuggerServer() : wxEvtHandler(),
m_socketServer(NULL), m_bEnterLoop(false)
{
}

CDebuggerServer::~CDebuggerServer()
{
	for (auto clientConnection : m_aConnections) { clientConnection->Kill(); }
	if (m_socketServer) m_socketServer->Destroy();
}

//events: 
void CDebuggerServer::AddHandler(wxEvtHandler* handler)
{
	m_aHandlers.push_back(handler);
}

void CDebuggerServer::RemoveHandler(wxEvtHandler* handler)
{
	for (auto it = m_aHandlers.begin(); it != m_aHandlers.end(); ++it)
	{
		if (*it == handler) { m_aHandlers.erase(it); break; }
	}
}

//Notify event 
void CDebuggerServer::NotifyEvent(wxEvent& event)
{
	wxPostEvent(this, event);
}

//special functions:
void CDebuggerServer::Continue()
{
	CMemoryWriter m_commandChannel;
	m_commandChannel.w_u16(CommandId_Continue);
	SendCommand(m_commandChannel.pointer(), m_commandChannel.size());
}

void CDebuggerServer::StepOver()
{
	CMemoryWriter m_commandChannel;
	m_commandChannel.w_u16(CommandId_StepOver);
	SendCommand(m_commandChannel.pointer(), m_commandChannel.size());
}

void CDebuggerServer::StepInto()
{
	CMemoryWriter m_commandChannel;
	m_commandChannel.w_u16(CommandId_StepInto);
	SendCommand(m_commandChannel.pointer(), m_commandChannel.size());
}

void CDebuggerServer::Pause()
{
	CMemoryWriter m_commandChannel;
	m_commandChannel.w_u16(CommandId_Pause);
	SendCommand(m_commandChannel.pointer(), m_commandChannel.size());
}

void CDebuggerServer::Stop(bool kill)
{
	CMemoryWriter m_commandChannel;
	m_commandChannel.w_u16(kill ? CommandId_Destroy : CommandId_Detach);
	SendCommand(m_commandChannel.pointer(), m_commandChannel.size());
}

void CDebuggerServer::InitializeBreakpoints(const wxString &sModuleName, unsigned int from, unsigned int to)
{
	std::map<unsigned int, int> &moduleOffsets = m_aOffsetPoints[sModuleName];
	moduleOffsets.clear(); for (unsigned int i = from; i < to; i++) moduleOffsets[i] = 0;
}

void CDebuggerServer::PatchBreakpoints(const wxString &sModuleName, unsigned int line, int offsetLine)
{
#pragma message("nouverbe to nouverbe: необходимо исправить проблему при очистке большого количества строк!")

	std::map<unsigned int, int> &moduleBreakpoints = m_aBreakpoints[sModuleName];
	for (auto it = moduleBreakpoints.begin(); it != moduleBreakpoints.end(); it++)
	{
		if (((it->first + it->second) >= line)) it->second += offsetLine;
	}

	std::map<unsigned int, int> &moduleOffsets = m_aOffsetPoints[sModuleName];
	for (auto it = moduleOffsets.begin(); it != moduleOffsets.end(); it++)
	{
		if (((it->first + it->second) >= line)) it->second += offsetLine;
	}

	CMemoryWriter m_commandChannel;

	m_commandChannel.w_u16(offsetLine > 0 ? CommandId_PatchInsertLine : CommandId_PatchDeleteLine);
	m_commandChannel.w_stringZ(sModuleName);
	m_commandChannel.w_u32(line);
	m_commandChannel.w_s32(offsetLine);

	SendCommand(m_commandChannel.pointer(), m_commandChannel.size());
}

bool CDebuggerServer::SaveBreakpoints(const wxString &sModuleName)
{
#pragma message("nouverbe to nouverbe: необходимо исправить проблему при сохрании точек, если есть удаленные с ними строки!")

	//initialize breakpoint 
	auto itBreakpoint = m_aBreakpoints.find(sModuleName);

	if (itBreakpoint != m_aBreakpoints.end())
	{
		std::map<unsigned int, int> &moduleBreakpoints = itBreakpoint->second, moduleBreakpointsNew;
		for (auto it = moduleBreakpoints.begin(); it != moduleBreakpoints.end(); it++)
		{
			if (!OffsetBreakpointInDB(itBreakpoint->first, it->first, it->second))
				return false;

			moduleBreakpointsNew.emplace(it->first + it->second, 0);
		}

		moduleBreakpoints.clear();

		for (auto it = moduleBreakpointsNew.begin(); it != moduleBreakpointsNew.end(); it++)
		{
			moduleBreakpoints.emplace(it->first, it->second);
		}
	}

	//initialize offsets 
	auto itOffsetpoint = m_aOffsetPoints.find(sModuleName);

	if (itOffsetpoint != m_aOffsetPoints.end())
	{
		std::map<unsigned int, int> &moduleOffsets = itOffsetpoint->second;
		std::map<unsigned int, int> ::iterator it = moduleOffsets.begin(); std::advance(it, moduleOffsets.size() - 1);

		if (it != moduleOffsets.end()) InitializeBreakpoints(itOffsetpoint->first, 0, it->first + it->second + 1);
		else InitializeBreakpoints(itOffsetpoint->first, 0, 1);
	}

	CMemoryWriter m_commandChannel;

	m_commandChannel.w_u16(CommandId_PatchComplete);
	m_commandChannel.w_stringZ(sModuleName);

	SendCommand(m_commandChannel.pointer(), m_commandChannel.size());

	return true;
}

bool CDebuggerServer::SaveAllBreakpoints()
{
#pragma message("nouverbe to nouverbe: необходимо исправить проблему при сохрании точек, если есть удаленные с ними строки!")

	//initialize breakpoint 
	for (auto itBreakpoint = m_aBreakpoints.begin(); itBreakpoint != m_aBreakpoints.end(); itBreakpoint++)
	{
		std::map<unsigned int, int> &moduleBreakpoints = itBreakpoint->second, moduleBreakpointsNew;
		for (auto it = moduleBreakpoints.begin(); it != moduleBreakpoints.end(); it++)
		{
			if (!OffsetBreakpointInDB(itBreakpoint->first, it->first, it->second))
				return false;

			moduleBreakpointsNew.emplace(it->first + it->second, 0);
		}

		moduleBreakpoints.clear();

		for (auto it = moduleBreakpointsNew.begin(); it != moduleBreakpointsNew.end(); it++)
		{
			moduleBreakpoints.emplace(it->first, it->second);
		}
	}

	//initialize offsets 
	for (auto itOffsetpoint = m_aOffsetPoints.begin(); itOffsetpoint != m_aOffsetPoints.end(); itOffsetpoint++)
	{
		std::map<unsigned int, int> &moduleOffsets = itOffsetpoint->second;
		std::map<unsigned int, int> ::iterator it = moduleOffsets.begin(); std::advance(it, moduleOffsets.size() - 1);

		if (it != moduleOffsets.end()) InitializeBreakpoints(itOffsetpoint->first, 0, it->first + it->second + 1);
		else InitializeBreakpoints(itOffsetpoint->first, 0, 1);
	}

	for (auto itBreakpoint = m_aBreakpoints.begin(); itBreakpoint != m_aBreakpoints.end(); itBreakpoint++)
	{
		CMemoryWriter m_commandChannel;

		m_commandChannel.w_u16(CommandId_PatchComplete);
		m_commandChannel.w_stringZ(itBreakpoint->first);

		SendCommand(m_commandChannel.pointer(), m_commandChannel.size());
	}

	return true;
}

bool CDebuggerServer::ToggleBreakpoint(const wxString &sModuleName, unsigned int line)
{
	unsigned int startLine = line; int locOffsetPrev = 0, locOffsetCurr = 0;
	std::map<unsigned int, int> &moduleOffsets = m_aOffsetPoints[sModuleName];

	for (auto it = moduleOffsets.begin(); it != moduleOffsets.end(); it++)
	{
		if (it->second < 0 && (int)it->first < -it->second) { locOffsetPrev = it->second; continue; }
		locOffsetCurr = it->second;
		if ((it->first + locOffsetPrev) <= line && (it->first + locOffsetCurr) >= line) { startLine = it->first; break; }
		locOffsetPrev = it->second;
	}

	std::map<unsigned int, int>::iterator itOffset = moduleOffsets.find(startLine);

	if (itOffset != moduleOffsets.end())
	{
		if (line != (itOffset->first + itOffset->second)) { wxMessageBox("Cannot set breakpoint in unsaved copy!"); return false; }
	}
	else
	{
		wxMessageBox("Cannot set breakpoint in unsaved copy!"); return false;
	}

	std::map<unsigned int, int> &moduleBreakpoints = m_aBreakpoints[sModuleName];
	std::map<unsigned int, int>::iterator itBreakpoint = moduleBreakpoints.find(itOffset->first);

	unsigned int currLine = itOffset->first; int offset = itOffset->second;

	if (itBreakpoint == moduleBreakpoints.end())
	{
		if (ToggleBreakpointInDB(sModuleName, currLine))
		{
			moduleBreakpoints.emplace(currLine, offset);

			CMemoryWriter m_commandChannel;
			m_commandChannel.w_u16(CommandId_ToggleBreakpoint);
			m_commandChannel.w_stringZ(sModuleName);
			m_commandChannel.w_u32(currLine);
			m_commandChannel.w_s32(offset);

			SendCommand(m_commandChannel.pointer(), m_commandChannel.size());
		}
		else
		{
			return false;
		}
	}

	return true;
}

bool CDebuggerServer::RemoveBreakpoint(const wxString &sModuleName, unsigned int line)
{
	unsigned int startLine = line; int locOffsetPrev = 0, locOffsetCurr = 0;
	std::map<unsigned int, int> &moduleOffsets = m_aOffsetPoints[sModuleName];

	for (auto it = moduleOffsets.begin(); it != moduleOffsets.end(); it++)
	{
		if (it->second < 0 && (int)it->first < -it->second) { locOffsetPrev = it->second; continue; }
		locOffsetCurr = it->second;
		if ((it->first + locOffsetPrev) <= line && (it->first + locOffsetCurr) >= line) { startLine = it->first; break; }
		locOffsetPrev = it->second;
	}

	std::map<unsigned int, int>::iterator itOffset = moduleOffsets.find(startLine);

	std::map<unsigned int, int> &moduleBreakpoints = m_aBreakpoints[sModuleName];
	std::map<unsigned int, int>::iterator itBreakpoint = moduleBreakpoints.find(itOffset->first);

	unsigned int currLine = itOffset->first;

	if (itBreakpoint != moduleBreakpoints.end())
	{
		if (RemoveBreakpointInDB(sModuleName, currLine))
		{
			moduleBreakpoints.erase(itBreakpoint);

			CMemoryWriter m_commandChannel;
			m_commandChannel.w_u16(CommandId_RemoveBreakpoint);
			m_commandChannel.w_stringZ(sModuleName);
			m_commandChannel.w_u32(currLine);

			SendCommand(m_commandChannel.pointer(), m_commandChannel.size());
		}
		else
		{
			return false;
		}
	}

	return true;
}

#include "common/reportManager.h"

void CDebuggerServer::RemoveAllBreakPoints()
{
	CMemoryWriter m_commandChannel;
	m_commandChannel.w_u16(CommandId_DeleteAllBreakpoints);
	SendCommand(m_commandChannel.pointer(), m_commandChannel.size());

	if (RemoveAllBreakPointsInDB())
	{
		m_aBreakpoints.clear();
		for (auto m_document : reportManager->GetDocumentsVector()) m_document->UpdateAllViews();
	}
	else
	{
		wxMessageBox("Error in : void CDebuggerServer::RemoveAllBreakPoints()");
	}
}

#if defined(_USE_64_BIT_POINT_IN_DEBUG)
void CDebuggerServer::AddExpression(const wxString &sExpression, unsigned long long id)
#else 
void CDebuggerServer::AddExpression(const wxString &sExpression, unsigned int id)
#endif 
{
	CMemoryWriter m_commandChannel;

	m_commandChannel.w_u16(CommandId_AddExpression);
	m_commandChannel.w_stringZ(sExpression);
#if defined(_USE_64_BIT_POINT_IN_DEBUG)
	m_commandChannel.w_u64(id);
#else 
	m_commandChannel.w_u32(id);
#endif 

	SendCommand(m_commandChannel.pointer(), m_commandChannel.size());

	//set expression in map 
	m_aExpressions.insert_or_assign(id, sExpression);
}

#if defined(_USE_64_BIT_POINT_IN_DEBUG)
void CDebuggerServer::ExpandExpression(const wxString &sExpression, unsigned long long id)
#else
void CDebuggerServer::ExpandExpression(const wxString &sExpression, unsigned int id)
#endif 
{
	CMemoryWriter m_commandChannel;

	m_commandChannel.w_u16(CommandId_ExpandExpression);
	m_commandChannel.w_stringZ(sExpression);
#if defined(_USE_64_BIT_POINT_IN_DEBUG)
	m_commandChannel.w_u64(id);
#else 
	m_commandChannel.w_u32(id);
#endif 

	SendCommand(m_commandChannel.pointer(), m_commandChannel.size());
}

#if defined(_USE_64_BIT_POINT_IN_DEBUG)
void CDebuggerServer::RemoveExpression(unsigned long long id)
#else
void CDebuggerServer::RemoveExpression(unsigned int id)
#endif 
{
	CMemoryWriter m_commandChannel;

	m_commandChannel.w_u16(CommandId_RemoveExpression);
#if defined(_USE_64_BIT_POINT_IN_DEBUG)
	m_commandChannel.w_u64(id);
#else 
	m_commandChannel.w_u32(id);
#endif 

	SendCommand(m_commandChannel.pointer(), m_commandChannel.size());

	//delete expression from map
	m_aExpressions.erase(id);
}

void CDebuggerServer::EvaluateToolTip(const wxString &sModuleName, const wxString &sExpression)
{
	if (debugServer->IsEnterLoop())
	{
		CMemoryWriter m_commandChannel;

		m_commandChannel.w_u16(CommandId_EvalToolTip);
		m_commandChannel.w_stringZ(sModuleName);
		m_commandChannel.w_stringZ(sExpression);

		SendCommand(m_commandChannel.pointer(), m_commandChannel.size());
	}
}

void CDebuggerServer::EvaluateAutocomplete(void *pointer, const wxString &sExpression, const wxString &sKeyWord, int currline)
{
	if (debugServer->IsEnterLoop())
	{
		CMemoryWriter m_commandChannel;

		m_commandChannel.w_u16(CommandId_EvalAutocomplete);
#if defined(_USE_64_BIT_POINT_IN_DEBUG)
		m_commandChannel.w_u64(reinterpret_cast<u64>(pointer));
#else 
		m_commandChannel.w_u32(reinterpret_cast<u32>(pointer));
#endif
		m_commandChannel.w_stringZ(sExpression);
		m_commandChannel.w_stringZ(sKeyWord);
		m_commandChannel.w_s32(currline);

		SendCommand(m_commandChannel.pointer(), m_commandChannel.size());
	}
}

std::vector<unsigned int> CDebuggerServer::GetDebugList(const wxString &sModuleName)
{
	std::map<unsigned int, int> &moduleBreakpoints = m_aBreakpoints[sModuleName]; std::vector<unsigned int> aBreakpointsVector;
	for (auto breakpoint : moduleBreakpoints) aBreakpointsVector.push_back(breakpoint.first + breakpoint.second);
	return aBreakpointsVector;
}

void CDebuggerServer::DeleteConnection(CClientSocketThread *client)
{
	if (!m_aConnections.size()) m_bEnterLoop = false;
	m_aConnections.erase(std::find(m_aConnections.begin(), m_aConnections.end(), client));

	wxSocketEvent event(1/*client->GetId()*/);
	event.m_event = wxSocketNotify::wxSOCKET_LOST;
	event.m_clientData = client;
	event.SetEventObject(m_socketServer);

	wxPostEvent(this, event);
}

void CDebuggerServer::RecvCommand(void *pointer, unsigned int length)
{
}

void CDebuggerServer::SendCommand(void *pointer, unsigned int length)
{
	for (auto connection : m_aConnections) { connection->SendCommand(pointer, length); }
}

void CDebuggerServer::OnSocketServerEvent(wxSocketEvent &event)
{
	if (event.GetSocketEvent() == wxSOCKET_CONNECTION)
	{
		wxSocketBase *m_sockConnection = m_socketServer->Accept(true);

		if (m_sockConnection)
		{
			CClientSocketThread *m_clientThread = new CClientSocketThread(m_sockConnection);
			if (m_clientThread->Run() == wxThreadError::wxTHREAD_NO_ERROR) { m_aConnections.push_back(m_clientThread); }
			else { if (m_clientThread->IsRunning()) m_clientThread->Kill(); }
		}
	}
	else if (event.GetSocketEvent() == wxSOCKET_LOST)
	{
		wxSocketServer::Shutdown();
	}
}

void CDebuggerServer::OnDebugEvent(wxDebugEvent &event)
{
	wxString fileName = event.GetFileName();
	wxString moduleName = event.GetModuleName();

	if (event.GetEventId() == EventId::EventId_EnterLoop)
	{
		if (!fileName.IsEmpty()) {

			wxDocument * const foundedDoc = reportManager->FindDocumentByPath(fileName);
			if (foundedDoc == NULL) {
				reportManager->CreateDocument(fileName, wxDOC_SILENT);
			}
		}
	}

	for (auto handler : m_aHandlers) {
		handler->AddPendingEvent(event);
	}
}

void CDebuggerServer::OnDebugToolTipEvent(wxDebugToolTipEvent &event)
{
	for (auto handler : m_aHandlers) {
		handler->AddPendingEvent(event);
	}
}

void CDebuggerServer::OnDebugAutoCompleteEvent(wxDebugAutocompleteEvent &event)
{
	for (auto handler : m_aHandlers) {
		handler->AddPendingEvent(event);
	}
}

CDebuggerServer::CClientSocketThread::CClientSocketThread(wxSocketBase *soket) : wxThread(wxTHREAD_DETACHED), m_socketClient(soket)
{
	// set the appropriate flags for the socket
	m_socketClient->SetFlags(wxSOCKET_WAITALL | wxSOCKET_BLOCK);
}

CDebuggerServer::CClientSocketThread::~CClientSocketThread() {}

void  CDebuggerServer::CClientSocketThread::OnKill()
{
	wxThread::Pause();

	// Send the exit event message to the UI.
	wxDebugEvent eventEndSession(EventId::EventId_SessionEnd, m_socketClient);
	debugServer->NotifyEvent(eventEndSession);

	DeleteConnection();
}

wxThread::ExitCode CDebuggerServer::CClientSocketThread::Entry()
{
	// Send the start event message to the UI.
	wxDebugEvent eventStartSession(EventId::EventId_SessionStart, m_socketClient);
	debugServer->NotifyEvent(eventStartSession);

	ExitCode retCode = 0;

	try
	{
		EntryClient();
	}
	catch (...)
	{
		retCode = (ExitCode)1;
	}

	// Send the exit event message to the UI.
	wxDebugEvent eventEndSession(EventId::EventId_SessionEnd, m_socketClient);
	debugServer->NotifyEvent(eventEndSession);

	DeleteConnection();
	return retCode;
}

void CDebuggerServer::CClientSocketThread::EntryClient()
{
	unsigned int length = 0;

	while (!TestDestroy() && (m_socketClient && !m_socketClient->Error()))
	{
		if (debugServer->TryLock())
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
			debugServer->Unlock();
		}
	}
}

void CDebuggerServer::CClientSocketThread::DeleteConnection()
{
	debugServer->DeleteConnection(this); m_socketClient->Destroy();
}

void CDebuggerServer::CClientSocketThread::RecvCommand(void *pointer, unsigned int length)
{
	CMemoryReader commandReader(pointer, length);

	u16 m_commandFromClient = commandReader.r_u16();

	if (m_commandFromClient == CommandId_GetBreakPoints)
	{
		//send expression 
		for (auto expression : debugServer->m_aExpressions)
		{
			CMemoryWriter m_commandChannel;

			m_commandChannel.w_u16(CommandId_AddExpression);
			m_commandChannel.w_stringZ(expression.second);

#if defined(_USE_64_BIT_POINT_IN_DEBUG)
			m_commandChannel.w_u64(expression.first);
#else 
			m_commandChannel.w_u32(expression.first);
#endif 

			SendCommand(m_commandChannel.pointer(), m_commandChannel.size());
		}

		CMemoryWriter commandChannel;

		commandChannel.w_u16(CommandId_SetBreakPoints);
		commandChannel.w_u32(debugServer->m_aBreakpoints.size());

		//send breakpoints with offsets 
		for (auto breakpoint : debugServer->m_aBreakpoints)
		{
			commandChannel.w_u32(breakpoint.second.size());
			commandChannel.w_stringZ(breakpoint.first);

			for (auto line : breakpoint.second)
			{
				commandChannel.w_u32(line.first);
				commandChannel.w_s32(line.second);
			}
			}

		commandChannel.w_u32(debugServer->m_aOffsetPoints.size());

		//send line offsets 
		for (auto offset : debugServer->m_aOffsetPoints)
		{
			commandChannel.w_u32(offset.second.size());
			commandChannel.w_stringZ(offset.first);

			for (auto line : offset.second)
			{
				commandChannel.w_u32(line.first);
				commandChannel.w_s32(line.second);
			}
		}

		SendCommand(commandChannel.pointer(), commandChannel.size());
		}
	else if (m_commandFromClient == CommandId_EnterLoop)
	{
		debugServer->m_bEnterLoop = true;

		if (mainFrame->IsFocusable())
		{
			mainFrame->Iconize(false); // restore the window if minimized
			mainFrame->SetFocus();     // focus on my window
			mainFrame->Raise();        // bring window to front
		}

		wxString sFileName; commandReader.r_stringZ(sFileName);
		wxString sModuleName; commandReader.r_stringZ(sModuleName);

		wxDebugEvent event(EventId::EventId_EnterLoop, m_socketClient);

		event.SetFileName(sFileName);
		event.SetModuleName(sModuleName);
		event.SetLine(commandReader.r_s32());

		debugServer->NotifyEvent(event);
	}
	else if (m_commandFromClient == CommandId_LeaveLoop)
	{
		debugServer->m_bEnterLoop = false;

		wxString sFileName; commandReader.r_stringZ(sFileName);
		wxString sModuleName; commandReader.r_stringZ(sModuleName);

		wxDebugEvent event(EventId::EventId_LeaveLoop, m_socketClient);

		event.SetFileName(sFileName);
		event.SetModuleName(sModuleName);
		event.SetLine(commandReader.r_s32());

		debugServer->NotifyEvent(event);
	}
	else if (m_commandFromClient == CommandId_EvalToolTip)
	{
		wxString sModuleName, sExpression, sResult;

		commandReader.r_stringZ(sModuleName);
		commandReader.r_stringZ(sExpression);
		commandReader.r_stringZ(sResult);

		if (debugServer->IsEnterLoop())
		{
			wxDebugToolTipEvent event(EventId::EventId_SetToolTip, m_socketClient);

			event.SetModuleName(sModuleName);
			event.SetExpression(sExpression);
			event.SetResult(sResult);

			debugServer->NotifyEvent(event);
		}
	}
	else if (m_commandFromClient == CommandId_EvalAutocomplete)
	{
#if defined(_USE_64_BIT_POINT_IN_DEBUG)
		CAutocomplectionCtrl *m_autocompleteCtrl = reinterpret_cast<CAutocomplectionCtrl *>(commandReader.r_u64());
#else 
		CAutocomplectionCtrl *m_autocompleteCtrl = reinterpret_cast<CAutocomplectionCtrl *>(commandReader.r_u32());
#endif 
		if (m_autocompleteCtrl) m_autocompleteCtrl->ShowAutoCompleteFromDebugger(commandReader);
	}
	else if (m_commandFromClient == CommandId_SetExpressions)
	{
		watchWindow->SetVariable(commandReader);
	}
	else if (m_commandFromClient == CommandId_ExpandExpression)
	{
		watchWindow->SetExpanded(commandReader);
	}
	else if (m_commandFromClient == CommandId_SetStack)
	{
		stackWindow->SetStack(commandReader);
	}
	else if (m_commandFromClient == CommandId_SetLocalVariables)
	{
	}
	else if (m_commandFromClient == CommandId_MessageFromEnterprise)
	{
		if (mainFrame->IsFocusable())
		{
			mainFrame->Iconize(false); // restore the window if minimized
			mainFrame->SetFocus();     // focus on my window
			mainFrame->Raise();        // bring window to front
		}

		wxString sModule, sError; unsigned int nLine;

		commandReader.r_stringZ(sModule);
		nLine = commandReader.r_u32();
		commandReader.r_stringZ(sError);

		outputWindow->OutputError(sError);
	}

	debugServer->RecvCommand(pointer, length);
	}

void CDebuggerServer::CClientSocketThread::SendCommand(void *pointer, unsigned int length)
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