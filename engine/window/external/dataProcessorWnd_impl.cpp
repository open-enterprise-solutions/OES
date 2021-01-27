////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : dataprocessor window
////////////////////////////////////////////////////////////////////////////

#include "dataProcessorWnd.h"
#include "window/property/objinspect.h"
#include "common/reportManager.h"
#include "appData.h"

#define SETITEMTYPE(hItem,xnType,xnChildImage)\
{\
CObjectData m_dataObj;\
m_dataObj.m_clsid=xnType;\
m_dataObj.m_nChildImage=xnChildImage;\
m_aMetaClassObj[hItem]=m_dataObj;\
}

#define	objectFormsName _("forms")
#define	objectModulesName _("modules")
#define	objectTemplatesName _("templates")
#define objectAttributesName _("attributes")
#define objectTablesName _("tables")
#define objectEnumerationsName _("enums")

#define	ICON_ATTRIBUTEGROUP	219
#define	ICON_ATTRIBUTE	219
#define	ICON_OBJECTGROUP 268//134
#define	ICON_OBJECT		270//134
#define	ICON_METADATA	209
#define	ICON_MAKETGROUP	318
#define	ICON_MAKET		79
#define	ICON_FORMGROUP	293
#define	ICON_FORM		294
#define	ICON_MODULEGROUP 317
#define	ICON_MODULE		308
#define	ICON_RUNMODULE	322
#define	ICON_CONFMODULE	241
#define	ICON_INITMODULE	309

//***********************************************************************
//*                         metadata                                    * 
//***********************************************************************

void CDataProcessorTree::ActivateItem(const wxTreeItemId &item)
{
	IMetaObject *m_currObject = GetMetaObject(item);

	if (!m_currObject)
		return;

	OpenFormMDI(m_currObject);
}

void CDataProcessorTree::CreateItem()
{
	IMetaObject *m_metaParent = NULL;

	wxTreeItemId hSelItem = m_metatreeWnd->GetSelection();
	wxTreeItemId hParentItem = hSelItem;

	if (!hSelItem.IsOk())
		return;

	CObjectData m_objData; bool m_dataFounded = false;

	while (hParentItem)
	{
		auto foundedIt = m_aMetaClassObj.find(hParentItem);

		if (foundedIt != m_aMetaClassObj.end())
		{
			hSelItem = foundedIt->first; m_objData = foundedIt->second;
			m_dataFounded = true;
			break;
		}

		hParentItem = m_metatreeWnd->GetItemParent(hParentItem);
	}

	if (!m_dataFounded)
		return;

	while (hParentItem)
	{
		auto foundedIt = m_aMetaObj.find(hParentItem);

		if (foundedIt != m_aMetaObj.end())
		{
			m_metaParent = foundedIt->second;
			break;
		}

		hParentItem = m_metatreeWnd->GetItemParent(hParentItem);
	}

	wxASSERT(m_metaParent);
	IMetaObject *m_newObject = m_metaData->CreateMetaObject(m_objData.m_clsid, m_metaParent);
	wxASSERT(m_newObject);
	wxTreeItemId hNewItem = m_metatreeWnd->AppendItem(hSelItem, m_newObject->GetName(), m_objData.m_nChildImage, m_objData.m_nChildImage);
	m_aMetaObj.insert_or_assign(hNewItem, m_newObject);

	if (m_objData.m_clsid == g_metaTableCLSID)
		SETITEMTYPE(hNewItem, g_metaAttributeCLSID, ICON_ATTRIBUTE);

	OpenFormMDI(m_newObject);

	//update choice if need
	UpdateChoiceSelection();

	//update toolbar
	UpdateToolbar(m_newObject, hNewItem);

	m_metatreeWnd->InvalidateBestSize();
	m_metatreeWnd->SelectItem(hNewItem);
	m_metatreeWnd->Expand(hNewItem);

	objectInspector->SelectObject(m_newObject, m_metatreeWnd->GetEventHandler());
}

void CDataProcessorTree::EditItem()
{
	wxTreeItemId selection = m_metatreeWnd->GetSelection();

	if (!selection.IsOk())
		return;

	IMetaObject *m_currObject = GetMetaObject(selection);

	if (!m_currObject)
		return;

	OpenFormMDI(m_currObject);
}

void CDataProcessorTree::RemoveItem()
{
	wxTreeItemId selection = m_metatreeWnd->GetSelection();

	if (!selection.IsOk())
		return;

	wxTreeItemIdValue m_cookie;
	wxTreeItemId hItem = m_metatreeWnd->GetFirstChild(selection, m_cookie);

	while (hItem)
	{
		EraseItem(hItem);
		hItem = m_metatreeWnd->GetNextChild(hItem, m_cookie);
	}

	IMetaObject *metaObject = GetMetaObject(selection);
	wxASSERT(metaObject);
	EraseItem(selection);
	m_metaData->RemoveMetaObject(metaObject);

	//Delete item from tree
	m_metatreeWnd->Delete(selection);

	//update choice if need
	UpdateChoiceSelection();
}

void CDataProcessorTree::EraseItem(const wxTreeItemId &item)
{
	IMetaObject *metaObject = GetMetaObject(item);

	if (metaObject)
	{
		auto itFounded = std::find_if(m_aMetaOpenedForms.begin(), m_aMetaOpenedForms.end(), [metaObject](CDocument *currDoc) { return metaObject == currDoc->GetMetaObject(); });

		if (itFounded != m_aMetaOpenedForms.end())
		{
			CDocument *m_foundedDoc = *itFounded;
			m_aMetaOpenedForms.erase(itFounded);
			m_foundedDoc->DeleteAllViews();
		}
	}

	m_aMetaClassObj.erase(item);
	m_aMetaObj.erase(item);
}

void CDataProcessorTree::PropertyItem()
{
	if (appData->GetAppMode() != eRunMode::DESIGNER_MODE)
		return;
	wxTreeItemId sel = m_metatreeWnd->GetSelection();
	objectInspector->ClearProperty();

	IMetaObject *metaObject = GetMetaObject(sel);

	UpdateToolbar(metaObject, sel);

	if (!metaObject)
		return;

	objectInspector->SelectObject(metaObject, m_metatreeWnd->GetEventHandler());
}

void CDataProcessorTree::CommandItem(unsigned int id)
{
	if (appData->GetAppMode() != eRunMode::DESIGNER_MODE)
		return;
	wxTreeItemId sel = m_metatreeWnd->GetSelection();
	IMetaObject *metaObject = GetMetaObject(sel);
	if (!metaObject)
		return;
	metaObject->ProcessCommand(id);
}

void CDataProcessorTree::PrepareContextMenu(wxMenu *defultMenu, const wxTreeItemId &item)
{
	IMetaObject *metaObject = GetMetaObject(item);

	if (metaObject
		&& !metaObject->PrepareContextMenu(defultMenu))
	{
		wxMenuItem *m_menuItem = defultMenu->Append(ID_METATREE_NEW, _("new"));
		m_menuItem->SetBitmap(wxGetImageBMPFromResource(IDB_EDIT_NEW));
		m_menuItem->Enable(!m_bReadOnly);
		m_menuItem = defultMenu->Append(ID_METATREE_EDIT, _("edit"));
		m_menuItem->SetBitmap(wxGetImageBMPFromResource(IDB_EDIT));
		m_menuItem = defultMenu->Append(ID_METATREE_REMOVE, _("remove"));
		m_menuItem->SetBitmap(wxGetImageBMPFromResource(IDB_EDIT_CUT));
		m_menuItem->Enable(!m_bReadOnly);
		defultMenu->AppendSeparator();
		m_menuItem = defultMenu->Append(ID_METATREE_PROPERTY, _("property"));
	}
	else if (!metaObject)
	{
		wxMenuItem *m_menuItem = defultMenu->Append(ID_METATREE_NEW, _("new"));
		m_menuItem->SetBitmap(wxGetImageBMPFromResource(IDB_EDIT_NEW));
		m_menuItem->Enable(!m_bReadOnly);
	}
}

void CDataProcessorTree::UpdateToolbar(IMetaObject *obj, const wxTreeItemId &item)
{
	m_metatreeToolbar->EnableTool(ID_METATREE_NEW, item != m_metatreeWnd->GetRootItem() && !m_bReadOnly);
	m_metatreeToolbar->EnableTool(ID_METATREE_EDIT, obj != NULL && item != m_metatreeWnd->GetRootItem());
	m_metatreeToolbar->EnableTool(ID_METATREE_REMOVE, obj != NULL && item != m_metatreeWnd->GetRootItem() && !m_bReadOnly);

	m_metatreeToolbar->Refresh();
}

void CDataProcessorTree::UpdateChoiceSelection()
{
	m_defaultFormValue->Clear();
	m_defaultFormValue->AppendString(_("<not selected>"));

	CMetaObjectDataProcessorValue *commonMetadata = m_metaData->GetDataProcessor();
	wxASSERT(commonMetadata);

	int defSelection = 0;

	for (auto metaForm : commonMetadata->GetObjectForms())
	{
		if (CMetaObjectDataProcessorValue::eFormDataProcessor != metaForm->GetTypeForm())
			continue;

		int selection_id = m_defaultFormValue->Append(metaForm->GetName(), reinterpret_cast<void *>(metaForm->GetMetaID()));

		if (commonMetadata->m_defaultFormObject == metaForm->GetMetaID()) {
			defSelection = selection_id;
		}
	}

	m_defaultFormValue->SetSelection(defSelection);
	m_defaultFormValue->SendSelectionChangedEvent(wxEVT_CHOICE);
}

void CDataProcessorTree::OnCloseDocument(CDocument *doc)
{
	auto itFounded = std::find(m_aMetaOpenedForms.begin(), m_aMetaOpenedForms.end(), doc);

	if (itFounded != m_aMetaOpenedForms.end())
		m_aMetaOpenedForms.erase(itFounded);
}

bool CDataProcessorTree::OpenFormMDI(IMetaObject *obj)
{
	CDocument *foundedDoc = GetDocument(obj);

	//не найден в списке уже существующих
	if (foundedDoc == NULL)
	{
		foundedDoc = reportManager->OpenFormMDI(obj, m_docParent, m_bReadOnly ? wxDOC_READONLY : wxDOC_NEW);

		//Значит, подходящего шаблона не было! 
		if (foundedDoc)
		{
			m_aMetaOpenedForms.push_back(foundedDoc); foundedDoc->Activate();
			return true;
		}
	}
	else
	{
		foundedDoc->Activate();
		return true;
	}

	return false;
}

bool CDataProcessorTree::OpenFormMDI(IMetaObject *obj, CDocument *&foundedDoc)
{
	foundedDoc = GetDocument(obj);

	//не найден в списке уже существующих
	if (foundedDoc == NULL)
	{
		foundedDoc = reportManager->OpenFormMDI(obj, m_docParent, m_bReadOnly ? wxDOC_READONLY : wxDOC_NEW);

		//Значит, подходящего шаблона не было! 
		if (foundedDoc)
		{
			m_aMetaOpenedForms.push_back(foundedDoc); foundedDoc->Activate();
			return true;
		}
	}
	else
	{
		foundedDoc->Activate();
		return true;
	}

	return false;
}

CDocument *CDataProcessorTree::GetDocument(IMetaObject *obj)
{
	CDocument *foundedDoc = NULL;

	auto itFounded = std::find_if(m_aMetaOpenedForms.begin(), m_aMetaOpenedForms.end(), [obj](CDocument *currDoc) {return obj == currDoc->GetMetaObject(); });

	if (itFounded != m_aMetaOpenedForms.end())
		foundedDoc = *itFounded;

	return foundedDoc;
}

bool CDataProcessorTree::RenameMetaObject(IMetaObject *obj, const wxString &sNewName)
{
	wxTreeItemId curItem = m_metatreeWnd->GetSelection();

	if (!curItem.IsOk())
		return false;

	if (m_metaData->RenameMetaObject(obj, sNewName))
	{
		CDocument *currDocument = GetDocument(obj);

		if (currDocument)
		{
			currDocument->SetTitle(obj->GetClassName() + wxT(": ") + sNewName);
			currDocument->OnChangeFilename(true);
		}

		//update choice if need
		UpdateChoiceSelection();

		m_metatreeWnd->SetItemText(curItem, sNewName);
		return true;
	}

	return false;
}

#include "common/codeproc.h"

void CDataProcessorTree::EditModule(const wxString &fullName, int lineNumber, bool setRunLine)
{
	IMetaObject *metaObject = m_metaData->FindByName(fullName);

	if (!metaObject)
		return;

	CDocument *m_foundedDoc = NULL;

	if (!OpenFormMDI(metaObject, m_foundedDoc))
		return;

	ICodeInfo *m_codeInfo = dynamic_cast<ICodeInfo *>(m_foundedDoc);

	if (m_codeInfo)
		m_codeInfo->SetCurrentLine(lineNumber, setRunLine);
}

void CDataProcessorTree::InitTree()
{
	m_treeDATAPROCESSORS = m_metatreeWnd->AddRoot(wxT("dataProcessor"), 225, 225, 0);
	SETITEMTYPE(m_treeDATAPROCESSORS, g_metaDataProcessorCLSID, ICON_METADATA);

	//Список аттрибутов 
	m_treeATTRIBUTES = m_metatreeWnd->AppendItem(m_treeDATAPROCESSORS, objectAttributesName, ICON_ATTRIBUTEGROUP, ICON_ATTRIBUTEGROUP);
	SETITEMTYPE(m_treeATTRIBUTES, g_metaAttributeCLSID, ICON_ATTRIBUTE);

	//список табличных частей 
	m_treeTABLES = m_metatreeWnd->AppendItem(m_treeDATAPROCESSORS, objectTablesName, 217, 217);
	SETITEMTYPE(m_treeTABLES, g_metaTableCLSID, 218);

	//Формы
	m_treeFORM = m_metatreeWnd->AppendItem(m_treeDATAPROCESSORS, objectFormsName, ICON_FORMGROUP, ICON_FORMGROUP);
	SETITEMTYPE(m_treeFORM, g_metaFormCLSID, ICON_FORM);

	//Таблицы
	m_treeTEMPLATES = m_metatreeWnd->AppendItem(m_treeDATAPROCESSORS, objectTablesName, ICON_MAKETGROUP, ICON_MAKETGROUP);
	SETITEMTYPE(m_treeTEMPLATES, g_metaTemplateCLSID, ICON_MAKET);
}

void CDataProcessorTree::ClearTree()
{
	for (auto doc : m_aMetaOpenedForms) {
		doc->DeleteAllViews();
	}

	m_aMetaObj.clear();
	m_aMetaClassObj.clear();

	//delete all child item
	if (m_treeATTRIBUTES.IsOk()) m_metatreeWnd->DeleteChildren(m_treeATTRIBUTES);
	if (m_treeTABLES.IsOk()) m_metatreeWnd->DeleteChildren(m_treeTABLES);
	if (m_treeFORM.IsOk()) m_metatreeWnd->DeleteChildren(m_treeFORM);
	if (m_treeTEMPLATES.IsOk()) m_metatreeWnd->DeleteChildren(m_treeTEMPLATES);

	//delete all items
	m_metatreeWnd->DeleteAllItems();

	//Initialize tree
	InitTree();
}

void CDataProcessorTree::FillData()
{
	CMetaObjectDataProcessorValue *commonMetadata = m_metaData->GetDataProcessor();
	wxASSERT(commonMetadata);
	m_metatreeWnd->SetItemText(m_treeDATAPROCESSORS, commonMetadata->GetName());

	//set parent object
	m_aMetaObj.insert_or_assign(m_treeDATAPROCESSORS, commonMetadata);

	//Список аттрибутов 
	for (auto metaAttribute : commonMetadata->GetObjectAttributes())
	{
		if (metaAttribute->IsDefaultAttribute())
			continue;

		wxTreeItemId hItem = m_metatreeWnd->AppendItem(m_treeATTRIBUTES, metaAttribute->GetName(), ICON_ATTRIBUTE, ICON_ATTRIBUTE);
		m_aMetaObj.insert_or_assign(hItem, metaAttribute);
	}

	//Список табличных частей 
	for (auto metaTable : commonMetadata->GetObjectTables())
	{
		wxTreeItemId hItem = m_metatreeWnd->AppendItem(m_treeTABLES, metaTable->GetName(), 218, 218);
		SETITEMTYPE(hItem, g_metaAttributeCLSID, ICON_ATTRIBUTE);
		m_aMetaObj.insert_or_assign(hItem, metaTable);

		for (auto metaAttribute : metaTable->GetObjectAttributes())
		{
			if (metaAttribute->IsDefaultAttribute())
				continue;

			wxTreeItemId hItemNew = m_metatreeWnd->AppendItem(hItem, metaAttribute->GetName(), ICON_ATTRIBUTE, ICON_ATTRIBUTE);
			m_aMetaObj.insert_or_assign(hItemNew, metaAttribute);
		}
	}

	//Формы
	for (auto metaForm : commonMetadata->GetObjectForms())
	{
		wxTreeItemId hItem = m_metatreeWnd->AppendItem(m_treeFORM, metaForm->GetName(), ICON_FORM, ICON_FORM);
		m_aMetaObj.insert_or_assign(hItem, metaForm);
	}

	//Таблицы
	for (auto metaTemplates : commonMetadata->GetObjectTemplates())
	{
		wxTreeItemId hItem = m_metatreeWnd->AppendItem(m_treeTEMPLATES, metaTemplates->GetName(), ICON_MAKET, ICON_MAKET);
		m_aMetaObj.insert_or_assign(hItem, metaTemplates);
	}

	//set value data
	m_nameValue->SetValue(commonMetadata->GetName());
	m_aliasValue->SetValue(commonMetadata->GetAlias());
	m_commentValue->SetValue(commonMetadata->GetComment());

	//set default form value 
	m_defaultFormValue->Clear();

	//append default value 
	m_defaultFormValue->AppendString("<not selected>");

	//update choice selection
	UpdateChoiceSelection(); 

	//update toolbar 
	UpdateToolbar(NULL, m_treeATTRIBUTES);
}

bool CDataProcessorTree::Load(CMetadataDataProcessor *metaData)
{
	ClearTree();
	m_metaData = metaData;
	m_metatreeWnd->Freeze();
	FillData(); //Fill all data from metadata
	m_metaData->SetMetaTree(this);
	m_metatreeWnd->SelectItem(m_treeATTRIBUTES);
	m_metatreeWnd->ExpandAll();
	m_metatreeWnd->Thaw();
	return true;
}

bool CDataProcessorTree::Save()
{
	CMetaObjectDataProcessorValue *m_commonMetadata = m_metaData->GetDataProcessor();
	wxASSERT(m_commonMetadata);

	m_commonMetadata->SetName(m_nameValue->GetValue());
	m_commonMetadata->SetAlias(m_aliasValue->GetValue());
	m_commonMetadata->SetComment(m_commentValue->GetValue());

	wxASSERT(m_metaData);

	if (m_metaData->IsModified())
		return m_metaData->SaveMetadata();

	return false;
}