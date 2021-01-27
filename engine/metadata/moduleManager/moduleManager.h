#ifndef _MODULE_MANAGER_H__
#define _MODULE_MANAGER_H__

#include "common/moduleInfo.h"
#include "metadata/metaObjects/metaObjectMetadata.h"
#include "metadata/metaObjects/metaModuleObject.h"
#include "metadata/objects/dataProcessor.h"

class IModuleManager : public CValue,
	public IModuleInfo
{
public:

	class CModuleValue : public CValue,
		public IModuleInfo
	{
		wxDECLARE_DYNAMIC_CLASS(CModuleValue);

	private:

		CMethods *m_methods;

	public:

		CModuleValue() {}
		CModuleValue(IModuleManager *moduleManager, CMetaModuleObject *moduleObject);
		virtual ~CModuleValue();

		//initalize common module
		bool CreateCommonModule();
		bool DestroyCommonModule();

		//get common module 
		CMetaModuleObject *GetModuleObject() const { return m_moduleObject; }

		//РАБОТА КАК АГРЕГАТНОГО ОБЪЕКТА

		//эти методы нужно переопределить в ваших агрегатных объектах:
		virtual CMethods* GetPMethods() const { PrepareNames();  return m_methods; };//получить ссылку на класс помощник разбора имен атрибутов и методов

		//этот метод автоматически вызывается для инициализации имен атрибутов и методов
		virtual void PrepareNames() const;

		//вызов метода
		virtual CValue Method(CMethodParameters &aParams) override;

		virtual wxString GetString() const override { return m_moduleObject->GetName(); };
		virtual wxString GetTypeString() const override { return wxT("module"); };

		//check is empty
		virtual inline bool IsEmpty() const override { return false; }

		//operator '=='
		virtual inline bool CompareValueEQ(const CValue &cParam) const override
		{
			CModuleValue *compareModule = dynamic_cast<CModuleValue *>(cParam.GetRef());
			if (compareModule) {
				return m_moduleObject == compareModule->GetModuleObject();
			}

			return false;
		}

		//operator '!='
		virtual inline bool CompareValueNE(const CValue &cParam) const override
		{
			CModuleValue *compareModule = dynamic_cast<CModuleValue *>(cParam.GetRef());
			if (compareModule) {
				return m_moduleObject != compareModule->GetModuleObject();
			}

			return false;
		}

	protected:

		IModuleManager *m_moduleManager;
		CMetaModuleObject *m_moduleObject;
	};

	class CMetadataValue : public CValue
	{
		wxDECLARE_DYNAMIC_CLASS(CMetadataValue);

	private:

		CMethods *m_methods;


	public:

		CMetadataValue() {}
		CMetadataValue(IMetadata *metaData);
		virtual ~CMetadataValue();

		//get common module 
		IMetadata *GetMetadata() const { return m_metaData; }

		virtual wxString GetTypeString() const override { return wxT("metadata"); }
		virtual wxString GetString() const override { return wxT("metadata"); }

		//check is empty
		virtual inline bool IsEmpty() const override { return false; }

		//operator '=='
		virtual inline bool CompareValueEQ(const CValue &cParam) const override
		{
			CMetadataValue *compareMetadata = dynamic_cast<CMetadataValue *>(cParam.GetRef());
			if (compareMetadata) {
				return m_metaData == compareMetadata->GetMetadata();
			}

			return false;
		}

		//operator '!='
		virtual inline bool CompareValueNE(const CValue &cParam) const override
		{
			CMetadataValue *compareMetadata = dynamic_cast<CMetadataValue *>(cParam.GetRef());
			if (compareMetadata) {
				return m_metaData != compareMetadata->GetMetadata();
			}

			return false;
		}

		//эти методы нужно переопределить в ваших агрегатных объектах:
		virtual CMethods* GetPMethods() const override { PrepareNames();  return m_methods; };//получить ссылку на класс помощник разбора имен атрибутов и методов
		virtual void PrepareNames() const override;//этот метод автоматически вызывается для инициализации имен атрибутов и методов
		virtual CValue Method(CMethodParameters &aParams) override;//вызов метода

		//****************************************************************************
		//*                              Override attribute                          *
		//****************************************************************************

		virtual void SetAttribute(CAttributeParameters &aParams, CValue &cVal) override;        //установка атрибута
		virtual CValue GetAttribute(CAttributeParameters &aParams) override;                   //значение атрибута

	protected:

		IMetadata *m_metaData;
	};

private:

	void Clear();

protected:

	//metadata and external variant
	IModuleManager(IMetadata *metaData, CMetaModuleObject *metaObject);

public:

	virtual ~IModuleManager();

	//Create common module
	virtual bool CreateMainModule() = 0;

	//destroy common module
	virtual bool DestroyMainModule() = 0;

	//start common module
	virtual bool StartMainModule() = 0;

	//exit common module
	virtual bool ExitMainModule(bool force = false) = 0;

	//эти методы нужно переопределить в ваших агрегатных объектах:
	virtual CMethods* GetPMethods() const { PrepareNames();  return m_methods; };//получить ссылку на класс помощник разбора имен атрибутов и методов
	//этот метод автоматически вызывается для инициализации имен атрибутов и методов
	virtual void PrepareNames() const;
	//вызов метода
	virtual CValue Method(CMethodParameters &aParams);

	virtual void SetAttribute(CAttributeParameters &aParams, CValue &cVal);        //установка атрибута
	virtual CValue GetAttribute(CAttributeParameters &aParams);                   //значение атрибута
	virtual int FindAttribute(const wxString &sName) const;

	virtual wxString GetString() const { return wxT("moduleManager"); };
	virtual wxString GetTypeString() const { return wxT("moduleManager"); };

	//check is empty
	virtual inline bool IsEmpty() const override { return false; }

	//compile modules:
	bool AddCompileModule(IMetaObject *moduleObject, CValue *object);
	bool RemoveCompileModule(IMetaObject *moduleObject);

	//templates:
	template <class T> inline bool FindCompileModule(IMetaObject *moduleObject, T *&objValue)
	{
		if (m_aCompileModules.find(moduleObject) != m_aCompileModules.end()) {
			objValue = dynamic_cast<T *>(m_aCompileModules[moduleObject]);
			return objValue != NULL;
		}
		objValue = NULL;
		return false;
	}

	template <class T> inline bool FindParentCompileModule(IMetaObject *moduleObject, T *&objValue)
	{
		IMetaObject *m_parentMetadata = moduleObject ? moduleObject->GetParent() : NULL;
		if (m_parentMetadata)
			return FindCompileModule(m_parentMetadata, objValue);
		return false;
	}

	//common modules:
	bool AddCommonModule(CMetaCommonModuleObject *commonModule, bool managerModule = false);
	bool AddCommonModuleAndRun(CMetaCommonModuleObject *commonModule);
	CValue *FindCommonModule(CMetaCommonModuleObject *commonModule);
	bool RenameCommonModule(CMetaCommonModuleObject *commonModule, const wxString &newName);
	bool RemoveCommonModule(CMetaCommonModuleObject *commonModule);

	//system object:
	CValue *GetObjectManager() { return m_objectManager; }
	CValue *GetSysObjectManager() { return m_objectSysManager; }
	CMetadataValue *GetMetaManager() { return m_metaManager; }

	virtual std::map<wxString, CValue *> &GetGlobalVariables() { return m_aValueGlVariables; }
	virtual std::map<wxString, CValue *> &GetContextVariables() { return m_compileModule->m_aContextValues; }

	//return external module
	virtual IDataObjectValue *GetObjectData() const { return NULL; }

protected:

	bool m_initialized;

	//global manager
	CValue *m_objectManager;
	//global sysmanager 
	CValue *m_objectSysManager;
	// global metamanager
	CMetadataValue *m_metaManager;

	//map with compile data
	std::map<IMetaObject *, CValue *> m_aCompileModules;
	//array of common modules
	std::vector<CModuleValue *> m_aCommonModules;
	//array of global variables
	std::map<wxString, CValue *> m_aValueGlVariables;

	friend class CMetadata;
	friend class CMetadataDataProcessor;

	friend class CModuleValue;

	CMethods *m_methods;
};

class CModuleManager : public IModuleManager
{
	//system events:
	bool BeforeStart();
	void OnStart();
	bool BeforeExit();
	void OnExit();

public:

	//metadata and external variant
	CModuleManager(IMetadata *metaData, CMetaObject *metaObject);

	//Create common module
	virtual bool CreateMainModule();

	//destroy common module
	virtual bool DestroyMainModule();

	//start common module
	virtual bool StartMainModule();

	//exit common module
	virtual bool ExitMainModule(bool force = false);
};

class CExternalModuleManager : public IModuleManager
{
	CObjectDataProcessorValue *m_objectData;

public:

	virtual CCompileModule *GetCompileModule() const;
	virtual CProcUnit *GetProcUnit() const;

	virtual std::map<wxString, CValue *> &GetContextVariables();

	//metadata and external variant
	CExternalModuleManager(IMetadata *metaData, CMetaObjectDataProcessorValue *metaObject);
	virtual ~CExternalModuleManager();

	//return external module
	virtual IDataObjectValue *GetObjectData() const { return m_objectData; }

	//Create common module
	virtual bool CreateMainModule();

	//destroy common module
	virtual bool DestroyMainModule();

	//start common module
	virtual bool StartMainModule();

	//exit common module
	virtual bool ExitMainModule(bool force = false);

	//этот метод автоматически вызывается для инициализации имен атрибутов и методов
	virtual void PrepareNames() const;
	//вызов метода
	virtual CValue Method(CMethodParameters &aParams);

	virtual void SetAttribute(CAttributeParameters &aParams, CValue &cVal);        //установка атрибута
	virtual CValue GetAttribute(CAttributeParameters &aParams);                   //значение атрибута
	virtual int FindAttribute(const wxString &sName) const;
};

#endif

