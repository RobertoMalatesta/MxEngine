// Copyright(c) 2019 - 2020, #Momo
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met :
// 
// 1. Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
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

#include "RenderEditor.h"
#include "ComponentEditor.h"
#include "Utilities/ImGui/ImGuiUtils.h"
#include "Core/Application/Rendering.h"
#include "Platform/Window/WindowManager.h"

namespace MxEngine
{
    void GUI::DrawRenderEditor(const char* name, bool* isOpen)
    {
        ImGui::Begin(name, isOpen);

        if (ImGui::TreeNode("fog settings"))
        {
            auto fogColor = Rendering::GetFogColor();
            auto fogDensity = Rendering::GetFogDensity();
            auto fogDistance = Rendering::GetFogDistance();

            if (ImGui::ColorEdit3("fog color", &fogColor[0]))
                Rendering::SetFogColor(fogColor);

            if (ImGui::DragFloat("fog density", &fogDensity, 0.001f, 0.0f, 10000.0f))
                Rendering::SetFogDensity(fogDensity);

            if (ImGui::DragFloat("fog distance", &fogDistance, 0.01f, 0.0f, 1.0f))
                Rendering::SetFogDistance(fogDistance);

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("shadows settings"))
        {
            int blurIterations = (int)Rendering::GetShadowBlurIterations();
            if(ImGui::DragInt("shadow blur iterations", &blurIterations, 0.01f))
                Rendering::SetShadowBlurIterations(Max(0, blurIterations));

            static bool lightingEnabled = true;
            if (ImGui::Checkbox("toggle global lighting", &lightingEnabled))
                Rendering::LoadMainShader(lightingEnabled);

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("window settings"))
        {
            static MxString title = WindowManager::GetTitle();;
            static auto size = WindowManager::GetSize();
            auto position = WindowManager::GetPosition();

            if (GUI::InputFloatOnClick("window width ", &size.x))
            {
                WindowManager::SetWidth(size.x);
                size = WindowManager::GetSize();
            }

            if (GUI::InputFloatOnClick("window height", &size.y))
            {
                WindowManager::SetHeight(size.y);
                size = WindowManager::GetSize();
            }

            if (GUI::InputTextOnClick("window title ", title, 48))
            {
                WindowManager::SetTitle(title);
                title = WindowManager::GetTitle();
            }

            ImGui::AlignTextToFramePadding();
            ImGui::Text("window position");
            ImGui::SameLine();
            if (ImGui::InputFloat2("", &position[0]))
                WindowManager::SetPosition(position);

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("renderer viewport"))
        {
            auto viewport = Rendering::GetViewport();
            if (viewport.IsValid())
            {
                GUI::DrawImageSaver(viewport->GetRenderTexture(), "take screenshot");
                GUI::CameraControllerEditor(*viewport);
            }
            else
            {
                ImGui::Text("- no viewport provided");
            }
            ImGui::TreePop();
        }
        // TODO: let user change viewport to other camera controller

        if (ImGui::TreeNode("debug settings"))
        {
            static int lineWidth = 3;
            if (GUI::InputIntOnClick("line width", &lineWidth))
                Rendering::GetController().GetRenderEngine().UseLineWidth(Max(1, lineWidth));

            auto& drawOverlay = Rendering::GetAdaptor().DebugDrawer.DrawAsScreenOverlay;
            ImGui::Checkbox("overlay debug", &drawOverlay);

            ImGui::TreePop();
        }

        ImGui::End();
    }
}