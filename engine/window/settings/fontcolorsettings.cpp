
#include "fontcolorsettings.h"
#include "XmlUtility.h"

#include <wx/xml/xml.h>

const char* FontColorSettings::s_displayItemName[] = { "Default", "Comment", "Keyword", "Operator", "String", "Number", "Error", "Warning", "Selection" };

FontColorSettings::FontColorSettings()
{

    m_font = wxFont(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);

    m_colors[DisplayItem_Default].foreColor     = wxColor(0x00, 0x00, 0x00);
    m_colors[DisplayItem_Default].backColor     = wxColor(0xFF, 0xFF, 0xFF);
    m_colors[DisplayItem_Default].bold          = false;
    m_colors[DisplayItem_Default].italic        = false;

    m_colors[DisplayItem_Comment].foreColor     = wxColor(0x00, 0x80, 0x00);
    m_colors[DisplayItem_Comment].backColor     = wxColor(0xFF, 0xFF, 0xFF);
    m_colors[DisplayItem_Comment].bold          = false;
    m_colors[DisplayItem_Comment].italic        = false;

    m_colors[DisplayItem_Keyword].foreColor     = wxColor(0x00, 0x00, 0xFF);
    m_colors[DisplayItem_Keyword].backColor     = wxColor(0xFF, 0xFF, 0xFF);
    m_colors[DisplayItem_Keyword].bold          = false;
    m_colors[DisplayItem_Keyword].italic        = false;
    
    m_colors[DisplayItem_Operator].foreColor    = wxColor(0x80, 0x80, 0x80);
    m_colors[DisplayItem_Operator].backColor    = wxColor(0xFF, 0xFF, 0xFF);
    m_colors[DisplayItem_Operator].bold         = false;
    m_colors[DisplayItem_Operator].italic       = false;

    m_colors[DisplayItem_String].foreColor      = wxColor(0xFF, 0x80, 0x80);
    m_colors[DisplayItem_String].backColor      = wxColor(0xFF, 0xFF, 0xFF);
    m_colors[DisplayItem_String].bold           = false;
    m_colors[DisplayItem_String].italic         = false;
    
    m_colors[DisplayItem_Number].foreColor      = wxColor(0xFF, 0x00, 0x00);
    m_colors[DisplayItem_Number].backColor      = wxColor(0xFF, 0xFF, 0xFF);
    m_colors[DisplayItem_Number].bold           = false;
    m_colors[DisplayItem_Number].italic         = false;

    m_colors[DisplayItem_Error].foreColor       = wxColor(0xFF, 0x00, 0x00);
    m_colors[DisplayItem_Error].backColor       = wxColor(0xFF, 0xFF, 0xFF);
    m_colors[DisplayItem_Error].bold            = true;
    m_colors[DisplayItem_Error].italic          = false;

    m_colors[DisplayItem_Warning].foreColor     = wxColor(0xFF, 0x00, 0x00);
    m_colors[DisplayItem_Warning].backColor     = wxColor(0xFF, 0xFF, 0xFF);
    m_colors[DisplayItem_Warning].bold          = false;
    m_colors[DisplayItem_Warning].italic        = false;

    m_colors[DisplayItem_Selection].foreColor   = wxColor(0xFF, 0xFF, 0xFF);
    m_colors[DisplayItem_Selection].backColor   = wxColor(0x0A, 0x24, 0x6A);
    m_colors[DisplayItem_Selection].bold        = false;
    m_colors[DisplayItem_Selection].italic      = false;

}

unsigned int FontColorSettings::GetNumDisplayItems() const
{
    return DisplayItem_Last;
}

const char* FontColorSettings::GetDisplayItemName(unsigned int i) const
{
    return s_displayItemName[i];
}

void FontColorSettings::SetFont(const wxFont& font)
{
    m_font = font;
}

const wxFont& FontColorSettings::GetFont() const
{
    return m_font;
}

wxFont FontColorSettings::GetFont(DisplayItem displayItem) const
{

    wxFont font = GetFont();

    if (m_colors[displayItem].bold)
    {
        font.SetWeight(wxFONTWEIGHT_BOLD);
    }

    if (m_colors[displayItem].italic)
    {
        font.SetStyle(wxFONTSTYLE_ITALIC);
    }

    return font;

}

const FontColorSettings::Colors& FontColorSettings::GetColors(DisplayItem displayItem) const
{
    return m_colors[displayItem];
}

void FontColorSettings::SetColors(DisplayItem displayItem, const Colors& colors)
{
    m_colors[displayItem] = colors;
}

wxXmlNode* FontColorSettings::Save(const wxString& tag) const
{
    wxXmlNode* root = new wxXmlNode(wxXML_ELEMENT_NODE, tag);    

    wxString fontDesc = m_font.GetNativeFontInfoDesc();
    root->AddChild( WriteXmlNode("font", fontDesc) );

    for (unsigned int i = 0; i < GetNumDisplayItems(); ++i)
    {
        wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "displayitem");    
        node->AddChild( WriteXmlNode("id", i) );
        node->AddChild( WriteXmlNode("foreColor", m_colors[i].foreColor) );
        node->AddChild( WriteXmlNode("backColor", m_colors[i].backColor) );
        node->AddChild( WriteXmlNodeBool("bold", m_colors[i].bold) );
        node->AddChild( WriteXmlNodeBool("italic", m_colors[i].italic) );
        root->AddChild(node);
    }

    return root;

}

void FontColorSettings::Load(wxXmlNode* root)
{

    wxXmlNode* node = root->GetChildren();
    while (node != NULL)
    {

        wxString data;

        if (ReadXmlNode(node, "font", data))
        {
            m_font.SetNativeFontInfo(data);
        }
        else if (node->GetName() == "displayitem")
        {

            Colors colors;
            unsigned int id = -1;

            wxXmlNode* child = node->GetChildren();    
            while (child != NULL)
            {

                ReadXmlNode(child, "id", id) ||
                ReadXmlNode(child, "foreColor", colors.foreColor) ||
                ReadXmlNode(child, "backColor", colors.backColor) ||
                ReadXmlNode(child, "bold",      colors.bold)      ||
                ReadXmlNode(child, "italic",    colors.italic);

                child = child->GetNext();

            }

            if (id != -1 && id < GetNumDisplayItems())
            {
                m_colors[id] = colors;
            }
        
        }

        node = node->GetNext();

    }

}