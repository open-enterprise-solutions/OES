////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : compile defines  
////////////////////////////////////////////////////////////////////////////

#include "compiler.h"

#pragma warning(push)
#pragma warning(disable : 4996)

wxBitmap wxGetImageBMPFromResource(long id)
{
	WXHBITMAP m_bmp_data = ::LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(id));
	wxBitmap m_bmp;
	m_bmp.SetHBITMAP(m_bmp_data);

	return m_bmp;
}

#include "definition.h"
#include "procUnit.h"

inline void ThrowErrorTypeOperation(const wxString &fromType)
{
	CProcUnit::Raise(); CTranslateError::Error(ERROR_TYPE_OPERATION, fromType.wc_str());
}

wxImageList *GetImageList()
{
	static bool bPictureForIcon = false;
	static wxImageList PictureForIcon(16, 16);

	if (!bPictureForIcon)
	{
		PictureForIcon.Add(wxGetImageBMPFromResource(IDB_BITMAP1), wxColour(0, 128, 128));
		PictureForIcon.Add(wxGetImageBMPFromResource(IDB_BITMAP2), wxColour(0, 128, 128));
		PictureForIcon.Add(wxGetImageBMPFromResource(IDB_BITMAP3), wxColour(0, 128, 128));
		PictureForIcon.Add(wxGetImageBMPFromResource(IDB_BITMAP4), wxColour(0, 128, 128));
		PictureForIcon.Add(wxGetImageBMPFromResource(IDB_BITMAP5), wxColour(0, 128, 128));
		PictureForIcon.Add(wxGetImageBMPFromResource(IDB_BITMAP6), wxColour(0, 128, 128));
		PictureForIcon.Add(wxGetImageBMPFromResource(IDB_BITMAP7), wxColour(0, 128, 128));
		PictureForIcon.Add(wxGetImageBMPFromResource(IDB_BITMAP8), wxColour(0, 128, 128));
		PictureForIcon.Add(wxGetImageBMPFromResource(IDB_BITMAP9), wxColour(0, 128, 128));
		PictureForIcon.Add(wxGetImageBMPFromResource(IDB_BITMAP10), wxColour(0, 128, 128));
		bPictureForIcon = true;
	}

	return &PictureForIcon;
}

#pragma warning(pop)