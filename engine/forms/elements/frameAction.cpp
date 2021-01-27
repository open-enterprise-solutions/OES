////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : frame action
////////////////////////////////////////////////////////////////////////////

#include "frame.h"
#include "appData.h"
#include "metadata/objects/baseObject.h"

enum
{
	enClose = 10000,
	enHelp,
};

//****************************************************************************
//*                              actions                                     *
//****************************************************************************

CAction CValueFrame::GetActions(form_identifier_t formType)
{
	CAction action;
	action.AddAction("close", enClose);
	action.AddAction("help", enHelp);

	if (m_metaObject) {
		m_metaObject->AddActions(action, formType);
	}

	return action;
}

void CValueFrame::AddActions(CAction &actions, form_identifier_t formType)
{
	CAction aFormActions = GetActions(formType);

	for (unsigned int i = 0; i < aFormActions.GetCount(); i++)
	{
		unsigned int action_id = aFormActions.GetID(i);
		actions.AddAction(aFormActions.GetNameByID(action_id), action_id);
	}
}

void CValueFrame::ExecuteAction(unsigned int action)
{
	if (!appData->IsEnterpriseMode()) return;
	
	switch (action)
	{
	case enClose: CloseForm(); break;
	case enHelp: HelpForm(); break;
	default:
	{
		if (m_sourceObject) m_sourceObject->ExecuteAction(action); break;
	}
	}
}