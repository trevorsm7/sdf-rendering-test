#include "GlfwInstance.h"

#include <AntTweakBar.h>
#include <cstdio>

bool GlfwInstance::init(const char* name, int width, int height)
{
    // Init GLFW
    glfwSetErrorCallback(callback_error);
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to init GLFW\n");
        return false;
    }

    // Use OpenGL Core v4.1
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    m_window = glfwCreateWindow(width, height, name, nullptr, nullptr);
    if (m_window == nullptr)
    {
        fprintf(stderr, "Failed to create window\n");
        return false;
    }

    glfwSetWindowUserPointer(m_window, this);

    // Set GLFW callbacks
    glfwSetKeyCallback(m_window, callback_key);
    glfwSetCursorPosCallback(m_window, callback_mouse_motion);
    glfwSetMouseButtonCallback(m_window, callback_mouse_button);
    //glfwSetScrollCallback(m_window, scrollCallback);
    glfwSetWindowSizeCallback(m_window, callback_window);
    //glfwSetFramebufferSizeCallback(m_window, resizeCallback);

    // Force vsync on current context
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);
    
    // Send an initial resize notification to scene
    //int width, height;
    //glfwGetWindowSize(m_window, &width, &height);
    callback_window(m_window, width, height);

    // Initialize AntTweakBar
    TwInit(TW_OPENGL_CORE, nullptr);

    return true;
}

void GlfwInstance::close()
{
    if (m_window)
    {
        TwTerminate();
        glfwTerminate();
        m_window = nullptr;
    }
}

void GlfwInstance::run()
{
    // The first event poll will take extra time; get it out of the way first
    glfwPollEvents();
    
    // TODO make sure to preload scene resources so we get off to a clean start?

    double lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(m_window))
    {
        int width, height;
        glfwGetFramebufferSize(m_window, &width, &height);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render scene
        if (m_scene)
            m_scene->render();

        glfwSwapBuffers(m_window);

        // Compute elapsed time since last update
        // TODO: check for overflow
        const double currentTime = glfwGetTime();
        const double elapsedTime = currentTime - lastTime;
        lastTime = currentTime;

        // Poll events after checking time for consistency
        glfwPollEvents();

        // Update scene
        if (m_scene)
            m_scene->update(elapsedTime);
    }
}

void GlfwInstance::callback_error(int error, const char* description)
{
    fprintf(stderr, "%s\n", description);
}

void GlfwInstance::callback_key(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // TODO add flag for if scene has mouse focus/priority for events
    if (TwEventKeyGLFW(key, action))
        return;

    // Ignore GLFW_REPEAT events
    if (action == GLFW_PRESS || action == GLFW_RELEASE)
    {
        //void* ptr = glfwGetWindowUserPointer(window);
        //auto instance = reinterpret_cast<GlfwInstance*>(ptr);

        /*if (instance && instance->m_scene)
        {
            auto keyIt = instance->m_keymap.find(key);
            if (keyIt != instance->m_keymap.end())
            {
                ControlEvent event;
                event.name = keyIt->second;
                event.down = (action == GLFW_PRESS);

                if (instance->m_scene->controlEvent(event))
                    return;
            }
        }*/

        // If key is unhandled, provide default behavior for escape
        if (key == GLFW_KEY_ESCAPE)
            glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

void GlfwInstance::callback_mouse_button(GLFWwindow* window, int button, int action, int mods)
{
    // TODO add flag for if scene has mouse focus/priority for events
    if (TwEventMouseButtonGLFW(button, action))
        return;

    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        //void* ptr = glfwGetWindowUserPointer(window);
        //auto instance = reinterpret_cast<GlfwInstance*>(ptr);

        /*if (instance && instance->m_scene)
        {
            double x, y;
            int width, height;
            glfwGetCursorPos(window, &x, &y);
            glfwGetWindowSize(window, &width, &height);

            MouseEvent event;
            event.x = floor(x);
            event.y = height - floor(y) - 1;
            event.w = width;
            event.h = height;
            event.down = (action == GLFW_PRESS);

            instance->m_scene->mouseEvent(event);
        }*/
    }
}

void GlfwInstance::callback_mouse_motion(GLFWwindow* window, double xpos, double ypos)
{
    void* ptr = glfwGetWindowUserPointer(window);
    auto instance = reinterpret_cast<GlfwInstance*>(ptr);
    
    float& fbScaleX = instance->m_fbScale.x;
    float& fbScaleY = instance->m_fbScale.y;

    // TODO add flag for if scene has mouse focus/priority for events
    TwMouseMotion(xpos * fbScaleX, ypos * fbScaleY);
}

void GlfwInstance::callback_window(GLFWwindow* window, int width, int height)
{
    void* ptr = glfwGetWindowUserPointer(window);
    auto instance = reinterpret_cast<GlfwInstance*>(ptr);

    /*if (instance && instance->m_scene)
        instance->m_scene->resize(width, height);*/

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    instance->m_fbScale.x = float(fbWidth) / float(width);
    instance->m_fbScale.y = float(fbHeight) / float(height);

    // AntTweakBar stupidly doesn't handle DPI scaling, so must use raw framebuffer size
    TwWindowSize(fbWidth, fbHeight);
}
