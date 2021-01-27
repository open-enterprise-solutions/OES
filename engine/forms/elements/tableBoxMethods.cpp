#include "tableBox.h"
#include "frame.h"
#include "metadata/objects/baseObject.h"
#include "compiler/methods.h"

inline bool CompareString(const wxString &lhs, const wxString &rhs)
{
	return lhs.CompareTo(rhs, wxString::ignoreCase) == 0;
}

void CValueTableBox::PrepareNames() const
{
	IControlElement::PrepareNames();

	m_methods->AppendAttribute(wxT("tableValue"));
	m_methods->AppendAttribute(wxT("currentRow"));
}

void CValueTableBox::SetAttribute(CAttributeParameters &aParams, CValue &cVal)
{
	if (CompareString(aParams.GetAttributeName(), wxT("tableValue")))
	{
		if (m_tableModel)
			m_tableModel->DecrRef();

		m_tableModel = cVal.ConvertToType<IValueTable *>();
		m_tableModel->IncrRef();
	}
	else if (CompareString(aParams.GetAttributeName(), wxT("currentRow")))
	{
		if (m_tableCurrentLine)
			m_tableCurrentLine->DecrRef();

		m_tableCurrentLine = NULL;

		IValueTable::IValueTableReturnLine *m_tableReturnLine = NULL;

		if (cVal.ConvertToValue(m_tableReturnLine))
		{
			if (m_tableModel == m_tableReturnLine->GetOwnerTable())
			{
				m_tableCurrentLine = m_tableReturnLine;
				m_tableCurrentLine->IncrRef();
			}
		}
	}

	IControlElement::SetAttribute(aParams, cVal);
}

CValue CValueTableBox::GetAttribute(CAttributeParameters &aParams)
{
	if (CompareString(aParams.GetAttributeName(), wxT("tableValue")))
	{
		return m_tableModel;
	}
	else if (CompareString(aParams.GetAttributeName(), wxT("currentRow")))
	{
		return m_tableCurrentLine;
	}

	return IControlElement::GetAttribute(aParams);
}

////////////////////////////////////////////////////////////////////////

void CValueTableBox::AddValue()
{
	CValueFrame *m_valueFrame = GetOwnerForm();
	if (m_valueFrame)
	{
		IMetaObjectValue *m_valueMeta = m_valueFrame->GetMetaObject();
		IDataObjectValue *m_objvalue = m_valueMeta->CreateObjectValue();
		m_objvalue->ShowValue();
	}

	//m_table->Refresh();
}

void CValueTableBox::CopyValue()
{
	//m_table->Refresh();
}

void CValueTableBox::EditValue()
{
	wxDataViewCtrl *m_table = dynamic_cast<wxDataViewCtrl *>(GetWxObject());

	// event is a wxDataViewEvent
	wxDataViewItem item = m_table->GetSelection();

	if (!item.IsOk()) return;

	// mpDataView is a wwxDataViewListCtrl*
	/*CTableBoxModel* model = (CTableBoxModel *)m_table->GetModel();
	int row = model->GetRow(item);

	CValue guid = model->GetData().GetRow(row, 0);

	CValueFrame *m_valueFrame = GetOwnerForm();
	if (m_valueFrame)
	{
		IMetaObjectValue *m_valueMeta = m_valueFrame->GetMetaObject();
		CValueReference *m_valueReference = m_valueMeta->FindObjectValue(guid.GetString());
		m_valueReference->ShowValue();
	}
	*/
	//m_table->Refresh();
}

void CValueTableBox::DeleteValue()
{
	//m_table->Refresh();
}