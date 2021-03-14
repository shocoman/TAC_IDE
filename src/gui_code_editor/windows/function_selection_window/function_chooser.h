///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 3.9.0 Feb  2 2021)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/sizer.h>
#include <wx/dialog.h>

#include <iostream>
#include <vector>
#include <string>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class FunctionChooser
///////////////////////////////////////////////////////////////////////////////
class FunctionChooser : public wxDialog
{
	private:

	protected:
		wxStaticText* m_staticText1;
		wxChoice* m_chosen_function;
		wxButton* m_ok_btn;

		// Virtual event handlers, override them in your derived class
		void OnOkPressed( wxCommandEvent& event ) {
                     EndModal(wxID_OK);
                }


	public:

		FunctionChooser( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Function Chooser"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE );
		FunctionChooser( wxWindow* parent, wxArrayString function_names );
		~FunctionChooser();

                wxString get_selected_function_name();

};

