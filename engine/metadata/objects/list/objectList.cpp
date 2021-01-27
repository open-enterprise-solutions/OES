////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : list data 
////////////////////////////////////////////////////////////////////////////

#include "objectList.h"
#include "forms/elements/frame.h"
#include "compiler/methods.h"
#include "appData.h"

CObjectListValue::CObjectListValue(IMetaObjectRefValue *metaObj, IMetaFormObject *formObject, IOwnerInfo *ownerControl) :
	IDataObjectList(metaObj, formObject, ownerControl)
{
	if (appData->IsEnterpriseMode()) {
		UpdateModel();
	}
}

//////////////////////////////////////////////////////////////////////
//               CObjectListValueReturnLine                              //
//////////////////////////////////////////////////////////////////////

wxIMPLEMENT_DYNAMIC_CLASS(CObjectListValue::CObjectListValueReturnLine, IValueTable::IValueTableReturnLine);

CObjectListValue::CObjectListValueReturnLine::CObjectListValueReturnLine() : IValueTableReturnLine(), m_methods(NULL), m_ownerTable(NULL), m_lineTable(wxNOT_FOUND) {}
CObjectListValue::CObjectListValueReturnLine::CObjectListValueReturnLine(CObjectListValue *ownerTable, int line) : IValueTableReturnLine(), m_methods(new CMethods()), m_ownerTable(ownerTable), m_lineTable(line) {}
CObjectListValue::CObjectListValueReturnLine::~CObjectListValueReturnLine() { if (m_methods) delete m_methods; }

void CObjectListValue::CObjectListValueReturnLine::PrepareNames() const
{
	std::vector<SEng> aAttributes;

	//for (auto &colInfo : m_ownerTable->m_aColumns->m_aColumnInfo)
	//{
	//	CValueTableColumn::CValueTableColumnInfo *m_columnInfo = colInfo.second;
	//	wxASSERT(m_columnInfo);

	//	SEng aAttribute;

	//	aAttribute.sName = m_columnInfo->GetColumnName();
	//	aAttribute.sAlias = wxT("default");
	//	aAttribute.iName = colInfo.first;

	//	aAttributes.push_back(aAttribute);
	//}

	m_methods->PrepareAttributes(aAttributes.data(), aAttributes.size());
}

void CObjectListValue::CObjectListValueReturnLine::SetAttribute(CAttributeParameters &aParams, CValue &cVal)
{
	int index = m_methods->GetAttributePosition(aParams.GetAttributeIndex());

	auto itFoundedByLine = m_ownerTable->m_aTableValues.begin();
	std::advance(itFoundedByLine, m_lineTable);

	//CValueTypeDescription *m_typeDescription = m_ownerTable->m_aColumns->GetColumnType(index);
	//itFoundedByLine->insert_or_assign(index, m_typeDescription ? m_typeDescription->AdjustValue(cVal) : cVal);
}

CValue CObjectListValue::CObjectListValueReturnLine::GetAttribute(CAttributeParameters &aParams)
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

SO_VALUE_REGISTER(CObjectListValue::CObjectListValueReturnLine, "listValueRow", CObjectListValueReturnLine, TEXT2CLSID("VL_LVCR"));