////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : frame - controls 
////////////////////////////////////////////////////////////////////////////

#include "frame.h"
#include "compiler/methods.h"

inline bool CompareString(const wxString &lhs, const wxString &rhs)
{
	return lhs.CompareTo(rhs, wxString::ignoreCase) == 0;
}

//////////////////////////////////////////////////////////////////////

wxIMPLEMENT_DYNAMIC_CLASS(CValueFrame::CValueFrameControls, CValue);


//////////////////////////////////////////////////////////////////////

CValueFrame::CValueFrameControls::CValueFrameControls() : CValue(eValueTypes::TYPE_VALUE, true),
m_ownerFrame(NULL), m_methods(NULL)
{
}

CValueFrame::CValueFrameControls::CValueFrameControls(CValueFrame *ownerFrame) : CValue(eValueTypes::TYPE_VALUE, true),
m_ownerFrame(ownerFrame), m_methods(new CMethods())
{
}

#include "compiler/valueMap.h"

CValueFrame::CValueFrameControls::~CValueFrameControls()
{
	if (m_methods)
		delete m_methods;
}

CValue CValueFrame::CValueFrameControls::GetItEmpty()
{
	return new CValueContainer::CValueReturnContainer();
}

CValue CValueFrame::CValueFrameControls::GetItAt(unsigned int idx)
{
	if (m_ownerFrame->m_aControls.size() < idx)
		return CValue();

	auto structurePos = m_ownerFrame->m_aControls.begin();
	std::advance(structurePos, idx);

	return new CValueContainer::CValueReturnContainer((*structurePos)->GetPropertyAsString("name"), CValue(*structurePos));
}

#include "appData.h"

CValue CValueFrame::CValueFrameControls::GetAt(const CValue &cKey)
{
	number_t number = cKey.GetNumber();

	if (m_ownerFrame->m_aControls.size() < number.ToUInt())
		return CValue();

	auto structurePos = m_ownerFrame->m_aControls.begin();
	std::advance(structurePos, number.ToUInt());
	return *structurePos;

	if (!appData->IsDesignerMode())
		CTranslateError::Error(_("Index goes beyond array"));

	return CValue();
}

bool CValueFrame::CValueFrameControls::Property(const CValue &cKey, CValue &cValueFound)
{
	wxString key = cKey.GetString(); 
	auto itFounded = std::find_if(m_ownerFrame->m_aControls.begin(), m_ownerFrame->m_aControls.end(), [key](IControlElement *control) { return CompareString(key, control->GetPropertyAsString("name")); });
	if (itFounded != m_ownerFrame->m_aControls.end()) { cValueFound = *itFounded; return true; }
	return false;
}

enum
{
	enControlCreate,
	enControlRemove,
	enControlProperty,
	enControlCount
};

void CValueFrame::CValueFrameControls::PrepareNames() const
{
	std::vector<SEng> aMethods = {
	{"createControl", "createControl(typeControl, parentElement)"},
	{"removeControl", "removeControl(controlElement)"},
	{"property", "property(key, valueFound)"}, 
	{"count", "count()"}
	};

	m_methods->PrepareMethods(aMethods.data(), aMethods.size());

	std::vector<SEng> aAttributes;

	for (auto &control : m_ownerFrame->m_aControls)
	{
		aAttributes.push_back({ (*control).GetPropertyAsString("name") });
	}

	m_methods->PrepareAttributes(aAttributes.data(), aAttributes.size());
}

CValue CValueFrame::CValueFrameControls::GetAttribute(CAttributeParameters &aParams)
{
	if (m_ownerFrame->m_aControls.size() < (unsigned int)aParams.GetAttributeIndex())
		return CValue();

	auto structurePos = m_ownerFrame->m_aControls.begin();
	std::advance(structurePos, aParams.GetAttributeIndex());
	return *structurePos;
}

#include "compiler/valueType.h"

CValue CValueFrame::CValueFrameControls::Method(CMethodParameters &aParams)
{
	switch (aParams.GetMethodIndex())
	{
	case enControlCreate:
		return m_ownerFrame->CreateControl(aParams[0].ConvertToType<CValueType *>(), aParams.GetParamCount() > 1 ? aParams[1].ConvertToType<IControlElement *>() : CValue());
	case enControlRemove:
		m_ownerFrame->RemoveControl(aParams[0].ConvertToType<IControlElement *>());
		break;
	case enControlProperty:
		return Property(aParams[0], aParams.GetParamCount() > 1 ? aParams[1] : CValue());
	case enControlCount:
		return Count();
	}

	return CValue();
}

//**********************************************************************
//*                       Runtime register                             *
//**********************************************************************

SO_VALUE_REGISTER(CValueFrame::CValueFrameControls, "controls", CValueFrameControls, TEXT2CLSID("VL_CNTR"));