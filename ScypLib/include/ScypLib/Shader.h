#pragma once
#include<string>
#include<unordered_map>

#include<glm/glm.hpp>

namespace sl
{
	class Shader
	{
	public:
		Shader(const std::string& vertex, const std::string& fragment, bool isPath);
		Shader() = default;
		Shader& operator=(Shader&& other) noexcept;
		~Shader();
		void SetUniform1f(const std::string& name, float v);
		void SetUniform1i(const std::string& name, int v);
		void SetUniform1iv(const std::string& name, int count, int* data);
		void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3);
		void SetUniform4i(const std::string& name, int v0, int v1, int v2, int v3);
		void SetUniformMat4f(const std::string& name, const glm::mat4& matrix);
		unsigned int GetHandle() const { return handle; };
	private:
		std::string LoadShader(const std::string& filepath);
		void ParseShader(const std::string& filepath, std::string& vertexShader, std::string& fragmentShader);
		unsigned int CompileShader(unsigned int type, const std::string& source);
		unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader);
		int GetUniformLocation(const std::string& name) const;
	private:
		unsigned int handle;
		mutable std::unordered_map<std::string, int> uniformLocationCache;

	};
}