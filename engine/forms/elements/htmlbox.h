#ifndef  _HTMLBOX_H__

#include "window.h"
#include <wx/html/htmlwin.h>

class CValueHTMLBox : public CValueWindow
{
	wxDECLARE_DYNAMIC_CLASS(CValueHTMLBox);

private:

	wxString m_name;

public:

	CValueHTMLBox(); 

	//methods 
	virtual void PrepareNames() const;                         //этот метод автоматически вызывается для инициализации имен атрибутов и методов
	virtual CValue Method(CMethodParameters &aParams);       //вызов метода

	virtual wxObject* Create(wxObject* parent, IVisualHost *visualHost) override;
	virtual void OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost) override;
	virtual void OnSelected(wxObject* wxobject) override;
	virtual void Update(wxObject* wxobject, IVisualHost *visualHost) override;
	virtual void Cleanup(wxObject* obj, IVisualHost *visualHost) override;

	virtual wxString GetClassName() const override { return wxT("htmlbox"); }
	virtual wxString GetObjectTypeName() const override { return wxT("container"); }

	//read&save propery 
	virtual void ReadProperty() override;
	virtual void SaveProperty() override;
};

#endif // ! _HTMLBOX_H__
