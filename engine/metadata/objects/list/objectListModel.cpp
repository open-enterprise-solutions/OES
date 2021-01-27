////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : list model 
////////////////////////////////////////////////////////////////////////////

#include "objectList.h"

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


void CObjectListValue::Prepend(const wxString &text)
{
	RowPrepended();
}

void CObjectListValue::DeleteItem(const wxDataViewItem &item)
{
	unsigned int row = GetRow(item);
	RowDeleted(row);
}

void CObjectListValue::DeleteItems(const wxDataViewItemArray &items)
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

// implementation of base class virtuals to define model
unsigned int CObjectListValue::GetColumnCount() const
{
	return 0;
}

wxString CObjectListValue::GetColumnType(unsigned int col) const
{
	return wxT("string");
}

void CObjectListValue::GetValueByRow(wxVariant &variant,
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

bool CObjectListValue::GetAttrByRow(unsigned int row, unsigned int col, wxDataViewItemAttr &attr) const
{
	return true;
}

bool CObjectListValue::SetValueByRow(const wxVariant &variant,
	unsigned int row, unsigned int col)
{
	auto itFounded = m_aTableValues.begin();
	std::advance(itFounded, row);

	if (itFounded->find(col) != itFounded->end())
	{
		CValue &cValue = itFounded->at(col);/*
		CValueTypeDescription *m_typeDescription = m_aColumns->GetColumnType(col);

		if (m_typeDescription)
		{
			CValue cString(variant.GetString());
			cValue = m_typeDescription->AdjustValue(cString);
		}
		else
		{
			cValue.SetValue(variant.GetString());
		}*/
		cValue.SetValue(variant.GetString());
	}

	return false;
}