////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : base control
////////////////////////////////////////////////////////////////////////////

#include "baseControl.h"
#include "frame.h"

void ParseControl(IControlElement *control, IControlElement *top, unsigned int &obj_id)
{
	if (control == top) return; 

	if (obj_id < control->GetControlID() && control->GetComponentType() != COMPONENT_TYPE_FRAME)
		obj_id = control->GetControlID();

	for (unsigned int i = 0; i < control->GetChildCount(); i++) 
		ParseControl(control->GetChild(i), top, obj_id);
}

void IControlElement::GenerateNewID()
{
	unsigned int obj_id = FORM_ID_START; // 1 is valueFrame  
	ParseControl(GetOwnerForm(), this, obj_id);
	m_obj_id = obj_id + 1;
}

IControlElement *GetControl(IControlElement *control, unsigned int obj_id)
{
	if (obj_id == control->GetControlID() && control->GetComponentType() != COMPONENT_TYPE_FRAME) return control;
	for (unsigned int i = 0; i < control->GetChildCount(); i++)
	{
		IControlElement *child = GetControl(control->GetChild(i), obj_id);
		if (child) return child;
	}
	return NULL; 
}

IControlElement *IControlElement::FindControlByID(unsigned int obj_id)
{
	if (obj_id == FORM_ID_START) return GetOwnerForm();
	else return GetControl(GetOwnerForm(), obj_id);
}