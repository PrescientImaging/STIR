
#include "stdlib.h"
#include "stir/find_STIR_config.h"
#include "stir/info.h"
#include "stir/config.h"
#include <iostream>
#include <fstream>

START_NAMESPACE_STIR

std::string find_STIR_config_file(const std::string& filename){
    
    std::string dir;
    dir = STIR_CONFIG_DIR;
    std::ifstream file(dir+"/"+filename);
    if (file)
        info("Using config file from "+dir);
    else
        error("Could note open "+dir+"/"+filename);
    
    return dir+"/"+filename;

}
END_NAMESPACE_STIR