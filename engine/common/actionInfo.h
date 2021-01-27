#ifndef _ACTIONINFO_H__
#define _ACTIONINFO_H__

#include "compiler/compiler.h"

class CAction
{
	struct base_action
	{
		wxString m_name;
		wxString m_description;
		wxBitmap m_bitmap;

		base_action() : m_name(wxEmptyString),
			m_description(wxEmptyString),
			m_bitmap(wxNullBitmap)
		{
		}

		base_action(wxString name, wxString description, wxBitmap bitmap) : m_name(name),
			m_description(description),
			m_bitmap(bitmap)
		{
		}
	};

	bool bInitialize;
	std::map<unsigned int, base_action> aActions;

public:

	CAction() : bInitialize(false) {}

	void AddAction(wxString name, unsigned int action)
	{
		aActions[action] = base_action(name, wxEmptyString, wxNullBitmap);
	}

	void AddAction(wxString name, wxString description, unsigned int action)
	{
		aActions[action] = base_action(name, description, wxNullBitmap);
	}

	void AddAction(wxString name, wxString description, wxBitmap bitmap, unsigned int action)
	{
		aActions[action] = base_action(name, description, bitmap);
	}

	wxString GetNameByID(unsigned int action)
	{
		if (aActions.find(action) != aActions.end())
			return aActions[action].m_name;

		return wxEmptyString;
	}

	wxString GetDesctiptionByID(unsigned int action)
	{
		if (aActions.find(action) != aActions.end())
			return aActions[action].m_description;

		return wxEmptyString;
	}

	unsigned int GetID(unsigned int position)
	{
		if (position > GetCount())
			return -1;

		auto it = aActions.begin();
		std::advance(it, position);

		return it->first;
	}

	unsigned int GetCount() { return aActions.size(); }

	void SetInitialize(bool initialize) { bInitialize = initialize; }
	bool GetInitialize() { return bInitialize; }
};

class IActionSource
{
public:

	//support actions 
	virtual CAction GetActions(form_identifier_t formType) = 0;
	virtual void AddActions(CAction &actions, form_identifier_t formType) = 0;
	virtual void ExecuteAction(unsigned int action) = 0;
};

#endif 