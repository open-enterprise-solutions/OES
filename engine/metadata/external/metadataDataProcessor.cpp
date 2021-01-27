////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : external metadata - for dataProcessors, reports
////////////////////////////////////////////////////////////////////////////

#include "metadataDataProcessor.h"

#define default_meta_id 10 //for dataProcessors

CMetadataDataProcessor::CMetadataDataProcessor() : IMetadata()
{
	//create main metaObject
	m_commonObject = new CMetaObjectDataProcessorValue(METAOBJECT_EXTERNAL);
	m_commonObject->SetMetadata(this);
	m_commonObject->SetClsid(g_metaDataProcessorCLSID);
	m_commonObject->SetMetaID(default_meta_id);

	//generate unique guid
	m_commonObject->GenerateGuid();

	if (m_commonObject->OnCreateMetaObject())
	{
		m_moduleManager = new CExternalModuleManager(this, m_commonObject);
		m_moduleManager->IncrRef();

		if (!m_commonObject->OnLoadMetaObject()) {
			wxASSERT_MSG(false, "m_commonObject->OnLoadMetaObject() == false");
		}

		if (!m_commonObject->OnRunMetaObject()) {
			wxASSERT_MSG(false, "m_commonObject->OnRunMetaObject() == false");
		}
	}

	m_commonObject->ReadProperty();
	m_commonObject->IncrRef();

	wxASSERT(m_moduleManager);
}

CMetadataDataProcessor::~CMetadataDataProcessor()
{
	//close 
	if (!CloseMetadata(true)) {
		wxASSERT_MSG(false, "CloseMetadata() == false");
	}

	//clear data 
	if (!ClearMetadata()) {
		wxASSERT_MSG(false, "ClearMetadata() == false");
	}

	//delete module manager
	wxDELETE(m_moduleManager);

	//delete common metaObject
	wxDELETE(m_commonObject);
}

bool CMetadataDataProcessor::CreateMetadata()
{
	return true;
}

bool CMetadataDataProcessor::LoadMetadata()
{
	return RunMetadata();
}

bool CMetadataDataProcessor::SaveMetadata()
{
	return true;
}

bool CMetadataDataProcessor::ClearMetadata()
{
	for (auto obj : m_aMetadataCommon)
	{
		if (!obj->IsDeleted() && !obj->OnDeleteMetaObject())
			return false;

		if (!ClearChildMetadata(obj))
			return false;
	}

	m_aMetadataCommon.clear();
	m_aMetadataSecondary.clear();

	return true;
}

bool CMetadataDataProcessor::ClearChildMetadata(IMetaObject *parentObj)
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

bool CMetadataDataProcessor::RunMetadata()
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

bool CMetadataDataProcessor::RunChildMetadata(IMetaObject *parentObj)
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

bool CMetadataDataProcessor::CloseMetadata(bool force)
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

bool CMetadataDataProcessor::CloseChildMetadata(IMetaObject *parentObj, bool force)
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

#include <fstream>

bool CMetadataDataProcessor::LoadFromFile(const wxString &fileName)
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

	m_fullPath = fileName;

	//loading common metadata and child item
	if (!LoadCommonMetadata(g_metaDataProcessorCLSID, readerData)) {
		//clear data 
		if (!ClearMetadata()) {
			wxASSERT_MSG(false, "ClearMetadata() == false");
		}
		return false;
	}

	//set modify for check metadata
	Modify(true);

	return LoadMetadata();
}

bool CMetadataDataProcessor::SaveToFile(const wxString &fileName)
{
	//common data
	CMemoryWriter writterData;

	//Save header info 
	if (!SaveHeader(writterData))
		return false;

	m_fullPath = fileName;

	//Save common object
	if (!SaveCommonMetadata(g_metaDataProcessorCLSID, writterData))
		return false;

	//Delete common object
	if (!DeleteCommonMetadata(g_metaDataProcessorCLSID))
		return false;

	std::ofstream datafile;
	datafile.open(fileName.ToStdWstring(), std::ios::binary);
	datafile.write(reinterpret_cast <char *> (writterData.pointer()), writterData.size());
	datafile.close();

	return true;
}

bool CMetadataDataProcessor::LoadHeader(CMemoryReader &readerData)
{
	CMemoryReader *readerMemory = readerData.open_chunk(eHeaderBlock);

	if (!readerMemory)
		return false;

	u64 metaSign = readerMemory->r_u64();

	if (metaSign != sign_dataProcessor)
		return false;

	u32 metaVersion = readerMemory->r_u32();

	wxString metaGuid;
	readerMemory->r_stringZ(metaGuid);

	readerMemory->close();
	return true;
}

bool CMetadataDataProcessor::LoadCommonMetadata(const CLASS_ID &clsid, CMemoryReader &readerData)
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
	m_commonObject->SetFullPath(m_fullPath);

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

bool CMetadataDataProcessor::LoadMetadata(const CLASS_ID &, CMemoryReader &readerData, IMetaObject *parentObj)
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

			m_newMetaObject->SetReadOnly(!m_readOnly);
			m_newMetaObject->SetFullPath(m_fullPath);

			if (parentObj)
			{
				m_newMetaObject->SetParent(parentObj);
				parentObj->AddChild(m_newMetaObject);
			}

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

bool CMetadataDataProcessor::LoadChildMetadata(const CLASS_ID &, CMemoryReader &readerData, IMetaObject *parentObj)
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

			m_newMetaObject->SetReadOnly(!m_readOnly);
			m_newMetaObject->SetFullPath(m_fullPath);

			if (parentObj)
			{
				m_newMetaObject->SetParent(parentObj);
				parentObj->AddChild(m_newMetaObject);
			}

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

bool CMetadataDataProcessor::SaveHeader(CMemoryWriter &writterData)
{
	CMemoryWriter writterMemory;
	writterMemory.w_u64(sign_dataProcessor); //sign 
	writterMemory.w_u32(OES_VERSION_NUMBER); // version 1 - DEFAULT
	writterMemory.w_stringZ(m_commonObject->GetDocPath()); //guid conf 

	writterData.w_chunk(eHeaderBlock, writterMemory.pointer(), writterMemory.size());
	return true;
}

bool CMetadataDataProcessor::SaveCommonMetadata(const CLASS_ID &clsid, CMemoryWriter &writterData, bool saveToFile)
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

bool CMetadataDataProcessor::SaveMetadata(const CLASS_ID &, CMemoryWriter &writterData, bool saveToFile)
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

bool CMetadataDataProcessor::SaveChildMetadata(const CLASS_ID &, CMemoryWriter &writterData, IMetaObject *parentObj, bool saveToFile)
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

bool CMetadataDataProcessor::DeleteCommonMetadata(const CLASS_ID &clsid)
{
	return DeleteMetadata(clsid);
}

bool CMetadataDataProcessor::DeleteMetadata(const CLASS_ID &clsid)
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

bool CMetadataDataProcessor::DeleteChildMetadata(const CLASS_ID &clsid, IMetaObject *parentObj)
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