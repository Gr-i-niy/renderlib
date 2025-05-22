#include "graphics/vulkan/ComputePipeline.h"
#include "graphics/vulkan/vk_initializers.h"
#include "graphics/vulkan/vk_pipelines.h" // For vkutil::load_shader_module
#include "graphics/vulkan/vulkan_wrappers.h" // For Vulkan wrappers
#include <fmt/core.h> // For fmt::println

ComputePipeline::ComputePipeline(const ComputePipelineConfig& config) 
    : _config(config) {
}

ComputePipeline::~ComputePipeline() {} // Empty destructor, unique_ptrs handle cleanup

// Updated signature from ComputePipeline.h
bool ComputePipeline::init(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, const VulkanAllocatedImage& drawImageTarget) {
    _device = device;
    
    // This was the change from Turn 51, which is correct for this signature.
    // The _config.descriptorSetLayout is set by ComputePipelineConfig.
    // The descriptorSetLayout parameter passed here IS for the pipeline layout.
    VkPipelineLayoutCreateInfo computeLayoutInfo{};
    computeLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    computeLayoutInfo.pNext = nullptr;
    computeLayoutInfo.pSetLayouts = &descriptorSetLayout; 
    computeLayoutInfo.setLayoutCount = 1;

    // VK_CHECK(vkCreatePipelineLayout(_device, &computeLayoutInfo, nullptr, &_pipelineLayout));
    _pipelineLayout = std::make_unique<VulkanPipelineLayout>(_device, computeLayoutInfo);
    
    VkShaderModule computeShader;
    if (!vkutil::load_shader_module(_config.shaderPath.c_str(), _device, &computeShader)) {
        fmt::println("Error when building the compute shader \n");
        return;
    }
    
    VkPipelineShaderStageCreateInfo stageInfo{};
    stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageInfo.pNext = nullptr;
    stageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    stageInfo.module = computeShader;
    stageInfo.pName = "main";
    
    VkComputePipelineCreateInfo computePipelineCreateInfo{};
    computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    computePipelineCreateInfo.pNext = nullptr;
    computePipelineCreateInfo.layout = _pipelineLayout->get(); // Use getter for raw handle
    computePipelineCreateInfo.stage = stageInfo;
    
    // VK_CHECK(vkCreateComputePipelines(_device, VK_NULL_HANDLE, 1,
    //                                 &computePipelineCreateInfo, nullptr,
    //                                 &_pipeline));
    _pipeline = std::make_unique<VulkanPipeline>(_device, computePipelineCreateInfo); // Use constructor taking createInfo
    
    if (_config.customSetupCallback) {
        // Ensure callback uses getters if it needs raw handles
        _config.customSetupCallback(_device, (_pipeline ? _pipeline->get() : VK_NULL_HANDLE), (_pipelineLayout ? _pipelineLayout->get() : VK_NULL_HANDLE));
    }
    
    // Descriptor writing for drawImageTarget as per prompt (simplified: assumes ComputePipeline doesn't own the set)
    // This part is problematic as ComputePipeline doesn't have a DescriptorWriter or a VkDescriptorSet member.
    // This should ideally be handled where the descriptor set is managed (e.g., VulkanEngine::init_descriptors).
    // For now, I will comment out the descriptor writing part as it requires further class changes.
    /*
    if (_pipelineLayout && _pipeline && drawImageTarget.getImageView() != VK_NULL_HANDLE) {
        // This assumes a descriptor set needs to be updated.
        // However, ComputePipeline doesn't have its own descriptor set for this target.
        // The gradient pipeline's target descriptor (_drawImageDescriptors) is managed by VulkanEngine.
        // DescriptorWriter writer;
        // writer.write_image(0, drawImageTarget.getImageView(), VK_NULL_HANDLE, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
        // How to get the VkDescriptorSet to update? This needs more context or class changes.
        // For example, if _config held a VkDescriptorSet* or if a global/engine allocator was used here.
        // fmt::println("ComputePipeline::init would update descriptor for drawImageTarget here.");
    }
    */
    
    vkDestroyShaderModule(_device, computeShader, nullptr);
    return true; // Return true on success
}

void ComputePipeline::bind(VkCommandBuffer cmd) {
    if (_pipeline) {
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, _pipeline->get()); // Use getter
    }
}

void ComputePipeline::bindDescriptorSets(VkCommandBuffer cmd, const VkDescriptorSet* descriptorSets, uint32_t setCount) {
    if (_pipelineLayout) {
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, _pipelineLayout->get(), // Use getter
                               0, setCount, descriptorSets, 0, nullptr);
    }
}

void ComputePipeline::dispatch(VkCommandBuffer cmd, uint32_t x, uint32_t y, uint32_t z) {
    vkCmdDispatch(cmd, x, y, z);
}

// void ComputePipeline::destroy() { // Removed
//     if (_device != VK_NULL_HANDLE) {
//         vkDestroyPipelineLayout(_device, _pipelineLayout, nullptr);
//         vkDestroyPipeline(_device, _pipeline, nullptr);
//         _pipeline = VK_NULL_HANDLE;
//         _pipelineLayout = VK_NULL_HANDLE;
//     }
// }
}