#include "userListWnd.h"
#include "database/databaseLayer.h"

class CUserListModel : public wxDataViewIndexListModel
{
public:
	
	CUserListModel() { }

	void Fill(const wxArrayString& strings)
	{
		m_strings = strings;

		Reset(m_strings.size());
	}

	// Implement base class pure virtual methods.
	unsigned GetColumnCount() const override { return 1; }
	wxString GetColumnType(unsigned) const override { return wxT("string"); }
	unsigned GetCount() const override { return m_strings.size(); }
	
	void GetValueByRow(wxVariant& val, unsigned row, unsigned) const override
	{
		val = m_strings[row];
	}
	
	bool SetValueByRow(const wxVariant&, unsigned, unsigned) override
	{
		return false;
	}

private:

	wxArrayString m_strings;
	wxDECLARE_NO_COPY_CLASS(CUserListModel);
};

userListWnd::userListWnd( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizerList = new wxBoxSizer( wxVERTICAL );

	m_auiToolBarUsers = new wxAuiToolBar( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_TB_HORZ_TEXT);
	m_auiToolBarUsers->Connect(wxEVT_MENU, wxCommandEventHandler(userListWnd::OnToolClicked), NULL, this);

	m_tool1 = m_auiToolBarUsers->AddTool( wxID_ANY, wxT("tool"), wxNullBitmap, wxNullBitmap, wxITEM_NORMAL, wxEmptyString, wxEmptyString, NULL );
	m_tool2 = m_auiToolBarUsers->AddTool( wxID_ANY, wxT("tool"), wxNullBitmap, wxNullBitmap, wxITEM_NORMAL, wxEmptyString, wxEmptyString, NULL );
	m_tool3 = m_auiToolBarUsers->AddTool( wxID_ANY, wxT("tool"), wxNullBitmap, wxNullBitmap, wxITEM_NORMAL, wxEmptyString, wxEmptyString, NULL );
	m_tool4 = m_auiToolBarUsers->AddTool( wxID_ANY, wxT("tool"), wxNullBitmap, wxNullBitmap, wxITEM_NORMAL, wxEmptyString, wxEmptyString, NULL );
	m_tool5 = m_auiToolBarUsers->AddTool( wxID_ANY, wxT("tool"), wxNullBitmap, wxNullBitmap, wxITEM_NORMAL, wxEmptyString, wxEmptyString, NULL );
	m_auiToolBarUsers->Realize();

	bSizerList->Add( m_auiToolBarUsers, 0, wxALL|wxEXPAND, 5 );

	wxDataViewTextRenderer *m_textRenderer = new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_INERT);
	wxDataViewColumn *m_column = new wxDataViewColumn("user", m_textRenderer, 0, FromDIP(200), wxALIGN_LEFT,
			wxDATAVIEW_COL_SORTABLE);

	m_dataViewUsers = new wxDataViewCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	m_dataViewUsers->Connect(wxEVT_RIGHT_DCLICK, wxMouseEventHandler(userListWnd::OnDataViewUsersOnRightDClick), NULL, this);

	m_dataViewUsers->AppendColumn(m_column);

	wxArrayString m_strings;
	m_strings.Add("oiuytrewq");
	m_strings.Add("tdsaASDFG");

	CUserListModel *m_model = new CUserListModel();
	m_model->Fill(m_strings);

	m_dataViewUsers->AssociateModel(m_model);

	bSizerList->Add( m_dataViewUsers, 1, wxALL|wxEXPAND, 5 );

	this->SetSizer( bSizerList );
	this->Layout();
	this->Centre( wxBOTH );
}

userListWnd::~userListWnd()
{
}
