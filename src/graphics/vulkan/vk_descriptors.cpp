#include "graphics/vulkan/vk_descriptors.h"

#include "graphics/vulkan/vk_types.h"

void DescriptorLayoutBuilder::add_binding(uint32_t binding,
                                          VkDescriptorType type) {
    VkDescriptorSetLayoutBinding newbind{};
    newbind.binding = binding;
    newbind.descriptorCount = 1;
    newbind.descriptorType = type;

    bindings.push_back(newbind);
}

void DescriptorLayoutBuilder::clear() {
    bindings.clear();
}

VkDescriptorSetLayout DescriptorLayoutBuilder::build(
        VkDevice device, VkShaderStageFlags shaderStages, const void* pNext,
        VkDescriptorSetLayoutCreateFlags flags) {
    for (auto& b : bindings) {
        b.stageFlags |= shaderStages;
    }

    VkDescriptorSetLayoutCreateInfo info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    info.pNext = pNext;

    info.pBindings = bindings.data();
    info.bindingCount = (uint32_t)bindings.size();
    info.flags = flags;

    VkDescriptorSetLayout set;
    VK_CHECK(vkCreateDescriptorSetLayout(device, &info, nullptr, &set));

    return set;
}

void DescriptorAllocator::init_pool(VkDevice device, uint32_t maxSets,
                                    std::span<PoolSizeRatio> poolRatios) {
    std::vector<VkDescriptorPoolSize> poolSizes;
    for (auto [type, ratio] : poolRatios) {
        poolSizes.emplace_back(VkDescriptorPoolSize{
                .type = type,
                .descriptorCount = static_cast<uint32_t>(ratio * maxSets)});
    }

    VkDescriptorPoolCreateInfo pool_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    pool_info.flags = 0;
    pool_info.maxSets = maxSets;
    pool_info.poolSizeCount = (uint32_t)poolSizes.size();
    pool_info.pPoolSizes = poolSizes.data();

    vkCreateDescriptorPool(device, &pool_info, nullptr, &pool);
}

void DescriptorAllocator::clear_descriptors(VkDevice device) const {
    vkResetDescriptorPool(device, pool, 0);
}

void DescriptorAllocator::destroy_pool(VkDevice device) const {
    vkDestroyDescriptorPool(device, pool, nullptr);
}

VkDescriptorSet DescriptorAllocator::allocate(
        VkDevice device, VkDescriptorSetLayout layout) const {
    VkDescriptorSetAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocInfo.pNext = nullptr;
    allocInfo.descriptorPool = pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;

    VkDescriptorSet ds;
    VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, &ds));

    return ds;
}

VkDescriptorPool DescriptorAllocatorGrowable::get_pool(VkDevice device) {
    VkDescriptorPool newPool;
    if (!readyPools.empty()) {
        newPool = readyPools.back();
        readyPools.pop_back();
    } else {
        // need to create a new pool
        newPool = create_pool(device, setsPerPool, ratios);

        setsPerPool = static_cast<uint32_t>(setsPerPool * 1.5);
        if (setsPerPool > 4092) {
            setsPerPool = 4092;
        }
    }

    return newPool;
}

VkDescriptorPool DescriptorAllocatorGrowable::create_pool(
        VkDevice device, uint32_t setCount,
        std::span<PoolSizeRatio> poolRatios) {
    std::vector<VkDescriptorPoolSize> poolSizes;
    for (PoolSizeRatio ratio : poolRatios) {
        poolSizes.emplace_back(VkDescriptorPoolSize{
                .type = ratio.type,
                .descriptorCount =
                        static_cast<uint32_t>(ratio.ratio * setCount)});
    }

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = 0;
    pool_info.maxSets = setCount;
    pool_info.poolSizeCount = (uint32_t)poolSizes.size();
    pool_info.pPoolSizes = poolSizes.data();

    VkDescriptorPool newPool;
    vkCreateDescriptorPool(device, &pool_info, nullptr, &newPool);
    return newPool;
}

// Constructor - replaces init()
DescriptorAllocatorGrowable::DescriptorAllocatorGrowable(VkDevice device, uint32_t initialSets,
                                                       std::span<PoolSizeRatio> poolRatios)
    : _device(device) { // Initialize _device
    // ratios.clear(); // Not needed as it's a new object or assigned from
    ratios.assign(poolRatios.begin(), poolRatios.end());

    uint32_t firstPoolSetCount = (initialSets == 0) ? 10u : initialSets; // Ensure first pool is not 0 sets
    
    const VkDescriptorPool newPool = create_pool(_device, firstPoolSetCount, ratios); // Use member _device and ratios
    readyPools.push_back(newPool);

    // Set setsPerPool for the *next* pool's dynamic growth potential
    setsPerPool = static_cast<uint32_t>(static_cast<float>(firstPoolSetCount) * 1.5f);
     if (firstPoolSetCount == 0) { // if initialSets was 0, firstPoolSetCount is 10.
        setsPerPool = static_cast<uint32_t>(10.0f * 1.5f); 
    } else if (setsPerPool == firstPoolSetCount && firstPoolSetCount > 0) { // Ensure growth if 1.5 factor results in same integer
        setsPerPool = firstPoolSetCount + 1;
    }
    if (setsPerPool == 0) { // Absolute fallback if initialSets was extremely small (e.g., 1 and 1*1.5 truncates to 1)
        setsPerPool = (firstPoolSetCount > 0) ? firstPoolSetCount + 1 : 10u;
    }


    if (setsPerPool > 4092) { // Cap growth
        setsPerPool = 4092;
    }
}

DescriptorAllocatorGrowable::~DescriptorAllocatorGrowable() {
    if (_device != VK_NULL_HANDLE) { // Only destroy if initialized (device is set)
        destroy_pools(_device); // Pass the stored device
    }
}

DescriptorAllocatorGrowable::DescriptorAllocatorGrowable(DescriptorAllocatorGrowable&& other) noexcept
    : _device(other._device),
      ratios(std::move(other.ratios)),
      fullPools(std::move(other.fullPools)),
      readyPools(std::move(other.readyPools)),
      setsPerPool(other.setsPerPool) {
    // Invalidate other to prevent double deletion by its destructor
    other._device = VK_NULL_HANDLE;
    other.setsPerPool = 0;
    // other.ratios, other.fullPools, other.readyPools are already in a valid empty state after std::move
}

DescriptorAllocatorGrowable& DescriptorAllocatorGrowable::operator=(DescriptorAllocatorGrowable&& other) noexcept {
    if (this != &other) {
        // Destroy existing resources in *this
        if (_device != VK_NULL_HANDLE) {
            destroy_pools(_device);
        }

        // Move resources from other to *this
        _device = other._device;
        ratios = std::move(other.ratios);
        fullPools = std::move(other.fullPools);
        readyPools = std::move(other.readyPools);
        setsPerPool = other.setsPerPool;

        // Invalidate other
        other._device = VK_NULL_HANDLE;
        other.setsPerPool = 0;
        // other.ratios, other.fullPools, other.readyPools are already in a valid empty state after std::move
    }
    return *this;
}

void DescriptorAllocatorGrowable::clear_pools(VkDevice device_param) { // Renamed param to avoid conflict
    // Use internal _device if valid, otherwise use parameter.
    // This allows calling clear_pools on a default-constructed object if a device is passed.
    VkDevice targetDevice = (_device != VK_NULL_HANDLE) ? _device : device_param;
    if (targetDevice == VK_NULL_HANDLE) return; // Cannot operate without a device

    for (const auto p : readyPools) {
        vkResetDescriptorPool(targetDevice, p, 0);
    }
    for (auto p : fullPools) {
        vkResetDescriptorPool(targetDevice, p, 0);
        readyPools.push_back(p);
    }
    fullPools.clear();
}

void DescriptorAllocatorGrowable::destroy_pools(VkDevice device_param) { // Renamed param
    VkDevice targetDevice = (_device != VK_NULL_HANDLE) ? _device : device_param;
     if (targetDevice == VK_NULL_HANDLE) return;

    for (const auto p : readyPools) {
        vkDestroyDescriptorPool(targetDevice, p, nullptr);
    }
    readyPools.clear();
    for (const auto p : fullPools) {
        vkDestroyDescriptorPool(targetDevice, p, nullptr);
    }
    fullPools.clear();
    ratios.clear(); 
    // setsPerPool = 0; // Not strictly necessary to reset here, as it's reset in move ops or on new construction.
    // _device is not nulled out here; The owning object's state (destructor or move op) handles that.
}

VkDescriptorSet DescriptorAllocatorGrowable::allocate(
        VkDevice device_param, VkDescriptorSetLayout layout, const void* pNext) { // Renamed param
    VkDevice targetDevice = (_device != VK_NULL_HANDLE) ? _device : device_param;
    if (targetDevice == VK_NULL_HANDLE) {
       // Optionally: Log an error or throw an exception if no valid device.
       return VK_NULL_HANDLE;
    }
    
    // get or create a pool to allocate from
    VkDescriptorPool poolToUse = get_pool(targetDevice); // Use targetDevice

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.pNext = pNext;
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = poolToUse;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;

    VkDescriptorSet ds;
    VkResult result = vkAllocateDescriptorSets(targetDevice, &allocInfo, &ds); // Use targetDevice

    // allocation failed. Try again
    if (result == VK_ERROR_OUT_OF_POOL_MEMORY ||
        result == VK_ERROR_FRAGMENTED_POOL) {
        fullPools.push_back(poolToUse);

        poolToUse = get_pool(targetDevice); // Use targetDevice
        allocInfo.descriptorPool = poolToUse;

        VK_CHECK(vkAllocateDescriptorSets(targetDevice, &allocInfo, &ds)); // Use targetDevice
    }

    readyPools.push_back(poolToUse);
    return ds;
}

void DescriptorWriter::write_buffer(int binding, VkBuffer buffer, size_t size,
                                    size_t offset, VkDescriptorType type) {
    const VkDescriptorBufferInfo& info =
            bufferInfos.emplace_back(VkDescriptorBufferInfo{
                    .buffer = buffer, .offset = offset, .range = size});

    VkWriteDescriptorSet write = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};

    write.dstBinding = static_cast<uint32_t>(binding);
    write.dstSet =
            VK_NULL_HANDLE;  // left empty for now until we need to write it
    write.descriptorCount = 1;
    write.descriptorType = type;
    write.pBufferInfo = &info;

    writes.push_back(write);
}

void DescriptorWriter::write_image(int binding, VkImageView image,
                                   VkSampler sampler, VkImageLayout layout,
                                   VkDescriptorType type) {
    const VkDescriptorImageInfo& info = imageInfos.emplace_back(
            VkDescriptorImageInfo{.sampler = sampler,
                                  .imageView = image,
                                  .imageLayout = layout});

    VkWriteDescriptorSet write = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};

    write.dstBinding = static_cast<uint32_t>(binding);
    write.dstSet =
            VK_NULL_HANDLE;  // left empty for now until we need to write it
    write.descriptorCount = 1;
    write.descriptorType = type;
    write.pImageInfo = &info;

    writes.push_back(write);
}

void DescriptorWriter::clear() {
    imageInfos.clear();
    writes.clear();
    bufferInfos.clear();
}

void DescriptorWriter::update_set(VkDevice device, VkDescriptorSet set) {
    for (VkWriteDescriptorSet& write : writes) {
        write.dstSet = set;
    }

    vkUpdateDescriptorSets(device, (uint32_t)writes.size(), writes.data(), 0,
                           nullptr);
}
