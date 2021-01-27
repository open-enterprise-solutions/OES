#ifndef _VALUETYPE_H__
#define _VALUETYPE_H__

#include <wx/wx.h>
#include "compiler/value.h"

class CValueType : public CValue
{
	wxDECLARE_DYNAMIC_CLASS(CValueType);

public:

	CLASS_ID GetOwnerType() { return m_type_id; }

public:

	CValueType();

	CValueType(CLASS_ID type_id);
	CValueType(const wxString &typeName);
	CValueType(const CValue &cObject);
	CValueType(const CValueType &typeObject);

	virtual inline bool IsEmpty() const override { return false; }

	virtual bool CompareValueEQ(const CValue &cParam) const
	{
		CValueType *m_rValue = value_cast<CValueType *>(cParam);
		wxASSERT(m_rValue);
		return m_type_id == m_rValue->m_type_id;
	}

	//operator '!='
	virtual bool CompareValueNE(const CValue &cParam) const
	{
		CValueType *m_rValue = value_cast<CValueType *>(cParam);
		wxASSERT(m_rValue);
		return m_type_id != m_rValue->m_type_id;
	}

	virtual wxString GetTypeString() const { return wxT("type"); }
	virtual wxString GetString() const { return m_classType; }

private:

	CLASS_ID m_type_id;
	wxString m_classType;
};

#endif