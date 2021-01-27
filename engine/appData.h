#ifndef _APPDATA_H__
#define _APPDATA_H__

#include <set>

#include "common/formdefs.h"
#include "common/types.h"
#include "utils/guid.h"

#include <wx/stc/stc.h>

class CValue;
class CMetadata;
class IMetaObject;
class CMethods;
class IModuleManager;
class CProcUnit;
class CMetadataTree;

struct CByteCode;

class DatabaseLayer;

#define appData         	(ApplicationData::Get())
#define appDataCreate(path) (ApplicationData::Get(path))
#define appDataDestroy()  	(ApplicationData::Destroy())

#define objectDatabase      appData->GetObjectDatabase()

enum eRunMode
{
	DESIGNER_MODE = 1,
	ENTERPRISE_MODE = 2,
	START_MODE = 3
};

// This class is a singleton class.
class ApplicationData
{
	static ApplicationData *s_instance;

	wxString m_projectDir;       // directorio raíz (mismo que el ejecutable)	
	DatabaseLayer* m_objDb;  // Base de datos de objetos

	bool m_warnOnAdditionsUpdate;	// flag to warn on additions update / class renames

	eRunMode m_runMode;

	wxString m_projectFile;
	wxString m_projectPath;

	Guid m_sessionGuid;

	wxString m_userPath;//каталог пользователя для записи настроек
	wxString m_userName;
	wxString m_userPassword;

	bool m_bSingleMode; //Монопольный режим

	// hiden constructor
	ApplicationData(const wxString &rootdir = wxT("."));

public:

	~ApplicationData();

	static ApplicationData* Get(const wxString &rootdir = wxT("."));

	// Force the static appData instance to Init()
	static bool Initialize(eRunMode eMode, const wxString &user, const wxString &password);
	static void Destroy();

	// Initialize application
	bool LoadApp(eRunMode eMode);
	bool SaveProject();

	DatabaseLayer *GetObjectDatabase() { return m_objDb; }
	void SetAppMode(eRunMode eMode) { m_runMode = eMode; }
	eRunMode GetAppMode() { return m_runMode; }
	bool IsDesignerMode() { return m_runMode == eRunMode::DESIGNER_MODE; }
	bool IsEnterpriseMode() { return m_runMode == eRunMode::ENTERPRISE_MODE; }
	wxString GetProjectFileName() { return m_projectFile; }
	const wxString &GetProjectPath() { return m_projectPath; }; /** Path to the md file that is opened. */
	void SetProjectPath(const wxString &path) { m_projectPath = path; };
	const wxString &GetApplicationPath() { return m_projectDir; };
	void SetApplicationPath(const wxString &path) { m_projectDir = path; };
};

#endif
