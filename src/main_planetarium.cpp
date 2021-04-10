#include "Planetarium.hpp"
#include "core/log.hpp"
#include "core/utils/utils.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#include <stb/stb_image.h>


int main(int argc, char* argv[]){
    UNUSED(argc); UNUSED(argv);

    if(change_cwd_to_selfpath() == EXIT_FAILURE)
        std::cerr << "Could not change the cwd to executable path, proceeding" << std::endl;

    log_start();

    Planetarium* p = new Planetarium();
    p->run();
    delete p;

            while(1){
        
    }

    return EXIT_SUCCESS;
}

