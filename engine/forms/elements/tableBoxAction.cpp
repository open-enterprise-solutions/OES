#include "tableBox.h"
#include "metadata/objects/baseObject.h"
#include "frame.h"

enum
{
	eAddValue = 1,
	eCopyValue,
	eEditValue,
	eDeleteValue
};

//****************************************************************************
//*                              actions                                     *
//****************************************************************************

CAction CValueTableBox::GetActions(form_identifier_t formType)
{
	CAction action;

	if (m_dataSource)
	{
		IControlElement *control = FindControlByID(m_dataSource);
		wxASSERT(control);
		control->AddActions(action, formType);
	}
	else
	{
		action.AddAction("add", eAddValue);
		action.AddAction("copy", eCopyValue);
		action.AddAction("edit", eEditValue);
		action.AddAction("delete", eDeleteValue);
	}

	return action;
}

void CValueTableBox::ExecuteAction(unsigned int action)
{
	if (m_dataSource)
	{
		IControlElement *control = FindControlByID(m_dataSource);
		wxASSERT(control);
		control->ExecuteAction(action);
	}
	else
	{
		switch (action)
		{
		case eAddValue: AddValue(); break;
		case eCopyValue: CopyValue(); break;
		case eEditValue: EditValue(); break;
		case eDeleteValue: DeleteValue(); break;
		}
	}
}

void CValueTableBox::PrepareOption(Property *property, OptionList &optionlist)
{
	optionlist.AddOption("<not selected>", 0);
	optionlist.AddOption("form", FORM_ACTION);
}