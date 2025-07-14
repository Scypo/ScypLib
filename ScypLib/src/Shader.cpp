#include<cassert>
#include<fstream>
#include<sstream>
#include<unordered_map>
#include<string>

#include<GL/glew.h>

#include"ScypLib/Shader.h"

namespace sl
{
    Shader::Shader(const std::string& vertex, const std::string& fragment, bool isPath)
    {
        if(isPath)
        {
            std::string vertexShader = LoadShader(vertex);
            std::string fragmentShader = LoadShader(fragment);
            handle = CreateShader(vertexShader, fragmentShader);
        }
        else
        {
            handle = CreateShader(vertex, fragment);
        }
    }

    Shader& Shader::operator=(Shader&& other) noexcept
    {
        if (this != &other)
        {
            if (handle != 0)
            {
                glDeleteProgram(handle);
            }

            handle = other.handle;
            uniformLocationCache = std::move(other.uniformLocationCache);
            other.handle = 0;
        }
        return *this;
    }

    Shader::~Shader()
    {
        glDeleteProgram(handle);
    }

    std::string Shader::LoadShader(const std::string& filepath)
    {
        std::ifstream file(filepath);
        assert(file.good());
        std::stringstream ss;
        std::string line;

        while (std::getline(file, line))
        {
            ss << line << '\n';
        }
        return ss.str();
    }

    void Shader::ParseShader(const std::string& filepath, std::string& vertexShader, std::string& fragmentShader)
    {
        std::ifstream file(filepath);
        assert(file.good());

        enum class  ShaderType
        {
            NONE = -1, VERTEX = 0, FRAGMENT = 1
        };
        ShaderType type = ShaderType::NONE;

        std::string line;
        std::stringstream ss[2];

        while (std::getline(file, line))
        {
            if (line.find("#shader") != std::string::npos)
            {
                if (line.find("vertex") != std::string::npos)
                {
                    type = ShaderType::VERTEX;
                }
                else if (line.find("fragment") != std::string::npos)
                {
                    type = ShaderType::FRAGMENT;
                }
            }
            else
            {
                ss[int(type)] << line << '\n';
            }
        }

        vertexShader = ss[0].str();
        fragmentShader = ss[1].str();
    }

    unsigned int Shader::CompileShader(unsigned int type, const std::string& source)
    {
        unsigned int id = glCreateShader(type);
        const char* src = source.c_str();
        glShaderSource(id, 1, &src, NULL);
        glCompileShader(id);
        int result;
        glGetShaderiv(id, GL_COMPILE_STATUS, &result);
        if (result == GL_FALSE)
        {
            int length;
            glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
            char* message = (char*)_malloca(length * sizeof(char));
            glGetShaderInfoLog(id, length, &length, message);
            throw std::runtime_error(message);
            glDeleteShader(id);
        }
        return id;
    }

    unsigned int Shader::CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
    {
        unsigned int program = glCreateProgram();
        unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
        unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

        glAttachShader(program, vs);
        glAttachShader(program, fs);

        glLinkProgram(program);
        glValidateProgram(program);

        glDeleteShader(vs);
        glDeleteShader(fs);

        return program;
    }

    int Shader::GetUniformLocation(const std::string& name) const
    {
        if (uniformLocationCache.contains(name)) return uniformLocationCache[name];

        int location = glGetUniformLocation(handle, name.c_str());
        assert(location != -1 && ("uniform" + name + "does not exist").c_str());
        uniformLocationCache[name] = location;
        return location;
    }

    void Shader::SetUniform4i(const std::string& name, int v0, int v1, int v2, int v3)
    {
        glUniform4i(GetUniformLocation(name), v0, v1, v2, v3);
    }

    void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3)
    {
        glUniform4f(GetUniformLocation(name), v0, v1, v2, v3);
    }

    void Shader::SetUniform1f(const std::string& name, float v)
    {
        glUniform1f(GetUniformLocation(name), v);
    }

    void Shader::SetUniform1i(const std::string& name, int v)
    {
        glUniform1i(GetUniformLocation(name), v);
    }

    void Shader::SetUniform1iv(const std::string& name, int count, int* data)
    {
        glUniform1iv(GetUniformLocation(name), count, data);
    }

    void Shader::SetUniformMat4f(const std::string& name, const glm::mat4& matrix)
    {
        glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]);
    }
}