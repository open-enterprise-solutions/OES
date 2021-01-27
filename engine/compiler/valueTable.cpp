////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : value table and key pair 
////////////////////////////////////////////////////////////////////////////

#include "valueTable.h"
#include "methods.h"
#include "functions.h"
#include "window/mainFrame.h"

#include "utils/stringutils.h"

//////////////////////////////////////////////////////////////////////

wxIMPLEMENT_DYNAMIC_CLASS(CValueTable, IValueTable);

//////////////////////////////////////////////////////////////////////

CMethods CValueTable::m_methods;

CValueTable::CValueTable() : IValueTable()
{
	m_aColumns = new CValueTableColumn(this);
	m_aColumns->IncrRef();

	//delete only in CValue
	//wxRefCounter::IncrRef(); 
}

CValueTable::CValueTable(const CValueTable& valueTable) : IValueTable(), m_aColumns(valueTable.m_aColumns)
{
	m_aColumns->IncrRef(); 

	//delete only in CValue
	//wxRefCounter::IncrRef();
}

CValueTable::~CValueTable() 
{
	if (m_aColumns) 
		m_aColumns->DecrRef(); 
}

// methods:
enum
{
	enAddRow = 0,
	enClone,
	enCount,
	enClear,
};

//attributes:
enum
{
	enColumns = 0,
};

void CValueTable::PrepareNames() const
{
	SEng aMethods[] =
	{
		{"add","add() - добавление строки"},

		{"clone","clone() - копирование таблицы"},
		{"count","count() - количество строк"},
		{"clear","clear() - очистка таблицы"},
	};

	int nCountM = sizeof(aMethods) / sizeof(aMethods[0]);
	m_methods.PrepareMethods(aMethods, nCountM);

	SEng aAttributes[] =
	{
		{"columns","columns"}
	};

	int nCountA = sizeof(aAttributes) / sizeof(aAttributes[0]);
	m_methods.PrepareAttributes(aAttributes, nCountA);
}

CValue CValueTable::GetAttribute(CAttributeParameters &aParams)
{
	CValue ret;

	switch (aParams.GetAttributeIndex())
	{
	case enColumns: return m_aColumns;
	}

	return ret;
}

CValue CValueTable::Method(CMethodParameters &aParams)
{
	CValue ret;

	switch (aParams.GetMethodIndex())
	{
	case enAddRow: return AddRow();
	case enClone: return Clone();
	case enCount: return (int)Count();
	case enClear: Clear(); break;
	}

	return ret;
}

#include "appData.h"

CValue CValueTable::GetAt(const CValue &cKey)
{
	unsigned int index = cKey.ToUInt(); 

	if (index < 0 || index >= m_aTableValues.size() && !appData->IsDesignerMode())
		CTranslateError::Error(_("Index goes beyond table"));

	return new CValueTableReturnLine(this, index);
}

//////////////////////////////////////////////////////////////////////
//               CValueTableColumn                                  //
//////////////////////////////////////////////////////////////////////

wxIMPLEMENT_DYNAMIC_CLASS(CValueTable::CValueTableColumn, IValueTable::IValueTableColumn);

CValueTable::CValueTableColumn::CValueTableColumn() : IValueTableColumn(), m_methods(NULL), m_ownerTable(NULL) {}
CValueTable::CValueTableColumn::CValueTableColumn(CValueTable *ownerTable) : IValueTableColumn(), m_methods(new CMethods()), m_ownerTable(ownerTable) {}
CValueTable::CValueTableColumn::~CValueTableColumn()
{
	for (auto& colInfo : m_aColumnInfo)
	{
		CValueTableColumnInfo *m_columnInfo = colInfo.second;
		wxASSERT(m_columnInfo);
		m_columnInfo->DecrRef();
	}

	if (m_methods) 
		delete m_methods;
}

//работа с массивом как с агрегатным объектом
//перечисление строковых ключей
enum
{
	enAddColumn = 0,
	enRemoveColumn
};

void CValueTable::CValueTableColumn::PrepareNames() const
{
	std::vector<SEng> aMethods =
	{
		{"addColumn","add(name, type, caption, width)"},
		{"removeColumn","removeColumn(name)"},
	};

	m_methods->PrepareMethods(aMethods.data(), aMethods.size());
}

inline bool CompareString(const wxString &lhs, const wxString &rhs)
{
	return lhs.CompareTo(rhs, wxString::ignoreCase) == 0;
}

#include "valueType.h"

CValue CValueTable::CValueTableColumn::Method(CMethodParameters &aParams)
{
	switch (aParams.GetMethodIndex())
	{
	case enAddColumn:
	{
		CValueType *m_valueType = NULL;

		if (aParams.GetParamCount() > 1)
			aParams[1].ConvertToValue(m_valueType);

		if (aParams.GetParamCount() > 3)
			return AddColumn(aParams[0].ToString(), m_valueType ? new CValueTypeDescription(m_valueType) : aParams[1].ConvertToType<CValueTypeDescription *>(), aParams[2].ToString(), aParams[3].ToInt());
		else if (aParams.GetParamCount() > 2)
			return AddColumn(aParams[0].ToString(), m_valueType ? new CValueTypeDescription(m_valueType) : aParams[1].ConvertToType<CValueTypeDescription *>(), aParams[2].ToString(), wxDVC_DEFAULT_WIDTH);
		else if (aParams.GetParamCount() > 1)
			return AddColumn(aParams[0].ToString(), m_valueType ? new CValueTypeDescription(m_valueType) : aParams[1].ConvertToType<CValueTypeDescription *>(), aParams[0].ToString(), wxDVC_DEFAULT_WIDTH);
		else
			return AddColumn(aParams[0].ToString(), NULL, aParams[0].ToString(), wxDVC_DEFAULT_WIDTH);
		break;
	}
	case enRemoveColumn:
	{
		wxString columnName = aParams[0].ToString();

		auto itFounded = std::find_if(m_aColumnInfo.begin(), m_aColumnInfo.end(),
			[columnName](std::pair<unsigned int, CValueTableColumnInfo *> columnData)
		{
			return CompareString(columnName, columnData.second->GetColumnName());
		});

		if (itFounded != m_aColumnInfo.end())
			RemoveColumn(itFounded->first);

		break;
	}
	}

	return CValue();
}

void CValueTable::CValueTableColumn::SetAt(const CValue &cKey, CValue &cVal)//индекс массива должен начинаться с 0
{
}

CValue CValueTable::CValueTableColumn::GetAt(const CValue &cKey) //индекс массива должен начинаться с 0
{
	unsigned int index = cKey.ToUInt();

	if ((index < 0 || index >= m_aColumnInfo.size() && !appData->IsDesignerMode()))
		CTranslateError::Error(_("Index goes beyond array"));

	auto itFounded = m_aColumnInfo.begin();
	std::advance(itFounded, index);
	return itFounded->second;
}

//////////////////////////////////////////////////////////////////////
//               CValueTableColumnInfo                              //
//////////////////////////////////////////////////////////////////////

wxIMPLEMENT_DYNAMIC_CLASS(CValueTable::CValueTableColumn::CValueTableColumnInfo, IValueTable::IValueTableColumn::IValueTableColumnInfo);

CValueTable::CValueTableColumn::CValueTableColumnInfo::CValueTableColumnInfo() : IValueTableColumnInfo(), m_methods(NULL) {}
CValueTable::CValueTableColumn::CValueTableColumnInfo::CValueTableColumnInfo(unsigned int colID, const wxString &colName, CValueTypeDescription *types, const wxString &caption, int width) : IValueTableColumnInfo(), m_methods(new CMethods()), m_columnID(colID), m_columnName(colName), m_columnTypes(types), m_columnCaption(caption), m_columnWidth(width) { if (m_columnTypes) m_columnTypes->IncrRef(); }
CValueTable::CValueTableColumn::CValueTableColumnInfo::~CValueTableColumnInfo() { if (m_columnTypes) m_columnTypes->DecrRef(); if (m_methods) delete m_methods; }

enum
{
	enColumnName,
	enColumnTypes,
	enColumnCaption,
	enColumnWidth
};

void CValueTable::CValueTableColumn::CValueTableColumnInfo::PrepareNames() const
{
	std::vector<SEng> aAttributes;

	{
		SEng aAttribute;
		aAttribute.sName = wxT("name");
		aAttribute.sAlias = wxT("default");
		aAttributes.push_back(aAttribute);
	}
	{
		SEng aAttribute;
		aAttribute.sName = wxT("types");
		aAttribute.sAlias = wxT("default");
		aAttributes.push_back(aAttribute);
	}
	{
		SEng aAttribute;
		aAttribute.sName = wxT("caption");
		aAttribute.sAlias = wxT("default");
		aAttributes.push_back(aAttribute);
	}
	{
		SEng aAttribute;
		aAttribute.sName = wxT("width");
		aAttribute.sAlias = wxT("default");
		aAttributes.push_back(aAttribute);
	}

	m_methods->PrepareAttributes(aAttributes.data(), aAttributes.size());
}

CValue CValueTable::CValueTableColumn::CValueTableColumnInfo::GetAttribute(CAttributeParameters &aParams)
{
	switch (aParams.GetAttributeIndex())
	{
	case enColumnName: return m_columnName;
	case enColumnTypes: return m_columnTypes ? m_columnTypes : CValue();
	case enColumnCaption: return m_columnCaption;
	case enColumnWidth: return m_columnWidth;
	}

	return CValue();
}

//////////////////////////////////////////////////////////////////////
//               CValueTableReturnLine                              //
//////////////////////////////////////////////////////////////////////

wxIMPLEMENT_DYNAMIC_CLASS(CValueTable::CValueTableReturnLine, IValueTable::IValueTableReturnLine);

CValueTable::CValueTableReturnLine::CValueTableReturnLine() : IValueTableReturnLine(), m_methods(NULL), m_ownerTable(NULL), m_lineTable(wxNOT_FOUND) {}
CValueTable::CValueTableReturnLine::CValueTableReturnLine(CValueTable *ownerTable, int line) : IValueTableReturnLine(), m_methods(new CMethods()), m_ownerTable(ownerTable), m_lineTable(line) {}
CValueTable::CValueTableReturnLine::~CValueTableReturnLine() { if (m_methods) delete m_methods; }

void CValueTable::CValueTableReturnLine::PrepareNames() const
{
	std::vector<SEng> aAttributes;

	for (auto &colInfo : m_ownerTable->m_aColumns->m_aColumnInfo)
	{
		CValueTableColumn::CValueTableColumnInfo *m_columnInfo = colInfo.second;
		wxASSERT(m_columnInfo);

		SEng aAttribute;

		aAttribute.sName = m_columnInfo->GetColumnName();
		aAttribute.sAlias = wxT("default");
		aAttribute.iName = colInfo.first;

		aAttributes.push_back(aAttribute);
	}

	m_methods->PrepareAttributes(aAttributes.data(), aAttributes.size());
}

void CValueTable::CValueTableReturnLine::SetAttribute(CAttributeParameters &aParams, CValue &cVal)
{
	int index = m_methods->GetAttributePosition(aParams.GetAttributeIndex());

	auto itFoundedByLine = m_ownerTable->m_aTableValues.begin();
	std::advance(itFoundedByLine, m_lineTable);

	CValueTypeDescription *m_typeDescription = m_ownerTable->m_aColumns->GetColumnType(index);
	itFoundedByLine->insert_or_assign(index, m_typeDescription ? m_typeDescription->AdjustValue(cVal) : cVal);
}

CValue CValueTable::CValueTableReturnLine::GetAttribute(CAttributeParameters &aParams)
{
	if (appData->IsDesignerMode())
		return CValue();

	int index = m_methods->GetAttributePosition(aParams.GetAttributeIndex());

	auto itFoundedByLine = m_ownerTable->m_aTableValues.begin();
	std::advance(itFoundedByLine, m_lineTable);

	auto itFoundedByIndex = itFoundedByLine->find(index);

	if (itFoundedByIndex != itFoundedByLine->end())
		return itFoundedByIndex->second;

	return CValue();
}

//**********************************************************************
//*                       Runtime register                             *
//**********************************************************************

VALUE_REGISTER(CValueTable, "table", TEXT2CLSID("VL_TABL"));

SO_VALUE_REGISTER(CValueTable::CValueTableColumn, "tableValueColumn", CValueTableColumn, TEXT2CLSID("VL_TAVC"));
SO_VALUE_REGISTER(CValueTable::CValueTableColumn::CValueTableColumnInfo, "tableValueColumnInfo", CValueTableColumnInfo, TEXT2CLSID("VL_TVCI"));
SO_VALUE_REGISTER(CValueTable::CValueTableReturnLine, "tableValueRow", CValueTableReturnLine, TEXT2CLSID("VL_TVCR"));