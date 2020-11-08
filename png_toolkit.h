
#ifndef PNG_TOOLKIT_H
#define PNG_TOOLKIT_H

#include <string>
#include <map>
//#include <memory>
#include "stb_image.h"
#include <iostream>
#include <fstream>
#include <vector>

struct image_data {
    stbi_uc* pixels;
    int w, h;
    int compPerPixel;
};

class png_toolkit {
public:
    enum class Error {
        WrongSize,
        WrongFormat,
        Ok
    };

    png_toolkit();
    ~png_toolkit();
    bool load(std::string const& pictureName);
    bool save(std::string const& pictureName);
    image_data getPixelData(void) const;
    void setPexelData(image_data&);

private:
    image_data imgData;
};

enum class FILTER {
    FILTER_WB,
    FILTER_RED,
    FILTER_THRESHOLD,
    FILTER_EDGE,
    FILTER_BLUR
};

struct config_params {
    int u, l, b, r;
    FILTER filter;
};

struct filter_edges {
    int x1, y1, x2, y2;
};

class config_parser {
public:
    config_parser(std::string&);
    config_params parse();
    bool end_of_file();
    ~config_parser();
private:
    int u, l, b, r;
    std::string str;
    std::ifstream fin;
};

class filter {
public:
    filter(config_params&);
    virtual image_data use_filter(image_data) = 0;
    ~filter() {}
protected:
    filter_edges set_edges(image_data&);
    void copy_pixels(stbi_uc*, image_data&);
protected:
    int u, l, b, r;
};

class wb_filter : public filter {
public:
    wb_filter(config_params&);
    image_data use_filter(image_data) override;
};

class red_filter : public filter {
public:
    red_filter(config_params&);
    image_data use_filter(image_data) override;
};

class threshold_filter : public filter {
private:
    bool is_below_median(image_data const&, int, int) const;
public:
    threshold_filter(config_params&);
    image_data use_filter(image_data) override;
};

class matrix_filter : public filter {
public:
    matrix_filter(config_params&, std::vector<int>&);
    virtual image_data use_filter(image_data) override;
protected:
    int get_norm() const;
    //void copy_pixels(stbi_uc*, image_data&);
protected:
    int kernel_size = 3;
    std::vector<int> kernel;
};

class edge_filter : public matrix_filter {
public:
    edge_filter(config_params&, std::vector<int>&);
    image_data use_filter(image_data) override;
};

class blur_filter : public matrix_filter {
public:
    blur_filter(config_params&, std::vector<int>&);
};

class factory {
public:
    factory() {}
    virtual filter* make_filter(config_params&) = 0;
    virtual ~factory() {}
};

class factory_impl : public factory {
public:
    factory_impl() {}
    filter* make_filter(config_params&) override;
    ~factory_impl() {}
};

#endif // PNG_TOOLKIT_H