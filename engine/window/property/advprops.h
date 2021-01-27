#ifndef _ADVPROPS_H__
#define _ADVPROPS_H__

#include "utils/fontcontainer.h"

#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

// -----------------------------------------------------------------------
// wxPGSizeProperty
// -----------------------------------------------------------------------

class wxPGSizeProperty : public wxPGProperty
{
    WX_PG_DECLARE_PROPERTY_CLASS( wxPGSizeProperty )
public:
    wxPGSizeProperty( const wxString& label = wxPG_LABEL,
                      const wxString& name  = wxPG_LABEL,
                      const wxSize& value = wxSize() );

	wxVariant ChildChanged(wxVariant& thisValue, int childIndex,
	                       wxVariant& childValue) const override;

	void RefreshChildren() override;

protected:
    void DoSetValue( const wxSize& value ) { m_value = WXVARIANT( value ); }
};

// -----------------------------------------------------------------------
// wxPGPointProperty
// -----------------------------------------------------------------------

class wxPGPointProperty : public wxPGProperty
{
    WX_PG_DECLARE_PROPERTY_CLASS( wxPGPointProperty )
public:
    wxPGPointProperty( const wxString& label = wxPG_LABEL,
                       const wxString& name  = wxPG_LABEL,
                       const wxPoint&  value = wxPoint() );
	~wxPGPointProperty() override;

	wxVariant ChildChanged(wxVariant& thisValue, int childIndex,
	                       wxVariant& childValue) const override;

	void RefreshChildren() override;

protected:
    void DoSetValue( const wxPoint& value ) { m_value = WXVARIANT( value ); }
};

// -----------------------------------------------------------------------
// wxPGBitmapProperty
// -----------------------------------------------------------------------

class wxPGBitmapProperty : public wxPGProperty
{
    WX_PG_DECLARE_PROPERTY_CLASS( wxPGBitmapProperty )

public:
    wxPGBitmapProperty( const wxString& label = wxPG_LABEL,
                        const wxString& name  = wxPG_LABEL,
                        const wxString& value = wxString() );

    virtual ~wxPGBitmapProperty();

    wxPGProperty *CreatePropertySource( int sourceIndex = 0 );
    wxPGProperty *CreatePropertyFilePath() ;
    wxPGProperty *CreatePropertyResourceName();
    wxPGProperty *CreatePropertyIconSize();
    wxPGProperty *CreatePropertyArtId();
    wxPGProperty *CreatePropertyArtClient();

    wxString SetupImage( const wxString &imgPath = wxEmptyString ) ;
    wxString SetupResource( const wxString &resName = wxEmptyString ) ;

	int prevSrc;
	void SetPrevSource(int src){prevSrc = src;}

	virtual wxVariant ChildChanged(wxVariant& thisValue, int childIndex,
	                               wxVariant& childValue) const override;

	virtual void OnSetValue() override;
	void CreateChildren();

	void UpdateChildValues(const wxString& value);
protected:

	void GetChildValues( const wxString& parentValue, wxArrayString& childValues ) const;

    static wxArrayString m_ids;
    static wxArrayString m_clients;
    wxArrayString m_strings;
};

// -----------------------------------------------------------------------
// wxSlider-based property editor
// -----------------------------------------------------------------------

#if wxUSE_SLIDER
//
// Implement an editor control that allows using wxSlider to edit value of
// wxFloatProperty (and similar).
//
// Note that new editor classes needs to be registered before use.
// This can be accomplished using wxPGRegisterEditorClass macro.
// Registeration can also be performed in a constructor of a
// property that is likely to require the editor in question.
//

class wxPGSliderEditor : public wxPGEditor
{
    wxDECLARE_DYNAMIC_CLASS( wxPGSliderEditor );
public:
    wxPGSliderEditor()
    :
    m_max( 10000 )
    {
    }

    virtual ~wxPGSliderEditor();
    virtual wxPGWindowList CreateControls(wxPropertyGrid* propgrid,
                                          wxPGProperty* property,
                                          const wxPoint& pos,
                                          const wxSize& size) const;
    virtual void UpdateControl( wxPGProperty* property, wxWindow* wnd ) const;
    virtual bool OnEvent( wxPropertyGrid* propgrid, wxPGProperty* property,
                          wxWindow* wnd, wxEvent& event ) const;
    virtual bool GetValueFromControl( wxVariant& variant, wxPGProperty* property, wxWindow* ctrl ) const;
    virtual void SetValueToUnspecified( wxPGProperty* property, wxWindow* ctrl ) const;

private:
    int m_max;
};
#endif // wxUSE_SLIDER

// -----------------------------------------------------------------------
// wxPGFontProperty
// -----------------------------------------------------------------------

class wxPGFontProperty : public wxPGProperty
{
    WX_PG_DECLARE_PROPERTY_CLASS(wxPGFontProperty)
public:

    wxPGFontProperty( const wxString& label = wxPG_LABEL, const wxString& name = wxPG_LABEL, const wxFontContainer& value = *wxNORMAL_FONT);
	~wxPGFontProperty() override;

	wxVariant ChildChanged(wxVariant& thisValue, int childIndex,
	                       wxVariant& childValue) const override;

	void RefreshChildren() override;

	void OnSetValue() override;
	wxString GetValueAsString(int argFlags = 0) const override;

	bool OnEvent(wxPropertyGrid* propgrid, wxWindow* primary, wxEvent& event) override;
};

// -----------------------------------------------------------------------
// wxPGSourceProperty
// -----------------------------------------------------------------------

#include "forms/elements/baseControl.h"

class wxPGSourceProperty : public wxPGProperty
{
	WX_PG_DECLARE_PROPERTY_CLASS(wxPGSourceProperty)

private:

	IControlElement *m_control;

public:

	wxPGSourceProperty(const wxString& label = wxPG_LABEL,
		const wxString& name = wxPG_LABEL,
		const long& value = 0);


	virtual ~wxPGSourceProperty();

	virtual wxString ValueToString(wxVariant& value, int argFlags = 0) const override;
	
	virtual bool StringToValue(wxVariant& variant,
		const wxString& text,
		int argFlags = 0) const override;

	virtual bool IntToValue(wxVariant& variant,
		int number,
		int argFlags = 0) const override;

	virtual bool OnEvent(wxPropertyGrid* propgrid,
		wxWindow* primary, wxEvent& event) override;

	// Shows string editor dialog. Value to be edited should be read from
	// value, and if dialog is not cancelled, it should be stored back and true
	// should be returned if that was the case.
	virtual bool OnButtonClick(wxPropertyGrid* propgrid, int& value);

	void SetValueObject(IControlElement *control);

	static bool DisplayEditorDialog(wxPGSourceProperty* prop,
		wxPropertyGrid* propGrid,
		int& value);
};

// -----------------------------------------------------------------------
// wxStringControlProperty
// -----------------------------------------------------------------------

class wxStringControlProperty : public wxStringProperty
{
	WX_PG_DECLARE_PROPERTY_CLASS(wxStringControlProperty)

public:

	wxStringControlProperty(const wxString& label = wxPG_LABEL,
		const wxString& name = wxPG_LABEL,
		const wxString& value = wxEmptyString) : wxStringProperty(label, name, value) {}

	virtual wxString ValueToString(wxVariant& value, int argFlags = 0) const override;
	virtual bool StringToValue(wxVariant& variant,
		const wxString& text,
		int argFlags = 0) const override;
};

#endif
