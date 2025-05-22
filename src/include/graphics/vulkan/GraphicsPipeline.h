#pragma once

#include "IPipeline.h"
#include "vk_types.h"
#include "vk_initializers.h"
// #include "vk_pipelines.h" // PipelineBuilder is used, so this might be needed or PipelineBuilder moved/forward declared
#include <functional>
#include <string>
#include <vector>
#include <memory> // For std::unique_ptr
#include "graphics/vulkan/vulkan_wrappers.h" // For Vulkan wrappers

class PipelineBuilder; // Forward declaration if vk_pipelines.h is too heavy or causes circular dependency

class GraphicsPipeline : public IPipeline {
public:
    struct GraphicsPipelineConfig {
        std::string vertexShaderPath;
        std::string fragmentShaderPath;
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
        VkFormat colorFormat;
        VkFormat depthFormat;
        VkCullModeFlags cullMode = VK_CULL_MODE_NONE;
        VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        bool depthTest = true;
        bool blending = false;
        VkCompareOp depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL;
        std::function<void(PipelineBuilder&)> customPipelineSetup = nullptr;
        std::vector<VkPushConstantRange> pushConstants;
    };

    GraphicsPipeline() = default;
    explicit GraphicsPipeline(const GraphicsPipelineConfig& config);
    
    void init(VkDevice device) override;
    void bind(VkCommandBuffer cmd) override;
    // void destroy() override; // Removed
    ~GraphicsPipeline() override; // Added destructor
    
    VkPipeline getPipeline() const override { return _pipeline ? _pipeline->get() : VK_NULL_HANDLE; } // Use getter
    VkPipelineLayout getLayout() const override { return _pipelineLayout ? _pipelineLayout->get() : VK_NULL_HANDLE; } // Use getter

    // Graphics pipeline specific methods
    void bindDescriptorSets(VkCommandBuffer cmd,
                            const VkDescriptorSet* descriptorSets,
                            uint32_t setCount, uint32_t firstSet = 0);
    void pushConstants(VkCommandBuffer cmd, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* data);

private:
    VkDevice _device = VK_NULL_HANDLE; // Keep for now, might be useful for init
    std::unique_ptr<VulkanPipeline> _pipeline;
    std::unique_ptr<VulkanPipelineLayout> _pipelineLayout;
    GraphicsPipelineConfig _config;
};