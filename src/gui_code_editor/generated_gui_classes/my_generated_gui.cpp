///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 3.9.0 Feb  2 2021)
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

	m_cfg_sizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Граф потока управления") ), wxVERTICAL );

	m_graph_panel = new ImagePanel( m_cfg_sizer->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_cfg_sizer->Add( m_graph_panel, 3, wxALL|wxEXPAND, 5 );


	bSizer2->Add( m_cfg_sizer, 5, wxEXPAND, 1 );

	wxBoxSizer* bSizer51;
	bSizer51 = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer* sbSizer4;
	sbSizer4 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Анализ") ), wxVERTICAL );

	m_scrolledWindow2 = new wxScrolledWindow( sbSizer4->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
	m_scrolledWindow2->SetScrollRate( 5, 5 );
	m_analysis_sizer = new wxBoxSizer( wxVERTICAL );


	m_scrolledWindow2->SetSizer( m_analysis_sizer );
	m_scrolledWindow2->Layout();
	m_analysis_sizer->Fit( m_scrolledWindow2 );
	sbSizer4->Add( m_scrolledWindow2, 1, wxEXPAND | wxALL, 5 );


	bSizer51->Add( sbSizer4, 1, wxEXPAND, 5 );

	wxStaticBoxSizer* sbSizer5;
	sbSizer5 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Оптимизация") ), wxVERTICAL );

	m_scrolledWindow1 = new wxScrolledWindow( sbSizer5->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
	m_scrolledWindow1->SetScrollRate( 5, 5 );
	m_optimization_sizer = new wxBoxSizer( wxVERTICAL );


	m_scrolledWindow1->SetSizer( m_optimization_sizer );
	m_scrolledWindow1->Layout();
	m_optimization_sizer->Fit( m_scrolledWindow1 );
	sbSizer5->Add( m_scrolledWindow1, 1, wxEXPAND | wxALL, 5 );


	bSizer51->Add( sbSizer5, 2, wxEXPAND, 5 );

	m_staticline1 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer51->Add( m_staticline1, 0, wxEXPAND | wxALL, 5 );

	m_back_to_code_btn = new wxButton( this, wxID_ANY, wxT("Сохранить и вернуться к коду"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer51->Add( m_back_to_code_btn, 0, wxALL|wxEXPAND, 5 );


	bSizer2->Add( bSizer51, 3, wxEXPAND, 5 );


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

	m_accept_optimization = new wxButton( this, wxID_ANY, wxT("Применить оптимизацию"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer8->Add( m_accept_optimization, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );


	this->SetSizer( bSizer8 );
	this->Layout();

	this->Centre( wxBOTH );
}

OptimizationShowDialog::~OptimizationShowDialog()
{
}
