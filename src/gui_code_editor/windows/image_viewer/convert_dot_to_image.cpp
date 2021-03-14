//
// Created by victor on 03.02.2021.
//

#include "convert_dot_to_image.hpp"

std::vector<char> renderGraphFromString(const std::string& dot_content) {
    GVC_t *gvc = gvContext();

    Agraph_t *g = agmemread(dot_content.c_str());

    gvLayout(gvc, g, "dot");
//    gvRender(gvc, g, "plain", stdout);

    char *result = nullptr;
    uint length = 0;
    gvRenderData(gvc, g, "png", &result, &length);
    std::vector<char> buffer(result, result+length);
    gvFreeRenderData(result);

    gvFreeLayout(gvc, g);
    agclose(g);
    gvFreeContext(gvc);

    return buffer;
}
