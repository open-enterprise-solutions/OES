#ifndef _CODEPROC_H__
#define _CODEPROC_H__

#include "window/autocomplete/autoComplectionCtrl.h"

class ICodeInfo
{
public:

	virtual void SetCurrentLine(int lineBreakpoint, bool setBreakpoint) = 0;
};

#endif 