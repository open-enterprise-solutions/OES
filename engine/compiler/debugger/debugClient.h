#ifndef __DEBUGGER_CLIENT_H__
#define __DEBUGGER_CLIENT_H__

#include <map>
#include <queue>
#include <wx/thread.h>
#include <wx/socket.h>

class CRunContext;

#define debugClient           (CDebuggerClient::Get())
#define debugClientInit()     (CDebuggerClient::Initialize())
#define debugClientDestroy()  (CDebuggerClient::Destroy())

#include "debugDefs.h"

class CDebuggerClient : public wxThread
{
	static CDebuggerClient *s_instance;

	bool m_bUseDebug;
	bool m_bDoLoop;
	bool m_bDebugLoop;
	bool m_bDebugStopLine;

	unsigned int m_nCurrentNumberStopContext;

	std::map <wxString, std::map<unsigned int, int>> m_aBreakpoints; //список точек 
	std::map <wxString, std::map<unsigned int, int>> m_aOffsetPoints; //список измененных переходов

#if defined(_USE_64_BIT_POINT_IN_DEBUG)
	std::map <unsigned long long, wxString> m_aExpressions;
#else 
	std::map <unsigned int, wxString> m_aExpressions;
#endif 

	CRunContext *m_pRunContext;
	wxSocketClient *m_socketClient;

	CDebuggerClient();

public:

	static CDebuggerClient* Get();
	// Force the static appData instance to Init()
	static void Initialize();

	static void Destroy();

	~CDebuggerClient();

	bool Connect(const wxString &hostName = wxT("localhost"), unsigned short port = 1650);
	bool Disconnect();

	void EnterDebugger(CRunContext *pContext, struct CByte &CurCode, int &nPrevLine);
	void WaitDebugger();

	void InitializeBreakpoints(const wxString &sModuleName, unsigned int from, unsigned int to);
	void SendErrorToDesigner(const wxString &moduleName, unsigned int line, const wxString &errorMessage);

	// This one is called by Kill() before killing the thread and is executed
	// in the context of the thread that called Kill().
	virtual void OnKill() override;

	// entry point for the thread - called by Run() and executes in the context
	// of this thread.
	virtual ExitCode Entry() override;

protected:

	virtual void EntryClient();

	//main loop
	inline void DoDebugLoop(const wxString &filePath, const wxString &module, int nLine, CRunContext *pSetRunContext);

	//special functions:
	inline void SendExpressions();
	inline void SendLocalVariables();
	inline void SendStack();

	//commands:
	void RecvCommand(void *pointer, unsigned int length);
	void SendCommand(void *pointer, unsigned int length);

	friend class CTranslateError;
};

#endif // __DEBUGGER_CLIENT_H__
