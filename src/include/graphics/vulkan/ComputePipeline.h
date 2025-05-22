#pragma once

#include "IPipeline.h"
#include "vk_types.h"
// #include "vk_initializers.h" // Likely included via vk_types or vk_engine
// #include "vk_pipelines.h" // For vkutil::load_shader_module, ensure it's available or move the function
#include <functional>
#include <string> // For std::string in ComputePipelineConfig
#include <memory> // For std::unique_ptr
#include "graphics/vulkan/vulkan_wrappers.h" // For Vulkan wrappers

class ComputePipeline : public IPipeline {
public:
    struct ComputePipelineConfig {
        VkDescriptorSetLayout descriptorSetLayout;
        std::string shaderPath;
        std::function<void(VkDevice, VkPipeline, VkPipelineLayout)> customSetupCallback = nullptr;
    };

    ComputePipeline() = default;
    explicit ComputePipeline(const ComputePipelineConfig& config);
    
    // Changed signature for Part 3 of current task, removed override
    bool init(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, const VulkanAllocatedImage& drawImage); 
    void bind(VkCommandBuffer cmd) override;
    // void destroy() override; // Removed
    ~ComputePipeline() override; // Added destructor
    
    VkPipeline getPipeline() const override { return _pipeline ? _pipeline->get() : VK_NULL_HANDLE; } // Use getter
    VkPipelineLayout getLayout() const override { return _pipelineLayout ? _pipelineLayout->get() : VK_NULL_HANDLE; } // Use getter

    // Specific to compute pipelines
    void dispatch(VkCommandBuffer cmd, uint32_t x, uint32_t y, uint32_t z = 1);
    void bindDescriptorSets(VkCommandBuffer cmd, const VkDescriptorSet* descriptorSets, uint32_t setCount);

private:
    VkDevice _device = VK_NULL_HANDLE; // Keep for now, might be useful for init
    std::unique_ptr<VulkanPipeline> _pipeline;
    std::unique_ptr<VulkanPipelineLayout> _pipelineLayout;
    ComputePipelineConfig _config;
};