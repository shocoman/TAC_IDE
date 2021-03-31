#include <fmt/ranges.h>
#include <iostream>

#include "src/gui_code_editor/the_app.h"
//#include "tac_project.h"
//#include <wx/wx.h>

int main(int argc, char **argv) {
    //    simulator_main(argc, argv);

    setenv("DISPLAY", "192.168.14.1:0", true);

    wxApp *pApp = new TheApp();
    wxApp::SetInstance(pApp);
    wxEntryStart(argc, argv);
    wxTheApp->OnInit();

    // wxWidgets event loop
    wxTheApp->OnRun();

    // cleaning up...
    wxTheApp->OnExit();
    wxEntryCleanup();

    //    wxString myText;
    //
    //    enum class TokenType { Other, Space, Commentary, Number, Identifier, Goto, Label, String };
    //    auto GetTextLength = [&] { return myText.length(); };
    //    auto GetCharAt = [&](int pos) { return myText.GetChar(pos).GetValue(); };
    //
    //    auto GetIdentifier = [&](int start_pos, int length) { return myText.SubString(start_pos, start_pos +
    //    length-1); }; auto ScanNextToken = [&](int pos) -> std::pair<TokenType, int> {
    //        auto IsValidIdentifierChar = [](auto &c, bool first) -> bool {
    //            return std::isalpha(c) || c == wxT('_') || c == wxT('.') || c == wxT('$') || (std::isdigit(c) && not
    //            first);
    //        };
    //
    //        wxChar c = GetCharAt(pos);
    //        if (c == wxT(' ') || c == wxT('\t') || c == wxT('\r') || c == wxT('\n')) {
    //            // Whitespaces
    //            return {TokenType::Space, 1};
    //        } else if (c == wxT('"')) {
    //            // String
    //            int end_pos = pos+1;
    //            while (GetCharAt(end_pos) != wxT('"') && end_pos < GetTextLength())
    //                ++end_pos;
    //            return {TokenType::String, end_pos - pos +1};
    //        } else if (pos + 1 < GetTextLength() && c == wxT('/') && GetCharAt(pos + 1) == wxT('/')) {
    //            // Commentary
    //            int end_pos = pos;
    //            while (GetCharAt(end_pos) != wxT('\n') && end_pos < GetTextLength())
    //                ++end_pos;
    //            if (end_pos != GetTextLength())
    //                ++end_pos;
    //            return {TokenType::Commentary, end_pos - pos};
    //        } else if (std::isdigit(c)) {
    //            // Number
    //            int end_pos = pos;
    //            bool has_dot = false;
    //
    //            do {
    //                if (c == wxT('.'))
    //                    has_dot = true;
    //                ++end_pos;
    //                c = GetCharAt(end_pos);
    //            } while ((std::isdigit(c) || c == wxT('.') && not has_dot) && end_pos < GetTextLength());
    //
    //            return {TokenType::Number, end_pos - pos};
    //        } else if (IsValidIdentifierChar(c, true)) {
    //            // Identifier, goto or label
    //            int end_pos = pos;
    //            do {
    //                c = GetCharAt(end_pos);
    //                ++end_pos;
    //            } while (IsValidIdentifierChar(c, false) && end_pos < GetTextLength());
    //
    //            if (c == wxT(':'))
    //                return {TokenType::Label, end_pos - pos};
    //            else if (GetIdentifier(pos, end_pos - pos - 1).CmpNoCase(wxT("goto")) == 0)
    //                return {TokenType::Goto, end_pos - pos - 1};
    //            else
    //                return {TokenType::Identifier, end_pos - pos - 1};
    //        } else {
    //            return {TokenType::Other, 1};
    //        }
    //    };
    //
    //    myText = " \"\" = 23 ; f z: ; 4 8 14.55. ";
    //
    //    int i = 0;
    //    while (i < myText.length()) {
    //
    //        auto [token_type, len] = ScanNextToken(i);
    //
    //        fmt::print("{:2}; ", len);
    //        switch (token_type) {
    //        case TokenType::Other:
    //            fmt::print("Other: '{}'\n", GetIdentifier(i, len));
    //            break;
    //        case TokenType::Space:
    //            fmt::print("Space\n");
    //            break;
    //        case TokenType::Commentary:
    //            fmt::print("Commentary\n");
    //            break;
    //        case TokenType::Number:
    //            fmt::print("Number: '{}'\n", GetIdentifier(i, len));
    //            break;
    //        case TokenType::Identifier:
    //            fmt::print("Identifier: '{}'\n", GetIdentifier(i, len));
    //            break;
    //        case TokenType::Goto:
    //            fmt::print("Goto\n");
    //            break;
    //        case TokenType::Label:
    //            fmt::print("Label: '{}'\n", GetIdentifier(i, len));
    //            break;
    //        case TokenType::String:
    //            fmt::print("String: '{}'\n", GetIdentifier(i, len));
    //            break;
    //        }
    //
    //        i += len;
    //    }

    return 0;
}