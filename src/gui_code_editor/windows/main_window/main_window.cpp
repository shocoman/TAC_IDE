
#include <windows/function_selection_window/function_chooser.h>
#include <wx/artprov.h>
#include <wx/filename.h>
#include <wx/menu.h>

#include "main_window.h"

// helper functions
enum wxbuildinfoformat {
    short_f, long_f
};

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

                EVT_MENU(myID_PROPERTIES, MainWindowFrame::OnProperties)
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
// simulator run
                EVT_MENU(myID_SIMULATOR_RUN, MainWindowFrame::OnSimulatorRun)
                EVT_MENU(myID_OPTIMIZATION_WINDOW, MainWindowFrame::OnOptimizationWindow)
                EVT_MENU(myID_PRINT_CFG_WINDOW, MainWindowFrame::OnDisplayCFG)

                EVT_MENU(myID_EDU_TOGGLE, MainWindowFrame::OnEduToggle)
                EVT_MENU(wxID_ABOUT, MainWindowFrame::OnAbout)
// editor
                EVT_STC_MODIFIED(wxID_ANY, MainWindowFrame::OnModified)
                EVT_STC_UPDATEUI(wxID_ANY, MainWindowFrame::OnEditorUpdateUI)
END_EVENT_TABLE()

MainWindowFrame::MainWindowFrame(wxFrame *frame, const wxString &title, const wxPoint &pos, const wxSize &size,
                                 long style)
        : wxFrame(frame, wxID_ANY, title, pos, size, style) {
    // create a menu bar
    m_menuBar = new wxMenuBar;
    CreateMenu();

    // main sizer of the application layout
    m_vbox = new wxBoxSizer(wxVERTICAL);

    // Toolbar
    m_toolBar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL | wxTB_TEXT);
    CreateToolbar();
    m_vbox->Add(m_toolBar, 0, wxEXPAND);
    m_vbox->Show(m_toolBar, m_menuBar->IsChecked(myID_TOOLBAR_TOGGLE));

    // tabs
    m_notebook = new wxAuiNotebook(this, wxID_ANY);
    // editor window
    nonameFileCounter = 1;
    AddNewEditorTab();

    m_vbox->Add(m_notebook, 1, wxEXPAND, 0);

    SetSizer(m_vbox);
    m_vbox->Layout();

    // status bar displays currently selected IR and cursor position
    CreateStatusBar(2);
    SetStatusText(wxT("Toy IR"), 0);
    SetIRDialect(myID_TOY_DIALECT);
    GetStatusBar()->Bind(wxEVT_LEFT_DCLICK, [&](wxMouseEvent &event) {
        if (m_menuBar->IsChecked(myID_TOY_DIALECT))
            SetIRDialect(myID_LLVMIR_DIALECT);
        else if (m_menuBar->IsChecked(myID_LLVMIR_DIALECT))
            SetIRDialect(myID_TOY_DIALECT);
    });

    Centre();

    m_notebook->Bind(wxEVT_AUINOTEBOOK_PAGE_CLOSE, [&](auto &e) { CloseTab(false); });
}

// common event handlers
void MainWindowFrame::OnClose(wxCloseEvent &event) {
    for (int i = m_notebook->GetPageCount() - 1; i >= 0; --i) {
        m_notebook->SetSelection(i);
        CloseTab(true);
    }

    if (GetSelectedEditor() && GetSelectedEditor()->IsModified()) {
        if (event.CanVeto())
            event.Veto(true);
    } else
        Destroy();
}

void MainWindowFrame::OnExit(wxCommandEvent &WXUNUSED(event)) { Close(true); }

void MainWindowFrame::OnAbout(wxCommandEvent &event) {
    wxString msg = wxString::Format("Build info: %s\nFile path: %s", wxbuildinfo(long_f),
                                    GetSelectedEditor()->GetFilename());
    wxMessageBox(msg);
}

void MainWindowFrame::OnMainToolbarToggle(wxCommandEvent &WXUNUSED(event)) {
    m_vbox->Show(m_toolBar, m_menuBar->IsChecked(myID_TOOLBAR_TOGGLE));
    m_vbox->Layout();
}

// file event handlers
void MainWindowFrame::OnFileNew(wxCommandEvent &event) { AddNewEditorTab(); }

void MainWindowFrame::OnFileOpen(wxCommandEvent &WXUNUSED(event)) {
    if (!GetSelectedEditor())
        return;
    wxFileDialog dlg(this, wxT("Open file"), wxEmptyString, wxEmptyString,
            // wxT("Three Address Code file(*.3ac)|*.3ac|Any file (*)|*"),
                     wxALL_FILES, wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_CHANGE_DIR);
    if (dlg.ShowModal() != wxID_OK)
        return;
    wxString fname = dlg.GetPath();
    FileOpen(fname);
}

void MainWindowFrame::OnFileSave(wxCommandEvent &WXUNUSED(event)) {
    EditorCtrl *editor = GetSelectedEditor();
    if (!editor)
        return;
//    if (editor->Modified())
    {
        editor->SaveFile();
    }

    wxFileName w(editor->GetFilename());
    w.Normalize();
    m_notebook->SetPageText(m_notebook->GetSelection(), w.GetName());
}

void MainWindowFrame::OnFileSaveAs(wxCommandEvent &WXUNUSED(event)) {
    if (!GetSelectedEditor())
        return;
    wxFileDialog dlg(this, wxT("Save file"), wxEmptyString, wxEmptyString,
            // wxT("Three Address Code file(*.3ac)|*.3ac|Any file (*)|*"),
                     wxALL_FILES, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (dlg.ShowModal() != wxID_OK)
        return;
    wxString filename = dlg.GetPath();
    GetSelectedEditor()->SaveFile(filename, false);
    wxFileName w(filename);
    w.Normalize();
    m_notebook->SetPageText(m_notebook->GetSelection(), w.GetName());
}

void MainWindowFrame::CloseTab(bool remove_tab) {
    auto *editor = GetSelectedEditor();
    if (!editor)
        return;
    if (editor->Modified()) {
        wxString msg = _("Source file is not saved, save before closing?");
        int msg_box = wxMessageBox(msg, _("Close"), wxYES_NO | wxICON_QUESTION);
        if (msg_box == wxYES) {
            editor->SaveFile();
            if (editor->Modified()) {
                wxMessageBox(_("Text could not be saved!"), _("Close abort"), wxOK | wxICON_EXCLAMATION);
                return;
            }
        }
    }
    if (remove_tab)
        m_notebook->DeletePage(m_notebook->GetSelection());
}

// properties event handlers
void MainWindowFrame::OnProperties(wxCommandEvent &WXUNUSED(event)) {
    if (!GetSelectedEditor())
        return;
    EditorCtrlProperties dlg(GetSelectedEditor(), 0);
}

// edit events
void MainWindowFrame::OnEdit(wxCommandEvent &event) {
    if (GetSelectedEditor())
        GetSelectedEditor()->GetEventHandler()->ProcessEvent(event);
}

void MainWindowFrame::OnModified(wxStyledTextEvent &WXUNUSED(event)) {
    if (GetSelectedEditor()->Modified()) {
        wxFileName w(GetSelectedEditor()->GetFilename());
        w.Normalize();
        m_notebook->SetPageText(m_notebook->GetSelection(), "*" + w.GetName());
    }
}

// entity event
void MainWindowFrame::OnEduToggle(wxCommandEvent &event) {
    if (!GetSelectedEditor())
        return;

    m_vbox->Layout();
    GetSelectedEditor()->GetEventHandler()->ProcessEvent(event);
}

// private functions
void MainWindowFrame::CreateMenu() {
    // File menu
    wxMenu *menuFile = new wxMenu;
    menuFile->Append(wxID_NEW, wxT("&Новый файл ..\tCtrl+N"));
    menuFile->Append(wxID_OPEN, wxT("&Открыть ..\tCtrl+O"));
    menuFile->Append(wxID_SAVE, wxT("&Сохранить\tCtrl+S"));
    menuFile->Append(wxID_SAVEAS, wxT("Сохранить как ..\tCtrl+Shift+S"));
    menuFile->AppendSeparator();
    menuFile->Append(myID_PROPERTIES, wxT("Свойства документа .."));
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT, wxT("&Выйти\tCtrl+Q"));

    // Edit menu
    wxMenu *menuEdit = new wxMenu;
    menuEdit->Append(wxID_UNDO, wxT("&Отменить\tCtrl+Z"));
    menuEdit->Append(wxID_REDO, wxT("&Повторить\tCtrl+Shift+Z"));
    menuEdit->AppendSeparator();
    menuEdit->Append(wxID_CUT, wxT("Вырезать\tCtrl+X"));
    menuEdit->Append(wxID_COPY, wxT("&Копировать\tCtrl+C"));
    menuEdit->Append(wxID_PASTE, wxT("&Вставить\tCtrl+V"));
    menuEdit->Append(wxID_CLEAR, wxT("&Удалить\tDel"));
    menuEdit->AppendSeparator();
    menuEdit->Append(wxID_SELECTALL, wxT("&Выбрать всё\tCtrl+A"));
    menuEdit->Append(myID_SELECTLINE, wxT("Выбрать строку\tCtrl+L"));

    // View menu
    wxMenu *menuView = new wxMenu;
    menuView->AppendCheckItem(myID_TOOLBAR_TOGGLE, wxT("Tool&bar"));
    menuView->AppendSeparator();
    menuView->AppendCheckItem(myID_OVERTYPE, wxT("Режим перезаписи (Overwrite mode)\tIns"));
    menuView->AppendCheckItem(myID_WRAPMODEON, wxT("Режим переноса строк (Wrap mode) \tCtrl+U"));
    menuView->AppendSeparator();
    menuView->AppendCheckItem(myID_DISPLAYEOL, wxT("Отображать окончания строк"));
    menuView->AppendCheckItem(myID_LINENUMBER, wxT("Отображать номера строк"));
    menuView->AppendCheckItem(myID_LONGLINEON, wxT("Отображать маркеры длинных строк"));
    menuView->AppendCheckItem(myID_WHITESPACE, wxT("Отображать пробелы"));
    menuView->AppendSeparator();
    menuView->AppendCheckItem(myID_BB, wxT("Подсвечивать базовые блоки"));

    // Tac dialect submenu
    wxMenu *menuTacDialect = new wxMenu;
    menuTacDialect->AppendRadioItem(myID_TOY_DIALECT, wxT("Toy IR"));
    menuTacDialect->AppendRadioItem(myID_LLVMIR_DIALECT, wxT("LLVM IR"));
//    menuTacDialect->Enable(myID_LLVMIR_DIALECT, LLVM_FOUND);
    menuTacDialect->Bind(wxEVT_MENU, [&](wxCommandEvent &evt) { SetIRDialect(evt.GetId()); });

    // Project menu
    wxMenu *menuProject = new wxMenu;
    menuProject->AppendSubMenu(menuTacDialect, wxT("Диалект трёхадресного кода"));

    // Examples submenu
    auto examplesFolderName = "_Examples";
    auto examplesFolderPath = wxString::Format("%s", examplesFolderName);
    if (wxDir::Exists(examplesFolderPath)) {
        std::map<wxString, wxMenu *> name_to_menu;

        wxArrayString filePaths;
        wxDir::GetAllFiles(examplesFolderPath, &filePaths, wxALL_FILES_PATTERN, wxDIR_DEFAULT);
        for (int path_i = 0; path_i < filePaths.size(); ++path_i) {
            auto &path = filePaths[path_i];

            wxFileName file_name(path);
            auto dirs = file_name.GetDirs();
            if (dirs.Index("..") != wxNOT_FOUND)
                dirs.Remove("..");

            for (int i = 0; i < dirs.size(); ++i) {
                auto &dir = dirs[i];

                if (name_to_menu.count(dir) == 0) {
                    name_to_menu[dir] = new wxMenu;
                    name_to_menu.at(dir)->Bind(
                            wxEVT_MENU,
                            [filePaths, this](wxCommandEvent &event) { FileOpen(filePaths[event.GetId()]); });
                    if (i != 0)
                        name_to_menu.at(dirs[i - 1])->AppendSubMenu(name_to_menu.at(dir), dir);
                }
            }

            name_to_menu.at(dirs.Last())->Append(path_i, file_name.GetName(), file_name.GetFullPath());
        }

        menuProject->AppendSeparator();
        menuProject->AppendSubMenu(name_to_menu.at(examplesFolderName), wxT("Примеры"));
    }

    // Tools menu
    wxMenu *menuTools = new wxMenu;
    menuTools->AppendCheckItem(myID_EDU_TOGGLE, wxT("Конвертировать комментарии (//) в аннотации и обратно\tCtrl+E"));
    menuTools->AppendSeparator();
    menuTools->Append(myID_OPTIMIZATION_WINDOW, wxT("Открыть окно оптимизации\tCtrl+I"));
    menuTools->Append(myID_PRINT_CFG_WINDOW, wxT("Отобразить граф потока управления\tCtrl+G"));
    menuTools->AppendSeparator();
    menuTools->Append(myID_SIMULATOR_RUN, wxT("Запустить программу..\tCtrl+R"));

    // Help menu
    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT, wxT("О программе ..\tCtrl+T"));

    // construct menu
    m_menuBar->Append(menuFile, wxT("&Файл"));
    m_menuBar->Append(menuEdit, wxT("&Правка"));
    m_menuBar->Append(menuView, wxT("&Вид"));
    m_menuBar->Append(menuProject, wxT("&Проект"));
    m_menuBar->Append(menuTools, wxT("&Инструменты"));
    m_menuBar->Append(menuHelp, wxT("&Помощь"));
    SetMenuBar(m_menuBar);

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

    m_toolBar->AddTool(wxID_NEW, wxT("Новый файл"), newb, newb, wxITEM_NORMAL, wxT("Создать новую вкладку"));
    m_toolBar->AddTool(wxID_OPEN, wxT("Открыть"), openb, openb, wxITEM_NORMAL, wxT("Открыть файл с кодом"));
    m_toolBar->AddTool(wxID_SAVE, wxT("Сохранить"), saveb, saveb, wxITEM_NORMAL, wxT("Сохранить файл с кодом"));
    m_toolBar->AddSeparator();
    m_toolBar->AddTool(wxID_UNDO, wxT("Отменить"), undob, undob, wxITEM_NORMAL, wxT("Отменить"));
    m_toolBar->AddTool(wxID_REDO, wxT("Повторить"), redob, redob, wxITEM_NORMAL, wxT("Повторить"));
    m_toolBar->AddSeparator();
    m_toolBar->AddTool(wxID_CUT, wxT("Вырезать"), cutb, cutb, wxITEM_NORMAL, wxT("Вырезать"));
    m_toolBar->AddTool(wxID_COPY, wxT("Копировать"), copyb, copyb, wxITEM_NORMAL, wxT("Копировать"));
    m_toolBar->AddTool(wxID_PASTE, wxT("Вставить"), pasteb, pasteb, wxITEM_NORMAL, wxT("Вставить"));

    m_toolBar->AddSeparator();
    m_toolBar->AddTool(myID_SIMULATOR_RUN, wxT("Запустить"), runb, runb, wxITEM_NORMAL, wxT("Запустить программу.."));

    m_toolBar->AddSeparator();
    m_toolBar->AddTool(myID_EDU_TOGGLE, wxT("Аннотации"), questionb, questionb, wxITEM_NORMAL,
                       wxT("Конвертировать комментарии в аннотации"));

    m_toolBar->Realize();

    Connect(wxID_NEW, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainWindowFrame::OnFileNew));
    Connect(wxID_OPEN, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainWindowFrame::OnFileOpen));
    Connect(wxID_SAVE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainWindowFrame::OnFileSave));
    Connect(wxID_UNDO, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainWindowFrame::OnEdit));
    Connect(wxID_REDO, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainWindowFrame::OnEdit));
    Connect(wxID_CUT, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainWindowFrame::OnEdit));
    Connect(wxID_COPY, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainWindowFrame::OnEdit));
    Connect(wxID_PASTE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainWindowFrame::OnEdit));

    Connect(myID_SIMULATOR_RUN, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainWindowFrame::OnSimulatorRun));
    Connect(myID_EDU_TOGGLE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainWindowFrame::OnEduToggle));
}

void MainWindowFrame::FileOpen(wxString fname) {
    wxFileName w(fname);
    w.Normalize();
    fname = w.GetFullPath();
    auto *new_editor = AddNewEditorTab();
    new_editor->LoadFile(fname);
    new_editor->SelectNone();
    m_notebook->SetPageText(m_notebook->GetSelection(), w.GetName());
}

void MainWindowFrame::OnSimulatorRun(wxCommandEvent &event) {
    wxString program_file = "_test.txt";
    GetSelectedEditor()->SaveFile(program_file, false);

    wxString simulatorFileName;
    if (m_menuBar->IsChecked(myID_LLVMIR_DIALECT)) {
#if LLVM_FOUND
        simulatorFileName = "llvm_simulator";
#else
        DisplayLLVMNotfoundMessage();
        return;
#endif
    } else if (m_menuBar->IsChecked(myID_TOY_DIALECT)) {
        simulatorFileName = "tac_simulator_exe";
    }

    wxFileName appPath(wxStandardPaths::Get().GetExecutablePath());
    appPath.SetFullName(simulatorFileName);

    const wxString &command = appPath.GetFullPath() + " " + program_file;
    fmt::print("CMD: {}\n", command.ToStdString());
    wxExecute(command, wxEXEC_ASYNC);
}

void MainWindowFrame::OnOptimizationWindow(wxCommandEvent &event) {
    wxString code = GetSelectedEditor()->GetText().ToAscii();

    if (m_menuBar->IsChecked(myID_LLVMIR_DIALECT)) {
#if LLVM_FOUND
        auto *m_optimization_window = new LLVMOptimizationWindow(this, "Optimization window", code);
        if (m_optimization_window->ShowModal() == wxID_OK) {
            auto *new_editor = AddNewEditorTab();
            new_editor->SetText(m_optimization_window->output_code);
        }
#else
        DisplayLLVMNotfoundMessage();
#endif
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
                    auto *new_editor = AddNewEditorTab();
                    new_editor->SetText(program.get_as_code());
                }
            } catch (std::exception &e) {
                const wxString &msg = wxString::Format(wxT("Произошла ошибка!\n'%s'"), e.what());
                wxMessageBox(msg, wxT("Произошла ошибка!"));
            }
        }
    }
}

void MainWindowFrame::DisplayLLVMNotfoundMessage() {
    wxString error_text = wxT("Так как во время сборки и компиляции программы не была найдена библиотека LLVM, данная опция вам недоступна.");
    wxMessageBox(error_text, wxT("Произошла ошибка!"), wxCENTER | wxOK | wxICON_ERROR);
}

void MainWindowFrame::OnDisplayCFG(wxCommandEvent &event) {
    wxString code = GetSelectedEditor()->GetText().ToAscii();

    if (m_menuBar->IsChecked(myID_LLVMIR_DIALECT)) {
#if LLVM_FOUND
        llvm::LLVMContext context;
        llvm::SMDiagnostic err;
        auto module = parseAssemblyString(code.ToStdString(), err, context);
        if (!module) {
            std::string error_text;
            llvm::raw_string_ostream error_stream(error_text);
            err.print("Program", llvm::errs());
            err.print(GetSelectedEditor()->GetFilename() + "\n", error_stream);
            wxMessageBox(error_text, wxT("Произошла ошибка!"), wxCENTER | wxOK | wxICON_ERROR);
            return;
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

            llvm::Function * func = module->getFunction(function_name.ToStdString());
            std::string string_stream;
            llvm::raw_string_ostream ostream(string_stream);
            llvm::DOTFuncInfo CFGInfo(func);
            llvm::WriteGraph(ostream, &CFGInfo);

            auto *graph_view = GraphView::fromDotFile(this, ostream.str());
            graph_view->ShowModal();
        }
#else
        DisplayLLVMNotfoundMessage();
#endif
    } else if (m_menuBar->IsChecked(myID_TOY_DIALECT)) {
        Program program;
        try {
            program = Program::from_program_code(code.ToStdString());
        } catch (yy::Parser::syntax_error &e) {
            wxMessageBox(wxString::Format(wxT("Синтаксическая ошибка!\n'%s'"), e.what()), wxT("Произошла ошибка!"),
                         wxCENTER | wxOK | wxICON_ERROR);
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

void MainWindowFrame::SetIRDialect(int id) {
    switch (id) {
        case myID_TOY_DIALECT:
            m_menuBar->Check(myID_TOY_DIALECT, true);
            SetStatusText("Toy IR", 0);
            break;
        case myID_LLVMIR_DIALECT:
            m_menuBar->Check(myID_LLVMIR_DIALECT, true);
            SetStatusText("LLVM IR", 0);
            break;
        default:
            break;
    }
}

EditorCtrl *MainWindowFrame::GetSelectedEditor() { return wxDynamicCast(m_notebook->GetCurrentPage(), EditorCtrl); }

EditorCtrl *MainWindowFrame::AddNewEditorTab() {
    auto *editor = new EditorCtrl(m_notebook);
    auto name = wxString::Format(wxT("%s%i"), NONAME, nonameFileCounter);
    m_notebook->AddPage(editor, name, true);
    editor->SetFilename(name);
    nonameFileCounter += 1;
    return editor;
}

void MainWindowFrame::OnEditorUpdateUI(wxStyledTextEvent &event) {
    // update cursor position in status bar
    auto *editor = GetSelectedEditor();
    auto line = editor->GetCurrentLine();
    auto pos = editor->GetCurrentPos() - editor->PositionFromLine(line);
    SetStatusText(wxString::Format("%d:%d", line + 1, pos + 1), 1);
}
