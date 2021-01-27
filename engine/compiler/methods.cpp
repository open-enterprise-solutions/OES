////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko, 2Ñ-team
//	Description : Processor unit 
////////////////////////////////////////////////////////////////////////////

#include "methods.h"

void CMethods::PrepareMethods(SEng *m_pMethods, unsigned int nCount)
{
	m_aTreeMethods.clear();

	if (m_pMethods)
	{
		m_aMethods.resize(nCount);

		for (unsigned int i = 0; i < nCount; i++)
		{
			m_aMethods[i].sName = m_pMethods[i].sName;
			m_aMethods[i].sShortDescription = m_pMethods[i].sShortDescription;
			m_aMethods[i].sAlias = m_pMethods[i].sAlias;
			m_aMethods[i].iName = m_pMethods[i].iName;

			wxString sName = m_pMethods[i].sName;
			sName.MakeUpper();
			m_aTreeMethods[sName] = (size_t)(i + 1);
		}
	}
	else
	{
		m_aMethods.clear();
	}
}

void CMethods::PrepareAttributes(SEng *m_pAttributes, unsigned int nCount)
{
	m_aTreeAttributes.clear();

	if (m_pAttributes)
	{
		m_aAttributes.resize(nCount);

		for (unsigned int i = 0; i < nCount; i++)
		{
			m_aAttributes[i].sName = m_pAttributes[i].sName;
			m_aAttributes[i].sAlias = m_pAttributes[i].sAlias;
			m_aAttributes[i].iName = m_pAttributes[i].iName;

			wxString sName = m_pAttributes[i].sName;
			sName.MakeUpper();
			m_aTreeAttributes[sName] = (size_t)(i + 1);
		}
	}
	else
	{
		m_aAttributes.clear();
	}
}

int CMethods::FindMethod(const wxString &sName) const
{
	auto itFounded = m_aTreeMethods.find(sName.Upper());
	if (itFounded != m_aTreeMethods.end()) return (int)itFounded->second - 1;
	return wxNOT_FOUND;
}


int CMethods::FindAttribute(const wxString &sName) const
{
	auto itFounded = m_aTreeAttributes.find(sName.Upper());
	if (itFounded != m_aTreeAttributes.end()) return (int)itFounded->second - 1;
	return wxNOT_FOUND;
}

int CMethods::AppendMethod(const wxString &methodName, const wxString &shortDescription)
{
	return AppendMethod(methodName, shortDescription, wxT("method"), wxNOT_FOUND);
}

int CMethods::AppendMethod(const wxString &methodName, const wxString &shortDescription, const wxString &alias)
{
	return AppendMethod(methodName, shortDescription, alias, wxNOT_FOUND);
}

int CMethods::AppendMethod(const wxString &methodName, const wxString &shortDescription, const wxString &alias, int reaName)
{
	auto itFounded = m_aTreeMethods.find(methodName.Upper());

	if (itFounded == m_aTreeMethods.end())
	{
		m_aMethods.emplace_back(methodName, shortDescription, alias, reaName);
		m_aTreeMethods[methodName.Upper()] = (size_t)(m_aMethods.size() + 1);

		return m_aMethods.size() + 1;
	}

	if (itFounded != m_aTreeAttributes.end()) return itFounded->second - 1;

	return wxNOT_FOUND;
}

void CMethods::RemoveMethod(const wxString & methodName)
{
	auto itFounded = m_aTreeMethods.find(methodName.Upper());
	if (itFounded != m_aTreeMethods.end()) { m_aTreeMethods.erase(itFounded); }
}

int CMethods::AppendAttribute(const wxString &attributeName)
{
	return AppendAttribute(attributeName, wxT("attribute"), wxNOT_FOUND);
}

int CMethods::AppendAttribute(const wxString &attributeName, const wxString &alias)
{
	return AppendAttribute(attributeName, alias, wxNOT_FOUND);
}

int CMethods::AppendAttribute(const wxString &attributeName, const wxString &alias, int realName)
{
	auto itFounded = m_aTreeAttributes.find(attributeName.Upper());

	if (itFounded == m_aTreeAttributes.end())
	{
		m_aAttributes.emplace_back(attributeName, wxEmptyString, alias, realName);
		m_aTreeAttributes[attributeName.Upper()] = (size_t)(m_aAttributes.size() + 1);

		return m_aAttributes.size() + 1;
	}

	if (itFounded != m_aTreeAttributes.end()) 
		return itFounded->second - 1;

	return wxNOT_FOUND;
}

void CMethods::RemoveAttribute(const wxString &attributeName)
{
	auto itFounded = m_aTreeAttributes.find(attributeName.Upper());
	if (itFounded != m_aTreeAttributes.end()) { m_aTreeAttributes.erase(itFounded); }
}

wxString CMethods::GetMethodName(unsigned int nName) const
{
	if (nName < GetNMethods()) 
		return m_aMethods[nName].sName;
	return wxEmptyString;
}

wxString CMethods::GetMethodDescription(unsigned int nName) const
{
	if (nName < GetNMethods()) 
		return m_aMethods[nName].sShortDescription;
	return wxEmptyString;
}

wxString CMethods::GetAttributeName(unsigned int nName) const
{
	if (nName < GetNAttributes()) 
		return m_aAttributes[nName].sName;
	return wxEmptyString;
}

wxString CMethods::GetMethodAlias(unsigned int nName) const
{
	if (nName < GetNMethods()) 
		return m_aMethods[nName].sAlias;
	return wxEmptyString;
}

wxString CMethods::GetAttributeAlias(unsigned int nName) const
{
	if (nName < GetNAttributes())  
		return m_aAttributes[nName].sAlias;
	return wxEmptyString;
}

int CMethods::GetMethodPosition(unsigned int nName) const
{
	if (nName < GetNMethods()) 
		return m_aAttributes[nName].iName;
	return wxNOT_FOUND;
}

int CMethods::GetAttributePosition(unsigned int nName) const
{
	if (nName < GetNAttributes()) 
		return m_aAttributes[nName].iName;
	return wxNOT_FOUND;
}