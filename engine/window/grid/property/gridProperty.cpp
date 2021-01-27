////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : grid property
////////////////////////////////////////////////////////////////////////////

#include "gridProperty.h"

CPropertyCell::CPropertyCell(wxGrid *grid) : IObjectBase(), m_ownerGrid(grid)
{
	m_category = new PropertyCategory("GridCell");

	PropertyCategory *m_common = new PropertyCategory("Common");
	m_common->AddProperty("name");
	m_common->AddProperty("font");
	m_common->AddProperty("background_colour");

	m_common->AddProperty("align_horz");
	m_common->AddProperty("align_vert");

	m_common->AddProperty("text_colour");
	m_common->AddProperty("value");

	m_category->AddCategory(m_common);
	m_properties["name"] = new Property("name", PropertyType::PT_WXNAME, this);
	m_properties["font"] = new Property("font", PropertyType::PT_WXFONT, this);
	m_properties["background_colour"] = new Property("background_colour", PropertyType::PT_WXCOLOUR, this);

	m_properties["align_horz"] = new PropertyOption("align_horz", GetOptionsAlignment(), this);
	m_properties["align_vert"] = new PropertyOption("align_vert", GetOptionsAlignment(), this);

	m_properties["text_colour"] = new Property("text_colour", PropertyType::PT_WXCOLOUR, this);
	m_properties["value"] = new Property("value", PropertyType::PT_WXSTRING, this);
}

void CPropertyCell::SetCellCoords(const wxGridCellCoords &coords)
{
	m_topLeftCoords = coords;
	m_bottomRightCoords = coords;

	wxString sName = wxString::Format("R%iC%i", coords.GetRow() + 1, coords.GetCol() + 1);
	m_properties["name"]->SetValue(sName);
}

void CPropertyCell::SetCellCoords(const wxGridCellCoords &topLeftCoords, const wxGridCellCoords &bottomRightCoords)
{
	m_topLeftCoords = topLeftCoords;
	m_bottomRightCoords = bottomRightCoords;

	wxString sName = wxString::Format("R%iC%i:R%iC%i", m_topLeftCoords.GetRow() + 1, m_topLeftCoords.GetCol() + 1, bottomRightCoords.GetRow() + 1, bottomRightCoords.GetCol() + 1);
	m_properties["name"]->SetValue(sName);
}

void CPropertyCell::ReadProperty() 
{
	m_properties["value"]->SetValue(m_ownerGrid->GetCellValue(m_topLeftCoords));
	m_properties["font"]->SetValue(m_ownerGrid->GetCellFont(m_topLeftCoords.GetRow(), m_topLeftCoords.GetCol()));
	m_properties["background_colour"]->SetValue(m_ownerGrid->GetCellBackgroundColour(m_topLeftCoords.GetRow(), m_topLeftCoords.GetCol()));
	m_properties["text_colour"]->SetValue(m_ownerGrid->GetCellTextColour(m_topLeftCoords.GetRow(), m_topLeftCoords.GetCol()));

	int horz, vert; 
	m_ownerGrid->GetCellAlignment(m_topLeftCoords.GetRow(), m_topLeftCoords.GetCol(), &horz, &vert);
	m_properties["align_horz"]->SetValue(horz);
	m_properties["align_vert"]->SetValue(vert);
}

void CPropertyCell::SaveProperty()
{
	if (m_topLeftCoords == m_bottomRightCoords)
	{
		m_ownerGrid->SetCellValue(m_topLeftCoords, m_properties["value"]->GetValue());
		m_ownerGrid->SetCellFont(m_topLeftCoords.GetRow(), m_topLeftCoords.GetCol(), m_properties["font"]->GetValueAsFont());
		m_ownerGrid->SetCellBackgroundColour(m_topLeftCoords.GetRow(), m_topLeftCoords.GetCol(), m_properties["background_colour"]->GetValueAsColour());
		m_ownerGrid->SetCellTextColour(m_topLeftCoords.GetRow(), m_topLeftCoords.GetCol(), m_properties["text_colour"]->GetValueAsColour());
		m_ownerGrid->SetCellAlignment(m_topLeftCoords.GetRow(), m_topLeftCoords.GetCol(), m_properties["align_horz"]->GetValueAsInteger(), m_properties["align_vert"]->GetValueAsInteger());
	}
	else
	{
		for (int row = m_topLeftCoords.GetRow(); row <= m_bottomRightCoords.GetRow(); row++)
		{
			for (int col = m_topLeftCoords.GetCol(); col <= m_bottomRightCoords.GetCol(); col++)
			{
				m_ownerGrid->SetCellValue(row, col, m_properties["value"]->GetValue());
				m_ownerGrid->SetCellFont(row, col, m_properties["font"]->GetValueAsFont());
				m_ownerGrid->SetCellBackgroundColour(row, col, m_properties["background_colour"]->GetValueAsColour());
				m_ownerGrid->SetCellTextColour(row, col, m_properties["text_colour"]->GetValueAsColour());
				m_ownerGrid->SetCellAlignment(row, col, m_properties["align_horz"]->GetValueAsInteger(), m_properties["align_vert"]->GetValueAsInteger());
			}
		}
	}
}