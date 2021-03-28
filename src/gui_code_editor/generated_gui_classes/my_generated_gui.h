///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 3.9.0 Feb  2 2021)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include "windows/image_viewer/image_panel.hpp"
#include "windows/optimization_window/toy_ir/my_html_window.hpp"
#include <wx/panel.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/scrolwin.h>
#include <wx/statline.h>
#include <wx/button.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/dialog.h>
#include <wx/html/htmlwin.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class OptimizationChooseDialog
///////////////////////////////////////////////////////////////////////////////
class OptimizationChooseDialog : public wxDialog
{
	private:

	protected:
		ImagePanel* m_graph_panel;
		wxScrolledWindow* m_scrolledWindow2;
		wxBoxSizer* m_analysis_sizer;
		wxScrolledWindow* m_scrolledWindow1;
		wxBoxSizer* m_optimization_sizer;
		wxStaticLine* m_staticline1;
		wxButton* m_back_to_code_btn;

	public:

		OptimizationChooseDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Optimization Choose Window"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 975,760 ), long style = wxDEFAULT_DIALOG_STYLE );
		~OptimizationChooseDialog();

};

///////////////////////////////////////////////////////////////////////////////
/// Class OptimizationShowDialog
///////////////////////////////////////////////////////////////////////////////
class OptimizationShowDialog : public wxDialog
{
	private:

	protected:
		MyHTMLWindow* m_htmlWin2;

	public:
		wxButton* m_accept_optimization;

		OptimizationShowDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Окно демонстрации оптимизации"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 640,480 ), long style = wxDEFAULT_DIALOG_STYLE );
		~OptimizationShowDialog();

};

