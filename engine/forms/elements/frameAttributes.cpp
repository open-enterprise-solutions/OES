////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : frame attributes
////////////////////////////////////////////////////////////////////////////

#include "frame.h"
#include "compiler/methods.h"

//*******************************************************************
//*                         Attributes                              *
//*******************************************************************

enum
{
	eThisForm = 1,
	eControls,
};

int CValueFrame::FindAttribute(const wxString &sName) const
{
	return m_methods->FindAttribute(sName);
}

void CValueFrame::SetAttribute(CAttributeParameters &aParams, CValue &cVal)
{
	wxString sAlias = m_methods->GetAttributeAlias(aParams.GetAttributeIndex());

	if (sAlias == wxT("procUnit")) { if (m_procUnit) m_procUnit->SetAttribute(aParams.GetAttributeName(), cVal); }
	else if (sAlias == wxT("attribute")) { IControlElement::SetAttribute(CAttributeParameters(m_methods->GetAttributePosition(aParams.GetAttributeIndex()), aParams.GetAttributeName()), cVal); }
}

CValue CValueFrame::GetAttribute(CAttributeParameters &aParams)
{
	wxString sAlias = m_methods->GetAttributeAlias(aParams.GetAttributeIndex());

	if (sAlias == wxT("procUnit")) { if (m_procUnit) return m_procUnit->GetAttribute(aParams.GetAttributeName()); }
	else if (sAlias == wxT("attribute")) { return IControlElement::GetAttribute(CAttributeParameters(m_methods->GetAttributePosition(aParams.GetAttributeIndex()), aParams.GetAttributeName())); }
	else if (sAlias == wxT("system"))
	{
		switch (m_methods->GetAttributePosition(aParams.GetAttributeIndex()))
		{
		case eThisForm: return this;
		case eControls: return m_frameControls; break;
		}
	}

	return CValue();
}