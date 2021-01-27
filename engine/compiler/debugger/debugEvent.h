#ifndef DEBUG_EVENT_H
#define DEBUG_EVENT_H

#include <wx/event.h>
#include <wx/socket.h>
#include <wx/buffer.h>

#include "debugDefs.h"

//
// Event definitions.
//

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_LOCAL_EVENT_TYPE(wxEVT_DEBUG_EVENT, -1)
DECLARE_LOCAL_EVENT_TYPE(wxEVT_DEBUG_TOOLTIP_EVENT, -1)
DECLARE_LOCAL_EVENT_TYPE(wxEVT_DEBUG_AUTOCOMPLETE_EVENT, -1)
END_DECLARE_EVENT_TYPES()

/**
 * Event class used to pass information from the debug server to the
 * wxWidget UI.
 */
	class wxDebugEvent : public wxEvent
{

public:

	/**
	 * Constructor.
	 */
	wxDebugEvent(EventId eventId, wxSocketBase *connection, wxEventType commandType = wxEVT_DEBUG_EVENT);

	/**
	 * Returns the event id of the event.
	 */
	EventId GetEventId() const;

	/**
	 * Returns the index of the script the event relates to.
	 */
	unsigned int GetScriptIndex() const;

	/**
	 * Sets the index of the script the event relates to.
	 */
	void SetScriptIndex(unsigned int scriptIndex);

	/**
	 * Returns the number of the line in the script the event relates to.
	 */
	unsigned int GetLine() const;

	/**
	 * Sets the number of the line in the script the event relates to.
	 */
	void SetLine(unsigned int scriptIndex);

	/**
	 * Gets the boolean value for the event. This is a generic value that's
	 * meaning depends on the event.
	 */
	bool GetEnabled() const;

	/**
	 * Sets the boolean value for the event. This is a generic value that's
	 * meaning depends on the event.
	 */
	void SetEnabled(bool enabled);

	/**
	 * Returns the message associated with the event. Not all events will have
	 * messages.
	 */
	const wxString& GetMessage() const;

	/**
	 * Sets the message associated with the event.
	 */
	void SetMessage(const wxString& message);

	/**
	* Returns the message associated with the event. Not all events will have
	* messages.
	*/
	const wxString& GetModuleName() const;

	/**
	 * Sets the message associated with the event.
	 */
	void SetModuleName(const wxString& moduleName);

	/**
	* Returns the message associated with the event. Not all events will have
	* messages.
	*/
	const wxString& GetFileName() const;

	/**
	 * Sets the message associated with the event.
	 */
	void SetFileName(const wxString& fileName);

	/**
	 * Returns the type of the string message (error, warning, etc.) This is only
	 * relevant when the event deals with a message.
	 */
	MessageType GetMessageType() const;

	/**
	 * Sets the type of the string message. This is only relevant when the event
	 * deals with a message.
	 */
	void SetMessageType(MessageType messageType);

	/**
	 * From wxEvent.
	 */
	virtual wxEvent* Clone() const;

private:

	wxSocketBase*   m_socket;
	EventId         m_eventId;

	unsigned int    m_scriptIndex;
	unsigned int    m_line;

	bool            m_enabled;

	wxString        m_message;
	MessageType     m_messageType;

	wxString        m_moduleName;
	wxString        m_fileName;
};

/**
 * Event for special tooltip func
 */

class wxDebugToolTipEvent : public wxDebugEvent
{

public:

	/**
	* Constructor.
	*/
	wxDebugToolTipEvent(EventId eventId, wxSocketBase *connection);

	/**
	* Returns the message associated with the event. Not all events will have
	* messages.
	*/
	const wxString& GetExpression() const;

	/**
	 * Sets the message associated with the event.
	 */
	void SetExpression(const wxString& expression);

	/**
	* Returns the message associated with the event. Not all events will have
	* messages.
	*/
	const wxString& GetResult() const;

	/**
	 * Sets the message associated with the event.
	 */
	void SetResult(const wxString& result);

	/**
	* From wxEvent.
	*/
	virtual wxEvent* Clone() const;

private:

	wxString m_expression;
	wxString m_result;
};

/**
 * Event for special autocomplete func
 */
class wxDebugAutocompleteEvent : public wxEvent
{
public:

	wxDebugAutocompleteEvent(EventId eventId);

	/**
	* Returns the event id of the event.
	*/
	EventId GetEventId() const;

	/**
	* Returns the position associated with the event. Not all events will have
	* position.
	*/
	int GetCurrentPos();

	/**
	 * Sets the position associated with the event.
	 */
	void SetCurrentPos(int currPosition);

	/**
	* Returns the position associated with the event. Not all events will have
	* position.
	*/
	wxString GetKeyWord();

	/**
	 * Sets the position associated with the event.
	 */
	void SetKeyWord(const wxString &keyWord);

	/**
	* From wxEvent.
	*/
	virtual wxEvent* Clone() const;

private:

	EventId         m_eventId;
	wxString        m_expression;
	wxString        m_keyword;
	int             m_currentPos;
};

typedef void (wxEvtHandler::*wxDebugEventFunction)(wxDebugEvent&);
typedef void (wxEvtHandler::*wxDebugToolTipEventFunction)(wxDebugToolTipEvent&);
typedef void (wxEvtHandler::*wxDebugAutocompleteEventFunction)(wxDebugAutocompleteEvent&);

#define wxDebugEventHandler(func) \
  wxEVENT_HANDLER_CAST(wxDebugEventFunction, func)

#define wxDebugToolTipEventHandler(func) \
  wxEVENT_HANDLER_CAST(wxDebugToolTipEventFunction, func)

#define wxDebugAutocompleteEventHandler(func) \
  wxEVENT_HANDLER_CAST(wxDebugAutocompleteEventFunction, func)

#define EVT_DEBUG(fn) \
    DECLARE_EVENT_TABLE_ENTRY( wxEVT_DEBUG_EVENT, 0, -1, \
    (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent( wxDebugEventFunction, &fn ), (wxObject *) NULL ),

#define EVT_DEBUG_TOOLTIP_EVENT(fn) \
    DECLARE_EVENT_TABLE_ENTRY( wxEVT_DEBUG_TOOLTIP_EVENT, 0, -1, \
    (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent( wxDebugToolTipEventFunction, &fn ), (wxObject *) NULL ),

#define EVT_DEBUG_AUTOCOMPLETE_EVENT(fn) \
    DECLARE_EVENT_TABLE_ENTRY( wxEVT_DEBUG_AUTOCOMPLETE_EVENT, 0, -1, \
    (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent( wxDebugAutocompleteEventFunction, &fn ), (wxObject *) NULL ),

#endif