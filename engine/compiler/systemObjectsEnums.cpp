////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : system objects 
////////////////////////////////////////////////////////////////////////////

#include "systemObjectsEnums.h"
#include "enumFactory.h"

wxIMPLEMENT_DYNAMIC_CLASS(CValueStatusMessage, CValue);
wxIMPLEMENT_DYNAMIC_CLASS(CValueQuestionMode, CValue);
wxIMPLEMENT_DYNAMIC_CLASS(CValueQuestionReturnCode, CValue);
wxIMPLEMENT_DYNAMIC_CLASS(CValueRoundMode, CValue);

//add new enumeration
ENUM_REGISTER(CValueStatusMessage, "statusMessage", TEXT2CLSID("EN_STMS"));
ENUM_REGISTER(CValueQuestionMode, "questionMode", TEXT2CLSID("EN_QSMD"));
ENUM_REGISTER(CValueQuestionReturnCode, "questionReturnCode", TEXT2CLSID("EN_QSRC"));
ENUM_REGISTER(CValueRoundMode, "roundMode", TEXT2CLSID("EN_ROMO"));