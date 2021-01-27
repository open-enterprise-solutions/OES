////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko, wxFormBuilder
//	Description : visual editor 
////////////////////////////////////////////////////////////////////////////

#include "visualEditorBase.h"
#include "forms/elements/baseControl.h"

wxObject* IVisualHost::Create(IControlElement *control, wxObject* parent) 
{
	return control->Create(parent, this);
};

void IVisualHost::OnCreated(IControlElement *control, wxObject* obj, wxWindow* wndParent) 
{
	control->OnCreated(obj, wndParent, this); 
};

void IVisualHost::OnSelected(IControlElement *control, wxObject* obj) 
{
	control->OnSelected(obj); 
};

void IVisualHost::Update(IControlElement *control, wxObject* obj) 
{
	control->Update(obj, this);
};

void IVisualHost::OnUpdated(IControlElement *control, wxObject* obj, wxWindow* wndParent) 
{
	control->OnUpdated(obj, wndParent, this); 
};

void IVisualHost::Cleanup(IControlElement *control, wxObject* obj) 
{
	control->Cleanup(obj, this);
};