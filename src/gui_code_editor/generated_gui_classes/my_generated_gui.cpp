///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct 26 2018)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "my_generated_gui.h"

///////////////////////////////////////////////////////////////////////////

OptimizationChooseDialog::OptimizationChooseDialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxHORIZONTAL );

	m_graph_panel = new ImagePanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	bSizer2->Add( m_graph_panel, 3, wxALL|wxEXPAND, 5 );

	optimizationChoosePanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer( wxVERTICAL );

	m_convert_to_ssa_btn = new wxButton( optimizationChoosePanel, wxID_ANY, wxT("Перевести в SSA форму"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer5->Add( m_convert_to_ssa_btn, 0, wxALL|wxEXPAND, 5 );

	m_convert_from_ssa_btn = new wxButton( optimizationChoosePanel, wxID_ANY, wxT("Вывести из SSA формы"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer5->Add( m_convert_from_ssa_btn, 0, wxALL|wxEXPAND, 5 );

	m_display_dfs_tree_btn = new wxButton( optimizationChoosePanel, wxID_ANY, wxT("Показать остовное дерево"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer5->Add( m_display_dfs_tree_btn, 0, wxALL|wxEXPAND, 5 );

	m_etc_btn = new wxButton( optimizationChoosePanel, wxID_ANY, wxT("..."), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer5->Add( m_etc_btn, 0, wxALL|wxEXPAND, 5 );


	optimizationChoosePanel->SetSizer( bSizer5 );
	optimizationChoosePanel->Layout();
	bSizer5->Fit( optimizationChoosePanel );
	bSizer2->Add( optimizationChoosePanel, 1, wxEXPAND|wxALL, 5 );


	this->SetSizer( bSizer2 );
	this->Layout();

	this->Centre( wxBOTH );
}

OptimizationChooseDialog::~OptimizationChooseDialog()
{
}

OptimizationShowDialog::OptimizationShowDialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer( wxVERTICAL );

	m_htmlWin2 = new MyHTMLWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHW_SCROLLBAR_AUTO );
	bSizer8->Add( m_htmlWin2, 2, wxALL|wxEXPAND, 5 );

	m_button12 = new wxButton( this, wxID_ANY, wxT("Применить оптимизацию"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer8->Add( m_button12, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );


	this->SetSizer( bSizer8 );
	this->Layout();

	this->Centre( wxBOTH );
}

OptimizationShowDialog::~OptimizationShowDialog()
{
}
