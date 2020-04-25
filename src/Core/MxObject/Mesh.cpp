// Copyright(c) 2019 - 2020, #Momo
// All rights reserved.
// 
// Redistributionand use in source and binary forms, with or without
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

#include "Mesh.h"
#include "Utilities/ObjectLoader/ObjectLoader.h"
#include "Utilities/Profiler/Profiler.h"
#include "Core/Interfaces/GraphicAPI/GraphicFactory.h"
#include "Utilities/LODGenerator/LODGenerator.h"
#include "Utilities/Format/Format.h"

#include <algorithm>

namespace MxEngine
{
	void CopyMaterial(Material& material, const MaterialInfo& mat, std::unordered_map<std::string, Ref<Texture>>& textures)
	{
		#define MAKE_TEX(tex) if(!mat.tex.empty()) {\
			if(textures.find(mat.tex) == textures.end())\
				textures[mat.tex] = Graphics::Instance()->CreateTexture(mat.tex);\
			material.tex = textures[mat.tex];}

		MAKE_TEX(map_Ka);
		MAKE_TEX(map_Kd);
		MAKE_TEX(map_Ks);
		MAKE_TEX(map_Ke);
		MAKE_TEX(map_d);
		MAKE_TEX(map_bump);
		MAKE_TEX(bump);

		material.Tf    = mat.Tf;
		material.Ka    = mat.Ka;
		material.Kd    = mat.Kd;
		material.Ke    = mat.Ke;
		material.Ks    = mat.Ks;
		material.illum = mat.illum;
		material.Ns    = mat.Ns;
		material.Ni    = mat.Ni;
		material.d     = mat.d;
		material.Tr    = mat.Tr;

		if (material.Ns == 0.0f) material.Ns = 128.0f; // bad as pow(0.0, 0.0) -> NaN
	}

	void Mesh::LoadFromFile(const std::string& filepath)
	{
		ObjectInfo objectInfo = ObjectLoader::Load(filepath);
		this->boundingBox = objectInfo.boundingBox;

		std::unordered_map<std::string, Ref<Texture>> textures;
		std::unordered_map<uintptr_t, Ref<Material>> materials;
		for (const auto& group : objectInfo.meshes)
		{
			if (group.useTexture)
			{
				if (group.material != nullptr)
				{
					auto material = MakeRef<Material>();
					CopyMaterial(*material, *group.material, textures);
					materials[(uintptr_t)group.material] = material;
				}
			}
		}

		std::vector<ObjectInfo> LODdata;
		{
			MAKE_SCOPE_TIMER("MxEngine::LODGenerator", "GenerateLODs");
			MAKE_SCOPE_PROFILER("LODGenerator::GenerareLODs");

			LODGenerator lod(objectInfo);
			std::array LODfactors = { 0.001f, 0.01f, 0.05f, 0.15f, 0.3f };
			for (size_t factor = 0; factor < LODfactors.size(); factor++)
			{
				LODdata.push_back(lod.CreateObject(LODfactors[factor]));

				#if defined(MXENGINE_DEBUG)
				size_t vertecies = 0;
				for (const auto& mesh : LODdata.back().meshes)
				{
					vertecies += mesh.faces.size();
				}
				Logger::Instance().Debug("MxEngine::LODGenerator", Format("LOD[{0}]: vertecies = {1}", factor + 1, vertecies));
				#endif
			}
		}
		LODdata.insert(LODdata.begin(), std::move(objectInfo));

		MAKE_SCOPE_TIMER("MxEngine::Mesh", "GenerateBuffers");
		MAKE_SCOPE_PROFILER("Mesh::GenerateBuffers"); 
		this->LODs.clear();
		for(const auto& lod : LODdata)
		{
			auto& meshes = this->LODs.emplace_back();
			for (const auto& mesh : lod.meshes)
			{
				auto VBO = Graphics::Instance()->CreateVertexBuffer(mesh.buffer.data(), mesh.buffer.size(), UsageType::STATIC_DRAW);
				auto IBO = Graphics::Instance()->CreateIndexBuffer(mesh.faces.data(), mesh.faces.size());
				auto VAO = Graphics::Instance()->CreateVertexArray();
				auto VBL = Graphics::Instance()->CreateVertexBufferLayout();

				VBL->PushFloat(3);
				if (mesh.useTexture) VBL->PushFloat(2);
				if (mesh.useNormal) VBL->PushFloat(3);
				VAO->AddBuffer(*VBO, *VBL);

				RenderObject object(std::move(mesh.name), std::move(VBO), std::move(VAO), std::move(IBO), materials[(uintptr_t)mesh.material], 
					mesh.useTexture, mesh.useNormal, mesh.buffer.size());
				meshes.push_back(std::move(object));
				if (!mesh.useTexture)
				{
					Logger::Instance().Warning("MxEngine::MxObject", "object file does not have texture data: " + filepath);
				}
				if (!mesh.useNormal)
				{
					Logger::Instance().Warning("MxEngine::MxObject", "object file does not have normal data: " + filepath);
				}
			}
		}
	}

	Mesh::Mesh(const std::string& filepath)
	{
		LoadFromFile(filepath);
	}

	void Mesh::Load(const std::string& filepath)
	{
		LoadFromFile(filepath);
	}

	std::vector<RenderObject>& Mesh::GetRenderObjects()
	{
		return this->LODs[this->currentLOD];
	}

	const std::vector<RenderObject>& Mesh::GetRenderObjects() const
	{
		return this->LODs[this->currentLOD];
	}

	Vector3 Mesh::GetObjectCenter() const
	{
		return (this->boundingBox.first + this->boundingBox.second) * 0.5f;
	}

	void Mesh::SetLOD(size_t LOD)
	{
		this->currentLOD = Min(LOD, (int)this->LODs.size() - 1);
	}

    size_t Mesh::GetLOD() const
    {
		return this->currentLOD;
    }

	const std::pair<Vector3, Vector3> Mesh::GetBoundingBox()
	{
		return this->boundingBox;
	}

	void RenderObject::GenerateMeshIndicies() const
	{
		std::vector<IndexBuffer::IndexType> indicies;
		indicies.reserve(this->vertexBufferSize * 3);
		for (int i = 0; i < this->vertexBufferSize; i += 3)
		{
			indicies.push_back(i + 0);
			indicies.push_back(i + 1);
			indicies.push_back(i + 1);
			indicies.push_back(i + 2);
			indicies.push_back(i + 2);
			indicies.push_back(i + 0);
		}
		this->meshIBO = Graphics::Instance()->CreateIndexBuffer(indicies.data(), indicies.size());
		this->meshGenerated = true;
	}

    RenderObject::RenderObject(std::string name, UniqueRef<VertexBuffer> VBO, UniqueRef<VertexArray> VAO, UniqueRef<IndexBuffer> IBO, Ref<Material> material, bool useTexture, bool useNormal, size_t sizeInFloats)
    {
		this->name = std::move(name);
		this->VBO = std::move(VBO);
		this->VAO = std::move(VAO);
		this->IBO = std::move(IBO);
		this->material = std::move(material);
		this->useTexture = useTexture;
		this->useNormal = useTexture;
		this->vertexBufferSize = sizeInFloats;
    }

	const VertexArray& RenderObject::GetVAO() const
	{
		MX_ASSERT(this->VBO != nullptr);
		return *this->VAO;
	}

	IndexBuffer& RenderObject::GetIBO() const
	{
		MX_ASSERT(this->IBO != nullptr);
		return *this->IBO;
	}

	const IndexBuffer& RenderObject::GetMeshIBO() const
	{
		if (!this->meshGenerated) GenerateMeshIndicies();
		return *this->meshIBO;
	}

	const Material& RenderObject::GetMaterial() const
	{
		return *this->material;
	}

	Material& RenderObject::GetMaterial()
	{
		return *this->material;
	}

	const std::string& RenderObject::GetName() const
	{
		return name;
	}

    bool RenderObject::UsesTexture() const
    {
		return this->useTexture;
    }

	bool RenderObject::UsesNormals() const
	{
		return this->useNormal;
	}

	size_t RenderObject::GetVertexBufferSize() const
	{
		return this->vertexBufferSize;
	}

	bool RenderObject::HasMaterial() const
	{
		return this->material != nullptr;
	}

	void Mesh::AddInstancedBuffer(UniqueRef<VertexBuffer> vbo, UniqueRef<VertexBufferLayout> vbl)
	{
		this->VBOs.push_back(std::move(vbo));
		this->VBLs.push_back(std::move(vbl));
		for (const auto& meshes : this->LODs)
		{
			for (const auto& mesh : meshes)
			{
				mesh.VAO->AddInstancedBuffer(*this->VBOs.back(), *this->VBLs.back());
			}
		}
	}

	VertexBuffer& Mesh::GetBufferByIndex(size_t index)
	{
		assert(index < this->VBOs.size());
		return *this->VBOs[index];
	}

	VertexBufferLayout& Mesh::GetBufferLayoutByIndex(size_t index)
	{
		assert(index < this->VBLs.size());
		return *this->VBLs[index];
	}

    size_t Mesh::GetBufferCount() const
    {
		return this->VBOs.size();
    }
}