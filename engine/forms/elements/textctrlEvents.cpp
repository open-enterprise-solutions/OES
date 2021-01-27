#include "widgets.h"
#include "frame.h"
#include "metadata/objects/baseObject.h"

#include <wx/popupwin.h>

void CValueTextCtrl::OnTextEnter(wxCommandEvent &event)
{
	wxTextCtrl *m_textctrl = dynamic_cast<wxTextCtrl *>(event.GetEventObject());

	IMetaObjectValue *m_metaObject = IMetaObjectValue::FindMetaObjectValue(m_type);
	if (!m_metaObject)
	{
		CValueFrame *m_ownerFrame = GetOwnerForm();
		IDataObjectSource *m_objData = m_ownerFrame->GetSourceObject();
		
		if (m_objData)
		{
			IMetaObjectValue *m_objMetaValue = m_objData->GetMetaObject();
			IMetaObject *m_objMeta = m_objMetaValue->FindMetaObjectByID(m_source);
			
			if (m_objData) {
				CAttributeParameters aParams(m_objMeta->FindAttribute(m_objMeta->GetName()), m_objMeta->GetName());
				m_objData->SetAttribute(aParams, CValue(m_textctrl->GetValue()));
			}

			m_ownerFrame->Modify(true);
		}
	}

	event.Skip();
}
