#ifndef _TABLES_H__
#define _TABLES_H__

#include "metadata/metaObjects/metaObject.h"
#include "metadata/metaObjects/attributes/attributes.h"

class CTableObject : public IMetaObject
{
	wxDECLARE_DYNAMIC_CLASS(CTableObject);

private:

	std::vector<IMetaObject *> m_metaObjects;

public:

	CTableObject();

	//get class name
	virtual wxString GetClassName() const override { return wxT("tabularSection"); }

	//create in this metaObject 
	virtual void AppendChild(IMetaObject *parentObj) { m_metaObjects.push_back(parentObj); }
	virtual std::vector<IMetaObject *> GetObjects() { return m_metaObjects; }

	virtual std::vector<CAttributeObject *> GetObjectAttributes();

	virtual bool HasAttributes() { return true; }
	virtual wxArrayString GetAttributes();

	//read & save property
	virtual void ReadProperty() override;
	virtual void SaveProperty() override;

protected:

	virtual bool LoadData(CMemoryReader &reader);
	virtual bool SaveData(CMemoryWriter &writer = CMemoryWriter());
};

#endif