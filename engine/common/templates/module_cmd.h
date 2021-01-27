#ifndef _MODULE_CMD_H__
#define _MODULE_CMD_H__

#include "common/cmdProc.h"
#include "window/autocomplete/autocomplectionctrl.h"

class CModuleCommandProcessor : public CCommandProcessor
{
	CAutocomplectionCtrl *m_code;

public:

	CModuleCommandProcessor(CAutocomplectionCtrl *code) : CCommandProcessor(), m_code(code) {}

	virtual bool Undo() override
	{
		m_code->Undo();
		return true;
	}

	virtual bool Redo() override
	{
		m_code->Redo();
		return true;
	}

	virtual bool CanUndo() const override
	{
		return m_code->CanUndo();
	}

	virtual bool CanRedo() const override
	{
		return m_code->CanRedo();
	}
};

#endif