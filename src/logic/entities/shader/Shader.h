#ifndef SHADER_H
#define SHADER_H

#include "../../../../lib/glad/glad_glfw.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <utility>
#include <variant>
#include <stack>
#include <map>
#include "../SceneNode.h"
#include "../../utils/VariantUtil.h"

using ShaderType = std::variant<bool, int, float, glm::vec3, glm::vec4, glm::mat4>;

class Shader {
public:
    unsigned int ID;

    std::string vertexPath;
    std::string tesselationControlPath;
    std::string tesselationEvaluationPath;
    std::string geometryPath;
    std::string fragmentPath;

    int maxUniformNameLength;
    int uniformCount;
    std::unique_ptr<char[]> uniformNameBuffer;
    bool active = true;

    Shader(std::string vertexShaderPath, std::string fragmentShaderPath) :
    vertexPath(std::move(vertexShaderPath)), geometryPath(), fragmentPath(std::move(fragmentShaderPath)), tesselationControlPath(),
    tesselationEvaluationPath() {
        initShader();
    }
    Shader(std::string vertexShaderPath, std::string geometryShaderPath, std::string fragmentShaderPath) :
    vertexPath(std::move(vertexShaderPath)), geometryPath(std::move(geometryShaderPath)),
    fragmentPath(std::move(fragmentShaderPath)), tesselationControlPath(), tesselationEvaluationPath()  {
        initShader();
    }
    Shader(std::string vertexShaderPath, std::string tesselationControlPath, std::string tesselationEvaluationPath, std::string fragmentShaderPath) :
            vertexPath(std::move(vertexShaderPath)), geometryPath(),
            fragmentPath(std::move(fragmentShaderPath)), tesselationControlPath(std::move(tesselationControlPath)),
            tesselationEvaluationPath(std::move(tesselationEvaluationPath))  {
        initShader();
    }

    void setActive(bool toggle) { active = toggle; };

    unsigned int loadShader(const std::string& path, int type, const std::string& name) {
        unsigned int shader;
        std::string code;
        std::ifstream file;
        file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try{
            file.open(path);
            std::stringstream stream;
            stream << file.rdbuf();
            file.close();
            code = stream.str();
        }
        catch (std::ifstream::failure &e) {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
        }
        const char *pCode = code.c_str();
        shader = glCreateShader(type);
        glShaderSource(shader, 1, &pCode, NULL);
        glCompileShader(shader);
        checkCompileErrors(shader, name);

        return shader;
    }

    void initShader() {
        unsigned int vertex, tesselationControl, tesselationEvaluation, geometry, fragment;

        vertex = loadShader(vertexPath, GL_VERTEX_SHADER, "VERTEX");
        if(!tesselationControlPath.empty()) tesselationControl = loadShader(tesselationControlPath, GL_TESS_CONTROL_SHADER, "TESS_CTRL");
        if(!tesselationEvaluationPath.empty()) tesselationEvaluation = loadShader(tesselationEvaluationPath, GL_TESS_EVALUATION_SHADER, "TESS_EVAL");
        if(!geometryPath.empty()) geometry = loadShader(geometryPath, GL_GEOMETRY_SHADER, "GEOMETRY");
        fragment = loadShader(fragmentPath, GL_FRAGMENT_SHADER, "FRAGMENT");

        ID = glCreateProgram();

        glAttachShader(ID, vertex);
        if(!tesselationControlPath.empty()) glAttachShader(ID, tesselationControl);
        if(!tesselationEvaluationPath.empty()) glAttachShader(ID, tesselationEvaluation);
        if(!geometryPath.empty()) glAttachShader(ID, geometry);
        glAttachShader(ID, fragment);

        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");

        glDeleteShader(vertex);
        if(!tesselationControlPath.empty()) glDeleteShader(tesselationControl);
        if(!tesselationEvaluationPath.empty()) glDeleteShader(tesselationEvaluation);
        if(!geometryPath.empty()) glDeleteShader(geometry);
        glDeleteShader(fragment);

        initFields();
    }

    void initFields() {
        glGetProgramiv(ID, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniformNameLength);
        glGetProgramiv(ID, GL_ACTIVE_UNIFORMS, &uniformCount);
        uniformNameBuffer = std::unique_ptr<char[]>(new char[maxUniformNameLength]);
        active = true;
    }

    // ------------------------------------------------------------------------
    // Activate the shader
    void use() const {
        glUseProgram(ID);
    }

    // ------------------------------------------------------------------------
    // Hot-reload the shader
    void hotReload() {
        glDeleteProgram(ID);
        initShader();
        // What about uniforms that have to be set on init (like color in flat).
    }
    // ------------------------------------------------------------------------
    // uniform setter

    void setUniform(const std::string &name, ShaderType &&value) const {
        std::visit(overloaded{
                [&](bool &v) { glUniform1i(glGetUniformLocation(ID, name.c_str()), (int) v); },
                [&](int &v) { glUniform1i(glGetUniformLocation(ID, name.c_str()), v); },
                [&](float &v) { glUniform1f(glGetUniformLocation(ID, name.c_str()), v); },
                [&](glm::vec3 &v) { glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &v[0]); },
                [&](glm::vec4 &v) { glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &v[0]); },
                [&](glm::mat4 &v) { glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &v[0][0]); }
        }, value);
    };

private:
    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    void checkCompileErrors(unsigned int shader, std::string type) {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM") {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog
                          << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        } else {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog
                          << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }
};

#endif
