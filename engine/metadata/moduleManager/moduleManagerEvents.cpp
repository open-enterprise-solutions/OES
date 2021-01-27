////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : main events
////////////////////////////////////////////////////////////////////////////

#include "moduleManager.h"
#include "appData.h"

//*********************************************************************************************************
//*                                   Events "moduleManager"                                              *
//*********************************************************************************************************

bool CModuleManager::BeforeStart()
{
	if (appData->IsEnterpriseMode())
	{
		try
		{
			CValue bCancel = false;
			if (m_procUnit) m_procUnit->CallFunction("BeforeStart", bCancel);
			return !bCancel.GetBoolean();
		}
		catch (...)
		{
			return false;
		};
	}; 

	return true;
}

void CModuleManager::OnStart()
{
	if (appData->IsEnterpriseMode())
	{
		try
		{
			if (m_procUnit) m_procUnit->CallFunction("OnStart");
		}
		catch (...)
		{
		};
	}; 
}

bool CModuleManager::BeforeExit()
{
	if (appData->IsEnterpriseMode())
	{
		try
		{
			CValue bCancel = false;
			if (m_procUnit) m_procUnit->CallFunction("BeforeExit", bCancel);
			return !bCancel.GetBoolean();
		}
		catch (...)
		{
			return false;
		};
	};

	return true;
}

void CModuleManager::OnExit()
{
	if (appData->IsEnterpriseMode())
	{
		try
		{
			if (m_procUnit) m_procUnit->CallFunction("OnExit");
		}
		catch (...)
		{
		};
	}; 
}