#include "widgets.h"
#include "frame.h"
#include "metadata/metadata.h"
#include "metadata/objects/baseObject.h"

//*******************************************************************
//*                            Owner support                        *
//*******************************************************************

void CValueTextCtrl::SendOwnerValue(CValue &cVal)
{
	CValueFrame *m_ownerFrame = GetOwnerForm();

	IDataObjectSource *m_objData = m_ownerFrame->GetSourceObject();
	IMetaObjectValue *m_objMetaValue = m_objData->GetMetaObject();

	IMetaObject *m_objMeta = m_objMetaValue->FindMetaObjectByID(m_source);

	if (m_objData) {
		CAttributeParameters aParams(m_objMeta->FindAttribute(m_objMeta->GetName()), m_objMeta->GetName());
		m_objData->SetAttribute(aParams, cVal);
	}
	
	m_ownerFrame->Modify(true);
	Update(GetWxObject(), false);
}