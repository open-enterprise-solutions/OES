#ifndef _VALUEARRAY_H__
#define _VALUEARRAY_H__

#include "value.h"

//��������� ��������
class CValueArray : public CValue
{
	wxDECLARE_DYNAMIC_CLASS(CValueArray);

private:

	std::vector <CValue> m_aValuesArray;

private:

	inline void CheckIndex(unsigned int index);

public:

	CValueArray();
	CValueArray(const std::vector <CValue> &arr);

	virtual ~CValueArray();

	virtual bool Init(CValue **aParams);

	virtual wxString GetTypeString() const { return wxT("array"); }
	virtual wxString GetString() const { return wxT("array"); }

	//check is empty
	virtual inline bool IsEmpty() const override { return m_aValuesArray.empty(); }

public:

	//������� -> ��������� ����
	//������ � �������� ��� � ���������� ��������
	static CMethods m_methods;

	virtual CMethods* GetPMethods() const { return &m_methods; }; //�������� ������ �� ����� �������� ������� ���� ��������� � �������
	virtual void PrepareNames() const;                         //���� ����� ������������� ���������� ��� ������������� ���� ��������� � �������
	virtual CValue Method(CMethodParameters &aParams);       //����� ������

	//����������� ������
	void Add(CValue &cVal);
	void Insert(unsigned int index, CValue &cVal);
	unsigned int Count() const { return m_aValuesArray.size(); }
	CValue Find(CValue &cVal);
	void Remove(unsigned int index);
	void Clear() { m_aValuesArray.clear(); }

	//array support 
	virtual CValue GetAt(const CValue &cKey);
	virtual void SetAt(const CValue &cKey, CValue &cVal);

	//������ � ����������� 
	virtual bool HasIterator() const { return true; }
	virtual CValue GetItAt(unsigned int idx) { return m_aValuesArray[idx]; }
	virtual unsigned int GetItSize() const { return Count(); }
};

#endif