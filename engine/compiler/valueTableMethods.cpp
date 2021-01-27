////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : table methods 
////////////////////////////////////////////////////////////////////////////

#include "valuetable.h"
#include "functions.h"

CValue CValueTable::AddRow()
{
	std::map<unsigned int, CValue> m_valueRow;

	for (auto &colData : m_aColumns->m_aColumnInfo)
	{
		CValueTypeDescription *m_typeDescription = m_aColumns->GetColumnType(colData.first);
		m_valueRow.insert_or_assign(colData.first, m_typeDescription ? m_typeDescription->AdjustValue(CValue()) : CValue());
	}

	if (!CTranslateError::IsSimpleMode())
		m_aTableValues.push_back(m_valueRow);

	if (!CTranslateError::IsSimpleMode())
		IValueTable::Reset(m_aTableValues.size());

	return new CValueTableReturnLine(this, m_aTableValues.size() - 1);
}

void CValueTable::Clear()
{
	if (CTranslateError::IsSimpleMode())
		return;

	IValueTable::Reset(0);
	m_aTableValues.clear();
}

void CValueTable::DeleteRow(unsigned int idx)
{
	if (m_aTableValues.size() < idx)
		return;

	auto itValuePos = m_aTableValues.begin();
	std::advance(itValuePos, idx);
	m_aTableValues.erase(itValuePos);

	if (!CTranslateError::IsSimpleMode())
		IValueTable::Reset(m_aTableValues.size());
}