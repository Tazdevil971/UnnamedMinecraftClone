#include "GeometryRenderer.hpp"

#include <vulkan/vulkan_core.h>

#include "BufferManager.hpp"
#include "Context.hpp"
#include "Managed.hpp"
#include "ShadowPass.hpp"

using namespace render;

GeometryRenderer::GeometryRenderer(VkRenderPass renderPass) {
    lightInfoUbo = BufferManager::get().allocateUbo(sizeof(LightInfoUbo));

    createPipeline(renderPass);
}

void GeometryRenderer::record(VkCommandBuffer commandBuffer,
                              const Camera& camera, float ratio,
                              const LightInfo& lights,
                              const Texture& depthTexture,
                              std::list<GeometryModel> models) {
    // Update UBO
    lightInfoUbo.write(
        LightInfoUbo{ShadowPass::computeShadowVP(camera.pos, lights.sunDir),
                     {lights.ambientColor, 1.0f},
                     {lights.sunDir, 1.0f},
                     {lights.sunColor, 1.0f},
                     {camera.pos, 1.0f}});

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      *pipeline);

    glm::mat4 vp = camera.computeVPMat(ratio);
    // glm::mat4 vp = ShadowPass::computeShadowVP(lights.sunDir);

    // Record models
    for (const auto& model : models)
        recordSingle(commandBuffer, vp, depthTexture, model);
}

void GeometryRenderer::recordSingle(VkCommandBuffer commandBuffer, glm::mat4 vp,
                                    const Texture& depthTexture,
                                    const GeometryModel& model) {
    if (model.mesh.isNull()) return;

    VkDescriptorSet descriptorSets[3] = {model.texture.descriptor,
                                         depthTexture.descriptor,
                                         lightInfoUbo.descriptor};

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            *pipelineLayout, 0, 3, descriptorSets, 0, nullptr);

    model.mesh.bind(commandBuffer);

    glm::mat4 m = model.computeModelMat();
    PushBuffer pushBuffer = {m, vp};

    vkCmdPushConstants(commandBuffer, *pipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushBuffer),
                       &pushBuffer);

    vkCmdDrawIndexed(commandBuffer, model.mesh.indexCount, 1, 0, 0, 0);
}

void GeometryRenderer::createPipeline(VkRenderPass renderPass) {
    VkDescriptorSetLayout descriptorSetLayouts[3] = {
        BufferManager::get().getTextureLayout(),
        BufferManager::get().getTextureLayout(),
        BufferManager::get().getUboLayout()};
    VkPushConstantRange pushConstantRange = {};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(PushBuffer);
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 3;
    pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
    pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(Context::get().getDevice(),
                               &pipelineLayoutCreateInfo, nullptr,
                               &*pipelineLayout) != VK_SUCCESS)
        throw std::runtime_error{"failed to create pipeline layout!"};

    ManagedShaderModule vertShaderModule{
        Context::get().loadShaderModule("GeometryVert.vert.spv")};
    ManagedShaderModule fragShaderModule{
        Context::get().loadShaderModule("GeometryFrag.frag.spv")};

    VkPipelineShaderStageCreateInfo vertStageInfo{};
    vertStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertStageInfo.module = *vertShaderModule;
    vertStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragStageInfo{};
    fragStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragStageInfo.module = *fragShaderModule;
    fragStageInfo.pName = "main";

    VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
    dynamicStateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

    static VkDynamicState DYNAMIC_STATES[] = {VK_DYNAMIC_STATE_VIEWPORT,
                                              VK_DYNAMIC_STATE_SCISSOR};
    dynamicStateInfo.dynamicStateCount = 2;
    dynamicStateInfo.pDynamicStates = DYNAMIC_STATES;

    auto bindingDescription = GeometryMesh::getBindingDescription();
    auto attributeDescriptions = GeometryMesh::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputStageInfo{};
    vertexInputStageInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputStageInfo.vertexBindingDescriptionCount = 1;
    vertexInputStageInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputStageInfo.vertexAttributeDescriptionCount =
        attributeDescriptions.size();
    vertexInputStageInfo.pVertexAttributeDescriptions =
        attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStageInfo{};
    inputAssemblyStageInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyStageInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyStageInfo.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportStateInfo{};
    viewportStateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateInfo.viewportCount = 1;
    viewportStateInfo.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizerStateInfo{};
    rasterizerStateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizerStateInfo.depthClampEnable = VK_FALSE;
    rasterizerStateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizerStateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizerStateInfo.lineWidth = 1.0f;
    rasterizerStateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizerStateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizerStateInfo.depthBiasEnable = VK_FALSE;
    rasterizerStateInfo.depthBiasConstantFactor = 0.0f;
    rasterizerStateInfo.depthBiasClamp = 0.0f;
    rasterizerStateInfo.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisamplingStateInfo{};
    multisamplingStateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisamplingStateInfo.sampleShadingEnable = VK_FALSE;
    multisamplingStateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisamplingStateInfo.minSampleShading = 1.0f;
    multisamplingStateInfo.pSampleMask = nullptr;
    multisamplingStateInfo.alphaToCoverageEnable = VK_FALSE;
    multisamplingStateInfo.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlendStateInfo{};
    colorBlendStateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendStateInfo.logicOpEnable = VK_FALSE;
    colorBlendStateInfo.logicOp = VK_LOGIC_OP_COPY;
    colorBlendStateInfo.attachmentCount = 1;
    colorBlendStateInfo.pAttachments = &colorBlendAttachment;
    colorBlendStateInfo.blendConstants[0] = 0.0f;
    colorBlendStateInfo.blendConstants[1] = 0.0f;
    colorBlendStateInfo.blendConstants[2] = 0.0f;
    colorBlendStateInfo.blendConstants[3] = 0.0f;

    VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo{};
    depthStencilStateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilStateInfo.depthTestEnable = VK_TRUE;
    depthStencilStateInfo.depthWriteEnable = VK_TRUE;
    depthStencilStateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencilStateInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilStateInfo.minDepthBounds = 0.0f;
    depthStencilStateInfo.maxDepthBounds = 1.0f;
    depthStencilStateInfo.stencilTestEnable = VK_FALSE;
    depthStencilStateInfo.front = {};
    depthStencilStateInfo.back = {};

    VkPipelineShaderStageCreateInfo stageInfos[] = {vertStageInfo,
                                                    fragStageInfo};

    VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stageCount = 2;
    pipelineCreateInfo.pStages = stageInfos;
    pipelineCreateInfo.pVertexInputState = &vertexInputStageInfo;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyStageInfo;
    pipelineCreateInfo.pViewportState = &viewportStateInfo;
    pipelineCreateInfo.pRasterizationState = &rasterizerStateInfo;
    pipelineCreateInfo.pMultisampleState = &multisamplingStateInfo;
    pipelineCreateInfo.pDepthStencilState = &depthStencilStateInfo;
    pipelineCreateInfo.pColorBlendState = &colorBlendStateInfo;
    pipelineCreateInfo.pDynamicState = &dynamicStateInfo;
    pipelineCreateInfo.layout = *pipelineLayout;
    pipelineCreateInfo.renderPass = renderPass;
    pipelineCreateInfo.subpass = 0;
    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineCreateInfo.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(Context::get().getDevice(), VK_NULL_HANDLE, 1,
                                  &pipelineCreateInfo, nullptr,
                                  &*pipeline) != VK_SUCCESS)
        throw std::runtime_error{"failed to create graphics pipeline"};
}