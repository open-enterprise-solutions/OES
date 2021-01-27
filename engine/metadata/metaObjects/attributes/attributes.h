#ifndef _ATTRIBUTES_H__
#define _ATTRIBUTES_H__

#include "metadata/metaObjects/metaObject.h"

class CAttributeObject : public IMetaObject
{
	wxDECLARE_DYNAMIC_CLASS(CAttributeObject);

private:

	enum class eDateVariant : short
	{
		eDate = 0,
		eDateTime,
		eTime
	};

	bool m_bIsDefaultAttribute;

	struct typedescription_t
	{
		int m_typeObject;

		union
		{
			eDateVariant m_dateTime;

			struct
			{
				unsigned char m_precision;
				unsigned char m_scale;
			};

			unsigned char m_lenght;

		};

		typedescription_t(int typeObject) : m_typeObject(typeObject) {}
		typedescription_t(int typeObject, unsigned char precision, unsigned char scale) : m_typeObject(typeObject), m_precision(precision), m_scale(scale) {}
		typedescription_t(int typeObject, eDateVariant dateTime) : m_typeObject(typeObject), m_dateTime(dateTime) {}
		typedescription_t(int typeObject, unsigned char length) : m_typeObject(typeObject), m_lenght(length) {}

	} m_typeDescription;

	wxString sSQLFieldName;

public:

	CAttributeObject();
	CAttributeObject(const wxString &name, unsigned int type, const wxString &SQLFieldName = wxEmptyString);
	CAttributeObject(const wxString &name, const wxString &alias, const wxString &comment, unsigned int type, const wxString &SQLFieldName = wxEmptyString);

	//get class name
	virtual wxString GetClassName() const override { return wxT("attribute"); }

	//get ref type 
	int GetTypeObject() { return m_typeDescription.m_typeObject; }

	//get suffix type for db 
	wxString GetSuffixObject();

	//get sql type for db 
	wxString GetSQLTypeObject();

	//get sql db name 
	wxString GetSQLFieldName();

	//get special data number 
	short GetPrecision() { return m_typeDescription.m_precision; }
	short GetScale() { return m_typeDescription.m_scale; }

	//get special data date  
	eDateVariant GetDateTime() { return m_typeDescription.m_dateTime; }

	//get special data string  
	short GetLenght() { return m_typeDescription.m_lenght; }

	//check if attribute is default 
	bool IsDefaultAttribute() { return m_bIsDefaultAttribute; }

	/**
	* Property events
	*/
	virtual void OnPropertyCreated();

	virtual void OnPropertyCreated(Property *m_property);
	virtual void OnPropertySelected(Property *m_property);
	virtual void OnPropertyChanged(Property *m_property);

	//read & save property
	virtual void ReadProperty() override;
	virtual void SaveProperty() override;

protected:

	virtual bool LoadData(CMemoryReader &reader);
	virtual bool SaveData(CMemoryWriter &writer = CMemoryWriter());
};

#endif