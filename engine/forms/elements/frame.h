#ifndef _FRAME_H_
#define _FRAME_H_

#include "window.h"

#define FORM_ID_START 1

#include "common/docInfo.h"
#include "common/moduleInfo.h"

#define thisForm wxT("thisForm")

//********************************************************************************************
//*                                     Defines                                              *
//********************************************************************************************

class CValueType;
class IMetaFormObject;

//********************************************************************************************
//*                                  Visual Document                                         *
//********************************************************************************************

#include "utils/guid.h"

class CVisualDocument : public CDocument
{
	CVisualView *m_visualHost;

public:

	//is demomode
	CVisualDocument() : CDocument(), m_guidForm() { m_visualHost = NULL; }
	//other cases 
	CVisualDocument(const Guid &guid) : CDocument(), m_guidForm(guid) { m_visualHost = NULL; }
	~CVisualDocument();

	virtual bool OnCloseDocument() override;
	virtual bool Save() override;

	void SetVisualView(CVisualView *visualHost) { m_visualHost = visualHost; }
	CVisualView *GetVisualView() { return m_visualHost; }

	Guid GetGuid() { return m_guidForm; }

	virtual void Modify(bool modify);

protected:

	Guid m_guidForm;
};

//********************************************************************************************
//*                                  Value Frame                                             *
//********************************************************************************************

class CValueFrame : public IControlElement,
	public IModuleInfo
{
	wxDECLARE_DYNAMIC_CLASS(CValueFrame);

protected:

	OptionList *GetOrient()
	{
		OptionList *m_optionlist = new OptionList();
		m_optionlist->AddOption("Vertical", wxVERTICAL);
		m_optionlist->AddOption("Horizontal", wxHORIZONTAL);
		return m_optionlist;
	}

	bool m_formModified;

public:

	wxString m_name = wxT("Frame");
	wxString m_title = wxT("New frame");

	Guid m_guidForm;

	wxColour m_fg = RGB(0, 120, 215);
	wxColour m_bg = RGB(240, 240, 240);

	wxString m_tooltip;
	bool m_context_menu;
	wxString m_context_help;
	bool m_enabled = true;
	bool m_visible = true;

	long m_style = wxCAPTION;
	long m_extra_style = 0;
	long m_center = 0;

	wxOrientation m_orient;

private:

	IDataObjectSource *m_sourceObject;

public:

	/**
	* Support generate id
	*/
	virtual void GenerateNewID() override { m_obj_id = FORM_ID_START; }

	/**
	* Support get/set object id
	*/
	virtual void SetControlID(unsigned int obj_id) override {}
	virtual unsigned int GetControlID() override { return FORM_ID_START; }

public:

	CValueFrame();

	virtual wxString GetClassName() const override { return wxT("frame"); }
	virtual wxString GetObjectTypeName() const override { return wxT("form"); }

	//****************************************************************************
	//*                              Override attribute                          *
	//****************************************************************************

	virtual void SetAttribute(CAttributeParameters &aParams, CValue &cVal);        //установка атрибута
	virtual CValue GetAttribute(CAttributeParameters &aParams);                   //значение атрибута
	virtual int FindAttribute(const wxString &sName) const;

	//****************************************************************************
	//*                              Support methods                             *
	//****************************************************************************

	virtual void PrepareNames() const;                             // этот метод автоматически вызывается для инициализации имен атрибутов и методов
	virtual CValue Method(CMethodParameters &aParams);       // вызов метода

	//****************************************************************************
	//*                              Support form context                        *
	//****************************************************************************

	void SetSourceObject(IDataObjectSource *sourceobject);
	IDataObjectSource *GetSourceObject() { return m_sourceObject; }

	void SetMetaObject(IMetaFormObject *metaFormObject, IMetaObjectValue *metaObject = NULL);

	IMetaFormObject *GetFormMetaObject() { return m_metaFormObject; }
	IMetaObjectValue *GetMetaObject() { return m_metaObject; }

	bool InitializeModule(bool runModule = true);

public:

	std::vector<IControlElement *> m_aControls;

	class CValueFrameControls : public CValue
	{
		wxDECLARE_DYNAMIC_CLASS(CValueFrameControls);

	public:

		CValueFrameControls();
		CValueFrameControls(CValueFrame *ownerFrame);
		virtual ~CValueFrameControls();

		virtual CMethods* GetPMethods() const { PrepareNames(); return m_methods; }; //получить ссылку на класс помощник разбора имен атрибутов и методов
		virtual void PrepareNames() const;                         //этот метод автоматически вызывается для инициализации имен атрибутов и методов
		virtual CValue Method(CMethodParameters &aParams);

		virtual CValue GetAttribute(CAttributeParameters &aParams); //значение атрибута
		virtual CValue GetAt(const CValue &cKey);

		virtual wxString GetTypeString() const { return wxT("controls"); }
		virtual wxString GetString() const { return wxT("controls"); }

		//Расширенные методы:
		bool Property(const CValue &cKey, CValue &cValueFound);
		unsigned int Count() const { return m_ownerFrame->m_aControls.size(); }

		//Работа с итераторами:
		virtual bool HasIterator() const { return true; }
		virtual CValue GetItEmpty();
		virtual CValue GetItAt(unsigned int idx);
		virtual unsigned int GetItSize() const { return Count(); }

	private:

		CMethods *m_methods;
		CValueFrame *m_ownerFrame;
	};

	CValueFrameControls *m_frameControls;

public:

	CVisualDocument *m_valueFrameDocument;

	IMetaObjectValue *m_metaObject; // ref to metadata
	IMetaFormObject *m_metaFormObject; // ref to metadata

protected:

	friend class CValueFrameControls;

	friend class CVisualDocument;
	friend class CFormView;

	bool ShowDocumentForm(bool demonstration = false);
	bool CloseFrame();

private:

	void ClearRecursive(IControlElement *control);

public:

	CValue CreateControl(const CValueType *classControl, const CValue &vControl);
	void RemoveControl(const CValue &vControl);

public:

	void ShowForm(bool demonstration = false);
	void ActivateForm();
	void UpdateForm();
	bool CloseForm();
	void HelpForm() {};

	//set&get modify 
	void Modify(bool modify) { m_formModified = modify; if (m_valueFrameDocument) m_valueFrameDocument->Modify(m_formModified); }
	bool IsModified() { return m_formModified; }
	//shown form 
	bool IsShown() { return m_valueFrameDocument != NULL; }

	//get visual document
	CVisualDocument *GetVisualDocument() { return m_valueFrameDocument; }

	//special proc
	virtual void Update(wxObject* wxobject, IVisualHost *visualHost);
	virtual void OnUpdated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost);

	/**
	* Set property data
	*/
	virtual void SetPropertyData(Property *property, const CValue &srcValue);

	/**
	* Get property data
	*/
	virtual CValue GetPropertyData(Property *property);

	//****************************************************************************
	//*                              actions                                     *
	//****************************************************************************

	virtual CAction GetActions(form_identifier_t formType);
	virtual void AddActions(CAction &actions, form_identifier_t formType);
	virtual void ExecuteAction(unsigned int action);

	//****************************************************************************
	//*                             Property                                     *
	//****************************************************************************

	virtual void ReadProperty() override;
	virtual void SaveProperty() override;

	virtual int GetComponentType() override { return COMPONENT_TYPE_FRAME; }
	virtual bool IsItem() override { return false; }

	virtual ~CValueFrame();
};

#endif 
