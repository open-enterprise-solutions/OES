#ifndef _DEBUGGER_DEFS_H__
#define _DEBUGGER_DEFS_H__

enum MessageType
{
	MessageType_Normal = 0,
	MessageType_Warning = 1,
	MessageType_Error = 2,
};

enum CodeState
{
	CodeState_Normal = 0,    // The code is normal.
	CodeState_Unavailable = 1,    // The code for the script was not available.
	CodeState_Binary = 2,    // The code was loaded as a binary/compiled file
};

enum EventId
{
	EventId_SessionStart = 1,   // Sent when the backend is ready to have its initialize function called
	EventId_SessionEnd = 2,    // This is used internally and shouldn't be sent.
	EventId_EnterLoop = 3,     // When enter to loop in enterprise mode 
	EventId_LeaveLoop = 4,      // When leave from loop in enterprise mode 

	EventId_SetToolTip = 5,      // When activate tooltip 
	EventId_StartAutocomplete = 6,      // When activate autocomplete 
	EventId_ShowAutocomplete = 7,      // When activate autocomplete 
	EventId_SetData = 8,      // When set data 
};

enum CommandId
{
	CommandId_Continue = 1,    // Continues execution until the next break point.
	CommandId_StepOver = 2,    // Steps to the next line, not entering any functions.
	CommandId_StepInto = 3,    // Steps to the next line, entering any functions.
	CommandId_ToggleBreakpoint = 4,    // Toggles a breakpoint on a line on and off.
	CommandId_RemoveBreakpoint = 5,    // Remove a breakpoint on a line on and off.
	CommandId_Pause = 6,    // Instructs the debugger to break on the next line of script code.
	CommandId_Detach = 8,    // Detaches the debugger from the process.
	CommandId_Destroy = 9,    // Destroy the enterprise from the process.

	CommandId_PatchInsertLine = 11,   // Adds a new line of code.
	CommandId_PatchDeleteLine = 12,   // Deletes a line of code.

	CommandId_PatchComplete = 13,   // Refresh patch line for dataProcessors

	CommandId_DeleteAllBreakpoints = 15,// Instructs the backend to clear all breakpoints set

	CommandId_EnterLoop = 16,// Instructs the backend to enter debug loop
	CommandId_LeaveLoop = 17,// Instructs the backend to leave debug loop

	CommandId_AddExpression = 18, // Get variables from enterprise 
	CommandId_ExpandExpression = 19, // Expand variable from enterprise 
	CommandId_RemoveExpression = 20, // Remove variable from enterprise 

	CommandId_GetBreakPoints = 22, // Wait breakpoits from designer
	CommandId_SetBreakPoints = 23, // Set breakpoits to enterprise

	CommandId_SetStack = 24, // Set up stack in designer 
	CommandId_SetLocalVariables = 25, // Set up local variables to designer 
	CommandId_SetExpressions = 26, //Set up all expressions

	CommandId_EvalToolTip = 27,
	CommandId_EvalAutocomplete = 28,

	CommandId_MessageFromEnterprise = 29 // When catch error in enterprise mode
};

#if defined(_LP64) || defined(__LP64__) || defined(__arch64__) || defined(_WIN64)
#define _USE_64_BIT_POINT_IN_DEBUG
#else 
#define _USE_64_BIT_POINT_IN_DEBUG 
#endif 

//#define _USE_NET_COMPRESSOR 

#endif 