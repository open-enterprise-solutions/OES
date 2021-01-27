#include "advprops.h"

#include "appData.h"
#include "utils/typeconv.h"
#include "metadata/objects/baseObject.h"

#include <wx/regex.h>

// -----------------------------------------------------------------------
// wxPGSizeProperty
// -----------------------------------------------------------------------
#if wxCHECK_VERSION(3, 1, 0)
wxPG_IMPLEMENT_PROPERTY_CLASS(wxPGSizeProperty, wxPGProperty, TextCtrl)
#else
WX_PG_IMPLEMENT_PROPERTY_CLASS(wxPGSizeProperty, wxPGProperty, wxSize, const wxSize&, TextCtrl)
#endif

wxPGSizeProperty::wxPGSizeProperty(const wxString& label,
	const wxString& name,
	const wxSize&   value) : wxPGProperty(label, name)
{
	DoSetValue(value);
	AddPrivateChild(new wxIntProperty(wxT("Width"), wxPG_LABEL, value.x));
	AddPrivateChild(new wxIntProperty(wxT("Height"), wxPG_LABEL, value.y));
}

void wxPGSizeProperty::RefreshChildren()
{
	if (GetChildCount() < 2) return;

	const wxSize& size = wxSizeRefFromVariant(m_value);

	Item(0)->SetValue((long)size.x);
	Item(1)->SetValue((long)size.y);
}

wxVariant wxPGSizeProperty::ChildChanged(wxVariant& thisValue, const int childIndex,
	wxVariant& childValue) const {
	wxSize& size = wxSizeRefFromVariant(thisValue);

	wxVariant();

	int val = childValue.GetLong();
	switch (childIndex)
	{
	case 0:
		size.x = val;
		break;
	case 1:
		size.y = val;
		break;
	}

	wxVariant newVariant;
	newVariant << size;
	return newVariant;
}

// -----------------------------------------------------------------------
// wxPGPointProperty
// -----------------------------------------------------------------------

#if wxCHECK_VERSION(3, 1, 0)
wxPG_IMPLEMENT_PROPERTY_CLASS(wxPGPointProperty, wxPGProperty, TextCtrl)
#else
WX_PG_IMPLEMENT_PROPERTY_CLASS(wxPGPointProperty, wxPGProperty, wxPoint, const wxPoint&, TextCtrl)
#endif

wxPGPointProperty::wxPGPointProperty(const wxString& label,
	const wxString& name,
	const wxPoint&  value) : wxPGProperty(label, name)
{
	DoSetValue(value);
	AddPrivateChild(new wxIntProperty(wxT("X"), wxPG_LABEL, value.x));
	AddPrivateChild(new wxIntProperty(wxT("Y"), wxPG_LABEL, value.y));
}

wxPGPointProperty::~wxPGPointProperty() { }

void wxPGPointProperty::RefreshChildren()
{
	if (GetChildCount() < 2) return;

	const wxPoint& point = wxPointRefFromVariant(m_value);

	Item(0)->SetValue((long)point.x);
	Item(1)->SetValue((long)point.y);
}

wxVariant wxPGPointProperty::ChildChanged(wxVariant& thisValue, const int childIndex,
	wxVariant& childValue) const {
	wxPoint& point = wxPointRefFromVariant(thisValue);

	wxVariant();

	int val = childValue.GetLong();
	switch (childIndex)
	{
	case 0:
		point.x = val;
		break;
	case 1:
		point.y = val;
		break;
	}

	wxVariant newVariant;
	newVariant << point;
	return newVariant;
}

// -----------------------------------------------------------------------
// wxPGBitmapProperty
// -----------------------------------------------------------------------

// static long gs_imageFilterIndex = -1; TODO: new wxPropertyGrid misses the
//                                             wxPG_FILE_FILTER_INDEX attribute ID
static wxString gs_imageInitialPath = wxEmptyString;

#if wxCHECK_VERSION(3, 1, 0)
wxPG_IMPLEMENT_PROPERTY_CLASS(wxPGBitmapProperty, wxPGProperty, TextCtrl)
#else
WX_PG_IMPLEMENT_PROPERTY_CLASS(wxPGBitmapProperty, wxPGProperty,
	wxString, const wxString&, TextCtrl)
#endif

	void wxPGBitmapProperty::GetChildValues(const wxString& parentValue, wxArrayString& childValues) const
{
	// some properties can contain value like "[-1;-1]" which must be modified due to use of ";" as a
	// string separator
	wxString values = parentValue;

	wxRegEx regex(wxT("\\[.+;.+\\]"));
	if (regex.IsValid())
	{
		if (regex.Matches(values))
		{
			wxString sizeVal = regex.GetMatch(values);
			sizeVal.Replace(wxT(";"), wxT("<semicolon>"));
			sizeVal.Replace(wxT("["), wxT(""));
			sizeVal.Replace(wxT("]"), wxT(""));
			regex.Replace(&values, sizeVal);
		}
	}

	childValues = wxStringTokenize(values, wxT(';'), wxTOKEN_RET_EMPTY_ALL);
	for (wxArrayString::iterator value = childValues.begin(); value != childValues.end(); ++value)
	{
		value->Trim(false);
		value->Replace(wxT("<semicolon>"), wxT(";"));
	}
}

wxPGBitmapProperty::wxPGBitmapProperty(const wxString& label,
	const wxString& name,
	const wxString& value) : wxPGProperty(label, name)
{
	SetValue(WXVARIANT(value));
}

void wxPGBitmapProperty::CreateChildren()
{
	wxString  propValue = m_value.GetString();
	wxVariant thisValue = WXVARIANT(propValue);
	wxVariant childValue;
	int       childIndex = 0;
	wxArrayString childVals;
	GetChildValues(propValue, childVals);
	wxString  source;
	if (childVals.Count() > 0)
	{
		source = childVals.Item(0);
	}
	else
	{
		source = _("Load From File");
	}
	prevSrc = -1;
	if (source == wxString(_("Load From File")))
	{
		childIndex = 0;
	}
	else if (source == wxString(_("Load From Embedded File")))
	{
		childIndex = 1;
	}
	else if (source == wxString(_("Load From Resource")))
	{
		childIndex = 2;
	}
	else if (source == wxString(_("Load From Icon Resource")))
	{
		childIndex = 3;
	}
	else if (source == wxString(_("Load From Art Provider")))
	{
		childIndex = 4;
	}

	childValue = WXVARIANT(childIndex);

	CreatePropertySource(childIndex);

	ChildChanged(thisValue, 0, childValue);
}

wxPGProperty *wxPGBitmapProperty::CreatePropertySource(int sourceIndex)
{
	wxPGChoices sourceChoices;

	// Add 'source' property (common for all other children)
	sourceChoices.Add(_("Load From File"));
	sourceChoices.Add(_("Load From Embedded File"));
	sourceChoices.Add(_("Load From Resource"));
	sourceChoices.Add(_("Load From Icon Resource"));
	sourceChoices.Add(_("Load From Art Provider"));

	wxPGProperty *srcProp = new wxEnumProperty(wxT("source"), wxPG_LABEL, sourceChoices, sourceIndex);
	srcProp->SetHelpString(wxString(_("Load From File:\n")) +
		wxString(_("Load the image from a file on disk.\n\n")) +
		wxString(_("Load From Embedded File:\n")) +
		wxString(_("C++ Only. Embed the image file in the exe and load it.\nFor other languages, behaves like \"Load From File\".\n\n")) +
		wxString(_("Load From Resource:\n")) +
		wxString(_("Windows Only. Load the image from a BITMAP resource in a .rc file\n\n")) +
		wxString(_("Load From Icon Resource:\n")) +
		wxString(_("Windows Only. Load the image from a ICON resource in a .rc file\n\n")) +
		wxString(_("Load From Art Provider:\n")) +
		wxString(_("Query registered providers for bitmap with given ID.\n\n")));
	AppendChild(srcProp);

	return srcProp;
}

wxPGProperty *wxPGBitmapProperty::CreatePropertyFilePath()
{
	// Add 'file_path' property (common for 'Load From File' and 'Load From Embedded File' choices)
	wxPGProperty *propFilePath = new wxImageFileProperty(wxT("file_path"), wxPG_LABEL);
	propFilePath->SetHelpString(_("Path to the image file."));

	if (!gs_imageInitialPath.IsEmpty())
	{
		//wxVariant initialPath( gs_imageInitialPath );
		//propFilePath->SetAttribute( wxPG_FILE_INITIAL_PATH, initialPath );
	}

	return propFilePath;
}

wxPGProperty *wxPGBitmapProperty::CreatePropertyResourceName()
{
	// Create 'resource_name' property (common for 'Load From Resource' and 'Load From Icon Resource' choices)
	wxPGProperty *propResName = new wxStringProperty(wxT("resource_name"), wxPG_LABEL);
	propResName->SetHelpString(_("Windows Only. Name of the resource in the .rc file."));

	return propResName;
}

wxPGProperty *wxPGBitmapProperty::CreatePropertyIconSize()
{
	// Create 'ico_size' property ('Load From Icon Resource' only)
	wxPGProperty *propIcoSize = new wxPGSizeProperty(wxT("ico_size"), wxPG_LABEL, wxDefaultSize);
	propIcoSize->SetHelpString(_("The size of the icon to use from a ICON resource with multiple icons in it."));

	return propIcoSize;
}

wxPGProperty *wxPGBitmapProperty::CreatePropertyArtId()
{
	wxPGChoices artIdChoices;

	// Create 'id' property ('Load From Art Provider' only)
	artIdChoices.Add(wxT("wxART_ADD_BOOKMARK"));
	artIdChoices.Add(wxT("wxART_DEL_BOOKMARK"));
	artIdChoices.Add(wxT("wxART_HELP_SIDE_PANEL"));
	artIdChoices.Add(wxT("wxART_HELP_SETTINGS"));
	artIdChoices.Add(wxT("wxART_HELP_BOOK"));
	artIdChoices.Add(wxT("wxART_HELP_FOLDER"));
	artIdChoices.Add(wxT("wxART_HELP_PAGE"));
	artIdChoices.Add(wxT("wxART_GO_BACK"));
	artIdChoices.Add(wxT("wxART_GO_FORWARD"));
	artIdChoices.Add(wxT("wxART_GO_UP"));
	artIdChoices.Add(wxT("wxART_GO_DOWN"));
	artIdChoices.Add(wxT("wxART_GO_TO_PARENT"));
	artIdChoices.Add(wxT("wxART_GO_HOME"));
	artIdChoices.Add(wxT("wxART_FILE_OPEN"));
	artIdChoices.Add(wxT("wxART_FILE_SAVE"));
	artIdChoices.Add(wxT("wxART_FILE_SAVE_AS"));
	artIdChoices.Add(wxT("wxART_GOTO_FIRST"));
	artIdChoices.Add(wxT("wxART_GOTO_LAST"));
	artIdChoices.Add(wxT("wxART_PRINT"));
	artIdChoices.Add(wxT("wxART_HELP"));
	artIdChoices.Add(wxT("wxART_TIP"));
	artIdChoices.Add(wxT("wxART_REPORT_VIEW"));
	artIdChoices.Add(wxT("wxART_LIST_VIEW"));
	artIdChoices.Add(wxT("wxART_NEW_DIR"));
	artIdChoices.Add(wxT("wxART_HARDDISK"));
	artIdChoices.Add(wxT("wxART_FLOPPY"));
	artIdChoices.Add(wxT("wxART_CDROM"));
	artIdChoices.Add(wxT("wxART_REMOVABLE"));
	artIdChoices.Add(wxT("wxART_FOLDER"));
	artIdChoices.Add(wxT("wxART_FOLDER_OPEN"));
	artIdChoices.Add(wxT("wxART_GO_DIR_UP"));
	artIdChoices.Add(wxT("wxART_EXECUTABLE_FILE"));
	artIdChoices.Add(wxT("wxART_NORMAL_FILE"));
	artIdChoices.Add(wxT("wxART_TICK_MARK"));
	artIdChoices.Add(wxT("wxART_CROSS_MARK"));
	artIdChoices.Add(wxT("wxART_ERROR"));
	artIdChoices.Add(wxT("wxART_QUESTION"));
	artIdChoices.Add(wxT("wxART_WARNING"));
	artIdChoices.Add(wxT("wxART_INFORMATION"));
	artIdChoices.Add(wxT("wxART_MISSING_IMAGE"));
	artIdChoices.Add(wxT("wxART_COPY"));
	artIdChoices.Add(wxT("wxART_CUT"));
	artIdChoices.Add(wxT("wxART_PASTE"));
	artIdChoices.Add(wxT("wxART_DELETE"));
	artIdChoices.Add(wxT("wxART_NEW"));
	artIdChoices.Add(wxT("wxART_UNDO"));
	artIdChoices.Add(wxT("wxART_REDO"));
	artIdChoices.Add(wxT("wxART_PLUS"));
	artIdChoices.Add(wxT("wxART_MINUS"));
	artIdChoices.Add(wxT("wxART_CLOSE"));
	artIdChoices.Add(wxT("wxART_QUIT"));
	artIdChoices.Add(wxT("wxART_FIND"));
	artIdChoices.Add(wxT("wxART_FIND_AND_REPLACE"));
	artIdChoices.Add(wxT("wxART_FULL_SCREEN"));
	artIdChoices.Add(wxT("wxART_EDIT"));

	artIdChoices.Add(wxT("gtk-about"));
	artIdChoices.Add(wxT("gtk-add"));
	artIdChoices.Add(wxT("gtk-apply"));
	artIdChoices.Add(wxT("gtk-bold"));
	artIdChoices.Add(wxT("gtk-cancel"));
	artIdChoices.Add(wxT("gtk-caps-lock-warning"));
	artIdChoices.Add(wxT("gtk-cdrom"));
	artIdChoices.Add(wxT("gtk-clear"));
	artIdChoices.Add(wxT("gtk-close"));
	artIdChoices.Add(wxT("gtk-color-picker"));
	artIdChoices.Add(wxT("gtk-convert"));
	artIdChoices.Add(wxT("gtk-copy"));
	artIdChoices.Add(wxT("gtk-cut"));
	artIdChoices.Add(wxT("gtk-delete"));
	artIdChoices.Add(wxT("gtk-dialog-authentication"));
	artIdChoices.Add(wxT("gtk-dialog-error"));
	artIdChoices.Add(wxT("gtk-dialog-info"));
	artIdChoices.Add(wxT("gtk-dialog-question"));
	artIdChoices.Add(wxT("gtk-dialog-warning"));
	artIdChoices.Add(wxT("gtk-warning"));
	artIdChoices.Add(wxT("gtk-discard"));
	artIdChoices.Add(wxT("gtk-disconnect"));
	artIdChoices.Add(wxT("gtk-dnd"));
	artIdChoices.Add(wxT("gtk-dnd-multiple"));
	artIdChoices.Add(wxT("gtk-edit"));
	artIdChoices.Add(wxT("gtk-execute"));
	artIdChoices.Add(wxT("gtk-file"));
	artIdChoices.Add(wxT("gtk-find"));
	artIdChoices.Add(wxT("gtk-find-and-replace"));
	artIdChoices.Add(wxT("gtk-fullscreen"));
	artIdChoices.Add(wxT("gtk-goto-bottom"));
	artIdChoices.Add(wxT("gtk-goto-first"));
	artIdChoices.Add(wxT("gtk-goto-last"));
	artIdChoices.Add(wxT("gtk-goto-top"));
	artIdChoices.Add(wxT("gtk-go-back"));
	artIdChoices.Add(wxT("gtk-go-down"));
	artIdChoices.Add(wxT("gtk-go-forward"));
	artIdChoices.Add(wxT("gtk-go-up"));
	artIdChoices.Add(wxT("gtk-harddisk"));
	artIdChoices.Add(wxT("gtk-indent"));
	artIdChoices.Add(wxT("gtk-index"));
	artIdChoices.Add(wxT("gtk-info"));
	artIdChoices.Add(wxT("gtk-italic"));
	artIdChoices.Add(wxT("gtk-jump-to"));
	artIdChoices.Add(wxT("gtk-justify-center"));
	artIdChoices.Add(wxT("gtk-justify-fill"));
	artIdChoices.Add(wxT("gtk-justify-left"));
	artIdChoices.Add(wxT("gtk-justify-right"));
	artIdChoices.Add(wxT("gtk-leave-fullscreen"));
	artIdChoices.Add(wxT("gtk-media-forward"));
	artIdChoices.Add(wxT("gtk-media-next"));
	artIdChoices.Add(wxT("gtk-media-forward"));
	artIdChoices.Add(wxT("gtk-media-pause"));
	artIdChoices.Add(wxT("gtk-media-play"));
	artIdChoices.Add(wxT("gtk-media-previous"));
	artIdChoices.Add(wxT("gtk-media-record"));
	artIdChoices.Add(wxT("gtk-media-rewind"));
	artIdChoices.Add(wxT("gtk-media-stop"));
	artIdChoices.Add(wxT("gtk-missing-image"));
	artIdChoices.Add(wxT("gtk-network"));
	artIdChoices.Add(wxT("gtk-new"));
	artIdChoices.Add(wxT("gtk-no"));
	artIdChoices.Add(wxT("gtk-ok"));
	artIdChoices.Add(wxT("gtk-open"));
	artIdChoices.Add(wxT("gtk-orientation-landscape"));
	artIdChoices.Add(wxT("gtk-orientation-portrait"));
	artIdChoices.Add(wxT("gtk-orientation-reverse-landscape"));
	artIdChoices.Add(wxT("gtk-orientation-reverse-portrait"));
	artIdChoices.Add(wxT("gtk-page-setup"));
	artIdChoices.Add(wxT("gtk-paste"));
	artIdChoices.Add(wxT("gtk-preferences"));
	artIdChoices.Add(wxT("gtk-print"));
	artIdChoices.Add(wxT("gtk-print-paused"));
	artIdChoices.Add(wxT("gtk-print-report"));
	artIdChoices.Add(wxT("gtk-print-warning"));
	artIdChoices.Add(wxT("gtk-properties"));
	artIdChoices.Add(wxT("gtk-quit"));
	artIdChoices.Add(wxT("gtk-redo"));
	artIdChoices.Add(wxT("gtk-refresh"));
	artIdChoices.Add(wxT("gtk-remove"));
	artIdChoices.Add(wxT("gtk-save"));
	artIdChoices.Add(wxT("gtk-save-as"));
	artIdChoices.Add(wxT("gtk-select-all"));
	artIdChoices.Add(wxT("gtk-select-color"));
	artIdChoices.Add(wxT("gtk-select-font"));
	artIdChoices.Add(wxT("gtk-sort-ascending"));
	artIdChoices.Add(wxT("gtk-sort-descending"));
	artIdChoices.Add(wxT("gtk-spell-check"));
	artIdChoices.Add(wxT("gtk-stop"));
	artIdChoices.Add(wxT("gtk-strikethrough"));
	artIdChoices.Add(wxT("gtk-undelete"));
	artIdChoices.Add(wxT("gtk-underline"));
	artIdChoices.Add(wxT("gtk-undo"));
	artIdChoices.Add(wxT("gtk-unindent"));
	artIdChoices.Add(wxT("gtk-yes"));
	artIdChoices.Add(wxT("gtk-zoom-100"));
	artIdChoices.Add(wxT("gtk-zoom-fit"));
	artIdChoices.Add(wxT("gtk-zoom-in"));
	artIdChoices.Add(wxT("gtk-zoom-out"));

	wxPGProperty *propArtId = new wxEditEnumProperty(wxT("id"), wxPG_LABEL, artIdChoices);
	propArtId->SetHelpString(_("Choose a wxArtID unique identifier of the bitmap or enter a wxArtID for your custom wxArtProvider. IDs with prefix 'gtk-' are available under wxGTK only."));

	return propArtId;
}

wxPGProperty *wxPGBitmapProperty::CreatePropertyArtClient()
{
	wxPGChoices artClientChoices;

	// Create 'client' property ('Load From Art Provider' only)
	artClientChoices.Add(wxT("wxART_TOOLBAR"));
	artClientChoices.Add(wxT("wxART_MENU"));
	artClientChoices.Add(wxT("wxART_BUTTON"));
	artClientChoices.Add(wxT("wxART_FRAME_ICON"));
	artClientChoices.Add(wxT("wxART_CMN_DIALOG"));
	artClientChoices.Add(wxT("wxART_HELP_BROWSER"));
	artClientChoices.Add(wxT("wxART_MESSAGE_BOX"));
	artClientChoices.Add(wxT("wxART_OTHER"));

	wxPGProperty *propArtClient = new wxEditEnumProperty(wxT("client"), wxPG_LABEL, artClientChoices);
	propArtClient->SetHelpString(_("Choose a wxArtClient identifier of the client (i.e. who is asking for the bitmap) or enter a wxArtClient for your custom wxArtProvider."));

	return propArtClient;
}

wxPGBitmapProperty::~wxPGBitmapProperty()
{
}

wxVariant wxPGBitmapProperty::ChildChanged(wxVariant& thisValue, const int childIndex,
	wxVariant& childValue) const {
	wxPGBitmapProperty* bp = (wxPGBitmapProperty*)this;

	wxString val = thisValue.GetString();
	wxArrayString childVals;
	GetChildValues(val, childVals);
	wxString newVal = val;

	// Find the appropriate new state
	switch (childIndex)
	{
		// source
	case 0:
	{
		unsigned int count = GetChildCount();

		// childValue.GetInteger() returns the chosen item index
		switch (childValue.GetInteger())
		{
			// 'Load From File' and 'Load From Embedded File'
		case 0:
		case 1:
		{
			if (prevSrc != 0 && prevSrc != 1)
			{
				for (unsigned int i = 1; i < count; i++)
				{
					wxPGProperty *p = Item(i);
					if (p)
					{
						wxLogDebug(wxT("wxOESBP::ChildChanged: Removing:%s"), p->GetLabel().c_str());
						GetGrid()->DeleteProperty(p);
					}
				}
				bp->AppendChild(bp->CreatePropertyFilePath());
			}

			if (childVals.GetCount() == 2)
				newVal = childVals.Item(0) + wxT("; ") + childVals.Item(1);
			else if (childVals.GetCount() > 1)
				newVal = childVals.Item(0) + wxT("; ");

			break;
		}
		// 'Load From Resource'
		case 2:
		{
			if (prevSrc != 2)
			{
				for (unsigned int i = 1; i < count; i++)
				{
					wxPGProperty *p = Item(i);
					if (p)
					{
						wxLogDebug(wxT("wxOESBP::ChildChanged: Removing:%s"), p->GetLabel().c_str());
						GetGrid()->DeleteProperty(p);
					}
				}
				bp->AppendChild(bp->CreatePropertyResourceName());
			}

			if (childVals.GetCount() == 2)
				newVal = childVals.Item(0) + wxT("; ") + childVals.Item(1);
			else if (childVals.GetCount() > 1)
				newVal = childVals.Item(0) + wxT("; ");

			break;
		}
		// 'Load From Icon Resource'
		case 3:
		{
			if (prevSrc != 3)
			{
				for (unsigned int i = 1; i < count; i++)
				{
					wxPGProperty *p = Item(i);
					if (p)
					{
						wxLogDebug(wxT("wxOESBP::ChildChanged: Removing:%s"), p->GetLabel().c_str());
						GetGrid()->DeleteProperty(p);
					}
				}
				bp->AppendChild(bp->CreatePropertyResourceName());
				bp->AppendChild(bp->CreatePropertyIconSize());
			}

			if (childVals.GetCount() == 3)
				newVal = childVals.Item(0) + wxT("; ") + childVals.Item(1) + wxT("; [") + childVals.Item(2) + wxT("]");
			else if (childVals.GetCount() > 1)
				newVal = childVals.Item(0) + wxT("; ; []");

			break;
		}
		// 'Load From Art Provider'
		case 4:
		{
			if (prevSrc != 4)
			{
				for (unsigned int i = 1; i < count; i++)
				{
					wxPGProperty *p = Item(i);
					if (p)
					{
						wxLogDebug(wxT("wxOESBP::ChildChanged: Removing:%s"), p->GetLabel().c_str());
						GetGrid()->DeleteProperty(p);
					}
				}
				bp->AppendChild(bp->CreatePropertyArtId());
				bp->AppendChild(bp->CreatePropertyArtClient());
			}

			if (childVals.GetCount() == 3)
				newVal = childVals.Item(0) + wxT("; ") + childVals.Item(1) + wxT("; ") + childVals.Item(2);
			else if (childVals.GetCount() > 1)
				newVal = childVals.Item(0) + wxT("; ; ");
			break;
		}
		}
		break;
	}

	// file_path || id || resource_name
	case 1:
	{
		if ((Item(0)->GetValueAsString() == _("Load From File")) || (Item(0)->GetValueAsString() == _("Load From Embedded File")))
		{
			// Save the initial file path TODO: Save the image filter index
			if (Item(1))
			{
				wxString img = childValue.GetString();
				img = bp->SetupImage(img);
				wxFileName imgPath(img);
				gs_imageInitialPath = imgPath.GetPath();

				if (!img.IsEmpty())
				{
					Item(1)->SetValue(WXVARIANT(img));
				}
				else
				{
					Item(1)->SetValueToUnspecified();
				}
				newVal = Item(0)->GetValueAsString() + wxT("; ") + img;
			}
		}
		break;
	}
	}

	bp->SetPrevSource(childValue.GetInteger());

	if (newVal != val)
	{
		wxVariant ret = WXVARIANT(newVal);
		bp->SetValue(ret);

		return ret;
	}
	return thisValue;
}

void wxPGBitmapProperty::UpdateChildValues(const wxString& value)
{
	wxArrayString childVals;
	GetChildValues(value, childVals);

	if (childVals[0].Contains(_("Load From File")) || childVals[0].Contains(_("Load From Embedded File")))
	{
		if (childVals.Count() > 1)
		{
			wxString img = childVals[1];
			img = SetupImage(img);
			wxFileName imgPath(img);
			gs_imageInitialPath = imgPath.GetPath();

			if (!img.IsEmpty())
			{
				Item(1)->SetValue(WXVARIANT(img));
			}
			else
			{
				Item(1)->SetValueToUnspecified();
			}
		}
	}
	else if (childVals[0].Contains(_("Load From Resource")))
	{
		if (childVals.Count() > 1)
		{
			Item(1)->SetValue(childVals[1]);
		}
	}
	else if (childVals[0].Contains(_("Load From Icon Resource")))
	{
		if (childVals.Count() > 1)
		{
			Item(1)->SetValue(childVals[1]);
		}

		if (childVals.Count() > 2)
		{
			// This child requires a wxSize as data type, not a wxString
			// The string format of a wxSize doesn't match the display format,
			// convert it like CObjectInspector does
			wxString aux = childVals[2];
			aux.Replace(wxT(";"), wxT(","));
			Item(2)->SetValue(WXVARIANT(TypeConv::StringToSize(aux)));
		}
	}
	else if (childVals[0].Contains(_("Load From Art Provider")))
	{
		if (childVals.Count() > 1)
		{
			Item(1)->SetValue(childVals[1]);
		}

		if (childVals.Count() > 2)
		{
			Item(2)->SetValue(childVals[2]);
		}
	}
}

void wxPGBitmapProperty::OnSetValue()
{
}

wxString wxPGBitmapProperty::SetupImage(const wxString &imgPath)
{
	if (!imgPath.IsEmpty())
	{
		wxFileName imgName = wxFileName(imgPath);

		// Allow user to specify any file path he needs (even if it seemingly doesn't exist)
		if (!imgName.FileExists()) return imgPath;

		wxString   res = wxT("");
		wxImage    img = wxImage(imgPath);

		if (!img.IsOk()) return res;

		// Setup for correct file_path
		if (imgName.IsAbsolute())
		{
			return TypeConv::MakeRelativeURL(imgPath, appData->GetProjectPath());
		}
		else
		{
			imgName.MakeAbsolute(appData->GetProjectPath());

			if (!imgName.FileExists()) return res;
		}
	}
	return imgPath;
}

wxString wxPGBitmapProperty::SetupResource(const wxString &resName)
{
	wxString res = wxEmptyString;
	// Keep old value from an icon resource only
	if (resName.Contains(wxT(";")) && resName.Contains(wxT("[")))
	{
		return resName.BeforeFirst(wxT(';'));
	}
	else if (resName.Contains(wxT(";")))
	{
		return res;
	}
	return resName;
}

// -----------------------------------------------------------------------
// wxSlider-based property editor
// -----------------------------------------------------------------------
#ifdef wxUSE_SLIDER

wxIMPLEMENT_DYNAMIC_CLASS(wxPGSliderEditor, wxPGEditor)

wxPGSliderEditor::~wxPGSliderEditor()
{
}

// Create controls and initialize event handling.
wxPGWindowList wxPGSliderEditor::CreateControls(wxPropertyGrid* propgrid,
	wxPGProperty*   property,
	const wxPoint&  pos,
	const wxSize&   sz) const
{
	wxCHECK_MSG(property->IsKindOf(wxCLASSINFO(wxFloatProperty)),
		NULL,
		wxT("Slider editor can only be used with wxFloatProperty or derivative."));

	// Use two stage creation to allow cleaner display on wxMSW
	wxSlider* ctrl = new wxSlider();
#ifdef __WXMSW__
	ctrl->Hide();
#endif
	wxString s = property->GetValueAsString();
	double v_d = 0;
	if (s.ToDouble(&v_d))
	{
		if (v_d < 0)
			v_d = 0;
		else if (v_d > 1)
			v_d = 1;
	}

	ctrl->Create(propgrid->GetPanel(),
		wxID_ANY,
		(int)(v_d * m_max),
		0,
		m_max,
		pos,
		sz,
		wxSL_HORIZONTAL);

	// Connect all required events to grid's OnCustomEditorEvent
	// (all relevenat wxTextCtrl, wxComboBox and wxButton events are
	// already connected)

#ifdef __WXMSW__
	ctrl->Show();
#endif

	return ctrl;
}

// Copies value from property to control
void wxPGSliderEditor::UpdateControl(wxPGProperty* property, wxWindow* wnd) const
{
	wxSlider *ctrl = (wxSlider*)wnd;
	assert(ctrl && ctrl->IsKindOf(CLASSINFO(wxSlider)));

	double val = property->GetValue().GetDouble();
	if (val < 0)
		val = 0;
	else if (val > 1)
		val = 1;

	ctrl->SetValue((int)(val * m_max));
}

// Control's events are redirected here
bool wxPGSliderEditor::OnEvent(wxPropertyGrid* WXUNUSED(propgrid),
	wxPGProperty* property, wxWindow* wnd, wxEvent& event) const
{
	if (event.GetEventType() == wxEVT_SCROLL_THUMBTRACK)
	{
		wxSlider *ctrl = wxDynamicCast(wnd, wxSlider);
		if (ctrl)
		{
			double val = (double)(ctrl->GetValue()) / (double)(m_max);
			property->SetValue(WXVARIANT(val));
			return true;
		}
	}

	return false;
}

bool wxPGSliderEditor::GetValueFromControl(wxVariant& variant, wxPGProperty* WXUNUSED(property), wxWindow* wnd) const
{
	wxSlider* ctrl = (wxSlider*)wnd;
	assert(ctrl && ctrl->IsKindOf(CLASSINFO(wxSlider)));

	variant = WXVARIANT((double)ctrl->GetValue() / (double)(m_max));

	return true;
}

void wxPGSliderEditor::SetValueToUnspecified(wxPGProperty* WXUNUSED(property), wxWindow* WXUNUSED(ctrl)) const
{
}

#endif //wxUSE_SLIDER

// -----------------------------------------------------------------------
// wxPGFontProperty
// -----------------------------------------------------------------------

#include <wx/fontdlg.h>
#include <wx/fontenum.h>

static const wxChar* gs_fp_es_family_labels[] = {
	wxT("Default"), wxT("Decorative"),
	wxT("Roman"), wxT("Script"),
	wxT("Swiss"), wxT("Modern"),
	wxT("Teletype"), wxT("Unknown"),
	(const wxChar*)NULL
};

static long gs_fp_es_family_values[] = {
	wxFONTFAMILY_DEFAULT, wxFONTFAMILY_DECORATIVE,
	wxFONTFAMILY_ROMAN, wxFONTFAMILY_SCRIPT,
	wxFONTFAMILY_SWISS, wxFONTFAMILY_MODERN,
	wxFONTFAMILY_TELETYPE, wxFONTFAMILY_UNKNOWN
};

static const wxChar* gs_fp_es_style_labels[] = {
	wxT("Normal"),
	wxT("Slant"),
	wxT("Italic"),
	(const wxChar*)NULL
};

static long gs_fp_es_style_values[] = {
	wxFONTSTYLE_NORMAL,
	wxFONTSTYLE_SLANT,
	wxFONTSTYLE_ITALIC
};

static const wxChar* gs_fp_es_weight_labels[] = {
	wxT("Normal"),
	wxT("Light"),
	wxT("Bold"),
	(const wxChar*)NULL
};

static long gs_fp_es_weight_values[] = {
	wxFONTWEIGHT_NORMAL,
	wxFONTWEIGHT_LIGHT,
	wxFONTWEIGHT_BOLD
};

#if wxCHECK_VERSION(3, 1, 0)
wxPG_IMPLEMENT_PROPERTY_CLASS(wxPGFontProperty, wxPGProperty, TextCtrlAndButton)
#else
WX_PG_IMPLEMENT_PROPERTY_CLASS(wxPGFontProperty, wxPGProperty,
	wxFont, const wxFont&, TextCtrlAndButton)
#endif


	wxPGFontProperty::wxPGFontProperty(const wxString& label, const wxString& name,
		const wxFontContainer& value)
	: wxPGProperty(label, name)
{
	SetValue(WXVARIANT(TypeConv::FontToString(value)));

	// Initialize font family choices list
	if (!wxPGGlobalVars->m_fontFamilyChoices)
	{
		wxFontEnumerator enumerator;
		enumerator.EnumerateFacenames();

		wxArrayString faceNames = enumerator.GetFacenames();

		faceNames.Sort();
		faceNames.Insert(wxEmptyString, 0);

		wxPGGlobalVars->m_fontFamilyChoices = new wxPGChoices(faceNames);
	}

	//wxString emptyString(wxEmptyString);

	AddPrivateChild(new wxIntProperty(_("Point Size"), wxT("Point Size"),
		value.m_pointSize));

	AddPrivateChild(new wxEnumProperty(_("Family"), wxT("Family"),
		gs_fp_es_family_labels, gs_fp_es_family_values,
		value.m_family));

	wxString faceName = value.m_faceName;
	// If font was not in there, add it now
	if (faceName.length() &&
		wxPGGlobalVars->m_fontFamilyChoices->Index(faceName) == wxNOT_FOUND)
		wxPGGlobalVars->m_fontFamilyChoices->AddAsSorted(faceName);

	wxPGProperty* p = new wxEnumProperty(_("Face Name"), wxT("Face Name"),
		*wxPGGlobalVars->m_fontFamilyChoices);

	p->SetValueFromString(faceName, wxPG_FULL_VALUE);

	AddPrivateChild(p);

	AddPrivateChild(new wxEnumProperty(_("Style"), wxT("Style"),
		gs_fp_es_style_labels, gs_fp_es_style_values, value.m_style));

	AddPrivateChild(new wxEnumProperty(_("Weight"), wxT("Weight"),
		gs_fp_es_weight_labels, gs_fp_es_weight_values, value.m_weight));

	AddPrivateChild(new wxBoolProperty(_("Underlined"), wxT("Underlined"),
		value.m_underlined));
}

wxPGFontProperty::~wxPGFontProperty() { }

void wxPGFontProperty::OnSetValue()
{
	// do nothing
}

wxString wxPGFontProperty::GetValueAsString(int argFlags) const
{
	return wxPGProperty::GetValueAsString(argFlags);
}

bool wxPGFontProperty::OnEvent(wxPropertyGrid* propgrid, wxWindow* WXUNUSED(primary),
	wxEvent& event)
{
	if (propgrid->IsMainButtonEvent(event))
	{
		// Update value from last minute changes

		wxFontData data;
		wxFont font = TypeConv::StringToFont(m_value.GetString());

		data.SetInitialFont(font);
		data.SetColour(*wxBLACK);

		wxFontDialog dlg(propgrid, data);
		if (dlg.ShowModal() == wxID_OK)
		{
			propgrid->EditorsValueWasModified();

			wxFontContainer fcont(dlg.GetFontData().GetChosenFont());

			wxVariant variant = WXVARIANT(TypeConv::FontToString(fcont));
			SetValueInEvent(variant);

			return true;
		}
	}
	return false;
}

void wxPGFontProperty::RefreshChildren()
{
	wxString fstr = m_value.GetString();
	wxFontContainer font = TypeConv::StringToFont(fstr);

	Item(0)->SetValue(font.m_pointSize);
	Item(1)->SetValue(font.m_family);
	Item(2)->SetValueFromString(font.m_faceName, wxPG_FULL_VALUE);
	Item(3)->SetValue(font.m_style);
	Item(4)->SetValue(font.m_weight);
	Item(5)->SetValue(font.m_underlined);
}

wxVariant
wxPGFontProperty::ChildChanged(wxVariant& thisValue, int ind, wxVariant& childValue) const
{
	wxFontContainer font = TypeConv::StringToFont(thisValue.GetString());

	if (ind == 0)
	{
		font.m_pointSize = childValue.GetLong();
	}
	else if (ind == 1)
	{
		int fam = childValue.GetLong();
		if (fam < wxFONTFAMILY_DEFAULT || fam > wxFONTFAMILY_TELETYPE) {
			fam = wxFONTFAMILY_DEFAULT;
		}
		font.m_family = static_cast<wxFontFamily>(fam);
	}
	else if (ind == 2)
	{
		wxString faceName;
		int faceIndex = childValue.GetLong();

		if (faceIndex >= 0)
			faceName = wxPGGlobalVars->m_fontFamilyChoices->GetLabel(faceIndex);

		font.m_faceName = faceName;
	}
	else if (ind == 3)
	{
		int st = childValue.GetLong();
		if (st != wxFONTSTYLE_NORMAL && st != wxFONTSTYLE_SLANT && st != wxFONTSTYLE_ITALIC) {
			st = wxFONTSTYLE_NORMAL;
		}
		font.m_style = static_cast<wxFontStyle>(st);
	}
	else if (ind == 4)
	{
		int wt = childValue.GetLong();
		if (wt != wxFONTWEIGHT_NORMAL &&
			wt != wxFONTWEIGHT_LIGHT &&
			wt != wxFONTWEIGHT_BOLD)
			wt = wxFONTWEIGHT_NORMAL;
		font.m_weight = static_cast<wxFontWeight>(wt);
	}
	else if (ind == 5)
	{
		font.m_underlined = childValue.GetBool();
	}

	thisValue = WXVARIANT(TypeConv::FontToString(font));

	return thisValue;
}

// -----------------------------------------------------------------------
// wxPGSourceProperty
// -----------------------------------------------------------------------

#include <wx/treectrl.h>
#include "metadata/metadata.h"
#include "forms/elements/frame.h"

wxPG_IMPLEMENT_PROPERTY_CLASS(wxPGSourceProperty, wxPGProperty, TextCtrlAndButton)

wxPGSourceProperty::wxPGSourceProperty(const wxString& label, const wxString& name,
	const long& value) : wxPGProperty(label, name)
{
	m_flags |= wxPG_PROP_ACTIVE_BTN; // Property button always enabled.
	m_flags |= wxPG_PROP_READONLY;

	SetValue(value);
}

wxPGSourceProperty::~wxPGSourceProperty() {}

wxString wxPGSourceProperty::ValueToString(wxVariant& value,
	int WXUNUSED(argFlags)) const
{
	int m_metaID = value.GetInteger();
	IMetaObject * metaObj = metadata->GetMetaObject(m_metaID);

	CValueFrame *control = m_control->GetOwnerForm();
	/*if (control && m_metaID < 0)
	{
		IMetaObjectValue *metaObject = control->GetMetaObject();
		if (metaObject)
		{
			for (auto def_metaobject : metaObject->GetDefaultObjectAttributes())
			{
				if (m_metaID == def_metaobject->GetMetaID()) 
					return def_metaobject->GetName();
			}
		}
	}*/

	if (!metaObj) {
		return wxT("<not selected>");
	}

	return metaObj->GetName();
}

bool wxPGSourceProperty::OnEvent(wxPropertyGrid* propGrid, wxWindow* WXUNUSED(primary),
	wxEvent& event)
{
	if (propGrid->IsMainButtonEvent(event))
	{
		// Update the value
		wxVariant useValue = propGrid->GetUncommittedPropertyValue();

		int val1 = useValue.GetInteger();
		int val_orig = val1;

		int value = val1;

		// Run editor dialog.
		if (OnButtonClick(propGrid, value))
		{
			val1 = value;

			if (val1 != val_orig)
			{
				SetValueInEvent(val1);
				return true;
			}
		}
	}
	return false;
}

bool wxPGSourceProperty::OnButtonClick(wxPropertyGrid* propGrid, int& value)
{
	return DisplayEditorDialog(this, propGrid, value);
}

void wxPGSourceProperty::SetValueObject(IControlElement *control)
{
	m_control = control;
}

bool wxPGSourceProperty::DisplayEditorDialog(wxPGSourceProperty* prop, wxPropertyGrid* propGrid, int& value)
{
	// launch editor dialog
	wxDialog* dlg = new wxDialog(propGrid, wxID_ANY, prop->GetLabel(), wxDefaultPosition, wxDefaultSize,
		wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxCLIP_CHILDREN);

	dlg->SetFont(propGrid->GetFont()); // To allow entering chars of the same set as the propGrid

	// Multi-line text editor dialog.
	const int spacing = wxPropertyGrid::IsSmallScreen() ? 4 : 8;
	wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* rowsizer = new wxBoxSizer(wxHORIZONTAL);

	wxTreeCtrl* tc = new wxTreeCtrl(dlg, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTR_HAS_BUTTONS | wxTR_LINES_AT_ROOT | wxTR_NO_LINES | wxTR_SINGLE | wxSUNKEN_BORDER);

	rowsizer->Add(tc, wxSizerFlags(1).Expand().Border(wxALL, spacing));
	topsizer->Add(rowsizer, wxSizerFlags(1).Expand());

	std::map<void *, CAttributeObject *> aAttributes;

	CValueFrame *control = prop->m_control->GetOwnerForm();

	if (control)
	{
		IMetaObjectValue *metaObject = control->GetMetaObject();

		if (metaObject)
		{
			wxTreeItemId root = tc->AddRoot("object");

			for (auto attribute : metaObject->GetObjectAttributes()) {
				wxTreeItemId m_item_id = tc->AppendItem(root, attribute->GetName());
				aAttributes[m_item_id.GetID()] = attribute;
			}

			tc->ExpandAll();
		}
	}

	wxStdDialogButtonSizer* buttonSizer = dlg->CreateStdDialogButtonSizer(wxOK | wxCANCEL);
	topsizer->Add(buttonSizer, wxSizerFlags(0).Right().Border(wxBOTTOM | wxRIGHT, spacing));

	dlg->SetSizer(topsizer);
	topsizer->SetSizeHints(dlg);

	if (!wxPropertyGrid::IsSmallScreen())
	{
		dlg->SetSize(400, 300);
		dlg->Move(propGrid->GetGoodEditorDialogPosition(prop, dlg->GetSize()));
	}

	int res = dlg->ShowModal();

	if (res == wxID_OK)
	{
		wxTreeItemId m_item_id = tc->GetSelection();
		CAttributeObject *m_attribute = aAttributes[m_item_id.GetID()];
		
		if (m_attribute) {
			Property *property = prop->m_control->GetProperty("type");
			property->SetValue(m_attribute->GetPropertyAsInteger("type"));

			value = m_attribute->GetMetaID();
		}

		dlg->Destroy();
		return true;
	}
	dlg->Destroy();
	return false;
}

bool wxPGSourceProperty::StringToValue(wxVariant& variant, const wxString& text, int argFlags) const
{
	if (text.empty())
	{
		variant.MakeNull();
		return true;
	}

	// We know it is a number, but let's still check
	// the return value.
	if (text.IsNumber())
	{
		// Remove leading zeros, so that the number is not interpreted as octal
		wxString::const_iterator i = text.begin();
		wxString::const_iterator iMax = text.end() - 1;  // Let's allow one, last zero though

		int firstNonZeroPos = 0;

		for (; i != iMax; ++i)
		{
			wxUniChar c = *i;
			if (c != wxS('0') && c != wxS(' '))
				break;
			firstNonZeroPos++;
		}

		wxString useText = text.substr(firstNonZeroPos, text.length() - firstNonZeroPos);

		const wxString variantType(variant.GetType());
		bool isPrevLong = variantType == wxPG_VARIANT_TYPE_LONG;

#if defined(wxLongLong_t) && wxUSE_LONGLONG
		wxLongLong_t value64 = 0;

		if (useText.ToLongLong(&value64, 10) &&
			(value64 >= INT_MAX || value64 <= INT_MIN)
			)
		{
			bool doChangeValue = isPrevLong;

			if (!isPrevLong && variantType == wxPG_VARIANT_TYPE_LONGLONG)
			{
				wxLongLong oldValue = variant.GetLongLong();
				if (oldValue.GetValue() != value64)
					doChangeValue = true;
			}

			if (doChangeValue)
			{
				wxLongLong ll(value64);
				variant = ll;
				return true;
			}
		}
#endif
		long value32;
		if (useText.ToLong(&value32, 0))
		{
			if (!isPrevLong || variant != value32)
			{
				variant = value32;
				return true;
			}
		}
	}
	else if (argFlags & wxPG_REPORT_ERROR)
	{
	}
	return false;
}

bool wxPGSourceProperty::IntToValue(wxVariant& variant,
	int value,
	int argFlags) const
{
	if (!variant.IsType(wxPG_VARIANT_TYPE_LONG) || variant != (long)value)
	{
		variant = (long)value;
		return true;
	}
	return false;
}

// -----------------------------------------------------------------------
// wxStringControlProperty
// -----------------------------------------------------------------------

wxPG_IMPLEMENT_PROPERTY_CLASS(wxStringControlProperty, wxStringProperty, TextCtrl)

wxString wxStringControlProperty::ValueToString(wxVariant& value, int argFlags) const
{
	wxString s = value.GetString();

	if (GetChildCount() && HasFlag(wxPG_PROP_COMPOSED_VALUE))
	{
		// Value stored in m_value is non-editable, non-full value
		if ((argFlags & wxPG_FULL_VALUE) ||
			(argFlags & wxPG_EDITABLE_VALUE) ||
			s.empty())
		{
			// Calling this under incorrect conditions will fail
			wxASSERT_MSG(argFlags & wxPG_VALUE_IS_CURRENT,
				wxS("Sorry, currently default wxPGProperty::ValueToString() ")
				wxS("implementation only works if value is m_value."));

			DoGenerateComposedValue(s, argFlags);
		}

		return s;
	}

	// If string is password and value is for visual purposes,
	// then return asterisks instead the actual string.
	if ((m_flags & wxPG_PROP_PASSWORD) && !(argFlags & (wxPG_FULL_VALUE | wxPG_EDITABLE_VALUE)))
		return wxString(wxS('*'), s.Length());

	return s;
}

#include "utils/stringutils.h"

bool wxStringControlProperty::StringToValue(wxVariant& variant,
	const wxString& text,
	int argFlags) const
{
	for (unsigned int i = 0; i < text.length(); i++)
	{
		if (!((text[i] == '_') ||
			(text[i] >= 'A' && text[i] <= 'Z') || (text[i] >= 'a' && text[i] <= 'z') ||
			(text[i] >= 'À' && text[i] <= 'ß') || (text[i] >= 'à' && text[i] <= 'ÿ') ||
			(text[i] >= '0' && text[i] <= '9')))
		{
			wxMessageBox(wxT("You can enter only numbers, letters and the symbol \"_\""), wxT("Error entering value")); return false;
		}
	}

	if (GetChildCount() && HasFlag(wxPG_PROP_COMPOSED_VALUE))
		return wxPGProperty::StringToValue(variant, text, argFlags);

	if (variant != text)
	{
		variant = text;
		return text.length() > 0;
	}

	return false;
}