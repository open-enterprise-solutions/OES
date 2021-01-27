#ifndef _REFERENCE_H__
#define _REFERENCE_H__

#include "compiler/value.h"
#include "utils/guid.h"

class CValueTabularRefSection; 

class CValueReference : public CValue
{
public:

	CValueReference(meta_identifier_t metaID, const Guid &objGuid = Guid());
	CValueReference(IMetaObjectRefValue *metaObject, const Guid &objGuid = Guid());
	virtual ~CValueReference();

	static CValue CreateFromPtr(void *ptr);

	//operator '=='
	virtual inline bool CompareValueEQ(const CValue &cParam) const
	{
		CValueReference *m_reference = dynamic_cast<CValueReference *>(cParam.GetRef());
		if (m_reference) return m_objGuid == m_reference->m_objGuid && m_metaObject == m_reference->m_metaObject;
		return false;
	}

	//operator '!='
	virtual inline bool CompareValueNE(const CValue &cParam) const
	{
		CValueReference *m_reference = dynamic_cast<CValueReference *>(cParam.GetRef());
		if (m_reference) return m_objGuid != m_reference->m_objGuid || m_metaObject != m_reference->m_metaObject;
		return false;
	}

	//override buffer
	virtual void SetBinaryData(int nPosition, PreparedStatement *preparedStatment);
	virtual void GetBinaryData(int nPosition, DatabaseResultSet *databaseResultSet);

	//support show 
	virtual void ShowValue();

	//get metatype
	unsigned int GetTypeRef();

	//check is empty
	virtual inline bool IsEmpty() const override { return !m_objGuid.isValid(); }

	//****************************************************************************
	//*                              Reference methods                           *
	//****************************************************************************

	CValue IsEmptyRef();
	CValue GetMetadata();
	CValue GetObject();

	//****************************************************************************
	//*                              Support methods                             *
	//****************************************************************************

	virtual CMethods* GetPMethods() const;                          // получить ссылку на класс помощник разбора имен атрибутов и методов
	virtual void PrepareNames() const;                             // этот метод автоматически вызывается для инициализации имен атрибутов и методов
	virtual CValue Method(CMethodParameters &aParams);       // вызов метода

	//****************************************************************************
	//*                              Override attribute                          *
	//****************************************************************************
	virtual void SetAttribute(CAttributeParameters &aParams, CValue &cVal);        //установка атрибута
	virtual CValue GetAttribute(CAttributeParameters &aParams);                   //значение атрибута

	//****************************************************************************
	//*                              Override type                               *
	//****************************************************************************
	virtual wxString GetString() const;
	virtual wxString GetTypeString() const;

	friend class IDataObjectRefValue;

private:

	void PrepareReference();
	bool ReadReferenceInDB();

protected:

	bool m_existInDB;

	IMetaObjectRefValue *m_metaObject;
	CMethods *m_methods;

	std::vector<CValueTabularRefSection *> m_aTableParts;

	std::map<wxString, CValue> m_aObjectValues;
	std::map<wxString, unsigned int> m_aObjectTypes;

	reference_t *m_reference_impl;

	Guid m_objGuid;
};

#endif 