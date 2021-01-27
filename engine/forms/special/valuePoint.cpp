////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : value point 
////////////////////////////////////////////////////////////////////////////

#include "valuePoint.h"
#include "compiler/methods.h"

#include "window/mainFrame.h"
#include "database/databaseLayer.h"

#include "utils/stringutils.h"
#include "utils/typeconv.h"

//////////////////////////////////////////////////////////////////////
wxIMPLEMENT_DYNAMIC_CLASS(CValuePoint, CValue);

CMethods CValuePoint::m_methods;

CValuePoint::CValuePoint() : CValue(eValueTypes::TYPE_VALUE), m_point(wxDefaultPosition)
{
}

CValuePoint::CValuePoint(const wxPoint &point) : CValue(eValueTypes::TYPE_VALUE), m_point(point)
{
}

bool CValuePoint::Init(CValue **aParams)
{
	if (aParams[0]->GetType() == eValueTypes::TYPE_STRING) { m_point = TypeConv::StringToPoint(aParams[0]->ToString()); return true; }
	else { m_point = wxPoint(aParams[0]->ToInt(), aParams[1]->ToInt()); return true; }
	return false;
}

CValuePoint::~CValuePoint()
{
}

enum
{
	eX,
	eY
};

void CValuePoint::PrepareNames() const
{
	std::vector <SEng>aAttributes;

	SEng attribute;

	attribute.sName = "x";
	aAttributes.push_back(attribute);
	attribute.sName = "y";
	aAttributes.push_back(attribute);

	m_methods.PrepareAttributes(aAttributes.data(), aAttributes.size());
}

void CValuePoint::SetAttribute(CAttributeParameters &aParams, CValue &cVal)
{
	switch (aParams.GetAttributeIndex())
	{
	case eX: m_point.x = cVal.ToInt(); break;
	case eY: m_point.y = cVal.ToInt(); break;
	}
}

CValue CValuePoint::GetAttribute(CAttributeParameters &aParams)
{
	switch (aParams.GetAttributeIndex())
	{
	case eX: return m_point.x;
	case eY: return m_point.y;
	}

	return CValue();
}

wxString CValuePoint::GetTypeString()const
{
	return "point";
}

wxString CValuePoint::GetString()const
{
	return TypeConv::PointToString(m_point);
}

//**********************************************************************
//*                       Runtime register                             *
//**********************************************************************

VALUE_REGISTER(CValuePoint, "point", TEXT2CLSID("VL_PONT"));