#include "SDFScene.h"

#include <cstdint>
#include <cstdio>
#include <memory>
#include <cmath>
#include <algorithm>

static GLuint loadShader(const char* shaderCode, GLenum shaderType)
{
    // Compile the shader file
    GLuint shader = glCreateShader(shaderType);
    const char* sourceArray[] = {shaderCode};
    glShaderSource(shader, 1, sourceArray, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (success == GL_FALSE)
    {
        // Get the length of the error log
        GLint logLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

        // Get the error log and print
        char* errorLog = new char [logLength];
        glGetShaderInfoLog(shader, logLength, &logLength, errorLog);
        fprintf(stderr, "%s\n", errorLog);
        delete[] errorLog;

        // Exit with failure
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

static void makeCircle(int texSize, float radius)
{
    auto texData = std::make_unique<int8_t[]>(texSize * texSize);
    for (int y = 0; y < texSize; ++y)
    {
        const float fy = y - (texSize / 2) + 0.5f;
        for (int x = 0; x < texSize; ++x)
        {
            const float fx = x - (texSize / 2) + 0.5f;
            texData[y*texSize+x] = (radius - sqrtf(fx*fx+fy*fy)) * 127 / radius;
            //printf("%d ", int(texData[y*texSize+x]));
        }
        //printf("\n");
    }

    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 4, 4, 0, GL_RED, GL_UNSIGNED_BYTE, texData);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8_SNORM, texSize, texSize, 0, GL_RED, GL_BYTE, texData.get());
}

static void makeSquare(int texSize, float radius)
{
    auto texData = std::make_unique<int8_t[]>(texSize * texSize);
    for (int y = 0; y < texSize; ++y)
    {
        const float fy = y - (texSize / 2) + 0.5f;
        const bool insideY = fy >= -radius || fy <= radius;
        
        for (int x = 0; x < texSize; ++x)
        {
            const float fx = x - (texSize / 2) + 0.5f;
            const bool insideX = fx >= -radius || fx <= radius;
            
            if (insideX)
            {
                if (insideY)
                {
                    texData[y*texSize+x] = std::min
                    ({
                        radius - fx,
                        radius + fx,
                        radius - fy,
                        radius + fy
                    }) * 127 / radius;
                }
                else
                {
                    texData[y*texSize+x] = (radius - fabsf(fy)) * 127 / radius;
                }
            }
            else
            {
                if (insideY)
                {
                    texData[y*texSize+x] = (radius - fabsf(fx)) * 127 / radius;
                }
                else
                {
                    texData[y*texSize+x] = std::min
                    ({
                        sqrtf((fx-radius)*(fx-radius) + (fy-radius)*(fy-radius)),
                        sqrtf((fx-radius)*(fx-radius) + (fy+radius)*(fy+radius)),
                        sqrtf((fx+radius)*(fx+radius) + (fy-radius)*(fy-radius)),
                        sqrtf((fx+radius)*(fx+radius) + (fy+radius)*(fy+radius))
                    }) * -127 / radius;
                }
            }
            //printf("%d ", int(texData[y*texSize+x]));
        }
        //printf("\n");
    }

    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 4, 4, 0, GL_RED, GL_UNSIGNED_BYTE, texData);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8_SNORM, texSize, texSize, 0, GL_RED, GL_BYTE, texData.get());
}

void SDFScene::computeSDF()
{
    glBindTexture(GL_TEXTURE_2D, m_texture);

    float size = exp2(m_texPow.get());
    float radius = m_radius.get() * size * 0.01f;
    if (m_drawCircle.get())
        makeCircle(size, radius);
    else
        makeSquare(size, radius);
}

bool SDFScene::init()
{
    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    computeSDF();

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
    
    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    float verData[8] = {0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 1.f, 1.f};
    glBufferData(GL_ARRAY_BUFFER, sizeof(verData), verData, GL_STATIC_DRAW);

    GLint a_vertex = 0;
    glEnableVertexAttribArray(a_vertex);
    glVertexAttribPointer(a_vertex, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindVertexArray(0);

    const char* vertexShader =
    "#version 410\n"
    "layout(location = 0) in vec2 a_vertex;\n"
    "out vec2 v_texCoord;\n"
    "void main() {\n"
    "v_texCoord = vec2(a_vertex.x, 1 - a_vertex.y);\n"
    "gl_Position = vec4((a_vertex - vec2(0.5, 0.5)) * 2, 0.0, 1.0);\n"
    "}\n";

    const char* fragmentShader =
    "#version 410\n"
    "uniform sampler2D u_texture;\n"
    "uniform float u_useSDFShader;\n"
    "in vec2 v_texCoord;\n"
    "out vec4 f_color;\n"
    "void main() {\n"
    "float sdfSample = texture(u_texture, v_texCoord).r;\n"
    "vec3 unshadedColor = sdfSample * vec3(1, 1, 1);\n"
    "float mask_outout = step(-0.46, sdfSample);\n"
    "float mask_outin = step(-0.4, sdfSample);\n"
    "float mask_outer = step(-0.03, sdfSample);\n"
    "float mask_inner = step(0.03, sdfSample);\n"
    "float mask_inout = step(0.4, sdfSample);\n"
    "float mask_inin = step(0.46, sdfSample);\n"
    //"float mask = clamp(texture(u_texture, v_texCoord).r + 0.05, 0.0, 0.1) * 10.0;\n"
    //"float mask = texture(u_texture, v_texCoord).r;\n"
    //"f_color = vec4(color * mask, 1.0);\n"
    "vec3 color_outout = vec3(1.0, 0.8, 0.8) * (mask_outout - mask_outin);\n"
    "vec3 color_outin = vec3(0.6, 0.3, 0.3) * (mask_outin - mask_outer);\n"
    "vec3 color_outer = vec3(0.8, 0.8, 1.0) * (mask_outer - mask_inner);\n"
    "vec3 color_inner = vec3(0.3, 0.3, 0.6) * (mask_inner - mask_inout);\n"
    "vec3 color_inout = vec3(0.8, 1.0, 0.8) * (mask_inout - mask_inin);\n"
    "vec3 color_inin = vec3(0.3, 0.6, 0.3) * mask_inin;\n"
    "vec3 shadedColor = color_outout + color_outin + color_outer + color_inner + color_inout + color_inin;\n"
    //"vec3 finalColor = u_useSDFShader * (color_outer + color_inner)\n"
    //"    + (1 - u_useSDFShader) * sdfSample * vec3(1, 1, 1);\n"
    "vec3 finalColor = u_useSDFShader * shadedColor + (1 - u_useSDFShader) * unshadedColor;\n"
    "f_color = vec4(finalColor, 1.0);\n"
    //"float r = clamp(texture(u_texture, v_texCoord).r + 0.5, 0, 0.5) * 2;\n"
    //"float g = clamp(texture(u_texture, v_texCoord).r, 0, 0.5) * 2;\n"
    //"float b = clamp(texture(u_texture, v_texCoord).r - 0.5, 0, 0.5) * 2;\n"
    //"f_color = vec4(r, g, b, 1.0);\n"
    "}\n";

    m_shader = glCreateProgram();
    glAttachShader(m_shader, loadShader(vertexShader, GL_VERTEX_SHADER));
    glAttachShader(m_shader, loadShader(fragmentShader, GL_FRAGMENT_SHADER));
    glLinkProgram(m_shader);

    GLint success;
    glGetProgramiv(m_shader, GL_LINK_STATUS, &success);

    if (success == GL_FALSE)
    {
        // Get the length of the error log
        GLint logLength = 0;
        glGetProgramiv(m_shader, GL_INFO_LOG_LENGTH, &logLength);

        // Get the error log and print
        char* errorLog = new char [logLength];
        glGetProgramInfoLog(m_shader, logLength, &logLength, errorLog);
        fprintf(stderr, "%s\n", errorLog);
        delete[] errorLog;

        // Exit with failure
        glDeleteProgram(m_shader);
        m_shader = 0;
        return false;
    }

    glUseProgram(m_shader);
    glUniform1i(glGetUniformLocation(m_shader, "u_texture"), 0);
    glUniform1f(glGetUniformLocation(m_shader, "u_useSDFShader"), m_useSDFShader.get() ? 1.f : 0.f);
    glUseProgram(0);

    // Create tweak bar
    //TwSetCurrentWindow(...);
    m_tweakBar = TwNewBar("TweakBar");
    TwDefine(" TweakBar size='150 400' color='96 216 224' fontsize=3 "); // "fontscaling=fb/window"
    m_drawCircle.init(m_tweakBar, "Draw Circle", "", std::bind(&SDFScene::computeSDF, this));
    m_useBilinear.init(m_tweakBar, "Bilinear Filter", "", [texture=m_texture](bool useBilinear)
    {
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, useBilinear ? GL_LINEAR : GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0);
    });
    m_useSDFShader.init(m_tweakBar, "SDF Shader", "", [shader=m_shader](bool useSDF)
    {
        glUseProgram(shader);
        glUniform1f(glGetUniformLocation(shader, "u_useSDFShader"), useSDF ? 1.f : 0.f);
        glUseProgram(0);
    });
    m_texPow.init(m_tweakBar, "Tex Pow", " min=2 max=6 ", std::bind(&SDFScene::computeSDF, this));
    m_radius.init(m_tweakBar, "Radius", " min=0.1 max=100 step=0.1 ", std::bind(&SDFScene::computeSDF, this));
    
    return true;
}

void SDFScene::close()
{
    if (m_tweakBar)
    {
        TwDeleteBar(m_tweakBar);
        m_tweakBar = nullptr;
    }
}

void SDFScene::render()
{
    // render scene
    glUseProgram(m_shader);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
    
    // Draw TweakBar on top
    //TwRefreshBar(tweakBar); // only necessary if we update parameters externally
    TwDraw();
}

void SDFScene::update(double elapsedTime)
{
    static double counter = 0;
    static float scale = 0.99f;
    counter += elapsedTime;
    bool updated = false;
    while (counter > 0.016)
    {
        updated = true;
        float radius = m_radius.get();
        if (radius < 0.8f) scale = 1.01;
        else if (radius > 4.f) scale = 0.99f;
        m_radius.set(radius * scale);
        counter -= 0.1;
    }
    if (updated)
        computeSDF();
}
