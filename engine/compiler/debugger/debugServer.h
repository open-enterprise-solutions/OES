#ifndef _DEBUGGER_SERVER_H__
#define _DEBUGGER_SERVER_H__

#include <wx/wx.h>
#include <wx/socket.h>
#include <wx/thread.h>
#include <map>

#define debugServer           (CDebuggerServer::Get())
#define debugServerInit()     (CDebuggerServer::Initialize())
#define debugServerDestroy()  (CDebuggerServer::Destroy())

#include "debugEvent.h"

class CDebuggerServer : public wxEvtHandler
{
	wxMutex mutex;

	wxSocketServer *m_socketServer;

	std::map <wxString, std::map<unsigned int, int>> m_aBreakpoints; //список точек 
	std::map <wxString, std::map<unsigned int, int>> m_aOffsetPoints; //список измененных переходов

#if defined(_USE_64_BIT_POINT_IN_DEBUG)
	std::map <unsigned long long, wxString> m_aExpressions;
#else 
	std::map <unsigned int, wxString> m_aExpressions;
#endif  

	bool m_bEnterLoop;

protected:

	static CDebuggerServer *s_instance;

	//support critical section in threading
	bool TryLock() { return mutex.TryLock() == wxMutexError::wxMUTEX_NO_ERROR; }
	void Lock() { mutex.Lock(); }
	void Unlock() { mutex.Unlock(); }

	class CClientSocketThread : public wxThread
	{
		friend class CDebuggerServer;

	private:

		wxSocketBase *m_socketClient;

	public:

		CClientSocketThread(wxSocketBase *soket);
		virtual ~CClientSocketThread();
		
		// This one is called by Kill() before killing the thread and is executed
		// in the context of the thread that called Kill().
		virtual void OnKill() override;
		
		// entry point for the thread - called by Run() and executes in the context
		// of this thread.
		virtual ExitCode Entry();

	protected:

		void EntryClient();

		void DeleteConnection();

		void RecvCommand(void *pointer, unsigned int length);
		void SendCommand(void *pointer, unsigned int length);
	};

	std::vector<CClientSocketThread *> m_aConnections;
	std::vector< wxEvtHandler* > m_aHandlers;

	CDebuggerServer();

public:

	static CDebuggerServer* Get();
	// Force the static appData instance to Init()
	static void Initialize();
	static void Destroy();

	virtual ~CDebuggerServer();

	//events: 
	void AddHandler(wxEvtHandler* handler);
	void RemoveHandler(wxEvtHandler* handler);

public:

	//special public function:
#if defined(_USE_64_BIT_POINT_IN_DEBUG)
	void AddExpression(const wxString &sExpression, unsigned long long id);
	void ExpandExpression(const wxString &sExpression, unsigned long long id);
	void RemoveExpression(unsigned long long id);
#else 
	void AddExpression(wxString sExpression, unsigned int id);
	void ExpandExpression(wxString sExpression, unsigned int id);
	void RemoveExpression(unsigned int id);
#endif 

	//evaluate for tooltip
	void EvaluateToolTip(const wxString &sModuleName, const wxString &sExpression);

	//support calc expression in debugloop
	void EvaluateAutocomplete(void *pointer, const wxString &sExpression, const wxString &sKeyWord, int currline);

	//get debug list
	std::vector<unsigned int> GetDebugList(const wxString &sModuleName);

	//special functions:
	void Continue();
	void StepOver();
	void StepInto();
	void Pause();
	void Stop(bool kill);

	//for breakpoints and offsets 
	void InitializeBreakpoints(const wxString &sModuleName, unsigned int from, unsigned int to);
	void PatchBreakpoints(const wxString &sModuleName, unsigned int line, int offsetLine);

	bool SaveBreakpoints(const wxString &sModuleName);
	bool SaveAllBreakpoints(); 

	bool ToggleBreakpoint(const wxString &sModuleName, unsigned int line);
	bool RemoveBreakpoint(const wxString &sModuleName, unsigned int line);
	void RemoveAllBreakPoints();

	bool HasConnections() { return m_aConnections.size() > 0; }
	bool IsEnterLoop() { return m_bEnterLoop; }

protected:

	void CreateServer(const wxString &hostName = wxT("localhost"), unsigned short port = 1650);

	//db support 
	void LoadBreakpoints();

	bool ToggleBreakpointInDB(const wxString &sModuleName, unsigned int line);
	bool RemoveBreakpointInDB(const wxString &sModuleName, unsigned int line);
	bool OffsetBreakpointInDB(const wxString &sModuleName, unsigned int line, int offset);
	bool RemoveAllBreakPointsInDB();

	//notify event: 
	void NotifyEvent(wxEvent& event);

	//commands:
	void DeleteConnection(CClientSocketThread *client);

	void RecvCommand(void *pointer, unsigned int length);
	void SendCommand(void *pointer, unsigned int length);

	//events:
	void OnSocketServerEvent(wxSocketEvent &evt);

	void OnDebugEvent(wxDebugEvent &event); 
	void OnDebugToolTipEvent(wxDebugToolTipEvent &event);
	void OnDebugAutoCompleteEvent(wxDebugAutocompleteEvent &event);

	wxDECLARE_EVENT_TABLE();
};

#endif