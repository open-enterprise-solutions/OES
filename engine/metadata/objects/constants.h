#ifndef _CONSTANTS_H__
#define _CONSTANTS_H__

#include "metadata/metaObjects/attributes/attributes.h"

class CConstantObject : public CAttributeObject
{
	wxDECLARE_DYNAMIC_CLASS(CConstantObject);

public:

	CConstantObject();

	//get class name
	virtual wxString GetClassName() const override { return wxT("constant"); }

	//read & save property
	virtual void ReadProperty() override;
	virtual void SaveProperty() override;

	friend class CManagerConstants; 

protected:

	virtual bool LoadData(CMemoryReader &reader);
	virtual bool SaveData(CMemoryWriter &writer = CMemoryWriter());
	virtual bool DeleteData();

	bool ReadConstInDB();
	bool SaveConstInDB();
	bool DeleteConstInDB();

private:

	wxString GetSpecialTableNameDB();
	wxString GetSpecialFiledNameDB();
};

#endif