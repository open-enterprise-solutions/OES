#include "toolbar.h"
#include "frame.h"

void ParseElements(IControlElement *element, OptionList &optionlist)
{
	if (element->GetClassName() == wxT("tablebox")) 
		optionlist.AddOption(element->GetPropertyAsString("name"), element->GetControlID());

	for (unsigned int i = 0; i < element->GetChildCount(); i++) ParseElements(element->GetChild(i), optionlist);
}

void CValueToolbar::PrepareOption(Property *property, OptionList &optionlist)
{
	optionlist.AddOption("<not selected>", 0);
	optionlist.AddOption("form", FORM_ACTION);

	ParseElements(GetOwnerForm(), optionlist);
}