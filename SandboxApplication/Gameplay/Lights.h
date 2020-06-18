#pragma once

void InitDirLight(MxObject& object)
{
    auto light = object.AddComponent<DirectionalLight>();
    light->AmbientColor  = { 0.3f, 0.3f, 0.3f };
    light->DiffuseColor  = { 0.3f, 0.3f, 0.3f };
    light->SpecularColor = { 1.0f, 1.0f, 1.0f };
    light->FollowViewport();
}

void InitPointLight(MxObject& object)
{
    auto light = object.AddComponent<PointLight>();
    light->AmbientColor  = {  1.0f, 0.3f,  0.0f };
    light->DiffuseColor  = {  1.0f, 0.3f,  0.0f };
    light->SpecularColor = {  1.0f, 0.3f,  0.0f };

    object.AddComponent<MeshSource>(Primitives::CreateCube());
    object.GetComponent<MeshRenderer>()->GetMaterial()->EmmisiveColor = light->AmbientColor + light->DiffuseColor;
    object.GetComponent<Transform>()
        ->SetPosition(MakeVector3(-3.0f, 2.0f, -3.0f))
            .SetScale(MakeVector3(1.0f / 3.0f));
}

void InitSpotLight(MxObject& object)
{
    auto light = object.AddComponent<SpotLight>();
    light->AmbientColor  = {   1.0f, 1.0f,  1.0f };
    light->DiffuseColor  = {   1.0f, 1.0f,  1.0f };
    light->SpecularColor = {   1.0f, 1.0f,  1.0f };
    light->Direction     = {  -1.0f, 1.3f, -1.0f };
    light->UseOuterAngle(35.0f);
    light->UseInnerAngle(15.0f);

    object.AddComponent<MeshSource>(Primitives::CreateCube());
    object.GetComponent<MeshRenderer>()->GetMaterial()->EmmisiveColor = light->AmbientColor + light->DiffuseColor;
    object.GetComponent<Transform>()
        ->SetPosition(MakeVector3(-15.0f, 3.0f,  0.0f))
            .SetScale(MakeVector3(1.0f / 3.0f));
}