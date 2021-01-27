#ifndef _METAFORMOBJECT_H__
#define _METAFORMOBJECT_H__

#include "metaModuleObject.h"

class IDataObjectSource;
class CValueFrame;

#define formDefaultID 100
#define formDefaultName _("defaultForm")

class IMetaFormObject : public CMetaModuleObject
{
	wxDECLARE_ABSTRACT_CLASS(IMetaFormObject);

	enum
	{
		ID_METATREE_OPEN_FORM = 19000,
	};

public:

	/**
	* Get type form
	*/
	virtual form_identifier_t GetTypeForm() { return formDefaultID; }

	IMetaFormObject(const wxString &name = wxEmptyString, const wxString &alias = wxEmptyString, const wxString &comment = wxEmptyString);

	CValueFrame *GenerateForm(IMetaObjectValue *metaValue = NULL, IDataObjectSource *owner = NULL);
	CValueFrame *GenerateFormAndRun(IMetaObjectValue *metaValue = NULL, IDataObjectSource *owner = NULL, bool runModule = true);
	CValueFrame *GenerateFormAndRun(bool runModule = true);

	void SetFormData(const wxString &formData) { m_sForm = formData; }
	wxString GetFormData() { return m_sForm; }

	//prepare menu for item
	virtual bool PrepareContextMenu(wxMenu *defultMenu);
	virtual void ProcessCommand(unsigned int id);

protected:

	virtual bool LoadData(CMemoryReader &reader);
	virtual bool SaveData(CMemoryWriter &writer = CMemoryWriter());

protected:

	wxString m_sForm;
};

class CMetaFormObject : public IMetaFormObject
{
	wxDECLARE_DYNAMIC_CLASS(CMetaFormObject);

private:

	form_identifier_t m_nTypeFrom;

private:

	friend class CVisualEditorContextForm;

	friend class IMetaObjectValue;
	friend class IDataObjectList;
	friend class IDataObjectValue;

public:

	CMetaFormObject(const wxString &name = wxEmptyString, const wxString &alias = wxEmptyString, const wxString &comment = wxEmptyString);

	virtual wxString GetClassName() const override { return wxT("form"); }

	/**
	* Get type form
	*/
	virtual form_identifier_t GetTypeForm() { return m_nTypeFrom; }

	/**
	* Property events
	*/
	virtual void OnPropertyCreated(Property *m_property);
	virtual void OnPropertySelected(Property *m_property);
	virtual void OnPropertyChanged(Property *m_property);

	//events:
	virtual bool OnCreateMetaObject();
	virtual bool OnLoadMetaObject();
	virtual bool OnSaveMetaObject();
	virtual bool OnDeleteMetaObject();

	//module manager is started or exit 
	virtual bool OnRunMetaObject();
	virtual bool OnCloseMetaObject();

	virtual void ReadProperty() override;
	virtual void SaveProperty() override;

protected:

	virtual bool LoadData(CMemoryReader &reader);
	virtual bool SaveData(CMemoryWriter &writer = CMemoryWriter());

	virtual void PrepareOption(Property *property, OptionList &optionlist) override;
};

class CMetaCommonFormObject : public IMetaFormObject
{
	wxDECLARE_DYNAMIC_CLASS(CMetaCommonFormObject);

public:

	CMetaCommonFormObject(const wxString &name = wxEmptyString, const wxString &alias = wxEmptyString, const wxString &comment = wxEmptyString);
	virtual wxString GetClassName() const override { return wxT("commonForm"); }

	//module manager is started or exit 
	virtual bool OnRunMetaObject();
	virtual bool OnCloseMetaObject();
};

#endif 