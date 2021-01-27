#ifndef _VALUETABLE_H__
#define _VALUETABLE_H__

#include "value.h"
#include "valueArray.h"
#include "valueMap.h"
#include "common/tableInfo.h"
#include "utils/stringutils.h"

//Поддержка таблиц
class CValueTable : public IValueTable
{
	wxDECLARE_DYNAMIC_CLASS(CValueTable);

public:

	class CValueTableColumn : public IValueTable::IValueTableColumn
	{
		wxDECLARE_DYNAMIC_CLASS(CValueTableColumn);

	public:

		class CValueTableColumnInfo : public IValueTable::IValueTableColumn::IValueTableColumnInfo
		{
			wxDECLARE_DYNAMIC_CLASS(CValueTableColumnInfo);

		private:

			unsigned int m_columnID;
			wxString m_columnName;
			CValueTypeDescription *m_columnTypes;
			wxString m_columnCaption;
			int m_columnWidth;

		public:

			CValueTableColumnInfo();
			CValueTableColumnInfo(unsigned int colID, const wxString &colName, CValueTypeDescription *types, const wxString &caption, int width);
			virtual ~CValueTableColumnInfo();

			virtual unsigned int GetColumnID() { return m_columnID; }
			virtual wxString GetColumnName() { return m_columnName; }
			virtual wxString GetColumnCaption() { return m_columnCaption; }
			virtual CValueTypeDescription *GetColumnTypes() { return m_columnTypes; }
			virtual int GetColumnWidth() { return m_columnWidth; }

			virtual CMethods* GetPMethods() const { PrepareNames(); return m_methods; }; //получить ссылку на класс помощник разбора имен атрибутов и методов
			virtual void PrepareNames() const;                         //этот метод автоматически вызывается для инициализации имен атрибутов и методов

			virtual CValue GetAttribute(CAttributeParameters &aParams); //значение атрибута

			virtual wxString GetTypeString() const { return wxT("tableValueColumnInfo"); }
			virtual wxString GetString() const { return wxT("tableValueColumnInfo"); }

			friend CValueTableColumn;

		protected:

			CMethods *m_methods;
		};

	public:

		CValueTableColumn();
		CValueTableColumn(CValueTable *ownerTable);
		virtual ~CValueTableColumn();

		CValueTableColumnInfo *AddColumn(const wxString &colName, CValueTypeDescription *types, const wxString &caption, int width)
		{
			unsigned int max_id = 0;

			for (auto &col : m_aColumnInfo)
			{
				if (max_id < col.first)
					max_id = col.first;
			}

			CValueTableColumnInfo *m_columnInfo = new CValueTableColumnInfo(max_id + 1, colName, types, caption, width);
			m_aColumnInfo.insert_or_assign(max_id + 1, m_columnInfo);

			for (auto &rowValue : m_ownerTable->m_aTableValues)
				rowValue.insert_or_assign(max_id + 1, types ? types->AdjustValue(CValue(NULL)) : CValue());

			m_columnInfo->IncrRef();
			return m_columnInfo;
		}

		CValueTypeDescription *GetColumnType(unsigned int col) const
		{
			CValueTableColumnInfo *m_columnInfo = m_aColumnInfo.at(col);
			wxASSERT(m_columnInfo);
			return m_columnInfo->m_columnTypes;
		}

		void RemoveColumn(unsigned int col)
		{
			for (auto &rowValue : m_ownerTable->m_aTableValues)
				rowValue.erase(col);

			auto foundedIt = m_aColumnInfo.find(col);
			m_aColumnInfo.erase(foundedIt);

			CValueTableColumnInfo *m_columnInfo = foundedIt->second;
			wxASSERT(m_columnInfo);
			m_columnInfo->DecrRef();
		}

		virtual IValueTableColumnInfo *GetColumnInfo(unsigned int idx) const
		{
			if (m_aColumnInfo.size() < idx)
				return NULL;

			auto foundedIt = m_aColumnInfo.begin();
			std::advance(foundedIt, idx);
			return foundedIt->second;
		}

		virtual unsigned int GetColumnCount() const { return m_aColumnInfo.size(); }

		virtual CMethods* GetPMethods() const { PrepareNames(); return m_methods; }; //получить ссылку на класс помощник разбора имен атрибутов и методов
		virtual void PrepareNames() const;

		virtual wxString GetTypeString() const { return wxT("tableValueColumn"); }
		virtual wxString GetString() const { return wxT("tableValueColumn"); }

		//РАБОТА КАК АГРЕГАТНОГО ОБЪЕКТА
		virtual CValue Method(CMethodParameters &aParams);

		//array support 
		virtual CValue GetAt(const CValue &cKey);
		virtual void SetAt(const CValue &cKey, CValue &cVal);

		//Работа с итераторами 
		virtual bool HasIterator() const { return true; }
		virtual CValue GetItAt(unsigned int idx)
		{
			auto itFounded = m_aColumnInfo.begin();
			std::advance(itFounded, idx);
			return itFounded->second;
		}

		virtual unsigned int GetItSize() const { return GetColumnCount(); }

		friend class CValueTable;

	protected:

		CValueTable *m_ownerTable;
		CMethods *m_methods;

		std::map<unsigned int, CValueTableColumnInfo *> m_aColumnInfo;

	} *m_aColumns;

	class CValueTableReturnLine : public IValueTableReturnLine
	{
		wxDECLARE_DYNAMIC_CLASS(CValueTableReturnLine);

	public:

		CValueTable *m_ownerTable; int m_lineTable;

	public:

		virtual unsigned int GetLineTable() const { return m_lineTable; }
		virtual IValueTable *GetOwnerTable() const { return m_ownerTable; }

		CValueTableReturnLine();
		CValueTableReturnLine(CValueTable *ownerTable, int line);
		virtual ~CValueTableReturnLine();

		virtual CMethods* GetPMethods() const { PrepareNames(); return m_methods; }; //получить ссылку на класс помощник разбора имен атрибутов и методов
		virtual void PrepareNames() const;                         //этот метод автоматически вызывается для инициализации имен атрибутов и методов

		virtual wxString GetTypeString() const { return wxT("tableValueRow"); }
		virtual wxString GetString() const { return wxT("tableValueRow"); }

		virtual void SetAttribute(CAttributeParameters &aParams, CValue &cVal); //установка атрибута
		virtual CValue GetAttribute(CAttributeParameters &aParams); //значение атрибута

	private:

		CMethods *m_methods;
	};

public:

	virtual IValueTableColumn *GetColumns() const
	{
		return m_aColumns;
	}

	virtual IValueTableReturnLine *GetRowAt(unsigned int line)
	{
		if (line > m_aTableValues.size())
			return NULL;

		return new CValueTableReturnLine(this, line);
	}

	CValueTable();
	CValueTable(const CValueTable& val);
	virtual ~CValueTable();

	virtual CValue GetAt(const CValue &cKey);

	wxString GetTypeString() const { return wxT("tableValue"); }
	wxString GetString() const { return wxT("tableValue"); }

	//check is empty
	virtual inline bool IsEmpty() const override { return m_aTableValues.empty(); }

	static CMethods m_methods;

	virtual CValue GetAttribute(CAttributeParameters &aParams);                   //значение атрибута

	virtual CMethods* GetPMethods() const { return &m_methods; }; //получить ссылку на класс помощник разбора имен атрибутов и методов
	virtual void PrepareNames() const;                         //этот метод автоматически вызывается для инициализации имен атрибутов и методов
	virtual CValue Method(CMethodParameters &aParams);       //вызов метода

	//support def. table (in runtime)
	void Prepend(const wxString &text);
	void DeleteItem(const wxDataViewItem &item);
	void DeleteItems(const wxDataViewItemArray &items);

	// implementation of base class virtuals to define model
	virtual unsigned int GetColumnCount() const override;
	virtual wxString GetColumnType(unsigned int col) const override;

	virtual void GetValueByRow(wxVariant &variant,
		unsigned int row, unsigned int col) const override;

	virtual bool GetAttrByRow(unsigned int row, unsigned int col, wxDataViewItemAttr &attr) const override;

	virtual bool SetValueByRow(const wxVariant &variant,
		unsigned int row, unsigned int col) override;

	//support def. methods (in runtime)
	CValue AddRow();
	CValue Clone() { return new CValueTable(*this); }
	unsigned int Count() { return m_aTableValues.size(); }
	void Clear();
	void DeleteRow(unsigned int idx);

	//Работа с итераторами 
	virtual bool HasIterator() const override { return true; }

	virtual CValue GetItEmpty() override
	{
		return new CValueTableReturnLine(this, wxNOT_FOUND);
	}

	virtual CValue GetItAt(unsigned int idx) override
	{
		if (idx > m_aTableValues.size())
			return CValue();

		return new CValueTableReturnLine(this, idx);
	}

	virtual unsigned int GetItSize() const override { return m_aTableValues.size(); }

protected:

	std::vector<std::map<unsigned int, CValue>> m_aTableValues;
};

#endif