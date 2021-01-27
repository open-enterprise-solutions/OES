////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : tabular sections - model
////////////////////////////////////////////////////////////////////////////

#include "tablePart.h"

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

void IValueTabularSection::Prepend(const wxString &text)
{
	RowPrepended();
}

void IValueTabularSection::DeleteItem(const wxDataViewItem &item)
{
	unsigned int row = GetRow(item);
	RowDeleted(row);
}

void IValueTabularSection::DeleteItems(const wxDataViewItemArray &items)
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

unsigned int IValueTabularSection::GetColumnCount() const
{
	return 0;
}

wxString IValueTabularSection::GetColumnType(unsigned int col) const
{
	return wxT("string");
}

void IValueTabularSection::GetValueByRow(wxVariant &variant,
	unsigned int row, unsigned int col) const
{
}

bool IValueTabularSection::GetAttrByRow(unsigned int row, unsigned int col, wxDataViewItemAttr &attr) const
{
	return true;
}

bool IValueTabularSection::SetValueByRow(const wxVariant &variant,
	unsigned int row, unsigned int col)
{
	return false;
}