////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : metaform property
////////////////////////////////////////////////////////////////////////////

#include "metaFormObject.h"
#include "forms/visualEditorView.h"
#include "metadata/metadata.h"
#include "metadata/objects/baseObject.h"
#include "appData.h"

void CMetaFormObject::OnPropertyCreated(Property *property)
{
}

void CMetaFormObject::OnPropertySelected(Property *property)
{
}

void CMetaFormObject::OnPropertyChanged(Property *property)
{
	if (property->GetName() == wxT("form_type")) {
		if (appData->IsDesignerMode()) {
			
			IModuleManager *moduleManager = m_metaData->GetModuleManager();
			wxASSERT(moduleManager);
			IMetaObjectValue *metaObjectValue = wxStaticCast(m_parent, IMetaObjectValue);
			wxASSERT(metaObjectValue);

			IMetadataTree *metaTree = m_metaData->GetMetaTree();
			
			if (metaTree) {
				metaTree->CloseMetaObject(this);
			}

			if (moduleManager->RemoveCompileModule(this)) {
				moduleManager->AddCompileModule(this, metaObjectValue->CreateObjectValue(this));
			}

			if (metaTree) {
				metaTree->OnPropertyChanged();
			}
		}
	}

	m_metaData->Modify(true);
}