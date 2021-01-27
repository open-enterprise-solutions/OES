////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : table models 
////////////////////////////////////////////////////////////////////////////

#include "valueTable.h"

static int my_sort_reverse(int *v1, int *v2)
{
	return *v2 - *v1;
}

static int my_sort(int *v1, int *v2)
{
	return *v1 - *v2;
}

//***********************************************************************************
//*                                  Model                                          *
//***********************************************************************************

void CValueTable::Prepend(const wxString &text)
{
	RowPrepended();
}

void CValueTable::DeleteItem(const wxDataViewItem &item)
{
	unsigned int row = GetRow(item);
	RowDeleted(row);
}

void CValueTable::DeleteItems(const wxDataViewItemArray &items)
{
	wxArrayInt rows;

	for (unsigned i = 0; i < items.GetCount(); i++)
	{
		unsigned int row = GetRow(items[i]);
		rows.Add(row);
	}

	// Sort in descending order so that the last
	// row will be deleted first. Otherwise the
	// remaining indeces would all be wrong.
	rows.Sort(my_sort_reverse);

	// This is just to test if wxDataViewCtrl can
	// cope with removing rows not sorted in
	// descending order
	rows.Sort(my_sort);
	RowsDeleted(rows);
}

unsigned int CValueTable::GetColumnCount() const
{
	return m_aColumns->GetColumnCount();
}

wxString CValueTable::GetColumnType(unsigned int col) const
{
	CValueTypeDescription *m_typeDescription = m_aColumns->GetColumnType(col);

	if (!m_typeDescription)
		return wxT("string");

	for (auto& typeValue : m_typeDescription->GetLongTypes())
	{
		if (typeValue == eValueTypes::TYPE_BOOLEAN)
			return wxT("bool");
		else if (typeValue == eValueTypes::TYPE_NUMBER)
			return wxT("number");
		else if (typeValue == eValueTypes::TYPE_DATE)
			return wxT("date");
		else if (typeValue == eValueTypes::TYPE_STRING)
			return wxT("string");
	}

	return wxT("string");
}

void CValueTable::GetValueByRow(wxVariant &variant,
	unsigned int row, unsigned int col) const
{
	auto itFounded = m_aTableValues.begin();
	std::advance(itFounded, row);

	if (itFounded->find(col) != itFounded->end())
	{
		const CValue &cValue = itFounded->at(col);
		variant = cValue.GetString();
	}
}

bool CValueTable::GetAttrByRow(unsigned int row, unsigned int col, wxDataViewItemAttr &attr) const
{
	return true;
}

bool CValueTable::SetValueByRow(const wxVariant &variant,
	unsigned int row, unsigned int col)
{
	auto itFounded = m_aTableValues.begin();
	std::advance(itFounded, row);

	if (itFounded->find(col) != itFounded->end())
	{
		CValue &cValue = itFounded->at(col);
		CValueTypeDescription *m_typeDescription = m_aColumns->GetColumnType(col);

		if (m_typeDescription)
		{
			CValue cString(variant.GetString());
			cValue = m_typeDescription->AdjustValue(cString);
		}
		else
		{
			cValue.SetValue(variant.GetString());
		}
	}

	return false;
}