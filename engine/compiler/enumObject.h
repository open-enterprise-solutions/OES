#ifndef ENUM_OBJECT_H__
#define ENUM_OBJECT_H__

#include "enum.h"

//default base class for all enumerations
template <typename valT>
class IEnumeration : public IEnumerationValue<valT>
{
	std::map<valT, wxString> m_aEnumValues;

protected:

	template <typename valType>
	class CEnumerationVariant : public IEnumerationVariant<valType>
	{
		wxString m_type;
		wxString m_description;
		CLASS_ID m_clsid;

		valType m_value;

	public:

		CEnumerationVariant(valType value, const CLASS_ID &clsid) : IEnumerationVariant(), m_value(value), m_clsid(clsid) {}
		void CreateEnumeration(const wxString &typeName, const wxString &description, valType value) { m_type = typeName; m_description = description; m_value = value; }

		virtual valT GetEnumValue() override { return m_value; }
		virtual void SetEnumValue(valT val) override { m_value = val; }

		//operator '=='
		virtual inline bool CompareValueEQ(const CValue &cParam) const override
		{
			CEnumerationVariant<valType> *compareEnumeration = dynamic_cast<CEnumerationVariant<valType> *>(cParam.GetRef());
			if (compareEnumeration) return m_value == compareEnumeration->m_value;
			IEnumeration<valType> *compareEnumerationOwner = dynamic_cast<IEnumeration<valType> *>(cParam.GetRef());
			if (compareEnumerationOwner) return m_value == compareEnumerationOwner->GetEnumValue();
			return false;
		}

		//operator '!='
		virtual inline bool CompareValueNE(const CValue &cParam) const override
		{
			CEnumerationVariant<valType> *compareEnumeration = dynamic_cast<CEnumerationVariant<valType> *>(cParam.GetRef());
			if (compareEnumeration) return m_value != compareEnumeration->m_value;
			IEnumeration<valType> *compareEnumerationOwner = dynamic_cast<IEnumeration<valType> *>(cParam.GetRef());
			if (compareEnumerationOwner) return m_value != compareEnumerationOwner->GetEnumValue();
			return true;
		}

		//get type id
		virtual CLASS_ID GetTypeID() const override { return m_clsid; }

		//check is empty
		virtual inline bool IsEmpty() const override { return false; }

		//type info
		virtual wxString GetTypeString() const override { return m_type; }

		//type conversion
		virtual wxString GetString() const override { return m_description; }
		virtual number_t GetNumber() const override { return m_value; }
	};

	friend class PropertyOption;

private:

	inline void CreateEnumeration(valT val)
	{
		wxASSERT(m_aEnumValues.find(val) != m_aEnumValues.end());

		if (m_value)
			m_value->CreateEnumeration(GetTypeString(), m_aEnumValues[val], val);
	}

public:

	inline void AddEnumeration(valT val, const wxString &description)
	{
		wxASSERT(m_aEnumValues.find(val) == m_aEnumValues.end());
		m_aEnumValues.insert_or_assign(val, description);
		m_aEnumsString.push_back(description);
	}

	IEnumeration() : IEnumerationValue(true), m_value(NULL) {}
	IEnumeration(valT defValue) : IEnumerationValue(), m_value(NULL) {}
	~IEnumeration() { if (m_value) m_value->DecrRef(); }

	virtual valT GetEnumValue() override { if (m_value) { return m_value->GetEnumValue(); } return valT(); }
	virtual void SetEnumValue(valT val) override { if (m_value) { m_value->SetEnumValue(val); } }

	//initialize enumeration 
	virtual void InitializeEnumeration() { PrepareNames(); }

	virtual void InitializeEnumeration(valT defValue)
	{
		wxASSERT(m_value == NULL);
		m_value = new CEnumerationVariant(defValue, ITypeValue::GetTypeID());
		m_value->IncrRef();

		CreateEnumeration(defValue);
	}

	virtual CValue GetAttribute(CAttributeParameters &aParams) override //�������� ��������
	{
		auto itEnums = m_aEnumValues.begin();
		std::advance(itEnums, aParams.GetAttributeIndex());

		if (itEnums != m_aEnumValues.end())
		{
			CEnumerationVariant<valT> *m_enumValue = new CEnumerationVariant<valT>(itEnums->first, ITypeValue::GetTypeID());
			m_enumValue->CreateEnumeration(GetTypeString(), m_aEnumValues[itEnums->first], itEnums->first);
			return m_enumValue;
		}

		return CValue();
	}

	//operator '=='
	virtual inline bool CompareValueEQ(const CValue &cParam) const override
	{
		if (m_value)
			return m_value->CompareValueEQ(cParam);
		else return CValue::CompareValueEQ(cParam);
	}

	//operator '!='
	virtual inline bool CompareValueNE(const CValue &cParam) const override
	{
		if (m_value)
			return m_value->CompareValueNE(cParam);
		else return CValue::CompareValueNE(cParam);
	}

	//check is empty
	virtual inline bool IsEmpty() const override { return false; }

	//type info
	virtual wxString GetTypeString() const = 0;

	//type conversion
	virtual wxString GetString() const override { return m_value ? m_value->GetString() : GetTypeString(); }
	virtual number_t GetNumber() const override { return m_value ? m_value->GetNumber() : wxNOT_FOUND; }

protected:

	CEnumerationVariant<valT> *m_value; //current enum value
};

#endif