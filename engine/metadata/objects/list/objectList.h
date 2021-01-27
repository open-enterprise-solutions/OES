#ifndef _OBJECT_LIST_H__
#define _OBJECT_LIST_H__

#include "metadata/objects/baseObject.h"

class CObjectListValue : public IDataObjectList
{
	void Prepend(const wxString &text);
	void DeleteItem(const wxDataViewItem &item);
	void DeleteItems(const wxDataViewItemArray &items);

	// implementation of base class virtuals to define model
	virtual unsigned int GetColumnCount() const override;
	virtual wxString GetColumnType(unsigned int col) const override;

	virtual void GetValueByRow(wxVariant &variant,
		unsigned int row, unsigned int col) const;

	virtual bool GetAttrByRow(unsigned int row, unsigned int col, wxDataViewItemAttr &attr) const;

	virtual bool SetValueByRow(const wxVariant &variant,
		unsigned int row, unsigned int col) override;

	virtual IValueTableColumn *GetColumns() const override { return NULL; }
	
	virtual IValueTableReturnLine *GetRowAt(unsigned int line)
	{
		if (line > m_aTableValues.size())
			return NULL;

		return new CObjectListValueReturnLine(this, line);
	};

public:

	class CObjectListValueReturnLine : public IValueTableReturnLine
	{
		wxDECLARE_DYNAMIC_CLASS(CObjectListValueReturnLine);

	public:

		CObjectListValue *m_ownerTable; int m_lineTable;

	public:

		virtual unsigned int GetLineTable() const { return m_lineTable; }
		virtual IValueTable *GetOwnerTable() const { return m_ownerTable; }

		CObjectListValueReturnLine();
		CObjectListValueReturnLine(CObjectListValue *ownerTable, int line);
		virtual ~CObjectListValueReturnLine();

		virtual CMethods* GetPMethods() const { PrepareNames(); return m_methods; }; //�������� ������ �� ����� �������� ������� ���� ��������� � �������
		virtual void PrepareNames() const;                         //���� ����� ������������� ���������� ��� ������������� ���� ��������� � �������

		virtual wxString GetTypeString() const { return wxT("listValueRow"); }
		virtual wxString GetString() const { return wxT("listValueRow"); }

		virtual void SetAttribute(CAttributeParameters &aParams, CValue &cVal); //��������� ��������
		virtual CValue GetAttribute(CAttributeParameters &aParams); //�������� ��������

	private:

		CMethods *m_methods;
	};

public:

	//������ � ����������� 
	virtual bool HasIterator() const override { return true; }

	virtual CValue GetItEmpty() override
	{
		return new CObjectListValueReturnLine(this, wxNOT_FOUND);
	}

	virtual CValue GetItAt(unsigned int idx) override
	{
		if (idx > m_aTableValues.size())
			return CValue();

		return new CObjectListValueReturnLine(this, idx);
	}

	virtual unsigned int GetItSize() const override { return m_aTableValues.size(); }

	//Constructor
	CObjectListValue(IMetaObjectRefValue *metaObj, IMetaFormObject *formObject, IOwnerInfo *ownerControl = NULL);

	//****************************************************************************
	//*                               Support model                              *
	//****************************************************************************

	virtual void UpdateModel();

	//****************************************************************************
	//*                              Support methods                             *
	//****************************************************************************

	static CMethods m_methods;

	virtual CMethods* GetPMethods() const;                          // �������� ������ �� ����� �������� ������� ���� ��������� � �������
	virtual void PrepareNames() const;                             // ���� ����� ������������� ���������� ��� ������������� ���� ��������� � �������
	virtual CValue Method(CMethodParameters &aParams);       // ����� ������

	//****************************************************************************
	//*                              Override attribute                          *
	//****************************************************************************
	virtual void SetAttribute(CAttributeParameters &aParams, CValue &cVal);        //��������� ��������
	virtual CValue GetAttribute(CAttributeParameters &aParams);                   //�������� ��������

	//support actions
	virtual void ExecuteAction(unsigned int action);
};

#endif 