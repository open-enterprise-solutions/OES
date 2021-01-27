#ifndef _VALUETABLEPART_H__

#include "common/tableInfo.h"
#include "metadata/metaObjects/tables/tables.h"
#include "utils/guid.h"

class IValueTabularSection : public IValueTable
{
	wxDECLARE_ABSTRACT_CLASS(IValueTabularSection);

public:

	virtual IValueTableColumn *GetColumns() const { return NULL; }
	virtual IValueTableReturnLine *GetRowAt(unsigned int line) { return NULL; };

	class CValueTabularSectionReturnLine : public IValueTableReturnLine
	{
		wxDECLARE_DYNAMIC_CLASS(CValueTabularSectionReturnLine);

	public:

		IValueTabularSection *m_ownerTable; int m_lineTable;

	public:

		virtual unsigned int GetLineTable() const { return 0; }
		virtual IValueTable *GetOwnerTable() const { return m_ownerTable; }

		CValueTabularSectionReturnLine() : IValueTableReturnLine() {}
		CValueTabularSectionReturnLine(IValueTabularSection *ownerTable, int line) : IValueTableReturnLine(), m_ownerTable(ownerTable), m_lineTable(line) {}
		virtual ~CValueTabularSectionReturnLine() {}

		virtual CMethods* GetPMethods() const { PrepareNames(); return &m_methods; }; //получить ссылку на класс помощник разбора имен атрибутов и методов
		virtual void PrepareNames() const;                         //этот метод автоматически вызывается для инициализации имен атрибутов и методов

		static CMethods m_methods;

		virtual void SetAttribute(CAttributeParameters &aParams, CValue &cVal); //установка атрибута
		virtual CValue GetAttribute(CAttributeParameters &aParams); //значение атрибута

		virtual wxString GetTypeString() const { return wxT("tablePartRow"); }
		virtual wxString GetString() const { return wxT("tablePartRow"); }

		friend class IValueTabularSection;

	private:

	};

	IValueTabularSection() {}
	IValueTabularSection(IMetaObjectValue *metaObject, CTableObject *tableObject) : m_metaTableObj(tableObject) {}
	virtual ~IValueTabularSection() {}

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

	virtual bool LoadDataFromDB() { return true; }
	virtual bool SaveDataInDB() { return true; }
	virtual bool DeleteDataInDB() { return true; }

protected:

	CTableObject *m_metaTableObj;

	struct CDataTable
	{
		std::map<wxString, CValue> aDataValues;
		std::map<wxString, unsigned int> aDataTypes;
	};

	std::vector<CDataTable> m_aObjectValues;
};

class CValueTabularSection : public IValueTabularSection
{
	wxDECLARE_DYNAMIC_CLASS(CValueTabularSection);

public:

	CValueTabularSection();
	CValueTabularSection(IMetaObjectValue *metaObject, CTableObject *tableObject, IDataObjectValue *dataObject);
	virtual ~CValueTabularSection();

	//****************************************************************************
	//*                              Support methods                             *
	//****************************************************************************

	static CMethods m_methods;

	virtual CMethods* GetPMethods() const;                          // получить ссылку на класс помощник разбора имен атрибутов и методов
	virtual void PrepareNames() const;                             // этот метод автоматически вызывается для инициализации имен атрибутов и методов
	virtual CValue Method(CMethodParameters &aParams);       // вызов метода

	//****************************************************************************
	//*                              Override attribute                          *
	//****************************************************************************

	virtual void SetAttribute(CAttributeParameters &aParams, CValue &cVal);        //установка атрибута
	virtual CValue GetAttribute(CAttributeParameters &aParams);                   //значение атрибута

	//****************************************************************************
	//*                              Override type                               *
	//****************************************************************************

	virtual wxString GetTypeString() const;
	virtual wxString GetString() const;

protected:

	IMetaObjectValue *m_metaObject;
	IDataObjectValue *m_dataObject;
};

#define TABLE_PART_NAME wxT("tbpt_")

class CValueTabularRefSection : public IValueTabularSection
{
	wxDECLARE_DYNAMIC_CLASS(CValueTabularRefSection);

public:

	CValueTabularRefSection();
	CValueTabularRefSection(IMetaObjectRefValue *metaObject, CTableObject *tableObject, const Guid &objGuid, bool newObject = false);
	virtual ~CValueTabularRefSection();

	virtual bool LoadDataFromDB();
	virtual bool SaveDataInDB();
	virtual bool DeleteDataInDB();

	//****************************************************************************
	//*                              Support methods                             *
	//****************************************************************************

	static CMethods m_methods;

	virtual CMethods* GetPMethods() const;                          // получить ссылку на класс помощник разбора имен атрибутов и методов
	virtual void PrepareNames() const;                             // этот метод автоматически вызывается для инициализации имен атрибутов и методов
	virtual CValue Method(CMethodParameters &aParams);       // вызов метода

	//****************************************************************************
	//*                              Override attribute                          *
	//****************************************************************************

	virtual void SetAttribute(CAttributeParameters &aParams, CValue &cVal);        //установка атрибута
	virtual CValue GetAttribute(CAttributeParameters &aParams);                   //значение атрибута

	//****************************************************************************
	//*                              Override type                               *
	//****************************************************************************

	virtual wxString GetTypeString() const;
	virtual wxString GetString() const;

protected:

	bool m_bNewObject;
	Guid m_objGuid; 
	IMetaObjectRefValue *m_metaObject;
};

#endif // !_VALUEUUID_H__
