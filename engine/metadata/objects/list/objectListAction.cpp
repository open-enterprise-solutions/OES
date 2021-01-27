////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : list actions 
////////////////////////////////////////////////////////////////////////////

#include "objectList.h"

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

void CObjectListValue::ExecuteAction(unsigned int action)
{
	switch (action)
	{
	case eSelectValue: SelectValue(); return;

	case eAddValue:    AddValue();    break;
	case eCopyValue:   CopyValue();   UpdateModel();  break;
	case eEditValue:   EditValue();   UpdateModel();  break;
	case eDeleteValue: DeleteValue(); UpdateModel();  break;
	}
}