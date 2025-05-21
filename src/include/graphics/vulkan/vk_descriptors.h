#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <span>
#include <vector>
#include <vulkan/vulkan_core.h>

struct DescriptorLayoutBuilder {
    std::vector<VkDescriptorSetLayoutBinding> bindings;

    void add_binding(uint32_t binding, VkDescriptorType type);
    void clear();
    VkDescriptorSetLayout build(VkDevice device,
                                VkShaderStageFlags shaderStages,
                                const void* pNext = nullptr,
                                VkDescriptorSetLayoutCreateFlags flags = 0);
};

struct DescriptorAllocator {
    struct PoolSizeRatio {
        VkDescriptorType type;
        float ratio;
    };

    VkDescriptorPool pool;

    void init_pool(VkDevice device, uint32_t maxSets,
                   std::span<PoolSizeRatio> poolRatios);
    void clear_descriptors(VkDevice device) const;
    void destroy_pool(VkDevice device) const;

    VkDescriptorSet allocate(VkDevice device,
                             VkDescriptorSetLayout layout) const;
};

struct DescriptorAllocatorGrowable {
public:
    struct PoolSizeRatio {
        VkDescriptorType type;
        float ratio;
    };

    DescriptorAllocatorGrowable() = default; // Added default constructor
    DescriptorAllocatorGrowable(VkDevice device, uint32_t initialSets,
                                std::span<PoolSizeRatio> poolRatios); // Added constructor
    ~DescriptorAllocatorGrowable(); // Added destructor

    DescriptorAllocatorGrowable(DescriptorAllocatorGrowable&& other) noexcept; // Added move constructor
    DescriptorAllocatorGrowable& operator=(DescriptorAllocatorGrowable&& other) noexcept; // Added move assignment

    // void init(VkDevice device, uint32_t initialSets, // Removed init
    //           std::span<PoolSizeRatio> poolRatios);
    void clear_pools(VkDevice device); // Kept
    void destroy_pools(VkDevice device); // Kept, will be called by destructor

    VkDescriptorSet allocate(VkDevice device, VkDescriptorSetLayout layout,
                             const void* pNext = nullptr);

private:
    VkDevice _device = VK_NULL_HANDLE; // Added _device member

    VkDescriptorPool get_pool(VkDevice device);
    VkDescriptorPool create_pool(VkDevice device, uint32_t setCount,
                                 std::span<PoolSizeRatio> poolRatios);

    std::vector<PoolSizeRatio> ratios;
    std::vector<VkDescriptorPool> fullPools;
    std::vector<VkDescriptorPool> readyPools;
    uint32_t setsPerPool = 0; // Initialize to ensure it has a defined state
};

struct DescriptorWriter {
    std::deque<VkDescriptorImageInfo> imageInfos;
    std::deque<VkDescriptorBufferInfo> bufferInfos;
    std::vector<VkWriteDescriptorSet> writes;

    void write_image(int binding, VkImageView image, VkSampler sampler,
                     VkImageLayout layout, VkDescriptorType type);
    void write_buffer(int binding, VkBuffer buffer, size_t size, size_t offset,
                      VkDescriptorType type);

    void clear();
    void update_set(VkDevice device, VkDescriptorSet set);
};
