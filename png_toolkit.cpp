#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <array>
#include "stb_image_write.h"
#include "png_toolkit.h"

png_toolkit::png_toolkit() {}

png_toolkit::~png_toolkit() {
    stbi_image_free(imgData.pixels);
}

bool png_toolkit::load(const std::string& pictureName) {
    imgData.pixels = stbi_load(pictureName.c_str(), &imgData.w, &imgData.h, &imgData.compPerPixel, 0);
    return imgData.pixels != nullptr;
}

bool png_toolkit::save(const std::string& pictureName) {
    return stbi_write_png(pictureName.c_str(),
        imgData.w, imgData.h,
        imgData.compPerPixel,
        imgData.pixels, 0) != 0;
}

image_data png_toolkit::getPixelData(void) const {
    return imgData;
}

void png_toolkit::setPexelData(image_data& newImgData) {
    std::swap(imgData, newImgData);
}

filter* factory_impl::make_filter(config_params& cfgp) {
    std::vector<int> kernel;
    switch (cfgp.filter) {
    case FILTER::FILTER_WB:
        return new wb_filter(cfgp);
    case FILTER::FILTER_RED:
        return new red_filter(cfgp);
    case FILTER::FILTER_THRESHOLD:
        return new threshold_filter(cfgp);
    case FILTER::FILTER_EDGE:
        kernel = { -1, -1, -1, -1, 9, -1, -1, -1, -1 };
        return new edge_filter(cfgp, kernel);
    case FILTER::FILTER_BLUR:
        kernel = { 1, 1, 1, 1, 1, 1, 1, 1, 1 };
        return new blur_filter(cfgp, kernel);
    }
    return nullptr;
}

config_parser::config_parser(std::string& filename) {
    //std::ifstream fin(filename);
    fin.open(filename);
    if (!fin.is_open()) {
        throw "unknow file";
    }
    u = l = b = r = 0;
}

bool config_parser::end_of_file() {
    return fin.eof();
}

config_parser::~config_parser() {
    fin.close();
}

config_params config_parser::parse() {
    config_params cfgp;
    char ch;
    fin >> str >> u >> l >> b >> r;
    cfgp.u = u;
    cfgp.l = l;
    cfgp.b = b;
    cfgp.r = r;
    if (str == "wb") {
        cfgp.filter = FILTER::FILTER_WB;
        //std::cout << "wb" << ' ';
    }
    else if (str == "red") {
        cfgp.filter = FILTER::FILTER_RED;
        //std::cout << "red" << ' ';
    }
    else if (str == "threshold") {
        cfgp.filter = FILTER::FILTER_THRESHOLD;
        //std::cout << "threshold" << ' ';
    }
    else if (str == "edge") {
        cfgp.filter = FILTER::FILTER_EDGE;
        //std::cout << "edge" << ' ';
    }
    else if (str == "blur") {
        cfgp.filter = FILTER::FILTER_BLUR;
        //std::cout << "blur" << ' ';
    }
    return cfgp;
}

filter::filter(config_params& cfgp) : u{ cfgp.u }, l{ cfgp.l }, b{ cfgp.b }, r{ cfgp.r } {}

filter_edges filter::set_edges(image_data& image) {
    filter_edges edges;
    if (u == 0) {
        edges.x1 = 0;
    }
    else {
        edges.x1 = image.w / u;
    }
    if (l == 0) {
        edges.y1 = 0;
    }
    else {
        edges.y1 = image.h / l;
    }
    if (b == 0) {
        edges.x2 = 0;
    }
    else {
        edges.x2 = image.w / b;
    }
    if (r == 0) {
        edges.y2 = 0;
    }
    else {
        edges.y2 = image.h / r;
    }

    if (edges.x1 > edges.x2) {
        std::swap(edges.x1, edges.x2);
    }
    if (edges.y1 > edges.y2) {
        std::swap(edges.y1, edges.y2);
    }
    return edges;
}

wb_filter::wb_filter(config_params& cfgp) : filter(cfgp) {}

red_filter::red_filter(config_params& cfgp) : filter(cfgp) {}

threshold_filter::threshold_filter(config_params& cfgp) : filter(cfgp) {}

matrix_filter::matrix_filter(config_params& cfgp, std::vector<int>& ker) : filter(cfgp), kernel_size{ 3 } {
    kernel = ker;
}

void filter::copy_pixels(stbi_uc* to, image_data& from) {
    for (int i = 0; i < from.w; ++i) {
        for (int j = 0; j < from.h; ++j) {
            to[3 * (i + j * from.w) + 0] = from.pixels[3 * (i + j * from.w) + 0];
            to[3 * (i + j * from.w) + 1] = from.pixels[3 * (i + j * from.w) + 1];
            to[3 * (i + j * from.w) + 2] = from.pixels[3 * (i + j * from.w) + 2];
        }
    }
}

edge_filter::edge_filter(config_params& cfgp, std::vector<int>& ker) : matrix_filter(cfgp, ker) {}

blur_filter::blur_filter(config_params& cfgp, std::vector<int>& ker) : matrix_filter(cfgp, ker) {}

image_data wb_filter::use_filter(image_data image) {
    filter_edges edges = set_edges(image);
    //std::cout << edges.x1 << ' ' << edges.y1 << ' ' << edges.x2 << ' ' << edges.y2 << '\n';
    for (int i = edges.x1; i < edges.x2; ++i) {
        for (int j = edges.y1; j < edges.y2; ++j) {
            stbi_uc r, g, b;
            r = image.pixels[3 * (i + j * image.w) + 0];
            g = image.pixels[3 * (i + j * image.w) + 1];
            b = image.pixels[3 * (i + j * image.w) + 2];
            stbi_uc intensity = (3 * r + 6 * g + b) / 10;
            image.pixels[3 * (i + j * image.w) + 0] = intensity;
            image.pixels[3 * (i + j * image.w) + 1] = intensity;
            image.pixels[3 * (i + j * image.w) + 2] = intensity;
        }
    }
    return image;
}

image_data red_filter::use_filter(image_data image) {
    filter_edges edges = set_edges(image);
    //std::cout << edges.x1 << ' ' << edges.y1 << ' ' << edges.x2 << ' ' << edges.y2;
    for (int i = edges.x1; i < edges.x2; ++i) {
        for (int j = edges.y1; j < edges.y2; ++j) {
            image.pixels[3 * (i + j * image.w) + 0] = 255;
            image.pixels[3 * (i + j * image.w) + 1] = 0;
            image.pixels[3 * (i + j * image.w) + 2] = 0;
        }
    }
    return image;
}

int comp(const int* i, const int* j) {
    return *i - *j;
}

bool threshold_filter::is_below_median(image_data const& image, int i, int j) const {
    int intensities[25];
    for (int i = 0; i < 25; ++i) {
        intensities[i] = -1;
    }

    //for (int i = 0; i < 25; ++i) {
    //    std::cout << " ### " << intensities[i] << " ### ";
    //}
    //std::cout << '\n';
    int counter = 0;
    int intensity = 0;
    for (int x = i - 2; x <= i + 2; ++x) {
        if (x >= 0 && x < image.w) {
            for (int y = j - 2; y <= j + 2; ++y) {
                if (y >= 0 && y < image.h) {
                    stbi_uc r = image.pixels[3 * (x + y * image.w) + 0];
                    stbi_uc g = image.pixels[3 * (x + y * image.w) + 1];
                    stbi_uc b = image.pixels[3 * (x + y * image.w) + 2];
                    //std::cout << 3 * r + 6 * g + b << '\n';
                    intensities[counter++] = (3 * r + 6 * g + b) / 10;
                    if (x == i && y == j) {
                        intensity = (3 * r + 6 * g + b) / 10;
                    }
                 }
            }
        }
    }

    //return intensity < 128;

    //for (int i = 0; i < 25; ++i) {
    //    std::cout << " ### " << intensities[i] << " ### ";
    //}
    //std::cout << '\n';

    int n = 0;
    for (int i = 0; i < 25 && intensities[i] != -1; ++i) {
        ++n;
    }
    qsort(intensities, n, sizeof(int), (int(*) (const void*, const void*)) comp);

    //std::cout << "\n";
    //for (int i = 0; i < n; ++i) {
    //    std::cout << intensities[i] << " ";
    //}
    //std::cout << '\n';

    return intensity < intensities[n / 2];
}

image_data threshold_filter::use_filter(image_data image) {
    config_params cfgp;
    cfgp.u = u; cfgp.l = l; cfgp.b = b; cfgp.r = r; cfgp.filter = FILTER::FILTER_WB;
    factory* ft = new factory_impl();
    filter* filt = ft->make_filter(cfgp);
    image = filt->use_filter(image);

    stbi_uc* new_image = new stbi_uc[image.w * image.h * image.compPerPixel];
    copy_pixels(new_image, image);

    filter_edges edges = set_edges(image);

    //std::ofstream os("test.txt");
    //os << " ### ";
    //for (int i = edges.x1; i < edges.x2; ++i) {
    //    for (int j = edges.y1; j < 100; ++j) {
    //        os << (int)image.pixels[3 * (i + j * image.w)] << " | ";
    //    }
    //    os << "\n ### ";
    //}


    for (int i = edges.x1; i < edges.x2; ++i) {
        for (int j = edges.y1; j < edges.y2; ++j) {
            if (is_below_median(image, i, j)) {
                new_image[3 * (i + j * image.w) + 0] = 0;
                new_image[3 * (i + j * image.w) + 1] = 0;
                new_image[3 * (i + j * image.w) + 2] = 0;
            }
            else {
                new_image[3 * (i + j * image.w) + 0] = image.pixels[3 * (i + j * image.w) + 0];
                new_image[3 * (i + j * image.w) + 1] = image.pixels[3 * (i + j * image.w) + 1];
                new_image[3 * (i + j * image.w) + 2] = image.pixels[3 * (i + j * image.w) + 2];
            }
        }
    }
    std::swap(image.pixels, new_image);
    delete[] new_image;
    //os.close();
    delete filt;
    delete ft;
    return image;
}

int matrix_filter::get_norm() const {
    int result = 0;
    for (int i = 0; i < kernel_size; ++i) {
        for (int j = 0; j < kernel_size; ++j) {
            result += kernel[i + j * kernel_size];
        }
    }
    return result;
}

image_data matrix_filter::use_filter(image_data image) {
    stbi_uc* new_image;
    //std::cout << image.w * image.h * image.compPerPixel;
    new_image = new stbi_uc[image.w * image.h * image.compPerPixel];
    copy_pixels(new_image, image);

    filter_edges edges = set_edges(image);
    int norm = get_norm();
    //std::cout << norm;

    //std::ofstream os("test.txt");
    
    for (int i = edges.x1; i < edges.x2; ++i) {
        for (int j = edges.y1; j < edges.y2; ++j) {
            int r = 0, g = 0, b = 0;
            //std::cout << (int)image.pixels[3 * (i + j * image.w) + 0] << " --- ";
            for (int x = i - 1; x <= i + 1; ++x) {
                for (int y = j - 1; y <= j + 1; ++y) {
                    int pixR = 0, pixG = 0, pixB = 0;

                    if (x >= 0 && x < image.w && y >= 0 && y < image.h) {
                        pixR = image.pixels[3 * (x + y * image.w) + 0];
                        pixG = image.pixels[3 * (x + y * image.w) + 1];
                        pixB = image.pixels[3 * (x + y * image.w) + 2];
                    }

                    //std::cout << kernel[x - (i - 1) + (y - (j - 1)) * kernel_size] << ' ';

                    r += pixR * kernel[x - (i - 1) + (y - (j - 1)) * kernel_size];
                    g += pixG * kernel[x - (i - 1) + (y - (j - 1)) * kernel_size];
                    b += pixB * kernel[x - (i - 1) + (y - (j - 1)) * kernel_size];

                }
            }


            //std::cout << '\n';

            r /= norm;
            if (r > 255) r = 255;
            else if (r < 0) r = 0;

            g /= norm;
            if (g > 255) g = 255;
            else if (g < 0) g = 0;

            b /= norm;
            if (b > 255) b = 255;
            else if (b < 0) b = 0;

            //std::cout << i << " --- " << j << '\n';
            new_image[3 * (i + j * image.w) + 0] = (stbi_uc)r;
            new_image[3 * (i + j * image.w) + 1] = (stbi_uc)g;
            new_image[3 * (i + j * image.w) + 2] = (stbi_uc)b;

            //std::cout << (int)new_image[3 * (i + j * image.w) + 0] << '\n';

        }
    }

    //os.close();
    std::swap(image.pixels, new_image);
    delete[] new_image;
    return image;
}

image_data edge_filter::use_filter(image_data image) {
    config_params cfgp;
    cfgp.u = u; cfgp.l = l; cfgp.b = b; cfgp.r = r; cfgp.filter = FILTER::FILTER_WB;
    factory* ft = new factory_impl();
    filter* filt = ft->make_filter(cfgp);
    image = filt->use_filter(image);

    image = matrix_filter::use_filter(image);

    delete filt;
    delete ft;
    return image;
}