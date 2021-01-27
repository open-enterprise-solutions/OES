#include "metadata/metadata.h"
#include "metadata/objects/dataProcessor.h"

#define sign_dataProcessor 0x1345F6621261E

class CMetadataDataProcessor : public IMetadata
{
	wxString m_fullPath;

	//common meta object
	CMetaObjectDataProcessorValue *m_commonObject;

public:

	CMetadataDataProcessor();
	virtual ~CMetadataDataProcessor();

	virtual CMetaObjectDataProcessorValue *GetDataProcessor() { return m_commonObject; }
	virtual IModuleManager *GetModuleManager() { return m_moduleManager; }

	//metadata 
	virtual bool CreateMetadata();
	virtual bool LoadMetadata();
	virtual bool SaveMetadata();
	virtual bool ClearMetadata();

	//run/close 
	virtual bool RunMetadata();
	virtual bool CloseMetadata(bool force = false);

	//load/save form file
	bool LoadFromFile(const wxString &fileName);
	bool SaveToFile(const wxString &fileName);

	virtual IMetaObject *GetCommonMetaObject() { return GetDataProcessor(); }

protected:

	//header loader/saver 
	bool LoadHeader(CMemoryReader &readerData);
	bool SaveHeader(CMemoryWriter &writterData);

	//loader/saver/deleter: 
	bool LoadCommonMetadata(const CLASS_ID &clsid, CMemoryReader &readerData);
	bool LoadMetadata(const CLASS_ID &clsid, CMemoryReader &readerData, IMetaObject *parentObj);
	bool LoadChildMetadata(const CLASS_ID &clsid, CMemoryReader &readerData, IMetaObject *parentObj);
	bool SaveCommonMetadata(const CLASS_ID &clsid, CMemoryWriter &writterData, bool saveToFile = false);
	bool SaveMetadata(const CLASS_ID &clsid, CMemoryWriter &writterData, bool saveToFile);
	bool SaveChildMetadata(const CLASS_ID &clsid, CMemoryWriter &writterData, IMetaObject *parentObj, bool saveToFile);
	bool DeleteCommonMetadata(const CLASS_ID &clsid);
	bool DeleteMetadata(const CLASS_ID &clsid);
	bool DeleteChildMetadata(const CLASS_ID &clsid, IMetaObject *parentObj);

	//run/close recursively:
	bool RunChildMetadata(IMetaObject *parentObj);
	bool CloseChildMetadata(IMetaObject *parentObj, bool force = false);
	bool ClearChildMetadata(IMetaObject *parentObj);
};