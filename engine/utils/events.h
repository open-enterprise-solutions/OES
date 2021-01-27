#ifndef _EVENT_H__
#define _EVENT_H__

#include <wx/event.h>
#include "common/formdefs.h"

class wxFrameEvent : public wxEvent
{
private:
	wxString m_string;

public:
	wxFrameEvent(wxEventType commandType = wxEVT_NULL);
	wxFrameEvent(const wxFrameEvent& event);
	~wxFrameEvent() override;

	wxString GetEventName();

	void SetString(const wxString& newString);
	wxString GetString();

	// required for sending with wxPostEvent()
	wxEvent* Clone() const override;
};

class wxFramePropertyEvent : public wxFrameEvent
{
public:
	wxFramePropertyEvent(wxEventType commandType, Property *property, const wxString &oldValue);
	wxFramePropertyEvent(const wxFramePropertyEvent& event);
	wxEvent* Clone() const override;
	wxString GetValue() { return m_strValue; }
	Property *GetFrameProperty() { return m_property; }
private:
	wxString m_strValue;
	Property *m_property;
};

class wxFrameEventHandlerEvent : public wxFrameEvent
{
public:
	wxFrameEventHandlerEvent(wxEventType commandType, Event* event);
	wxFrameEventHandlerEvent(const wxFrameEventHandlerEvent& event);
	wxEvent* Clone() const override;
	Event *GetFrameEventHandler() { return m_event; }
private:
	Event *m_event;
};

class wxFrameObjectEvent : public wxFrameEvent
{
public:
	wxFrameObjectEvent(wxEventType commandType, IControlElement* object);
	wxFrameObjectEvent(const wxFrameObjectEvent& event);
	wxEvent* Clone() const override;
	IControlElement* GetFrameObject() { return m_object; }

private:
	IControlElement* m_object;
};

typedef void (wxEvtHandler::*wxFrameEventFunction)        (wxFrameEvent&);
typedef void (wxEvtHandler::*wxFramePropertyEventFunction)(wxFramePropertyEvent&);
typedef void (wxEvtHandler::*wxFrameObjectEventFunction)  (wxFrameObjectEvent&);
typedef void (wxEvtHandler::*wxFrameEventHandlerEventFunction)  (wxFrameEventHandlerEvent&);

#define wxFrameEventHandler(func) \
  wxEVENT_HANDLER_CAST(wxFrameEventFunction, func)

#define wxFramePropertyEventHandler(func) \
  wxEVENT_HANDLER_CAST(wxFramePropertyEventFunction, func)

#define wxFrameObjectEventHandler( func ) \
  wxEVENT_HANDLER_CAST( wxFrameObjectEventFunction, func )

#define wxFrameEventEventHandler(func) \
 wxEVENT_HANDLER_CAST(wxFrameEventHandlerEventFunction, func)

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_LOCAL_EVENT_TYPE(wxEVT_PROJECT_LOADED, -1)
DECLARE_LOCAL_EVENT_TYPE(wxEVT_PROJECT_SAVED, -1)
DECLARE_LOCAL_EVENT_TYPE(wxEVT_OBJECT_EXPANDED, -1)
DECLARE_LOCAL_EVENT_TYPE(wxEVT_OBJECT_SELECTED, -1)
DECLARE_LOCAL_EVENT_TYPE(wxEVT_OBJECT_CREATED, -1)
DECLARE_LOCAL_EVENT_TYPE(wxEVT_OBJECT_REMOVED, -1)
DECLARE_LOCAL_EVENT_TYPE(wxEVT_PROPERTY_MODIFIED, -1)
DECLARE_LOCAL_EVENT_TYPE(wxEVT_PROJECT_REFRESH, -1)
DECLARE_LOCAL_EVENT_TYPE(wxEVT_CODE_GENERATION, -1)
DECLARE_LOCAL_EVENT_TYPE(wxEVT_EVENT_HANDLER_MODIFIED, -1)
END_DECLARE_EVENT_TYPES()

#define EVT_PROJECT_LOADED(fn) \
  wx__DECLARE_EVT0(wxEVT_PROJECT_LOADED,wxFrameEventHandler(fn))

#define EVT_PROJECT_SAVED(fn) \
  wx__DECLARE_EVT0(wxEVT_PROJECT_SAVED,wxFrameEventHandler(fn))

#define EVT_OBJECT_EXPANDED(fn) \
  wx__DECLARE_EVT0(wxEVT_OBJECT_EXPANDED,wxFrameObjectEventHandler(fn))

#define EVT_OBJECT_SELECTED(fn) \
  wx__DECLARE_EVT0(wxEVT_OBJECT_SELECTED,wxFrameObjectEventHandler(fn))

#define EVT_OBJECT_CREATED(fn) \
  wx__DECLARE_EVT0(wxEVT_OBJECT_CREATED,wxFrameObjectEventHandler(fn))

#define EVT_OBJECT_REMOVED(fn) \
  wx__DECLARE_EVT0(wxEVT_OBJECT_REMOVED,wxFrameObjectEventHandler(fn))

#define EVT_PROPERTY_MODIFIED(fn) \
  wx__DECLARE_EVT0(wxEVT_PROPERTY_MODIFIED,wxFramePropertyEventHandler(fn))

#define EVT_FRAME_EVENT_HANDLER_MODIFIED(fn) \
  wx__DECLARE_EVT0(wxEVT_EVENT_HANDLER_MODIFIED,wxFrameEventEventHandler(fn))

#define EVT_PROJECT_REFRESH(fn) \
  wx__DECLARE_EVT0(wxEVT_PROJECT_REFRESH,wxFrameEventHandler(fn))

#define EVT_CODE_GENERATION(fn) \
  wx__DECLARE_EVT0(wxEVT_CODE_GENERATION,wxFrameEventHandler(fn))

#endif