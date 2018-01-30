#ifndef __SDFSCENE_H__
#define __SDFSCENE_H__

#include "GlfwInstance.h"
#include "TwWrapper.h"

#include <cstdint>

class SDFScene : public Scene
{
public:
    // NOTE have to move these before the template decl
    void computeSDF();

private:
    TwBar* m_tweakBar;
    GLuint m_texture;
    GLuint m_shader;
    GLuint m_vao, m_vbo;
    TwWrapper<int32_t> m_texPow;
    TwWrapper<float> m_radius;
    TwWrapper<bool> m_drawCircle;
    TwWrapper<bool> m_useBilinear;
    TwWrapper<bool> m_useSDFShader;

public:
    SDFScene(): m_tweakBar(nullptr), m_texture(0), m_shader(0), m_vao(0), m_vbo(0),
        m_texPow(5), m_radius(4.f), m_drawCircle(true), m_useBilinear(true), m_useSDFShader(true) {}
    ~SDFScene() {close();}

    bool init();
    void close();
    
    void render() override;
    void update(double elapsedTime) override;
    
    static constexpr const char* const NAME = "SDF Test";
    static constexpr const int WIDTH = 800;
    static constexpr const int HEIGHT = 800;
};

#endif //__SDFSCENE_H__
