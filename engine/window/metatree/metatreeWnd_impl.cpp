////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : metatree window
////////////////////////////////////////////////////////////////////////////

#include "metatreeWnd.h"
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

#define metadataName _("metadata")

#define commonModulesName _("commonModules")
#define commonFormsName _("commonForms")
#define commonTemplatesName _("commonTemplates")

#define constantsName _("constants")

#define catalogsName _("catalogs")
#define documentsName _("documents")
#define enumerationsName _("enumerations")
#define dataProcessorName _("dataProcessors")

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
#define	ICON_MODULE		309
#define	ICON_RUNMODULE	322
#define	ICON_CONFMODULE	241


//***********************************************************************
//*                         metadata                                    * 
//***********************************************************************

void CMetadataTree::ActivateItem(const wxTreeItemId &item)
{
	IMetaObject *m_currObject = GetMetaObject(item);

	if (!m_currObject)
		return;

	OpenFormMDI(m_currObject);
}

void CMetadataTree::CreateItem()
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

	//Advanced mode
	if (m_objData.m_clsid == g_metaCatalogCLSID)
		AddCatalogItem(m_newObject, hNewItem);
	else if (m_objData.m_clsid == g_metaDocumentCLSID)
		AddDocumentItem(m_newObject, hNewItem);
	else if (m_objData.m_clsid == g_metaEnumerationCLSID)
		AddEnumerationItem(m_newObject, hNewItem);
	else if (m_objData.m_clsid == g_metaDataProcessorCLSID)
		AddDataProcessorItem(m_newObject, hNewItem);

	if (m_objData.m_clsid == g_metaTableCLSID)
		SETITEMTYPE(hNewItem, g_metaAttributeCLSID, ICON_ATTRIBUTE);

	OpenFormMDI(m_newObject);

	UpdateToolbar(m_newObject, hNewItem);

	m_metatreeWnd->InvalidateBestSize();
	m_metatreeWnd->SelectItem(hNewItem);
	m_metatreeWnd->Expand(hNewItem);

	objectInspector->SelectObject(m_newObject, m_metatreeWnd->GetEventHandler());
}

void CMetadataTree::EditItem()
{
	wxTreeItemId selection = m_metatreeWnd->GetSelection();

	if (!selection.IsOk())
		return;

	IMetaObject *m_currObject = GetMetaObject(selection);

	if (!m_currObject)
		return;

	OpenFormMDI(m_currObject);
}

void CMetadataTree::RemoveItem()
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
}

void CMetadataTree::EraseItem(const wxTreeItemId &item)
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

void CMetadataTree::PropertyItem()
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

void CMetadataTree::CommandItem(unsigned int id)
{
	if (appData->GetAppMode() != eRunMode::DESIGNER_MODE)
		return;
	wxTreeItemId sel = m_metatreeWnd->GetSelection();
	IMetaObject *metaObject = GetMetaObject(sel);
	if (!metaObject)
		return;
	metaObject->ProcessCommand(id);
}

void CMetadataTree::PrepareContextMenu(wxMenu *defultMenu, const wxTreeItemId &item)
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

void CMetadataTree::UpdateToolbar(IMetaObject *obj, const wxTreeItemId &item)
{
	m_metatreeToolbar->EnableTool(ID_METATREE_NEW, item != m_metatreeWnd->GetRootItem() && !m_bReadOnly);
	m_metatreeToolbar->EnableTool(ID_METATREE_EDIT, obj != NULL && item != m_metatreeWnd->GetRootItem());
	m_metatreeToolbar->EnableTool(ID_METATREE_REMOVE, obj != NULL && item != m_metatreeWnd->GetRootItem() && !m_bReadOnly);

	m_metatreeToolbar->Refresh();
}

void CMetadataTree::OnCloseDocument(CDocument *doc)
{
	auto itFounded = std::find(m_aMetaOpenedForms.begin(), m_aMetaOpenedForms.end(), doc);

	if (itFounded != m_aMetaOpenedForms.end())
		m_aMetaOpenedForms.erase(itFounded);
}

bool CMetadataTree::OpenFormMDI(IMetaObject *obj)
{
	CDocument *m_foundedDoc = GetDocument(obj);

	//не найден в списке уже существующих
	if (m_foundedDoc == NULL)
	{
		m_foundedDoc = reportManager->OpenFormMDI(obj, m_docParent, m_bReadOnly ? wxDOC_READONLY : wxDOC_NEW);

		//Значит, подходящего шаблона не было! 
		if (m_foundedDoc)
		{
			m_aMetaOpenedForms.push_back(m_foundedDoc); m_foundedDoc->Activate();
			return true;
		}
	}
	else
	{
		m_foundedDoc->Activate();
		return true;
	}

	return false;
}

bool CMetadataTree::OpenFormMDI(IMetaObject *obj, CDocument *&foundedDoc)
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

CDocument *CMetadataTree::GetDocument(IMetaObject *obj)
{
	CDocument *m_foundedDoc = NULL;

	auto itFounded = std::find_if(m_aMetaOpenedForms.begin(), m_aMetaOpenedForms.end(), [obj](CDocument *currDoc) {return obj == currDoc->GetMetaObject(); });

	if (itFounded != m_aMetaOpenedForms.end())
		m_foundedDoc = *itFounded;

	return m_foundedDoc;
}

bool CMetadataTree::RenameMetaObject(IMetaObject *obj, const wxString &sNewName)
{
	wxTreeItemId curItem = m_metatreeWnd->GetSelection();

	if (!curItem.IsOk())
		return false;

	if (m_metaData->RenameMetaObject(obj, sNewName))
	{
		CDocument *m_currDocument = GetDocument(obj);

		if (m_currDocument)
		{
			m_currDocument->SetTitle(obj->GetClassName() + wxT(": ") + sNewName);
			m_currDocument->OnChangeFilename(true);
		}

		m_metatreeWnd->SetItemText(curItem, sNewName);
		return true;
	}

	return false;
}

#include "common/codeproc.h"
#include "metadata/objects/baseObject.h"
#include "metadata/metaObjects/tables/tables.h"

void CMetadataTree::AddCatalogItem(IMetaObject *metaObj, const wxTreeItemId &hParentID)
{
	IMetaObjectValue *metaObjValue = dynamic_cast<IMetaObjectValue *>(metaObj);
	wxASSERT(metaObjValue);

	//Список аттрибутов 
	wxTreeItemId hAttributes = m_metatreeWnd->AppendItem(hParentID, objectAttributesName, ICON_ATTRIBUTEGROUP, ICON_ATTRIBUTEGROUP);
	SETITEMTYPE(hAttributes, g_metaAttributeCLSID, ICON_ATTRIBUTE);

	for (auto metaAttribute : metaObjValue->GetObjectAttributes())
	{
		if (metaAttribute->IsDefaultAttribute())
			continue;

		wxTreeItemId hItem = m_metatreeWnd->AppendItem(hAttributes, metaAttribute->GetName(), ICON_ATTRIBUTE, ICON_ATTRIBUTE);
		m_aMetaObj.insert_or_assign(hItem, metaAttribute);
	}

	//список табличных частей 
	wxTreeItemId hTables = m_metatreeWnd->AppendItem(hParentID, objectTablesName, 217, 217);
	SETITEMTYPE(hTables, g_metaTableCLSID, 218);

	for (auto metaTable : metaObjValue->GetObjectTables())
	{
		wxTreeItemId hItem = m_metatreeWnd->AppendItem(hTables, metaTable->GetName(), 218, 218);
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
	wxTreeItemId hForm = m_metatreeWnd->AppendItem(hParentID, objectFormsName, ICON_FORMGROUP, ICON_FORMGROUP);
	SETITEMTYPE(hForm, g_metaFormCLSID, ICON_FORM);

	for (auto metaForm : metaObjValue->GetObjectForms())
	{
		wxTreeItemId hItem = m_metatreeWnd->AppendItem(hForm, metaForm->GetName(), ICON_FORM, ICON_FORM);
		m_aMetaObj.insert_or_assign(hItem, metaForm);
	}

	//Таблицы
	wxTreeItemId hTemplates = m_metatreeWnd->AppendItem(hParentID, objectTablesName, ICON_MAKETGROUP, ICON_MAKETGROUP);
	SETITEMTYPE(hTemplates, g_metaTemplateCLSID, ICON_MAKET);

	for (auto metaTemplates : metaObjValue->GetObjectTemplates())
	{
		wxTreeItemId hItem = m_metatreeWnd->AppendItem(hTemplates, metaTemplates->GetName(), ICON_MAKET, ICON_MAKET);
		m_aMetaObj.insert_or_assign(hItem, metaTemplates);
	}
}

void CMetadataTree::AddDocumentItem(IMetaObject *metaObj, const wxTreeItemId &hParentID)
{
	IMetaObjectValue *metaObjValue = dynamic_cast<IMetaObjectValue *>(metaObj);
	wxASSERT(metaObjValue);

	//Список аттрибутов 
	wxTreeItemId hAttributes = m_metatreeWnd->AppendItem(hParentID, objectAttributesName, ICON_ATTRIBUTEGROUP, ICON_ATTRIBUTEGROUP);
	SETITEMTYPE(hAttributes, g_metaAttributeCLSID, ICON_ATTRIBUTE);

	for (auto metaAttribute : metaObjValue->GetObjectAttributes())
	{
		if (metaAttribute->IsDefaultAttribute())
			continue;

		wxTreeItemId hItem = m_metatreeWnd->AppendItem(hAttributes, metaAttribute->GetName(), ICON_ATTRIBUTE, ICON_ATTRIBUTE);
		m_aMetaObj.insert_or_assign(hItem, metaAttribute);
	}

	//список табличных частей 
	wxTreeItemId hTables = m_metatreeWnd->AppendItem(hParentID, objectTablesName, 217, 217);
	SETITEMTYPE(hTables, g_metaTableCLSID, 218);

	for (auto metaTable : metaObjValue->GetObjectTables())
	{
		wxTreeItemId hItem = m_metatreeWnd->AppendItem(hTables, metaTable->GetName(), 218, 218);
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
	wxTreeItemId hForm = m_metatreeWnd->AppendItem(hParentID, objectFormsName, ICON_FORMGROUP, ICON_FORMGROUP);
	SETITEMTYPE(hForm, g_metaFormCLSID, ICON_FORM);

	for (auto metaForm : metaObjValue->GetObjectForms())
	{
		wxTreeItemId hItem = m_metatreeWnd->AppendItem(hForm, metaForm->GetName(), ICON_FORM, ICON_FORM);
		m_aMetaObj.insert_or_assign(hItem, metaForm);
	}

	//Таблицы
	wxTreeItemId hTemplates = m_metatreeWnd->AppendItem(hParentID, objectTablesName, ICON_MAKETGROUP, ICON_MAKETGROUP);
	SETITEMTYPE(hTemplates, g_metaTemplateCLSID, ICON_MAKET);

	for (auto metaTemplates : metaObjValue->GetObjectTemplates())
	{
		wxTreeItemId hItem = m_metatreeWnd->AppendItem(hTemplates, metaTemplates->GetName(), ICON_MAKET, ICON_MAKET);
		m_aMetaObj.insert_or_assign(hItem, metaTemplates);
	}
}

void CMetadataTree::AddEnumerationItem(IMetaObject *metaObj, const wxTreeItemId &hParentID)
{
	IMetaObjectValue *metaObjValue = dynamic_cast<IMetaObjectValue *>(metaObj);
	wxASSERT(metaObjValue);

	//Enumerations
	wxTreeItemId hEnums = m_metatreeWnd->AppendItem(hParentID, objectEnumerationsName, 215, 215);
	SETITEMTYPE(hEnums, g_metaEnumCLSID, 215);

	for (auto metaEnumerations : metaObjValue->GetObjectEnums())
	{
		wxTreeItemId hItem = m_metatreeWnd->AppendItem(hEnums, metaEnumerations->GetName(), 215, 215);
		m_aMetaObj.insert_or_assign(hItem, metaEnumerations);
	}

	//Формы
	wxTreeItemId hForm = m_metatreeWnd->AppendItem(hParentID, objectFormsName, ICON_FORMGROUP, ICON_FORMGROUP);
	SETITEMTYPE(hForm, g_metaFormCLSID, ICON_FORM);

	for (auto metaForm : metaObjValue->GetObjectForms())
	{
		wxTreeItemId hItem = m_metatreeWnd->AppendItem(hForm, metaForm->GetName(), ICON_FORM, ICON_FORM);
		m_aMetaObj.insert_or_assign(hItem, metaForm);
	}

	//Таблицы
	wxTreeItemId hTemplates = m_metatreeWnd->AppendItem(hParentID, objectTablesName, ICON_MAKETGROUP, ICON_MAKETGROUP);
	SETITEMTYPE(hTemplates, g_metaTemplateCLSID, ICON_MAKET);

	for (auto metaTemplates : metaObjValue->GetObjectTemplates())
	{
		wxTreeItemId hItem = m_metatreeWnd->AppendItem(hTemplates, metaTemplates->GetName(), ICON_MAKET, ICON_MAKET);
		m_aMetaObj.insert_or_assign(hItem, metaTemplates);
	}
}

void CMetadataTree::AddDataProcessorItem(IMetaObject *metaObj, const wxTreeItemId &hParentID)
{
	IMetaObjectValue *metaObjValue = dynamic_cast<IMetaObjectValue *>(metaObj);
	wxASSERT(metaObjValue);

	//Список аттрибутов 
	wxTreeItemId hAttributes = m_metatreeWnd->AppendItem(hParentID, objectAttributesName, ICON_ATTRIBUTEGROUP, ICON_ATTRIBUTEGROUP);
	SETITEMTYPE(hAttributes, g_metaAttributeCLSID, ICON_ATTRIBUTE);

	for (auto metaAttribute : metaObjValue->GetObjectAttributes())
	{
		if (metaAttribute->IsDefaultAttribute())
			continue;

		wxTreeItemId hItem = m_metatreeWnd->AppendItem(hAttributes, metaAttribute->GetName(), ICON_ATTRIBUTE, ICON_ATTRIBUTE);
		m_aMetaObj.insert_or_assign(hItem, metaAttribute);
	}

	//список табличных частей 
	wxTreeItemId hTables = m_metatreeWnd->AppendItem(hParentID, objectTablesName, 217, 217);
	SETITEMTYPE(hTables, g_metaTableCLSID, 218);

	for (auto metaTable : metaObjValue->GetObjectTables())
	{
		wxTreeItemId hItem = m_metatreeWnd->AppendItem(hTables, metaTable->GetName(), 218, 218);
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
	wxTreeItemId hForm = m_metatreeWnd->AppendItem(hParentID, objectFormsName, ICON_FORMGROUP, ICON_FORMGROUP);
	SETITEMTYPE(hForm, g_metaFormCLSID, ICON_FORM);

	for (auto metaForm : metaObjValue->GetObjectForms())
	{
		wxTreeItemId hItem = m_metatreeWnd->AppendItem(hForm, metaForm->GetName(), ICON_FORM, ICON_FORM);
		m_aMetaObj.insert_or_assign(hItem, metaForm);
	}

	//Таблицы
	wxTreeItemId hTemplates = m_metatreeWnd->AppendItem(hParentID, objectTablesName, ICON_MAKETGROUP, ICON_MAKETGROUP);
	SETITEMTYPE(hTemplates, g_metaTemplateCLSID, ICON_MAKET);

	for (auto metaTemplates : metaObjValue->GetObjectTemplates())
	{
		wxTreeItemId hItem = m_metatreeWnd->AppendItem(hTemplates, metaTemplates->GetName(), ICON_MAKET, ICON_MAKET);
		m_aMetaObj.insert_or_assign(hItem, metaTemplates);
	}
}

void CMetadataTree::EditModule(const wxString &fullName, int lineNumber, bool setRunLine)
{
	IMetaObject *metaObject = m_metaData->FindByName(fullName);

	if (!metaObject)
		return;

	if (m_bReadOnly)
		return;

	CDocument *m_foundedDoc = NULL;

	if (!OpenFormMDI(metaObject, m_foundedDoc))
		return;

	ICodeInfo *m_codeInfo = dynamic_cast<ICodeInfo *>(m_foundedDoc);

	if (m_codeInfo)
		m_codeInfo->SetCurrentLine(lineNumber, setRunLine);
}

void CMetadataTree::InitTree()
{
	//*****************************************************************************************************
	//*                                      Common objects                                               *
	//*****************************************************************************************************

	m_treeMETADATA = m_metatreeWnd->AddRoot(wxT("configuration"), 225, 225, 0);
	SETITEMTYPE(m_treeMETADATA, g_metaCommonMetadataCLSID, ICON_METADATA);

	m_treeMODULES = m_metatreeWnd->AppendItem(m_treeMETADATA, commonModulesName, ICON_MODULEGROUP, ICON_MODULEGROUP);
	SETITEMTYPE(m_treeMODULES, g_metaCommonModuleCLSID, ICON_MODULE);

	m_treeFORMS = m_metatreeWnd->AppendItem(m_treeMETADATA, commonFormsName, ICON_FORMGROUP, ICON_FORMGROUP);
	SETITEMTYPE(m_treeFORMS, g_metaCommonFormCLSID, ICON_FORM);

	m_treeTEMPLATES = m_metatreeWnd->AppendItem(m_treeMETADATA, commonTemplatesName, ICON_MAKETGROUP, ICON_MAKETGROUP);
	SETITEMTYPE(m_treeTEMPLATES, g_metaCommonTemplateCLSID, ICON_MAKET);

	m_treeCONSTANTS = m_metatreeWnd->AppendItem(m_treeMETADATA, constantsName, 86, 86);
	SETITEMTYPE(m_treeCONSTANTS, g_metaConstantCLSID, ICON_ATTRIBUTE);

	//*****************************************************************************************************
	//*                                      Custom objects                                               *
	//*****************************************************************************************************

	m_treeCATALOGS = m_metatreeWnd->AppendItem(m_treeMETADATA, catalogsName, 85, 85);
	SETITEMTYPE(m_treeCATALOGS, g_metaCatalogCLSID, 226);

	m_treeDOCUMENTS = m_metatreeWnd->AppendItem(m_treeMETADATA, documentsName, 171, 171);
	SETITEMTYPE(m_treeDOCUMENTS, g_metaDocumentCLSID, 216);

	m_treeENUMERATIONS = m_metatreeWnd->AppendItem(m_treeMETADATA, enumerationsName, 599, 599);
	SETITEMTYPE(m_treeENUMERATIONS, g_metaEnumerationCLSID, 600);

	m_treeDATAPROCESSORS = m_metatreeWnd->AppendItem(m_treeMETADATA, dataProcessorName, 88, 88);
	SETITEMTYPE(m_treeDATAPROCESSORS, g_metaDataProcessorCLSID, 598);

	//Set item bold and name
	m_metatreeWnd->SetItemText(m_treeMETADATA, wxT("configuration"));
	m_metatreeWnd->SetItemBold(m_treeMETADATA);
}

void CMetadataTree::ClearTree()
{
	for (auto doc : m_aMetaOpenedForms) {
		doc->DeleteAllViews();
	}

	m_aMetaObj.clear();
	m_aMetaClassObj.clear();

	//*****************************************************************************************************
	//*                                      Common objects                                               *
	//*****************************************************************************************************

	if (m_treeMODULES.IsOk()) m_metatreeWnd->DeleteChildren(m_treeMODULES);
	if (m_treeFORMS.IsOk()) m_metatreeWnd->DeleteChildren(m_treeFORMS);
	if (m_treeTEMPLATES.IsOk()) m_metatreeWnd->DeleteChildren(m_treeTEMPLATES);
	if (m_treeCONSTANTS.IsOk()) m_metatreeWnd->DeleteChildren(m_treeCONSTANTS);

	//*****************************************************************************************************
	//*                                      Custom objects                                               *
	//*****************************************************************************************************

	if (m_treeCATALOGS.IsOk()) m_metatreeWnd->DeleteChildren(m_treeCATALOGS);
	if (m_treeDOCUMENTS.IsOk()) m_metatreeWnd->DeleteChildren(m_treeDOCUMENTS);
	if (m_treeENUMERATIONS.IsOk()) m_metatreeWnd->DeleteChildren(m_treeENUMERATIONS);
	if (m_treeDATAPROCESSORS.IsOk()) m_metatreeWnd->DeleteChildren(m_treeDATAPROCESSORS);

	//delete all items
	m_metatreeWnd->DeleteAllItems();

	//Initialize tree
	InitTree();
}

void CMetadataTree::FillData()
{
	IMetaObject *m_commonMetadata = m_metaData->GetCommonMetaObject();
	wxASSERT(m_commonMetadata);
	m_metatreeWnd->SetItemText(m_treeMETADATA, m_metaData->GetMetadataName());

	//set parent object
	m_aMetaObj.insert_or_assign(m_treeMETADATA, m_commonMetadata);

	//****************************************************************
	//*                          CommonModules                       *
	//****************************************************************
	for (auto metaModule : m_metaData->GetMetaObjects(g_metaCommonModuleCLSID))
	{
		wxTreeItemId hItem = m_metatreeWnd->AppendItem(m_treeMODULES, metaModule->GetName(), ICON_MODULE, ICON_MODULE);
		m_aMetaObj.insert_or_assign(hItem, metaModule);
	}

	//****************************************************************
	//*                          CommonForms                         *
	//****************************************************************
	for (auto metaForm : m_metaData->GetMetaObjects(g_metaCommonFormCLSID))
	{
		wxTreeItemId hItem = m_metatreeWnd->AppendItem(m_treeFORMS, metaForm->GetName(), ICON_FORM, ICON_FORM);
		m_aMetaObj.insert_or_assign(hItem, metaForm);
	}

	//****************************************************************
	//*                          CommonMakets                        *
	//****************************************************************
	for (auto metaTemplate : m_metaData->GetMetaObjects(g_metaCommonTemplateCLSID))
	{
		wxTreeItemId hItem = m_metatreeWnd->AppendItem(m_treeTEMPLATES, metaTemplate->GetName(), ICON_MAKET, ICON_MAKET);
		m_aMetaObj.insert_or_assign(hItem, metaTemplate);
	}

	//****************************************************************
	//*                          Constants                           *
	//****************************************************************
	for (auto metaConstant : m_metaData->GetMetaObjects(g_metaConstantCLSID))
	{
		wxTreeItemId hItem = m_metatreeWnd->AppendItem(m_treeCONSTANTS, metaConstant->GetName(), ICON_ATTRIBUTE, ICON_ATTRIBUTE);
		m_aMetaObj.insert_or_assign(hItem, metaConstant);
	}

	//****************************************************************
	//*                        Catalogs                              *
	//****************************************************************
	for (auto catalog : m_metaData->GetMetaObjects(g_metaCatalogCLSID))
	{
		wxTreeItemId hItem = m_metatreeWnd->AppendItem(m_treeCATALOGS, catalog->GetName(), 226, 226);
		AddCatalogItem(catalog, hItem);
		m_aMetaObj.insert_or_assign(hItem, catalog);
	}

	//****************************************************************
	//*                        Documents                             *
	//****************************************************************
	for (auto document : m_metaData->GetMetaObjects(g_metaDocumentCLSID))
	{
		wxTreeItemId hItem = m_metatreeWnd->AppendItem(m_treeDOCUMENTS, document->GetName(), 216, 216);
		AddDocumentItem(document, hItem);
		m_aMetaObj.insert_or_assign(hItem, document);
	}

	//****************************************************************
	//*                          Enumerations                        *
	//****************************************************************
	for (auto enumeration : m_metaData->GetMetaObjects(g_metaEnumerationCLSID))
	{
		wxTreeItemId hItem = m_metatreeWnd->AppendItem(m_treeENUMERATIONS, enumeration->GetName(), 600, 600);
		AddEnumerationItem(enumeration, hItem);
		m_aMetaObj.insert_or_assign(hItem, enumeration);
	}

	//****************************************************************
	//*                          Data processor                      *
	//****************************************************************
	for (auto dataProcessor : m_metaData->GetMetaObjects(g_metaDataProcessorCLSID))
	{
		wxTreeItemId hItem = m_metatreeWnd->AppendItem(m_treeDATAPROCESSORS, dataProcessor->GetName(), 598, 598);
		AddDataProcessorItem(dataProcessor, hItem);
		m_aMetaObj.insert_or_assign(hItem, dataProcessor);
	}

	//update toolbar 
	UpdateToolbar(m_commonMetadata, m_treeMETADATA);
}

bool CMetadataTree::Load(CMetadata *metadataObj)
{
	ClearTree();
	m_metaData = metadataObj ? metadataObj : CMetadata::Get();
	m_metatreeWnd->Freeze();
	FillData(); //Fill all data from metadata
	m_metaData->SetMetaTree(this);
	m_metatreeWnd->SelectItem(m_treeMETADATA);
	m_metatreeWnd->Expand(m_treeMETADATA);
	m_metatreeWnd->Thaw();
	return true;
}

bool CMetadataTree::Save()
{
	wxASSERT(m_metaData);

	if (m_metaData->IsModified() && wxMessageBox("Configuration '" + m_metaData->GetMetadataName() + "' has been changed. Save?", wxT("Save project"), wxYES_NO | wxCENTRE | wxICON_QUESTION, this) == wxYES)
		return m_metaData->SaveMetadata();

	return false;
}