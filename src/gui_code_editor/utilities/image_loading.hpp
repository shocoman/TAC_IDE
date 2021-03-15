//
// Created by victor on 15.03.2021.
//

#ifndef TOY_IDE_SRC_GUI_CODE_EDITOR_UTILITIES_IMAGE_LOADING_HPP
#define TOY_IDE_SRC_GUI_CODE_EDITOR_UTILITIES_IMAGE_LOADING_HPP

#include <iostream>
#include <vector>
#include <wx/animate.h>
#include <wx/mstream.h>


wxImage LoadImageFromData(std::vector<char> &data, wxBitmapType type = wxBITMAP_TYPE_PNG);


#endif // TOY_IDE_SRC_GUI_CODE_EDITOR_UTILITIES_IMAGE_LOADING_HPP
