#include <iostream>
#include "png_toolkit.h"

int main(int argc, char* argv[]) {
    // toolkit filter_name base_pic_name sudent_tool student_pic_name limitPix limitMSE
    // toolkit near test images!
    try {
        if (argc != 4) {
            throw "Not enough arguments";
        }

        std::string filename = argv[1];

        config_parser* cfg = new config_parser(filename);

        png_toolkit studTool;
        studTool.load(argv[2]);
        factory* ft = new factory_impl();

        while (!cfg->end_of_file()) {
            config_params cfgp = cfg->parse();
            filter* filt = ft->make_filter(cfgp);

            image_data image = filt->use_filter(studTool.getPixelData());

            studTool.setPexelData(image);

            delete filt;
        }

        studTool.save(argv[3]);
        delete cfg;
        delete ft;
    }
    catch (const char* str) {
        std::cout << "Error: " << str << std::endl;
        return 1;
    }

    return 0;
}