////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : table model information
////////////////////////////////////////////////////////////////////////////

#include "tableInfo.h"

static int my_sort(int *v1, int *v2)
{
	return *v2 - *v1;
}

wxIMPLEMENT_ABSTRACT_CLASS(IValueTable, CValue);
wxIMPLEMENT_ABSTRACT_CLASS(IValueTable::IValueTableColumn, CValue);
wxIMPLEMENT_ABSTRACT_CLASS(IValueTable::IValueTableColumn::IValueTableColumnInfo, CValue);
wxIMPLEMENT_ABSTRACT_CLASS(IValueTable::IValueTableReturnLine, CValue);

IValueTable::IValueTable(unsigned int initial_size)
	: CValue(eValueTypes::TYPE_VALUE), wxDataViewModel()
{
	// IDs are ordered until an item gets deleted or inserted
	m_ordered = true;

	// build initial index
	unsigned int i;

	for (i = 1; i < initial_size + 1; i++)
		m_hash.Add(wxDataViewItem(wxUIntToPtr(i)));

	m_nextFreeID = initial_size + 1;
}

void IValueTable::Reset(unsigned int new_size)
{
	/* wxDataViewModel:: */ BeforeReset();

	m_hash.Clear();

	// IDs are ordered until an item gets deleted or inserted
	m_ordered = true;

	// build initial index
	unsigned int i;
	for (i = 1; i < new_size + 1; i++)
		m_hash.Add(wxDataViewItem(wxUIntToPtr(i)));

	m_nextFreeID = new_size + 1;

	/* wxDataViewModel:: */ AfterReset();
}

void IValueTable::RowPrepended()
{
	m_ordered = false;

	unsigned int id = m_nextFreeID;
	m_nextFreeID++;

	wxDataViewItem item(wxUIntToPtr(id));
	m_hash.Insert(item, 0);
	ItemAdded(wxDataViewItem(0), item);

}

void IValueTable::RowInserted(unsigned int before)
{
	m_ordered = false;

	unsigned int id = m_nextFreeID;
	m_nextFreeID++;

	wxDataViewItem item(wxUIntToPtr(id));
	m_hash.Insert(item, before);
	ItemAdded(wxDataViewItem(0), item);
}

void IValueTable::RowAppended()
{
	unsigned int id = m_nextFreeID;
	m_nextFreeID++;

	wxDataViewItem item(wxUIntToPtr(id));
	m_hash.Add(item);
	ItemAdded(wxDataViewItem(0), item);
}

void IValueTable::RowDeleted(unsigned int row)
{
	m_ordered = false;

	wxDataViewItem item(m_hash[row]);
	m_hash.RemoveAt(row);
	/* wxDataViewModel:: */ ItemDeleted(wxDataViewItem(0), item);
}

void IValueTable::RowsDeleted(const wxArrayInt &rows)
{
	m_ordered = false;

	wxDataViewItemArray array;
	unsigned int i;
	for (i = 0; i < rows.GetCount(); i++)
	{
		wxDataViewItem item(m_hash[rows[i]]);
		array.Add(item);
	}

	wxArrayInt sorted = rows;
	sorted.Sort(my_sort);
	for (i = 0; i < sorted.GetCount(); i++)
		m_hash.RemoveAt(sorted[i]);

	/* wxDataViewModel:: */ ItemsDeleted(wxDataViewItem(0), array);
}

void IValueTable::RowChanged(unsigned int row)
{
	/* wxDataViewModel:: */ ItemChanged(GetItem(row));
}

void IValueTable::RowValueChanged(unsigned int row, unsigned int col)
{
	/* wxDataViewModel:: */ ValueChanged(GetItem(row), col);
}

unsigned int IValueTable::GetRow(const wxDataViewItem &item) const
{
	if (m_ordered)
		return wxPtrToUInt(item.GetID()) - 1;

	// assert for not found
	return (unsigned int)m_hash.Index(item);
}

wxDataViewItem IValueTable::GetItem(unsigned int row) const
{
	wxASSERT(row < m_hash.GetCount());
	return wxDataViewItem(m_hash[row]);
}

unsigned int IValueTable::GetChildren(const wxDataViewItem &item, wxDataViewItemArray &children) const
{
	if (item.IsOk())
		return 0;

	children = m_hash;

	return m_hash.GetCount();
}

bool IValueTable::IValueTableColumn::HasColumnID(unsigned int col_id)
{
	return GetColumnByID(col_id) != NULL;
}

IValueTable::IValueTableColumn::IValueTableColumnInfo *IValueTable::IValueTableColumn::GetColumnByID(unsigned int col_id)
{
	for (unsigned int idx = 0; idx < GetColumnCount(); idx++)
	{
		IValueTableColumnInfo *m_columnInfo = GetColumnInfo(idx);
		wxASSERT(m_columnInfo);

		if (col_id == m_columnInfo->GetColumnID())
			return m_columnInfo;
	}

	return NULL;
}

IValueTable::IValueTableReturnLine::IValueTableReturnLine() : CValue(eValueTypes::TYPE_VALUE, true) {}

IValueTable::IValueTableReturnLine::~IValueTableReturnLine() { }