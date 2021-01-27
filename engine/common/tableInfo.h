#ifndef _TABLE_MODEL_H__
#define _TABLE_MODEL_H__

#include "compiler/value.h"
#include "compiler/valueTypeDescription.h"

#include <wx/dataview.h>

//Поддержка таблиц
class IValueTable : public CValue,
	public wxDataViewModel
{
	wxDECLARE_ABSTRACT_CLASS(IValueTable);

public:

	class IValueTableColumn : public CValue
	{
		wxDECLARE_ABSTRACT_CLASS(IValueTableColumn);

	public:

		class IValueTableColumnInfo : public CValue
		{
			wxDECLARE_ABSTRACT_CLASS(IValueTableColumnInfo);

		public:

			virtual unsigned int GetColumnID() = 0;
			virtual wxString GetColumnName() = 0;
			virtual wxString GetColumnCaption() = 0;
			virtual CValueTypeDescription *GetColumnTypes() = 0;
			virtual int GetColumnWidth() = 0;

			IValueTableColumnInfo() : CValue(eValueTypes::TYPE_VALUE, true) {}
			virtual ~IValueTableColumnInfo() {}
		};

	public:

		virtual bool HasColumnID(unsigned int col_id);
		virtual IValueTableColumnInfo *GetColumnByID(unsigned int col_id);

		virtual IValueTableColumnInfo *GetColumnInfo(unsigned int idx) const = 0;
		virtual unsigned int GetColumnCount() const = 0;

		IValueTableColumn() : CValue(eValueTypes::TYPE_VALUE, true) {}
		virtual ~IValueTableColumn() {}
	};

	class IValueTableReturnLine : public CValue
	{
		wxDECLARE_ABSTRACT_CLASS(IValueTableReturnLine);

	public:

		virtual unsigned int GetLineTable() const = 0;
		virtual IValueTable *GetOwnerTable() const = 0;

		IValueTableReturnLine();
		virtual ~IValueTableReturnLine();

		//operator '=='
		virtual inline bool CompareValueEQ(const CValue &cParam) const override
		{
			IValueTableReturnLine *tableReturnLine = NULL;
			if (cParam.ConvertToValue(tableReturnLine))
			{
				IValueTable *ownerTable = GetOwnerTable(); unsigned int lineTable = GetLineTable();
				wxASSERT(ownerTable);
				if (ownerTable == tableReturnLine->GetOwnerTable()
					&& lineTable == GetLineTable())
				{
					return true;
				}
			}
			return false;
		}

		//operator '!='
		virtual inline bool CompareValueNE(const CValue &cParam) const override
		{
			IValueTableReturnLine *tableReturnLine = NULL;
			if (cParam.ConvertToValue(tableReturnLine))
			{
				IValueTable *ownerTable = GetOwnerTable(); unsigned int lineTable = GetLineTable();
				wxASSERT(ownerTable);
				if (ownerTable == tableReturnLine->GetOwnerTable()
					&& lineTable == GetLineTable())
				{
					return false;
				}
			}
			return true;
		}
	};

public:

	void RowPrepended();
	void RowInserted(unsigned int before);
	void RowAppended();
	void RowDeleted(unsigned int row);
	void RowsDeleted(const wxArrayInt &rows);
	void RowChanged(unsigned int row);
	void RowValueChanged(unsigned int row, unsigned int col);
	void Reset(unsigned int new_size);

	// derived classes should override these methods instead of
	// {Get,Set}Value() and GetAttr() inherited from the base class

	virtual void GetValueByRow(wxVariant &variant,
		unsigned int row, unsigned int col) const = 0;

	virtual bool SetValueByRow(const wxVariant &variant,
		unsigned int row, unsigned int col) = 0;

	virtual bool GetAttrByRow(unsigned int row, unsigned int col,
		wxDataViewItemAttr &attr) const
	{
		return false;
	}

	virtual bool IsEnabledByRow(unsigned int row,
		unsigned int col) const
	{
		return true;
	}

	virtual wxDataViewItem GetItem(unsigned int row) const;

	// helper methods provided by list models only
	virtual unsigned GetRow(const wxDataViewItem &item) const;

	// implement base methods
	virtual unsigned int GetChildren(const wxDataViewItem &item, wxDataViewItemArray &children) const override;

	// returns the number of rows
	virtual unsigned int GetCount() const { return (unsigned int)m_hash.GetCount(); }

	// implement some base class pure virtual directly
	virtual wxDataViewItem GetParent(const wxDataViewItem & item) const override
	{
		// items never have valid parent in this model
		return wxDataViewItem();
	}

	virtual bool IsContainer(const wxDataViewItem &item) const override
	{
		// only the invisible (and invalid) root item has children
		return !item.IsOk();
	}

	// and implement some others by forwarding them to our own ones
	virtual void GetValue(wxVariant &variant,
		const wxDataViewItem &item, unsigned int col) const override
	{
		GetValueByRow(variant, GetRow(item), col);
	}

	virtual bool SetValue(const wxVariant &variant,
		const wxDataViewItem &item, unsigned int col) override
	{
		return SetValueByRow(variant, GetRow(item), col);
	}

	virtual bool GetAttr(const wxDataViewItem &item, unsigned int col,
		wxDataViewItemAttr &attr) const override
	{
		return GetAttrByRow(GetRow(item), col, attr);
	}

	virtual bool IsEnabled(const wxDataViewItem &item, unsigned int col) const override
	{
		return IsEnabledByRow(GetRow(item), col);
	}

	virtual bool IsListModel() const override { return true; }

	//////////////////////////////////////////////////////////////////////////////////////////////////

	virtual bool AutoCreateColumns() { return true; }
	virtual bool EditableRows() { return true; }

	virtual IValueTableReturnLine *GetRowAt(wxDataViewItem &item)
	{
		return GetRowAt(GetRow(item));
	}

	virtual IValueTableReturnLine *GetRowAt(unsigned int line) = 0;
	virtual IValueTableColumn *GetColumns() const = 0;

	//ref counter 
	virtual void DecrRef() override
	{
		if (CValue::GetRefCount() > 1)
			CValue::DecrRef();
		else if (wxRefCounter::GetRefCount() < 2)
			CValue::DecrRef();
	}

	IValueTable(unsigned int initial_size = 0);
	virtual ~IValueTable() {}

private:

	wxDataViewItemArray m_hash;
	unsigned int m_nextFreeID;
	bool m_ordered;
};

#endif 