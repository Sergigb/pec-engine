#include <unordered_map>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#include <stb/stb_image.h>

#include "core/log.hpp"
#include "core/utils/utils.hpp"
#include "core/loading/load_star_system.hpp"


int main(int argc, char* argv[]){
    UNUSED(argc); UNUSED(argv);

    if(change_cwd_to_selfpath() == EXIT_FAILURE)
        std::cerr << "Could not change the cwd to executable path, proceeding" << std::endl;

    log_start();

    struct planetary_system system;
    load_star_system(system);


    return EXIT_SUCCESS;
}

