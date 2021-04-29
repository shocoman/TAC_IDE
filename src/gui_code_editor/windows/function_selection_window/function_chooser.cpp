#include "function_chooser.h"

FunctionChooser::FunctionChooser(wxWindow *parent, wxArrayString function_names) : FunctionChooser(parent) {
    m_chosen_function->Set(function_names);
    if (function_names.size() > 0)
        m_chosen_function->Select(0);
}

FunctionChooser::FunctionChooser(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos,
                                 const wxSize &size, long style)
    : wxDialog(parent, id, title, pos, size, style) {
    this->SetSizeHints(wxDefaultSize, wxDefaultSize);

    wxBoxSizer *bSizer1;
    bSizer1 = new wxBoxSizer(wxVERTICAL);

    m_staticText1 = new wxStaticText(this, wxID_ANY, wxT("Выберите функцию:"), wxDefaultPosition, wxDefaultSize, 0);
    m_staticText1->Wrap(-1);
    bSizer1->Add(m_staticText1, 0, wxALL | wxEXPAND, 5);

    wxArrayString m_chosen_functionChoices;
    m_chosen_function = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_chosen_functionChoices, 0);
    m_chosen_function->SetSelection(0);
    bSizer1->Add(m_chosen_function, 0, wxALL | wxEXPAND, 5);

    m_ok_btn = new wxButton(this, wxID_ANY, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0);
    bSizer1->Add(m_ok_btn, 0, wxALL | wxEXPAND, 5);

    this->SetSizer(bSizer1);
    this->Layout();
//    bSizer1->Fit(this);

    this->Centre(wxBOTH);

    m_ok_btn->Bind(wxEVT_BUTTON, [&](wxCommandEvent &event) {
        if (m_chosen_function->GetSelection() < m_chosen_function->GetCount())
            EndModal(wxID_OK);
        else
            EndModal(wxID_CANCEL);
    });
}

wxString FunctionChooser::get_selected_function_name() { return m_chosen_function->GetStringSelection(); }

