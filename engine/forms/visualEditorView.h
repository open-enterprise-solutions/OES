#ifndef _VISUAL_EDITOR_VIEW_H_
#define _VISUAL_EDITOR_VIEW_H_

#include "visualEditor.h"
#include "common/docInfo.h"

class CVisualView : public IVisualHost
{
	CValueFrame *m_valueFrame;
	CDocument *m_document;

	bool m_formDemonstration;

public:

	// construction
	CVisualView(CDocument *m_doc, CValueFrame *valueFrame, wxWindow *parent, bool demonstration = false) :
		IVisualHost(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, wxFrameNameStr),
		m_document(m_doc), m_valueFrame(valueFrame), m_formDemonstration(demonstration)
	{
		m_formHandler = NULL;
	}

	virtual~CVisualView();

	void CreateFrame();
	void UpdateFrame();

	virtual bool IsDemonstration() override { return m_formDemonstration; }

	virtual CValueFrame *GetValueFrame() override { return m_valueFrame; }
	virtual void SetValueFrame(CValueFrame *valueFrame) { m_valueFrame = valueFrame; }

	virtual wxWindow *GetParentBackgroundWindow() override { return this; }
	virtual wxWindow *GetBackgroundWindow() override { return this; }

	void ShowForm();
	void ActivateForm();
	void UpdateForm();
	bool CloseForm();

protected:

	friend class CValueFrame;
};

#endif