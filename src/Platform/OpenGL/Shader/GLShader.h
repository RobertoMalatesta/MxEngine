// Copyright(c) 2019 - 2020, #Momo
// All rights reserved.
// 
// Redistributionand use in sourceand binary forms, with or without
// modification, are permitted provided that the following conditions are met :
// 
// 1. Redistributions of source code must retain the above copyright notice, this
// list of conditionsand the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditionsand the following disclaimer in the documentation
// and /or other materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once

#include "Core/Interfaces/GraphicAPI/Shader.h"
#include "Utilities/Math/Math.h"

#include <unordered_map>

namespace MxEngine
{
	class GLShader : public Shader
	{
        enum class ShaderType
        {
            FRAGMENT_SHADER = 0x8B30,
            VERTEX_SHADER = 0x8B31,
        };

		#ifdef _DEBUG
		std::string vertexShaderPath, fragmentShaderPath;
		#endif
        using UniformType = int;
        using UniformCache = std::unordered_map<std::string, UniformType>;
        using ShaderId = unsigned int;

		mutable UniformCache uniformCache;

        std::string ReadFile(const std::string& path) const;
		ShaderId CompileShader(ShaderType type, std::string& source, const std::string& name) const;
		IBindable::IdType CreateProgram(ShaderId vertexShader, ShaderId fragmentShader) const;
		UniformType GetUniformLocation(const std::string& uniformName) const;
	public:
		GLShader();
		GLShader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
		GLShader(const GLShader&) = delete;
		GLShader(GLShader&& shader) noexcept;
		~GLShader();

        // Inherited via IShader
        virtual void Bind() const override;
        virtual void Unbind() const override;
        virtual void Load(const std::string& vertexShaderPath, const std::string& fragmentShaderPath) override;
        virtual void SetUniformFloat(const std::string& name, float f) const override;
        virtual void SetUniformVec3(const std::string& name, float f1, float f2, float f3) const override;
        virtual void SetUniformVec4(const std::string& name, float f1, float f2, float f3, float f4) const override;
        virtual void SetUniformVec3(const std::string& name, const Vector3& vec) const override;
        virtual void SetUniformVec4(const std::string& name, const Vector3& vec) const override;
        virtual void SetUniformMat4(const std::string& name, const Matrix4x4& matrix) const override;
        virtual void SetUniformMat3(const std::string& name, const Matrix3x3& matrix) const override;
        virtual void SetUniformInt(const std::string& name, int i) const override;
    };
}