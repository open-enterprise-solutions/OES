#ifndef _METHODS_H__
#define _METHODS_H__

#include "compiler.h"

//Список ключевых слов, которые не могут быть именами переменных и функций
typedef struct SEnglishDef
{
	wxString sName;
	wxString sShortDescription;
	wxString sAlias;

	int iName;

	SEnglishDef() : sName(wxEmptyString), sShortDescription(wxEmptyString), sAlias("attribute"), iName(wxNOT_FOUND) {}
	SEnglishDef(const wxString &name) : sName(name), sShortDescription(wxEmptyString), sAlias(sAlias), iName(wxNOT_FOUND) {}
	SEnglishDef(const wxString &name, const wxString &shortdescription) : sName(name), sShortDescription(shortdescription), sAlias("attribute"), iName(wxNOT_FOUND) {}
	SEnglishDef(const wxString &name, const wxString &shortdescription, const wxString &alias) : sName(name), sShortDescription(shortdescription), sAlias(sAlias), iName(wxNOT_FOUND) {}
	SEnglishDef(const wxString &name, const wxString &shortdescription, const wxString &alias, int realname) : sName(name), sShortDescription(shortdescription), sAlias(sAlias), iName(realname) {}

} SEng;

class CMethods
{
	//attributes & methods
	std::map<wxString, int> m_aTreeMethods;//дерево наименований методов
	std::map<wxString, int> m_aTreeAttributes;//дерево наименований атрибутов

	std::vector<SEng> m_aMethods; //список методов (англ)
	std::vector<SEng> m_aAttributes; //список атрибутов (англ)

public:

	void PrepareMethods(SEng *Methods, unsigned int nCount);

	int AppendMethod(const wxString &methodName, const wxString &shortDescription);
	int AppendMethod(const wxString &methodName, const wxString &shortDescription, const wxString &alias);
	int AppendMethod(const wxString &methodName, const wxString &shortDescription, const wxString &alias, int realname);
	void RemoveMethod(const wxString &methodName);

	int FindMethod(const wxString &methodName) const;

	wxString GetMethodName(unsigned int methodID) const;
	wxString GetMethodDescription(unsigned int methodID) const;
	wxString GetMethodAlias(unsigned int methodID) const;

	int GetMethodPosition(unsigned int methodID) const;
	unsigned int GetNMethods() const noexcept { return m_aMethods.size(); }

	void PrepareAttributes(SEng *Attributes, unsigned int nCount);

	int AppendAttribute(const wxString &attributeName);
	int AppendAttribute(const wxString &attributeName, const wxString &alias);
	int AppendAttribute(const wxString &attributeName, const wxString &alias, int realname);
	void RemoveAttribute(const wxString &attributeName);

	int FindAttribute(const wxString &attributeName) const;

	wxString GetAttributeName(unsigned int attributeID) const;
	wxString GetAttributeAlias(unsigned int attributeID) const;

	int GetAttributePosition(unsigned int attributeID) const;
	unsigned int GetNAttributes() const noexcept { return m_aAttributes.size(); }
};

#endif
