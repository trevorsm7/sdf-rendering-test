#ifndef __GLFWINSTANCE_H__
#define __GLFWINSTANCE_H__

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

class Scene
{
public:
    virtual ~Scene() {}
    
    virtual void render() = 0;
    virtual void update(double elapsedTime) = 0;
};

class GlfwInstance
{
private:
    struct {float x, y;} m_fbScale;
    GLFWwindow* m_window;
    Scene* m_scene;

public:
    GlfwInstance(): m_window(nullptr), m_scene(nullptr) {}
    ~GlfwInstance() {close();}

    // TODO maintain list of scenes that can be toggled between?
    void setScene(Scene* scene) {m_scene = scene;}

    bool init(const char* name, int width, int height);
    void close();
    void run();

    static void callback_error(int error, const char* description);
    static void callback_key(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void callback_mouse_button(GLFWwindow* window, int button, int action, int mods);
    static void callback_mouse_motion(GLFWwindow* window, double xpos, double ypos);
    static void callback_window(GLFWwindow* window, int width, int height);
};

#endif //__GLFWINSTANCE_H__
