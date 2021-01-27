#include "toolbar.h"
#include "metadata/objects/baseObject.h"
#include "frame.h"

void CValueToolBarItem::Attach(void *pObj)
{
}

void CValueToolBarItem::Detach()
{
}

void CValueToolBarItem::PrepareOption(Property *property, OptionList &optionlist)
{
	optionlist.AddOption("< not selected >", 0);

	CValueToolbar *m_toolbar = dynamic_cast<CValueToolbar *> (GetParent());
	if (!m_toolbar) return;

	IControlElement *m_sourceElement = m_toolbar->m_actionSource != wxNOT_FOUND ? FindControlByID(m_toolbar->m_actionSource) : NULL;

	if (m_sourceElement)
	{
		CAction action = m_sourceElement->GetActions(m_sourceElement->GetTypeForm());
		
		for (unsigned int i = 0; i < action.GetCount(); i++)
		{
			unsigned int action_id = action.GetID(i);
			optionlist.AddOption(action.GetNameByID(action_id), action_id);
		}
	}
}

void CValueToolBarSeparator::Attach(void *pObj)
{
}

void CValueToolBarSeparator::Detach()
{
}
