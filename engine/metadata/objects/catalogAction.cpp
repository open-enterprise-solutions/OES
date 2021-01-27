////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : catalog action
////////////////////////////////////////////////////////////////////////////

#include "catalog.h"

enum
{
	eWrite = 1,
	eFill,
	eCopy,
	eMarkAsDelete,
	eDelete
};

enum
{
	eSelectValue = 6,
	eAddValue,
	eCopyValue,
	eEditValue,
	eDeleteValue
};

CAction CMetaObjectCatalogValue::GetActions(form_identifier_t formType)
{
	CAction aCatalogActions;

	switch (formType)
	{
	    case eFormObject:
		{
			aCatalogActions.AddAction("write", eWrite);
			aCatalogActions.AddAction("fill",  eFill);
			aCatalogActions.AddAction("copy",  eCopy);
			break;
		}
		case eFormList:
		{
			aCatalogActions.AddAction("add",    eAddValue);
			aCatalogActions.AddAction("copy",   eCopyValue);
			aCatalogActions.AddAction("edit",   eEditValue);
			aCatalogActions.AddAction("delete", eDeleteValue);
			break;
		}
		case eFormSelect:
		{
			aCatalogActions.AddAction("select", eSelectValue);
			aCatalogActions.AddAction("add",    eAddValue);
			aCatalogActions.AddAction("copy",   eCopyValue);
			aCatalogActions.AddAction("edit",   eEditValue);
			aCatalogActions.AddAction("delete", eDeleteValue);
			break;
		}
	}

	return aCatalogActions;
}

void CMetaObjectCatalogValue::AddActions(CAction &actions, form_identifier_t formType)
{
	CAction aCatalogActions = GetActions(formType);

	for (unsigned int i = 0; i < aCatalogActions.GetCount(); i++)
	{
		unsigned int action_id = aCatalogActions.GetID(i);
		actions.AddAction(aCatalogActions.GetNameByID(action_id), action_id);
	}
}

void CObjectCatalogValue::ExecuteAction(unsigned int action)
{
	switch (action)
	{
	case eWrite: WriteObject(); break;
	case eFill: FillObject(this); break;
	case eCopy: CopyObject().ShowValue(); break;
	case eDelete: DeleteObject(); break;
	}
}
