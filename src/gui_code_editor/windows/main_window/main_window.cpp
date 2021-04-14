
#include <windows/function_selection_window/function_chooser.h>
#include <wx/artprov.h>
#include <wx/filename.h>
#include <wx/menu.h>

#include "main_window.h"

// helper functions
enum wxbuildinfoformat { short_f, long_f };

wxString wxbuildinfo(wxbuildinfoformat format) {
    wxString wxbuild(wxVERSION_STRING);

    if (format == long_f) {
#if defined(__WXMSW__)
        wxbuild << _T("-Windows");
#elif defined(__WXMAC__)
        wxbuild << _T("-Mac");
#elif defined(__UNIX__)
        wxbuild << _T("-Linux");
#endif

#if wxUSE_UNICODE
        wxbuild << _T("-Unicode build");
#else
        wxbuild << _T("-ANSI build");
#endif // wxUSE_UNICODE
    }

    return wxbuild;
}

BEGIN_EVENT_TABLE(MainWindowFrame, wxFrame)
// common
EVT_CLOSE(MainWindowFrame::OnClose)
EVT_MENU(myID_TOOLBAR_TOGGLE, MainWindowFrame::OnMainToolbarToggle)
// file
EVT_MENU(wxID_NEW, MainWindowFrame::OnFileNew)
EVT_MENU(wxID_OPEN, MainWindowFrame::OnFileOpen)
EVT_MENU(wxID_SAVE, MainWindowFrame::OnFileSave)
EVT_MENU(wxID_SAVEAS, MainWindowFrame::OnFileSaveAs)
EVT_MENU(wxID_CLOSE, MainWindowFrame::OnFileClose)
// properties
EVT_MENU(myID_PROPERTIES, MainWindowFrame::OnProperties)
// exit
EVT_MENU(idMenuQuit, MainWindowFrame::OnExit)
EVT_MENU(wxID_EXIT, MainWindowFrame::OnExit)
// Menu items with standard IDs forwarded to the editor.
EVT_MENU(wxID_CLEAR, MainWindowFrame::OnEdit)
EVT_MENU(wxID_CUT, MainWindowFrame::OnEdit)
EVT_MENU(wxID_COPY, MainWindowFrame::OnEdit)
EVT_MENU(wxID_PASTE, MainWindowFrame::OnEdit)
EVT_MENU(wxID_SELECTALL, MainWindowFrame::OnEdit)
EVT_MENU(wxID_REDO, MainWindowFrame::OnEdit)
EVT_MENU(wxID_UNDO, MainWindowFrame::OnEdit)
EVT_MENU(wxID_FIND, MainWindowFrame::OnEdit)
// And all our edit-related menu commands.
EVT_MENU_RANGE(myID_EDIT_FIRST, myID_EDIT_LAST, MainWindowFrame::OnEdit)
// target platform change
// EVT_MENU (myID_X86,              MainWindowFrame::OnTargetPlatformChange)
// EVT_MENU (myID_ARM,              MainWindowFrame::OnTargetPlatformChange)
// EVT_MENU (myID_MIPS32,           MainWindowFrame::OnTargetPlatformChange)
// symbol table
// EVT_MENU (myID_SYMBOL_TABLE,     MainWindowFrame::OnSymbolTable)
// simulator run
EVT_MENU(myID_SIMULATOR_RUN, MainWindowFrame::OnSimulatorRun)
EVT_MENU(myID_OPTIMIZATION_WINDOW, MainWindowFrame::OnOptimizationWindow)
EVT_MENU(myID_PRINT_CFG_WINDOW, MainWindowFrame::OnDisplayCFG)
// educational mode
EVT_MENU(myID_EDU_TOGGLE, MainWindowFrame::OnEduToggle)
EVT_MENU(myID_EDU_HOME, MainWindowFrame::OnEduPageChange)
EVT_MENU(myID_EDU_PREV, MainWindowFrame::OnEduPageChange)
EVT_MENU(myID_EDU_NEXT, MainWindowFrame::OnEduPageChange)
// options
// EVT_MENU (myID_OPTIONS,          MainWindowFrame::OnOptions)
// pages
// EVT_MENU (myID_PAGENEXT,         MainWindowFrame::OnPageNext)
// EVT_MENU (myID_PAGEPREV,         MainWindowFrame::OnPagePrev)
// help
// EVT_MENU (wxID_HELP_CONTENTS,    MainWindowFrame::OnHelpContents)
EVT_MENU(wxID_ABOUT, MainWindowFrame::OnAbout)
// editor
EVT_STC_MODIFIED(wxID_ANY, MainWindowFrame::OnModified)
END_EVENT_TABLE()

MainWindowFrame::MainWindowFrame(wxFrame *frame, const wxString &title)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(790, 550),
              wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE) {
    // create a menu bar
    m_menuBar = new wxMenuBar;
    CreateMenu();

    // main sizer of the application layout
    m_vbox = new wxBoxSizer(wxVERTICAL);

#if wxUSE_TOOLBAR
    // create a tool bar with some frequently used buttons
    m_toolBar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL | wxTB_TEXT);
    // create a tool bar with educational mode buttons
    m_edu_toolBar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL | wxTB_TEXT);
    CreateToolbar();

    m_vbox->Add(m_toolBar, 0, wxEXPAND);
    m_vbox->Add(m_edu_toolBar, 0, wxEXPAND);

    if (m_menuBar->IsChecked(myID_TOOLBAR_TOGGLE))
        m_vbox->Show(m_toolBar);
    else
        m_vbox->Hide(m_toolBar);
    if (m_menuBar->IsChecked(myID_EDU_TOGGLE))
        m_vbox->Show(m_edu_toolBar);
    else
        m_vbox->Hide(m_edu_toolBar);

#endif // wxUSE_TOOLBAR

    m_notebook = new wxNotebook(this, myID_NOTEBOOK, wxDefaultPosition, wxDefaultSize, 0, _T("myID_NOTEBOOK"));

    // editor window
    auto *editorPanel =
        new wxPanel(m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER);
    auto *editorSizer = new wxBoxSizer(wxVERTICAL);

    m_editor = new EditorCtrl(editorPanel, myID_EDITOR, wxDefaultPosition, wxDefaultSize);
    editorSizer->Add(m_editor, 1, wxALL | wxEXPAND, 0);
    editorPanel->SetSizer(editorSizer);

    nonameFileCounter = 1;
    m_notebook->AddPage(editorPanel, NONAME + wxString() << nonameFileCounter++, true);

    m_vbox->Add(m_notebook, 1, wxEXPAND, 0);

    SetSizer(m_vbox);
    m_vbox->Layout();

    CreateStatusBar(2);
    SetStatusText(_("Welcome to the Three Address Code IDE!"), 0);
    SetStatusText(wxEmptyString, 1); // wxbuildinfo(short_f), 1);

    Centre();
}

MainWindowFrame::~MainWindowFrame() = default;

// common event handlers
void MainWindowFrame::OnClose(wxCloseEvent &event) {
    wxCommandEvent evt;
    OnFileClose(evt);
    if (m_editor && m_editor->IsModified()) {
        if (event.CanVeto())
            event.Veto(true);
        return;
    }
    Destroy();
}

void MainWindowFrame::OnQuit(wxCommandEvent &event) { Destroy(); }

void MainWindowFrame::OnExit(wxCommandEvent &WXUNUSED(event)) { Close(true); }

void MainWindowFrame::OnAbout(wxCommandEvent &event) {
    wxString msg = wxbuildinfo(long_f);
    wxMessageBox(msg, _("Welcome to..."));
}

void MainWindowFrame::OnMainToolbarToggle(wxCommandEvent &WXUNUSED(event)) {
    if (m_menuBar->IsChecked(myID_TOOLBAR_TOGGLE))
        m_vbox->Show(m_toolBar);
    else
        m_vbox->Hide(m_toolBar);
    m_vbox->Layout();
}

// file event handlers
void MainWindowFrame::OnFileNew(wxCommandEvent &event) { m_editor->SelectNone(); }

void MainWindowFrame::OnFileOpen(wxCommandEvent &WXUNUSED(event)) {
    if (!m_editor)
        return;
    wxFileDialog dlg(this, wxT("Open file"), wxEmptyString, wxEmptyString,
                     //                     wxT("Three Address Code file(*.3ac)|*.3ac|Any file (*)|*"),
                     wxT("Any file (*)|*"), wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_CHANGE_DIR);
    if (dlg.ShowModal() != wxID_OK)
        return;
    wxString fname = dlg.GetPath();
    FileOpen(fname);
}

void MainWindowFrame::OnFileSave(wxCommandEvent &WXUNUSED(event)) {
    if (!m_editor)
        return;
    if (!m_editor->Modified()) {
        wxMessageBox(_("There is nothing to save!"), _("Save file"), wxOK | wxICON_EXCLAMATION);
        return;
    }
    m_editor->SaveFile();
    wxFileName w(m_editor->GetFilename());
    w.Normalize();
    m_notebook->SetPageText(m_notebook->GetSelection(), w.GetName());
}

void MainWindowFrame::OnFileSaveAs(wxCommandEvent &WXUNUSED(event)) {
    if (!m_editor)
        return;
    wxFileDialog dlg(this, wxT("Save file"), wxEmptyString, wxEmptyString,
                     wxT("Three Address Code file(*.3ac)|*.3ac|Any file (*)|*"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() != wxID_OK)
        return;
    wxString filename = dlg.GetPath();
    m_editor->SaveFile(filename, true);
    wxFileName w(filename);
    w.Normalize();
    m_notebook->SetPageText(m_notebook->GetSelection(), w.GetName());
}

void MainWindowFrame::OnFileClose(wxCommandEvent &WXUNUSED(event)) {
    if (!m_editor)
        return;
    if (m_editor->Modified()) {
        if (wxMessageBox(_("Source file is not saved, save before closing?"), _("Close"), wxYES_NO | wxICON_QUESTION) ==
            wxYES) {
            m_editor->SaveFile();
            if (m_editor->Modified()) {
                wxMessageBox(_("Text could not be saved!"), _("Close abort"), wxOK | wxICON_EXCLAMATION);
                return;
            }
        }
    }
    m_editor->SetFilename(wxEmptyString);
    m_editor->Clear();
    m_editor->DiscardEdits();
    m_editor->ClearAll();
    m_editor->SetSavePoint();
    m_notebook->SetPageText(m_notebook->GetSelection(), NONAME + wxString() << nonameFileCounter++);
}

// properties event handlers
void MainWindowFrame::OnProperties(wxCommandEvent &WXUNUSED(event)) {
    if (!m_editor)
        return;
    EditorCtrlProperties dlg(m_editor, 0);
}

// edit events
void MainWindowFrame::OnEdit(wxCommandEvent &event) {
    if (m_editor)
        m_editor->GetEventHandler()->ProcessEvent(event);
}

void MainWindowFrame::OnModified(wxStyledTextEvent &WXUNUSED(event)) {
    if (!m_editor->Modified())
        return;
    wxFileName w(m_editor->GetFilename());
    w.Normalize();
    m_notebook->SetPageText(m_notebook->GetSelection(), wxT("*") + w.GetName());
}

// entity event
void MainWindowFrame::OnEduToggle(wxCommandEvent &event) {
    OnFileClose(event);
    if (m_menuBar->IsChecked(myID_EDU_TOGGLE)) {
        m_vbox->Show(m_edu_toolBar);
        m_notebook->SetPageText(m_notebook->GetSelection(), m_editor->GetFilename());
    } else
        m_vbox->Hide(m_edu_toolBar);
    m_vbox->Layout();
    if (m_editor)
        m_editor->GetEventHandler()->ProcessEvent(event);
}

void MainWindowFrame::OnEduPageChange(wxCommandEvent &event) {
    m_notebook->SetPageText(m_notebook->GetSelection(), m_editor->GetFilename());
}

// private functions
void MainWindowFrame::CreateMenu() {
    // File menu
    wxMenu *menuFile = new wxMenu;
    menuFile->Append(wxID_NEW, _("&New ..\tCtrl+N"));
    menuFile->Append(wxID_OPEN, _("&Open ..\tCtrl+O"));
    menuFile->Append(wxID_SAVE, _("&Save\tCtrl+S"));
    menuFile->Append(wxID_SAVEAS, _("Save &as ..\tCtrl+Shift+S"));
    menuFile->Append(wxID_CLOSE, _("&Close\tCtrl+W"));
    menuFile->AppendSeparator();
    menuFile->Append(myID_PROPERTIES, _("Proper&ties ..\tCtrl+I"));
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT, _("&Quit\tCtrl+Q"));

    // Edit menu
    wxMenu *menuEdit = new wxMenu;
    menuEdit->Append(wxID_UNDO, _("&Undo\tCtrl+Z"));
    menuEdit->Append(wxID_REDO, _("&Redo\tCtrl+Shift+Z"));
    menuEdit->AppendSeparator();
    menuEdit->Append(wxID_CUT, _("Cu&t\tCtrl+X"));
    menuEdit->Append(wxID_COPY, _("&Copy\tCtrl+C"));
    menuEdit->Append(wxID_PASTE, _("&Paste\tCtrl+V"));
    menuEdit->Append(wxID_CLEAR, _("&Delete\tDel"));
    menuEdit->AppendSeparator();
    menuEdit->Append(wxID_FIND, _("&Find\tCtrl+F"));
    menuEdit->Enable(wxID_FIND, false);
    menuEdit->Append(myID_FINDNEXT, _("Find &next\tF3"));
    menuEdit->Enable(myID_FINDNEXT, false);
    menuEdit->Append(myID_REPLACE, _("&Replace\tCtrl+H"));
    menuEdit->Enable(myID_REPLACE, false);
    menuEdit->Append(myID_REPLACENEXT, _("Replace &again\tShift+F4"));
    menuEdit->Enable(myID_REPLACENEXT, false);
    menuEdit->AppendSeparator();
    menuEdit->Append(myID_GOTO, _("&Goto\tCtrl+G"));
    menuEdit->Enable(myID_GOTO, false);
    menuEdit->AppendSeparator();
    menuEdit->Append(wxID_SELECTALL, _("&Select all\tCtrl+A"));
    menuEdit->Append(myID_SELECTLINE, _("Select &line\tCtrl+L"));

    // View menu
    wxMenu *menuView = new wxMenu;
    menuView->AppendCheckItem(myID_TOOLBAR_TOGGLE, _("Tool&bar"));
    menuView->AppendSeparator();
    menuView->AppendCheckItem(myID_OVERTYPE, _("&Overwrite mode\tIns"));
    menuView->AppendCheckItem(myID_WRAPMODEON, _("&Wrap mode\tCtrl+U"));
    menuView->AppendSeparator();
    menuView->AppendCheckItem(myID_DISPLAYEOL, _("Show line &endings"));
    menuView->AppendCheckItem(myID_LINENUMBER, _("Show line &numbers"));
    menuView->AppendCheckItem(myID_LONGLINEON, _("Show &long line marker"));
    menuView->AppendCheckItem(myID_WHITESPACE, _("Show white&space"));
    menuView->AppendSeparator();
    menuView->AppendCheckItem(myID_BB, _("Show &basic blocks"));

    // target platform submenu
    wxMenu *menuTargetPlatform = new wxMenu;
    menuTargetPlatform->AppendRadioItem(myID_X86, _("x86"));
    menuTargetPlatform->AppendRadioItem(myID_ARM, _("Arm"));
    menuTargetPlatform->AppendRadioItem(myID_MIPS32, _("MIPS32"));

    // Tac dialect submenu
    wxMenu *menuTacDialect = new wxMenu;
    menuTacDialect->AppendRadioItem(myID_TOY_DIALECT, _("Toy"));
    menuTacDialect->AppendRadioItem(myID_LLVMIR_DIALECT, _("LLVM IR"));
    //    menuTacDialect->Bind(wxEVT_MENU, [](wxCommandEvent & evt) {
    //      wxMessageBox("Toy dialect selected", "Your selection", wxOK | wxICON_INFORMATION);
    //    }, myID_TOY_DIALECT);
    //    menuTacDialect->Bind(wxEVT_MENU, [](wxCommandEvent & evt) {
    //      wxMessageBox("LLVM IR selected", "Your selection", wxOK | wxICON_INFORMATION);
    //    }, myID_LLVMIR_DIALECT);

    // Project menu
    wxMenu *menuProject = new wxMenu;
    menuProject->AppendSubMenu(menuTargetPlatform, _("Target platform"));
    menuProject->AppendSubMenu(menuTacDialect, _("TAC dialect"));
    menuProject->Append(myID_OPTIMIZATION_WINDOW, _("Optimization"));
    menuProject->Append(myID_PRINT_CFG_WINDOW, _("Print CFG"));
    menuProject->AppendSeparator();
    menuProject->Append(myID_SYMBOL_TABLE, _("Edit &symbol table.."));
    menuProject->AppendSeparator();

    // Examples submenu
    if (wxDir::Exists(wxT("../_TestCode"))) {
        wxMenu *filesExamplesMenu = new wxMenu;
        wxArrayString txtFiles;
        wxDir::GetAllFiles(wxT("../_TestCode"), &txtFiles, _T("*.txt"), wxDIR_FILES);
        for (int i = 0; i < txtFiles.size(); ++i)
            filesExamplesMenu->Append(i, wxFileName(txtFiles[i]).GetName(), txtFiles[i]);
        filesExamplesMenu->Bind(wxEVT_MENU,
                                [txtFiles, this](wxCommandEvent &event) { FileOpen(txtFiles[event.GetId()]); });
        menuProject->AppendSubMenu(filesExamplesMenu, wxT("Examples"));
    }

    // Simulator menu
    wxMenu *menuSimulator = new wxMenu;
    menuSimulator->Append(myID_SIMULATOR_RUN, _("&Run in simulator.."));

    // Tools menu
    wxMenu *menuTools = new wxMenu;
    menuTools->AppendCheckItem(myID_EDU_TOGGLE, _("&Toggle educational mode\tCtrl+T"));
    menuTools->AppendSeparator();
    menuTools->Append(myID_OPTIONS, _("&Options"));

    // Window menu
    wxMenu *menuWindow = new wxMenu;
    menuWindow->Append(myID_PAGEPREV, _("&Previous\tCtrl+Shift+Tab"));
    menuWindow->Append(myID_PAGENEXT, _("&Next\tCtrl+Tab"));

    // Help menu
    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(wxID_HELP_CONTENTS, _("Help &Contents"));
    menuHelp->Append(wxID_ABOUT, _("&About ..\tCtrl+D"));

    // construct menu
    m_menuBar->Append(menuFile, _("&File"));
    m_menuBar->Append(menuEdit, _("&Edit"));
    m_menuBar->Append(menuView, _("&View"));
    m_menuBar->Append(menuProject, _("&Project"));
    m_menuBar->Append(menuSimulator, _("&Simulator"));
    m_menuBar->Append(menuTools, _("&Tools"));
    m_menuBar->Append(menuWindow, _("&Window"));
    m_menuBar->Append(menuHelp, _("&Help"));
    SetMenuBar(m_menuBar);

    m_menuBar->Check(myID_ARM, true);
    m_menuBar->Check(myID_TOOLBAR_TOGGLE, false);
    m_menuBar->Check(myID_BB, true);
    m_menuBar->Check(myID_EDU_TOGGLE, false);
}

void MainWindowFrame::CreateToolbar() {
    wxBitmap newb(wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_NEW")), wxART_TOOLBAR));
    wxBitmap openb(wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_FILE_OPEN")), wxART_TOOLBAR));
    wxBitmap saveb(wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_FILE_SAVE")), wxART_TOOLBAR));
    wxBitmap undob(wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_UNDO")), wxART_TOOLBAR));
    wxBitmap redob(wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_REDO")), wxART_TOOLBAR));
    wxBitmap cutb(wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_CUT")), wxART_TOOLBAR));
    wxBitmap copyb(wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_COPY")), wxART_TOOLBAR));
    wxBitmap pasteb(wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_PASTE")), wxART_TOOLBAR));
    wxBitmap findb(wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_FIND")), wxART_TOOLBAR));
    wxBitmap replaceb(
        wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_FIND_AND_REPLACE")), wxART_TOOLBAR));
    wxBitmap symtableb(wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_REPORT_VIEW")), wxART_TOOLBAR));
    wxBitmap targetb(wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_TICK_MARK")), wxART_TOOLBAR));
    wxBitmap runb(wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_EXECUTABLE_FILE")), wxART_TOOLBAR));
    wxBitmap optionsb(wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_LIST_VIEW")), wxART_TOOLBAR));
    wxBitmap questionb(wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_QUESTION")), wxART_TOOLBAR));

    wxBitmap homeb(wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_GO_HOME")), wxART_TOOLBAR));
    wxBitmap prevb(wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_GO_BACK")), wxART_TOOLBAR));
    wxBitmap nextb(wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_GO_FORWARD")), wxART_TOOLBAR));

    m_toolBar->AddTool(wxID_NEW, wxT("New"), newb, newb, wxITEM_NORMAL, wxT("Create new source file"));
    m_toolBar->AddTool(wxID_OPEN, wxT("Open"), openb, openb, wxITEM_NORMAL, wxT("Open source file"));
    m_toolBar->AddTool(wxID_SAVE, wxT("Save"), saveb, saveb, wxITEM_NORMAL, wxT("Save source file"));
    m_toolBar->AddSeparator();
    m_toolBar->AddTool(wxID_UNDO, wxT("Undo"), undob, undob, wxITEM_NORMAL, wxT("Undo"));
    m_toolBar->AddTool(wxID_REDO, wxT("Redo"), redob, redob, wxITEM_NORMAL, wxT("Redo"));
    m_toolBar->AddSeparator();
    m_toolBar->AddTool(wxID_CUT, wxT("Cut"), cutb, cutb, wxITEM_NORMAL, wxT("Cut"));
    m_toolBar->AddTool(wxID_COPY, wxT("Copy"), copyb, copyb, wxITEM_NORMAL, wxT("Copy"));
    m_toolBar->AddTool(wxID_PASTE, wxT("Paste"), pasteb, pasteb, wxITEM_NORMAL, wxT("Paste"));
    m_toolBar->AddSeparator();
    m_toolBar->AddTool(wxID_FIND, wxT("Find"), findb, findb, wxITEM_NORMAL, wxT("Find"));
    m_toolBar->AddTool(wxID_REPLACE, wxT("Replace"), replaceb, replaceb, wxITEM_NORMAL, wxT("Replace"));
    m_toolBar->AddSeparator();
    m_toolBar->AddTool(myID_SYMBOL_TABLE, wxT("Symbol Table"), symtableb, symtableb, wxITEM_NORMAL,
                       wxT("Edit symbol table.."));
    m_toolBar->AddSeparator();
    m_toolBar->AddTool(myID_TARGET_PLATFORM, wxT("Target Platform"), targetb, targetb, wxITEM_DROPDOWN,
                       wxT("Choose target platform"));

    m_toolBar->AddSeparator();
    m_toolBar->AddTool(myID_SIMULATOR_RUN, wxT("Run"), runb, runb, wxITEM_NORMAL, wxT("Run in Simulator.."));
    m_toolBar->AddSeparator();
    m_toolBar->AddTool(myID_OPTIONS, wxT("Options"), optionsb, optionsb, wxITEM_NORMAL, wxT("Open options dialog.."));
    m_toolBar->AddSeparator();
    m_toolBar->AddTool(wxID_HELP_CONTENTS, wxT("Help"), questionb, questionb, wxITEM_NORMAL,
                       wxT("Open help Contents.."));

    m_toolBar->Realize();

    m_edu_toolBar->AddTool(myID_EDU_HOME, wxT("First page"), homeb, homeb, wxITEM_NORMAL,
                           wxT("Show first educational page"));
    m_edu_toolBar->AddTool(myID_EDU_PREV, wxT("Previous page"), prevb, prevb, wxITEM_NORMAL,
                           wxT("Show previous educational page"));
    m_edu_toolBar->AddTool(myID_EDU_NEXT, wxT("Next page"), nextb, nextb, wxITEM_NORMAL,
                           wxT("Show next educational page"));
    m_edu_toolBar->AddTool(myID_EDU_RESET, wxT("Reset page"), undob, undob, wxITEM_NORMAL,
                           wxT("Reset educational page"));

    m_edu_toolBar->Realize();

    Connect(wxID_NEW, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainWindowFrame::OnFileNew));
    Connect(wxID_OPEN, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainWindowFrame::OnFileOpen));
    Connect(wxID_SAVE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainWindowFrame::OnFileSave));
    Connect(wxID_UNDO, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainWindowFrame::OnEdit));
    Connect(wxID_REDO, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainWindowFrame::OnEdit));
    Connect(wxID_CUT, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainWindowFrame::OnEdit));
    Connect(wxID_COPY, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainWindowFrame::OnEdit));
    Connect(wxID_PASTE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainWindowFrame::OnEdit));

    //    Connect(wxID_FIND, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainWindowFrame::OnEdit));
    //    Connect(wxID_REPLACE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainWindowFrame::OnEdit));
    //    Connect(myID_SYMBOL_TABLE, wxEVT_COMMAND_TOOL_CLICKED,
    //    wxCommandEventHandler(MainWindowFrame::OnSymbolTable)); Connect(myID_TARGET_PLATFORM,
    //    wxEVT_COMMAND_TOOL_CLICKED,
    //            wxCommandEventHandler(MainWindowFrame::OnTargetPlatformChange));
    //    Connect(myID_SIMULATOR_RUN, wxEVT_COMMAND_TOOL_CLICKED,
    //            wxCommandEventHandler(MainWindowFrame::OnSimulatorRun));
    //    Connect(myID_OPTIONS, wxEVT_COMMAND_TOOL_CLICKED,
    //    wxCommandEventHandler(MainWindowFrame::OnOptions)); Connect(wxID_HELP_CONTENTS,
    //    wxEVT_COMMAND_TOOL_CLICKED,
    //            wxCommandEventHandler(MainWindowFrame::OnHelpContents));

    Connect(myID_EDU_HOME, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainWindowFrame::OnEdit));
    Connect(myID_EDU_PREV, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainWindowFrame::OnEdit));
    Connect(myID_EDU_NEXT, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainWindowFrame::OnEdit));
    Connect(myID_EDU_RESET, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainWindowFrame::OnEdit));
}

void MainWindowFrame::FileOpen(wxString fname) {
    wxFileName w(fname);
    w.Normalize();
    fname = w.GetFullPath();
    m_editor->LoadFile(fname);
    m_editor->SelectNone();
    m_notebook->SetPageText(m_notebook->GetSelection(), w.GetName());
}

void MainWindowFrame::OnSimulatorRun(wxCommandEvent &event) {
    wxString program_file = "_test.txt";
    m_editor->SaveFile(program_file, false);
    wxFileName f(wxStandardPaths::Get().GetExecutablePath());
    wxString appPath(f.GetPath());

    if (m_menuBar->IsChecked(myID_LLVMIR_DIALECT)) {
        wxString simulator_path = "/llvm_simulator";
        std::string command = std::string(simulator_path + " " + program_file);
        wxExecute(appPath + command, wxEXEC_SYNC);
    } else if (m_menuBar->IsChecked(myID_TOY_DIALECT)) {
        wxString simulator_path = "/standard_tac_simulator_exe";
        std::string command = std::string(simulator_path + " " + program_file);
        wxExecute(appPath + command, wxEXEC_SYNC);
    }
}

void MainWindowFrame::OnOptimizationWindow(wxCommandEvent &event) {
    wxString code = m_editor->GetText();

    if (m_menuBar->IsChecked(myID_LLVMIR_DIALECT)) {
        m_optimization_window = new LLVMOptimizationWindow(this, "Optimization window", code);
        if (m_optimization_window->ShowModal() == wxID_OK) {
            m_editor->SetText(m_optimization_window->output_code);
        }
    } else if (m_menuBar->IsChecked(myID_TOY_DIALECT)) {
        Program program;
        try {
            program = Program::from_program_code(code.ToStdString());
        } catch (yy::Parser::syntax_error &e) {
            wxMessageBox(wxString::Format(wxT("Синтаксическая ошибка!\n'%s'"), e.what()), wxT("Произошла ошибка!"));
        }

        wxArrayString func_names;
        for (auto n : program.get_function_names())
            func_names.push_back(n);

        auto *function_chooser = new FunctionChooser(this, func_names);
        if (func_names.size() == 1 || !func_names.empty() && function_chooser->ShowModal() == wxID_OK) {
            auto &selected_function =
                func_names.size() == 1
                    ? program.functions[0]
                    : *program.get_function_by_name(function_chooser->get_selected_function_name().ToStdString());

            auto *optimization_dialog = new ToyOptimizationChooseWindow(this, selected_function);

            try {
                if (optimization_dialog->ShowModal() == wxID_OK) {
                    m_editor->SetText(program.get_as_code());
                }
            } catch (std::exception &e) {
                const wxString &msg = wxString::Format(wxT("Произошла ошибка!\n'%s'"), e.what());
                wxMessageBox(msg, wxT("Произошла ошибка!"));
            }
        }
    }
}

void MainWindowFrame::OnDisplayCFG(wxCommandEvent &event) {
    wxString code = m_editor->GetText();

    if (m_menuBar->IsChecked(myID_LLVMIR_DIALECT)) {
        llvm::LLVMContext context;
        llvm::SMDiagnostic err;
        auto module = parseAssemblyString(code.ToStdString(), err, context);
        if (!module) {
            err.print("PROGRAM", llvm::errs());
        }

        wxArrayString func_names;
        for (auto &f : module->functions()) {
            if (!f.isDeclaration())
                func_names.push_back(f.getName().str());
        }

        auto *function_chooser = new FunctionChooser(this, func_names);
        if (func_names.size() == 1 || function_chooser->ShowModal() == wxID_OK) {
            wxString function_name =
                func_names.size() == 1 ? func_names[0] : function_chooser->get_selected_function_name();

            llvm::Function *func = module->getFunction(function_name.ToStdString());
            std::string string_stream;
            llvm::raw_string_ostream ostream(string_stream);
            llvm::DOTFuncInfo CFGInfo(func);
            llvm::WriteGraph(ostream, &CFGInfo);

            auto *graph_view = GraphView::fromDotFile(this, ostream.str());
            graph_view->ShowModal();
        }
    } else if (m_menuBar->IsChecked(myID_TOY_DIALECT)) {

        Program program;
        try {
            program = Program::from_program_code(code.ToStdString());
        } catch (yy::Parser::syntax_error &e) {
            wxMessageBox(wxString::Format(wxT("Синтаксическая ошибка!\n'%s'"), e.what()), wxT("Произошла ошибка!"));
        }

        wxArrayString func_names;
        for (auto n : program.get_function_names())
            func_names.push_back(n);

        auto *function_chooser = new FunctionChooser(this, func_names);
        if (func_names.size() == 1 || !func_names.empty() && function_chooser->ShowModal() == wxID_OK) {
            auto &selected_function =
                func_names.size() == 1
                    ? program.functions[0]
                    : *program.get_function_by_name(function_chooser->get_selected_function_name().ToStdString());

            auto png_image_data = selected_function.print_cfg("cfg.png");
            auto *graph_view = GraphView::fromImageData(this, png_image_data);
            graph_view->ShowModal();
        }
    }
}
