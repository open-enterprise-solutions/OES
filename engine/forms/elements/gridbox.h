#ifndef _GRID_H__
#define _GRID_H__

#include "window.h"
#include "window/grid/gridCommon.h"

class CValueGridBox : public CValueWindow
{
	wxDECLARE_DYNAMIC_CLASS(CValueGridBox);

private:

	wxString m_name;

public:

	CValueGridBox();

	virtual wxObject* Create(wxObject* parent, IVisualHost *visualHost) override;
	virtual void OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost) override;
	virtual void OnSelected(wxObject* wxobject) override;
	virtual void Update(wxObject* wxobject, IVisualHost *visualHost) override;
	virtual void Cleanup(wxObject* obj, IVisualHost *visualHost) override;

	virtual wxString GetClassName() const override { return wxT("gridbox"); }
	virtual wxString GetObjectTypeName() const override { return wxT("container"); }

	//read&save propery 
	virtual void ReadProperty() override;
	virtual void SaveProperty() override;
};

#endif