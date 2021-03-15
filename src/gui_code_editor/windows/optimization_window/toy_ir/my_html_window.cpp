//
// Created by victor on 13.03.2021.
//


#include "my_html_window.hpp"
#include "wx/html/m_templ.h"

TAG_HANDLER_BEGIN(MY_CUSTOM_TAG, "ABYSS")
  TAG_HANDLER_PROC(tag) {
        MyHTMLWindow *win = wxDynamicCast(m_WParser->GetWindowInterface()->GetHTMLWindow(), MyHTMLWindow);
        win->myfunc(tag, m_WParser);
        return false;
    }
TAG_HANDLER_END(MYBIND)

TAGS_MODULE_BEGIN(MyCustomTags)
        TAGS_MODULE_ADD(MY_CUSTOM_TAG)
TAGS_MODULE_END(MyCustomTags)