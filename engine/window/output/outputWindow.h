#ifndef OUTPUT_WINDOW_H__
#define OUTPUT_WINDOW_H__

#include <wx/wx.h>
#include <wx/stc/stc.h>

//
// Forward declarations.
//

class CMainFrame;
class COutputWindow;
class FontColorSettings;

#define outputWindow           (COutputWindow::Get())

/**
 *
 */
class COutputWindow : public wxStyledTextCtrl
{
	static COutputWindow *s_instance;

	/**
	* Constructor.
	*/
	COutputWindow(wxWindow* parent, wxWindowID winid);

private:

	enum eStatusMessage
	{
		eMessage = 1,
		eWarning,
		eError
	};

public:

	static COutputWindow* Get();

	/**
	 * Sets the font and color settings used in the output window.
	 */
	void SetFontColorSettings(const FontColorSettings& settings);

	/**
	 * Called when the user double clicks in the window.
	 */
	void OnDoubleClick(wxMouseEvent& event);

	/**
	 * Adds a message to the end of the log.
	 */
	void OutputMessage(const wxString& message);

	/**
	 * Adds a warning message to the end of the log.
	 */
	void OutputWarning(const wxString& message);

	/**
	 * Adds an error message to the end of the log.
	 */
	void OutputError(const wxString& message);

	/**
	 * Returns the line that the cursor is positioned on.
	 */
	int GetCurrentLine() const;

private:

	/**
	 * Outputs text using the passed in text attribute. This method will only
	 * cause the output window to scroll down if the insertion point is already
	 * at the bottom of the text.
	 */
	void SharedOutput(const wxString& message, eStatusMessage status);

protected:

	wxDECLARE_EVENT_TABLE();
};

#endif