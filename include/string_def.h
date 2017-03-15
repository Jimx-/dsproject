//
// Created by jimx on 16-12-16.
//

#ifndef WEEABOO_STRINGS_H
#define WEEABOO_STRINGS_H

const InternString Renderer::LIGHTING_SHADER = "lighting";
const InternString Renderer::BONE_ANIM_SHADER = "bone_animation";
const InternString Renderer::GEOMETRY_PASS_SHADER = "geometry_pass";
const InternString Renderer::LIGHTING_PASS_SHADER = "lighting_pass";
const InternString Renderer::SSAO_SHADER = "SSAO_shader";
const InternString Renderer::SSAO_BLUR_SHADER = "SSAO_blur_shader";
const InternString Renderer::DEPTH_MAP_SHADER = "depth_map_shader";
const InternString Renderer::HDR_BLEND_SHADER = "HDR_blend_shader";
const InternString Renderer::GAUSSIAN_BLUR_SHADER = "gaussian_blur_shader";
const InternString Renderer::BILLBOARD_SHADER = "billboard_shader";
const InternString Renderer::TEXT_OVERLAY_SHADER = "text_overlay_shader";
const InternString Renderer::MINIMAP_SHADER = "minimap_shader";
const InternString Renderer::GUI_SHADER = "gui_shader";

const InternString ShaderProgram::MVP = "uMVP";
const InternString ShaderProgram::VP = "uVP";
const InternString ShaderProgram::MODEL = "uModel";
const InternString ShaderProgram::VIEW = "uView";
const InternString ShaderProgram::VIEW_POS = "uViewPos";
const InternString ShaderProgram::PROJECTION = "uProjection";
const InternString ShaderProgram::DIFFUSE_TEXTURE = "uDiffuse";
const InternString ShaderProgram::NORMAL_MAP = "uNormalMap";
const InternString ShaderProgram::BONE_TRANSFORMS = "uBoneTransforms[0]";

const InternString ShaderProgram::MAT_ROUGHNESS = "uRoughness";
const InternString ShaderProgram::MAT_METALLIC = "uMetallic";

const InternString ShaderProgram::GBUFFER_POSITION = "gPosition";
const InternString ShaderProgram::GBUFFER_NORMAL = "gNormal";
const InternString ShaderProgram::GBUFFER_ALBEDO_SPEC = "gAlbedoSpec";

#endif //WEEABOO_STRINGS_H

