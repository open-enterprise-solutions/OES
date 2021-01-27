////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : metadata 
////////////////////////////////////////////////////////////////////////////

#include "metadata.h"
#include "metadata/objects/baseManager.h"
#include "compiler/enumFactory.h"
#include "compiler/systemObjects.h"
#include "compiler/debugger/debugClient.h"
#include "compiler/debugger/debugServer.h"
#include "database/databaseLayer.h"
#include "database/databaseErrorCodes.h"
#include "appData.h"

CMetadata* CMetadata::s_instance = NULL;

CMetadata* CMetadata::Get()
{
	if (!s_instance) s_instance = new CMetadata();
	return s_instance;
}

void CMetadata::Destroy()
{
	if (appData->IsEnterpriseMode())
		debugClientDestroy();
	else if (appData->IsDesignerMode())
		debugServerDestroy();

	enumFactoryDestroy();

	wxDELETE(s_instance);
}

#define _CONFIGSAVE wxString("CONFIGSAVE")
#define _CONFIG wxString("CONFIG")
#define _COMPILE_DATA wxString("COMPILE_DATA")
#define _USERS wxString("USERS")

#include "utils/stringutils.h"

//**************************************************************************************************
//*                                          metadata                                              *
//**************************************************************************************************

CMetadata::CMetadata(bool readOnly) : IMetadata(readOnly),
m_commonObject(m_commonObject)
{
	m_sConfigPath = wxT("sys.database");
	m_sDefaultSource = _CONFIG;

	//create main metaObject
	m_commonObject = new CMetaObject();
	m_commonObject->SetMetadata(this);
	m_commonObject->SetClsid(g_metaCommonMetadataCLSID);

	//generate unique guid
	m_commonObject->GenerateGuid();

	if (m_commonObject->OnCreateMetaObject())
	{
		m_moduleManager = new CModuleManager(this, m_commonObject);
		m_moduleManager->IncrRef();

		if (!m_commonObject->OnLoadMetaObject()) {
			wxASSERT_MSG(false, "m_commonObject->OnLoadMetaObject() == false");
		}
	}

	m_commonObject->ReadProperty();
	m_commonObject->IncrRef();

	wxASSERT(m_moduleManager);
}

CMetadata::~CMetadata()
{
	//clear data 
	if (!ClearMetadata()) {
		wxASSERT_MSG(false, "ClearMetadata() == false");
	}

	//delete module manager
	wxDELETE(m_moduleManager);

	//delete common metaObject
	wxDELETE(m_commonObject);
}

bool CMetadata::CreateMetadata()
{
	if (!objectDatabase->IsOpen())
		return false;

	//db for designer 
	if (!objectDatabase->TableExists(_CONFIGSAVE))
	{
		objectDatabase->RunQuery("CREATE TABLE " + _CONFIGSAVE + "("
			"fileName VARCHAR(128) NOT NULL PRIMARY KEY,"
			"attributes INTEGER,"
			"dataSize INTEGER NOT NULL," 			//binary medatadata
			"binaryData BLOB NOT NULL);");         	//size of binary medatadata

		objectDatabase->RunQuery("CREATE INDEX " + _CONFIGSAVE + "_INDEX ON " + _CONFIGSAVE + " ("
			"fileName);");
	}

	//db for enterprise - TODO
	if (!objectDatabase->TableExists(_CONFIG))
	{
		objectDatabase->RunQuery("CREATE TABLE " + _CONFIG + "("
			"fileName VARCHAR(128) NOT NULL PRIMARY KEY,"
			"attributes INTEGER,"
			"dataSize INTEGER NOT NULL," 			//binary medatadata
			"binaryData BLOB NOT NULL);");         	//size of binary medatadata

		objectDatabase->RunQuery("CREATE INDEX " + _CONFIG + "_INDEX ON " + _CONFIG + " ("
			"fileName);");
	}

	//compile data for better performance - TODO
	if (!objectDatabase->TableExists(_COMPILE_DATA))
	{
		objectDatabase->RunQuery("CREATE TABLE " + _COMPILE_DATA + " ("
			"fileName VARCHAR(128) NOT NULL,"
			"dataSize INTEGER NOT NULL," 			//binary medatadata
			"binaryData BLOB NOT NULL);");         	//size of binary medatadata

		objectDatabase->RunQuery("CREATE INDEX " + _COMPILE_DATA + "_INDEX ON " + _COMPILE_DATA + " ("
			"fileName);");
	}

	//create users or nothin 
	if (!objectDatabase->TableExists(_USERS))
	{
		objectDatabase->RunQuery("CREATE TABLE " + _USERS + "("
			"guid              CHAR(36)   NOT NULL PRIMARY KEY,"
			"name              VARCHAR(128)  NOT NULL,"
			"dataSize          INTEGER       NOT NULL,"
			"binaryData        BLOB      NOT NULL);");

		objectDatabase->RunQuery("CREATE INDEX " + _USERS + "_INDEX ON " + _USERS + " ("
			"guid,"
			"name);");
	}

	return objectDatabase->IsOpen();
}

bool CMetadata::LoadMetadata()
{
	if (!CreateMetadata())
		return false;

	DatabaseResultSet *m_resultConfig = objectDatabase->RunQueryWithResults("SELECT binaryData, dataSize FROM %s; ", m_sDefaultSource);

	if (!m_resultConfig)
		return false;

	//load metadata from DB 
	while (m_resultConfig->Next())
	{
		wxMemoryBuffer binaryData;
		m_resultConfig->GetResultBlob("binaryData", binaryData);
		CMemoryReader m_metaReader(binaryData.GetData(), m_resultConfig->GetResultInt("dataSize"));

		//check is file empty
		if (m_metaReader.eof())
			return false;

		//check metadata 
		if (!LoadHeader(m_metaReader))
			return false;

		//load metadata 
		if (!LoadCommonMetadata(g_metaCommonMetadataCLSID, m_metaReader))
			return false;
	}

	m_resultConfig->Close();

	//Initialize debugger
	if (appData->IsEnterpriseMode())
	{
		debugClientInit();
		debugClient->WaitDebugger();
	}
	else if (appData->IsDesignerMode())
	{
		debugServerInit();
	}

	return RunMetadata();
}

bool CMetadata::SaveMetadata()
{
#if defined(_USE_SAVE_METADATA_IN_TRANSACTION)
	//begin transaction 
	objectDatabase->BeginTransaction();
#endif 

	//common data
	CMemoryWriter m_writterData;

	//Save header info 
	if (!SaveHeader(m_writterData)) {
#if defined(_USE_SAVE_METADATA_IN_TRANSACTION)
		objectDatabase->RollBack(); return false;
#else 
		return false;
#endif
	}

	//Save common object
	if (!SaveCommonMetadata(g_metaCommonMetadataCLSID, m_writterData)) {
#if defined(_USE_SAVE_METADATA_IN_TRANSACTION)
		objectDatabase->RollBack(); return false;
#else 
		return false;
#endif
	}

	//Delete common object
	if (!DeleteCommonMetadata(g_metaCommonMetadataCLSID)) {
#if defined(_USE_SAVE_METADATA_IN_TRANSACTION)
		objectDatabase->RollBack(); return false;
#else 
		return false;
#endif
	}

	PreparedStatement *prepStatement = objectDatabase->PrepareStatement("UPDATE OR INSERT INTO %s (fileName, dataSize, binaryData) VALUES(?, ?, ?) MATCHING (fileName); ", m_sDefaultSource);

	if (!prepStatement)
		return false;

	prepStatement->SetParamString(1, m_sDefaultSource);
	prepStatement->SetParamInt(2, m_writterData.size());
	prepStatement->SetParamBlob(3, m_writterData.pointer(), m_writterData.size());

	if (prepStatement->RunQuery() == DATABASE_LAYER_QUERY_RESULT_ERROR)
	{
#if defined(_USE_SAVE_METADATA_IN_TRANSACTION)
		objectDatabase->RollBack();
#endif
	}

#if defined(_USE_SAVE_METADATA_IN_TRANSACTION)
	objectDatabase->Commit();
#endif 

	Modify(false);

	return objectDatabase->CloseStatement(prepStatement);
}

bool CMetadata::RunMetadata()
{
	if (m_moduleManager->CreateMainModule())
	{
		if (!m_moduleManager->StartMainModule())
			return false;

		for (auto obj : m_aMetadataCommon)
		{
			if (obj->IsDeleted())
				continue;

			if (!obj->OnRunMetaObject())
				return false;

			if (!RunChildMetadata(obj))
				return false;
		}

		return true;
	}

	return false;
}

bool CMetadata::RunChildMetadata(IMetaObject *parentObj)
{
	for (auto obj : parentObj->GetObjects())
	{
		if (obj->IsDeleted())
			continue;

		if (!obj->OnRunMetaObject())
			return false;

		if (!RunChildMetadata(obj))
			return false;
	}

	return true;
}

bool CMetadata::CloseMetadata(bool force)
{
	if (m_moduleManager->ExitMainModule(force))
	{
		for (auto obj : m_aMetadataCommon)
		{
			if (obj->IsDeleted())
				continue;

			if (!obj->OnCloseMetaObject())
				return false;

			if (!CloseChildMetadata(obj, force))
				return false;
		}

		if (!m_commonObject->OnCloseMetaObject()) {
			return false;
		}

		if (!m_moduleManager->DestroyMainModule())
			return false;

		return true;
	}

	return false;
}

bool CMetadata::CloseChildMetadata(IMetaObject *parentObj, bool force)
{
	for (auto obj : parentObj->GetObjects())
	{
		if (obj->IsDeleted())
			continue;

		if (!obj->OnCloseMetaObject())
			return false;

		if (!CloseChildMetadata(obj, force))
			return false;
	}

	return true;
}

bool CMetadata::ClearMetadata()
{
	for (auto obj : m_aMetadataCommon)
	{
		if (!obj->IsDeleted() && !obj->OnDeleteMetaObject())
			return false;

		if (!ClearChildMetadata(obj))
			return false;
	}

	if (!m_commonObject->OnDeleteMetaObject()) {
		wxASSERT_MSG(false, "m_commonObject->OnDeleteMetaObject() == false");
		return false;
	}

	m_aMetadataCommon.clear();
	m_aMetadataSecondary.clear();

	return true;
}

bool CMetadata::ClearChildMetadata(IMetaObject *parentObj)
{
	for (auto obj : parentObj->GetObjects())
	{
		if (!obj->IsDeleted() && !obj->OnDeleteMetaObject())
			return false;

		if (!ClearChildMetadata(obj))
			return false;
	}

	parentObj->DecrRef();

	return true;
}

#include <fstream>

bool CMetadata::LoadFromFile(const wxString &fileName)
{
	//close data 
	if (!CloseMetadata(true)) {
		wxASSERT_MSG(false, "CloseMetadata() == false");
		return false;
	}

	//clear data 
	if (!ClearMetadata()) {
		wxASSERT_MSG(false, "ClearMetadata() == false");
		return false;
	}

	std::ifstream in(fileName.ToStdWstring(), std::ios::in | std::ios::binary);

	if (!in.is_open())
		return false;

	//go to end
	in.seekg(0, in.end);
	//get size of file
	std::streamsize fsize = in.tellg();
	//go to beginning
	in.seekg(0, in.beg);

	wxMemoryBuffer tempBuffer(fsize);
	in.read((char *)tempBuffer.GetWriteBuf(fsize), fsize);

	CMemoryReader readerData(tempBuffer.GetData(), tempBuffer.GetBufSize());

	if (readerData.eof())
		return false;

	in.close();

	//Save header info 
	if (!LoadHeader(readerData))
		return false;

	//loading common metadata and child item
	if (!LoadCommonMetadata(g_metaCommonMetadataCLSID, readerData)) {
		//clear data 
		if (!ClearMetadata()) {
			wxASSERT_MSG(false, "ClearMetadata() == false");
		}
		return false;
	}

	//set modify for check metadata
	Modify(true);

	return RunMetadata();
}

bool CMetadata::SaveToFile(const wxString &fileName)
{
	//common data
	CMemoryWriter writterData;

	//Save header info 
	if (!SaveHeader(writterData))
		return false;

	//Save common object
	if (!SaveCommonMetadata(g_metaCommonMetadataCLSID, writterData, true))
		return false;

	//Delete common object
	if (!DeleteCommonMetadata(g_metaCommonMetadataCLSID))
		return false;

	std::ofstream datafile;
	datafile.open(fileName.ToStdWstring(), std::ios::binary);
	datafile.write(reinterpret_cast <char *> (writterData.pointer()), writterData.size());
	datafile.close();

	return true;
}

bool CMetadata::LoadHeader(CMemoryReader &readerData)
{
	CMemoryReader *readerMemory = readerData.open_chunk(eHeaderBlock);

	if (!readerMemory)
		return false;

	u64 metaSign = readerMemory->r_u64();

	if (metaSign != sign_metadata)
		return false;

	u32 metaVersion = readerMemory->r_u32();

	wxString metaGuid;
	readerMemory->r_stringZ(metaGuid);

	readerMemory->close();
	return true;
}

bool CMetadata::LoadCommonMetadata(const CLASS_ID &clsid, CMemoryReader &readerData)
{
	CMemoryReader *readerMemory = readerData.open_chunk(clsid);

	if (!readerMemory)
		return false;

	u64 meta_id = 0;
	CMemoryReader *readerMetaMemory = readerMemory->open_chunk_iterator(meta_id);

	if (!readerMetaMemory)
		return true;

	std::shared_ptr <CMemoryReader>readerDataMemory(readerMetaMemory->open_chunk(eDataBlock));

	m_commonObject->SetReadOnly(!m_readOnly);

	if (!m_commonObject->LoadMetaObject(this, *readerDataMemory))
		return false;

	std::shared_ptr <CMemoryReader> readerChildMemory(readerMetaMemory->open_chunk(eChildBlock));

	if (readerChildMemory)
	{
		if (!LoadMetadata(clsid, *readerChildMemory, m_commonObject))
			return false;
	}

	return true;
}

bool CMetadata::LoadMetadata(const CLASS_ID &, CMemoryReader &readerData, IMetaObject *parentObj)
{
	CLASS_ID clsid = 0;
	CMemoryReader *prevReaderMemory = NULL;

	while (!readerData.eof())
	{
		CMemoryReader *readerMemory = readerData.open_chunk_iterator(clsid, &*prevReaderMemory);

		if (!readerMemory)
			break;

		u64 meta_id = 0;
		CMemoryReader *prevReaderMetaMemory = NULL;

		while (!readerMemory->eof())
		{
			CMemoryReader *readerMetaMemory = readerMemory->open_chunk_iterator(meta_id, &*prevReaderMetaMemory);

			if (!readerMetaMemory)
				break;

			wxASSERT(clsid != 0);
			wxString classType = CValue::GetNameObjectFromID(clsid);
			wxASSERT(classType.Length() > 0);
			IMetaObject *m_newMetaObject = CValue::CreateAndConvertObjectRef<IMetaObject *>(classType);
			wxASSERT(m_newMetaObject);

			m_newMetaObject->SetClsid(clsid);
			m_newMetaObject->SetMetaID(meta_id);

			if (parentObj)
			{
				m_newMetaObject->SetParent(parentObj);
				parentObj->AddChild(m_newMetaObject);
			}

			m_newMetaObject->SetReadOnly(!m_readOnly);

			m_aMetadataCommon.push_back(m_newMetaObject);

			std::shared_ptr <CMemoryReader>readerDataMemory(readerMetaMemory->open_chunk(eDataBlock));

			if (!m_newMetaObject->LoadMetaObject(this, *readerDataMemory))
				return false;

			if (parentObj)
				parentObj->AppendChild(m_newMetaObject);

			m_newMetaObject->ReadProperty();
			m_newMetaObject->IncrRef();

			std::shared_ptr <CMemoryReader> readerChildMemory(readerMetaMemory->open_chunk(eChildBlock));

			if (readerChildMemory)
			{
				if (!LoadChildMetadata(clsid, *readerChildMemory, m_newMetaObject))
					return false;
			}

			prevReaderMetaMemory = readerMetaMemory;
		}

		prevReaderMemory = readerMemory;
	};

	return true;
}

bool CMetadata::LoadChildMetadata(const CLASS_ID &, CMemoryReader &readerData, IMetaObject *parentObj)
{
	CLASS_ID clsid = 0;
	CMemoryReader *prevReaderMemory = NULL;

	while (!readerData.eof())
	{
		CMemoryReader *readerMemory = readerData.open_chunk_iterator(clsid, &*prevReaderMemory);

		if (!readerMemory)
			break;

		u64 meta_id = 0;
		CMemoryReader *prevReaderMetaMemory = NULL;

		while (!readerData.eof())
		{
			CMemoryReader *readerMetaMemory = readerMemory->open_chunk_iterator(meta_id, &*prevReaderMetaMemory);

			if (!readerMetaMemory)
				break;

			wxASSERT(clsid != 0);
			wxString classType = CValue::GetNameObjectFromID(clsid);
			wxASSERT(classType.Length() > 0);
			IMetaObject *m_newMetaObject = CValue::CreateAndConvertObjectRef<IMetaObject *>(classType);
			wxASSERT(m_newMetaObject);

			m_newMetaObject->SetClsid(clsid);
			m_newMetaObject->SetMetaID(meta_id);

			if (parentObj)
			{
				m_newMetaObject->SetParent(parentObj);
				parentObj->AddChild(m_newMetaObject);
			}

			m_newMetaObject->SetReadOnly(!m_readOnly);

			m_aMetadataSecondary.push_back(m_newMetaObject);

			std::shared_ptr <CMemoryReader>readerDataMemory(readerMetaMemory->open_chunk(eDataBlock));

			if (!m_newMetaObject->LoadMetaObject(this, *readerDataMemory))
				return false;

			if (parentObj)
				parentObj->AppendChild(m_newMetaObject);

			m_newMetaObject->ReadProperty();
			m_newMetaObject->IncrRef();

			std::shared_ptr <CMemoryReader> readerChildMemory(readerMetaMemory->open_chunk(eChildBlock));

			if (readerChildMemory)
			{
				if (!LoadChildMetadata(clsid, *readerChildMemory, m_newMetaObject))
					return false;
			}

			prevReaderMetaMemory = readerMetaMemory;
		}

		prevReaderMemory = readerMemory;
	}

	return true;
}

bool CMetadata::SaveHeader(CMemoryWriter &writterData)
{
	CMemoryWriter writterMemory;
	writterMemory.w_u64(sign_metadata); //sign 
	writterMemory.w_u32(OES_VERSION_NUMBER); // version 1 - DEFAULT
	writterMemory.w_stringZ(m_commonObject->GetDocPath()); //guid conf 

	writterData.w_chunk(eHeaderBlock, writterMemory.pointer(), writterMemory.size());
	return true;
}

bool CMetadata::SaveCommonMetadata(const CLASS_ID &clsid, CMemoryWriter &writterData, bool saveToFile)
{
	//Save common object
	CMemoryWriter writterMemory;

	CMemoryWriter writterMetaMemory;
	CMemoryWriter writterDataMemory;

	if (!m_commonObject->SaveMetaObject(this, writterDataMemory, saveToFile)) {
		return false;
	}

	writterMetaMemory.w_chunk(eDataBlock, writterDataMemory.pointer(), writterDataMemory.size());

	CMemoryWriter writterChildMemory;

	if (!SaveMetadata(clsid, writterChildMemory, saveToFile))
		return false;

	writterMetaMemory.w_chunk(eChildBlock, writterChildMemory.pointer(), writterChildMemory.size());
	writterMemory.w_chunk(m_commonObject->GetMetaID(), writterMetaMemory.pointer(), writterMetaMemory.size());

	writterData.w_chunk(clsid, writterMemory.pointer(), writterMemory.size());
	return true;
}

bool CMetadata::SaveMetadata(const CLASS_ID &, CMemoryWriter &writterData, bool saveToFile)
{
	for (auto obj : m_aMetadataCommon)
	{
		CMemoryWriter writterMemory;

		if (obj->IsDeleted())
			continue;

		CMemoryWriter writterMetaMemory;
		CMemoryWriter writterDataMemory;

		if (!obj->SaveMetaObject(this, writterDataMemory, saveToFile)) {
			return false;
		}

		writterMetaMemory.w_chunk(eDataBlock, writterDataMemory.pointer(), writterDataMemory.size());

		CMemoryWriter writterChildMemory;

		if (!SaveChildMetadata(obj->GetClsid(), writterChildMemory, obj, saveToFile)) {
			return false;
		}

		writterMetaMemory.w_chunk(eChildBlock, writterChildMemory.pointer(), writterChildMemory.size());
		writterMemory.w_chunk(obj->GetMetaID(), writterMetaMemory.pointer(), writterMetaMemory.size());

		writterData.w_chunk(obj->GetClsid(), writterMemory.pointer(), writterMemory.size());
	}

	return true;
}

bool CMetadata::SaveChildMetadata(const CLASS_ID &, CMemoryWriter &writterData, IMetaObject *parentObj, bool saveToFile)
{
	for (auto obj : parentObj->GetObjects())
	{
		CMemoryWriter writterMemory;

		if (obj->IsDeleted())
			continue;

		CMemoryWriter writterMetaMemory;
		CMemoryWriter writterDataMemory;

		if (!obj->SaveMetaObject(this, writterDataMemory, saveToFile)) {
			return false;
		}

		writterMetaMemory.w_chunk(eDataBlock, writterDataMemory.pointer(), writterDataMemory.size());

		CMemoryWriter writterChildMemory;

		if (!SaveChildMetadata(obj->GetClsid(), writterChildMemory, obj, saveToFile)) {
			return false;
		}

		writterMetaMemory.w_chunk(eChildBlock, writterChildMemory.pointer(), writterChildMemory.size());
		writterMemory.w_chunk(obj->GetMetaID(), writterMetaMemory.pointer(), writterMetaMemory.size());

		writterData.w_chunk(obj->GetClsid(), writterMemory.pointer(), writterMemory.size());
	}

	return true;
}

bool CMetadata::DeleteCommonMetadata(const CLASS_ID &clsid)
{
	return DeleteMetadata(clsid);
}

bool CMetadata::DeleteMetadata(const CLASS_ID &clsid)
{
	for (auto obj : m_aMetadataCommon)
	{
		if (obj->IsDeleted())
		{
			if (!obj->DeleteMetaObject(this))
				return false;

			if (!DeleteChildMetadata(obj->GetClsid(), obj))
				return false;

			auto itFounded = std::find(m_aMetadataCommon.begin(), m_aMetadataCommon.end(), obj);

			if (itFounded != m_aMetadataCommon.end())
				m_aMetadataCommon.erase(itFounded);

			obj->DecrRef();
		}
		else {
			if (!DeleteChildMetadata(obj->GetClsid(), obj))
				return false;
		}
	}

	return true;
}

bool CMetadata::DeleteChildMetadata(const CLASS_ID &clsid, IMetaObject *parentObj)
{
	for (auto obj : parentObj->GetObjects())
	{
		if (obj->IsDeleted()) {

			if (!obj->DeleteMetaObject(this))
				return false;

			if (!DeleteChildMetadata(obj->GetClsid(), obj))
				return false;

			parentObj->RemoveChild(obj);

			auto itFounded = std::find(m_aMetadataSecondary.begin(), m_aMetadataSecondary.end(), obj);

			if (itFounded != m_aMetadataSecondary.end())
				m_aMetadataSecondary.erase(itFounded);

			obj->DecrRef();
		}
		else {
			if (!DeleteChildMetadata(obj->GetClsid(), obj))
				return false;
		}
	}

	return true;
}

OptionList CMetadata::GetTypelist()
{
	OptionList optionList;

	optionList.AddOption("bool", eValueTypes::TYPE_BOOLEAN);
	optionList.AddOption("number", eValueTypes::TYPE_NUMBER);
	optionList.AddOption("date", eValueTypes::TYPE_DATE);
	optionList.AddOption("string", eValueTypes::TYPE_STRING);

	for (auto metaObject : m_aMetadataCommon)
	{
		if (metaObject->IsRefObject()) {
			optionList.AddOption(metaObject->GetClassName() + wxT(".") + metaObject->GetName(), metaObject->GetMetaID());
		}
	}

	return optionList;
}