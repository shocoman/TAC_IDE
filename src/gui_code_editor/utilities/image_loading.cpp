//
// Created by victor on 15.03.2021.
//

#include "image_loading.hpp"


wxImage LoadImageFromData(std::vector<char> &data, wxBitmapType type) {
    wxMemoryInputStream image_data_stream(data.data(), data.size());
    wxImage image;
    if (!image.LoadFile(image_data_stream, type))
        std::cout << "Graph image loading error!" << std::endl;
    return image;
}
