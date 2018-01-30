#include "GlfwInstance.h"
#include "SDFScene.h"

#include <cstdio>

int main(int argc, char* argv[])
{
    GlfwInstance instance;
    
    if (!instance.init(SDFScene::NAME, SDFScene::WIDTH, SDFScene::HEIGHT))
    {
        fprintf(stderr, "Failed to init instance\n");
        return -1;
    }
    
    SDFScene scene;
    
    if (!scene.init())
    {
        fprintf(stderr, "Failed to init scene\n");
        return -1;
    }
    
    instance.setScene(&scene);
    
    instance.run();

    scene.close();

    instance.close();

    return 0;
}
