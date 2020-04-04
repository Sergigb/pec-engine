#include "log.hpp"
#include "utils.hpp"
#include "App.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#include <stb/stb_image.h>


int main(){
	if(change_cwd_to_selfpath() == EXIT_FAILURE)
		std::cerr << "Could not change the cwd to executable path, proceeding" << std::endl;

	log_start();
/*
    App* app = new App();
    app->run();
    delete app;
*/
    while(1){
        App* app = new App();
        delete app;
    }



	return EXIT_SUCCESS;
}



