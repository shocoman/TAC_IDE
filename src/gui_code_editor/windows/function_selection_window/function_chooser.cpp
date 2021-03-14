///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 3.9.0 Feb  2 2021)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "function_chooser.h"

///////////////////////////////////////////////////////////////////////////



FunctionChooser::FunctionChooser( wxWindow* parent, wxArrayString function_names ) : FunctionChooser(parent) {
    m_chosen_function->Set(function_names);
    if (function_names.size() > 0)
        m_chosen_function->Select(0);
}

FunctionChooser::FunctionChooser( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );

	m_staticText1 = new wxStaticText( this, wxID_ANY, wxT("Choose function:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1->Wrap( -1 );
	bSizer1->Add( m_staticText1, 0, wxALL|wxEXPAND, 5 );

	wxArrayString m_chosen_functionChoices;
	m_chosen_function = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_chosen_functionChoices, 0 );
	m_chosen_function->SetSelection( 0 );
	bSizer1->Add( m_chosen_function, 0, wxALL|wxEXPAND, 5 );

	m_ok_btn = new wxButton( this, wxID_ANY, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer1->Add( m_ok_btn, 0, wxALL|wxEXPAND, 5 );


	this->SetSizer( bSizer1 );
	this->Layout();
	bSizer1->Fit( this );

	this->Centre( wxBOTH );

	// Connect Events
	m_ok_btn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( FunctionChooser::OnOkPressed ), NULL, this );
}

FunctionChooser::~FunctionChooser()
{
	// Disconnect Events
	m_ok_btn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( FunctionChooser::OnOkPressed ), NULL, this );

}

wxString FunctionChooser::get_selected_function_name() {
    return m_chosen_function->GetStringSelection();
}
