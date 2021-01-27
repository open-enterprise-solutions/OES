#pragma once 

#define ICON_SIZE 22
#define TOOL_SIZE 22
#define SMALL_ICON_SIZE 14

#include <wx/wx.h>
#include <map>

class AppBitmaps
{
public:
	static wxBitmap GetBitmap( wxString iconname, unsigned int size = 0 );
	static void LoadBitmaps( wxString filepath, wxString iconpath );
};
