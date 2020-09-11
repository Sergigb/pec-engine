#include "log.hpp"
#include "utils.hpp"
#include "App.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#include <stb/stb_image.h>


int main(int argc, char* argv[]){
    UNUSED(argc); UNUSED(argv);

    if(change_cwd_to_selfpath() == EXIT_FAILURE)
        std::cerr << "Could not change the cwd to executable path, proceeding" << std::endl;

    log_start();

    App* app = new App();
    app->run();
    delete app;

    return EXIT_SUCCESS;
}

