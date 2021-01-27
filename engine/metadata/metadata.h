#ifndef _METADATA_H__
#define _METADATA_H__

#include "compiler/compiler.h"
#include "metadata/moduleManager/moduleManager.h"
#include "metadata/metaObjects/metaObjectMetadata.h"

#define metadata            (CMetadata::Get())
#define metadataDestroy()  	(CMetadata::Destroy())

class CDocument;

class IMetadataTree
{
public:

	virtual void SetReadOnly(bool readOnly = true) = 0;

	virtual void Modify(bool modify) = 0;

	virtual bool OpenFormMDI(IMetaObject *obj) = 0;
	virtual bool OpenFormMDI(IMetaObject *obj, CDocument *&foundedDoc) = 0;

	virtual void OnPropertyChanged() = 0; 

	virtual void CloseMetaObject(IMetaObject *obj) = 0;
	virtual void OnCloseDocument(CDocument *doc) = 0;
};

#define sign_metadata 0x1236F362122FE

class IMetadata
{
	IMetadataTree *m_metaTree;

public:

	IMetadata(bool readOnly = false) : m_moduleManager(NULL), m_metaTree(NULL), m_readOnly(readOnly), m_modify(false) {}
	virtual ~IMetadata() {}

	virtual void AppendSecondaryMetadata(IMetaObject *obj) { m_aMetadataSecondary.push_back(obj); }
	virtual void RemoveSecondaryMetadata(IMetaObject *obj) {
		auto foundedIt = std::find(m_aMetadataSecondary.begin(), m_aMetadataSecondary.end(), obj);
		if (foundedIt != m_aMetadataSecondary.end()) m_aMetadataSecondary.erase(foundedIt);
	}

	virtual IModuleManager *GetModuleManager() { return m_moduleManager; };

	virtual bool IsModified() { return m_modify; }
	virtual void Modify(bool modify) {
		if (m_metaTree)
			m_metaTree->Modify(modify);
		m_modify = modify;
	}

	virtual IMetaObject *GetCommonMetaObject() { return NULL; }

	//metadata 
	virtual bool CreateMetadata() = 0;
	virtual bool LoadMetadata() = 0;
	virtual bool SaveMetadata() = 0;
	virtual bool ClearMetadata() = 0;

	//run/close 
	virtual bool RunMetadata() = 0;
	virtual bool CloseMetadata(bool force = false) = 0;

	//metaobject
	IMetaObject *CreateMetaObject(const CLASS_ID &clsid, IMetaObject *parentMetaObj);

	bool RenameMetaObject(IMetaObject *obj, const wxString &sNewName);
	void RemoveMetaObject(IMetaObject *obj, IMetaObject *objParent = NULL);

	//Get metaobjects 
	virtual std::vector<IMetaObject *> GetMetaObjects(const CLASS_ID &clsid);

	//find object
	virtual IMetaObject *FindByName(const wxString &fullName);

	//get metaObject 
	virtual IMetaObject *GetMetaObject(meta_identifier_t meta_id);

	//Associate this metadata with 
	virtual IMetadataTree *GetMetaTree() { return m_metaTree; }
	virtual void SetMetaTree(IMetadataTree *metaTree) { m_metaTree = metaTree; }

	//ID's 
	virtual int GenerateNewID();

protected:

	//Generate new name
	virtual wxString GetNewName(const CLASS_ID &clsid, IMetaObject *metaParent, const wxString &sPrefix = wxEmptyString);

protected:

	bool m_modify;
	bool m_readOnly;

	enum
	{
		eHeaderBlock = 0x2320,
		eDataBlock = 0x2350,
		eChildBlock = 0x2370
	};

	//map with child elem 
	std::vector<IMetaObject *> m_aMetadataCommon;
	std::vector<IMetaObject *> m_aMetadataSecondary;

	//common module manager
	IModuleManager *m_moduleManager;
};

class CMetadata : public IMetadata
{
	static CMetadata* s_instance;

private:

	wxString m_sConfigPath;
	wxString m_sDefaultSource;

private:

	//common meta object
	CMetaObject *m_commonObject;

public:

	CMetadata(bool readOnly = false);
	virtual ~CMetadata();

	static CMetadata* Get();
	static void Destroy();

	wxString GetMetadataName() { return m_commonObject->GetName(); }
	wxString GetConfigPath() { return m_sConfigPath; }
	wxString GetDefaultSource() { return m_sDefaultSource; }

	//Available types 
	class OptionList GetTypelist();

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

	virtual IMetaObject *GetCommonMetaObject() { return m_commonObject; }

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

#endif 