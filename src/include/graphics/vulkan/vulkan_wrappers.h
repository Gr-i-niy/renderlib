#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h> // For VMA types

class VulkanFence {
public:
    VulkanFence(VkDevice device, VkFenceCreateFlags flags = 0) : device_(device), fence_(VK_NULL_HANDLE) {
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = flags;
        vkCreateFence(device_, &fenceInfo, nullptr, &fence_);
    }

    ~VulkanFence() {
        if (fence_ != VK_NULL_HANDLE) {
            vkDestroyFence(device_, fence_, nullptr);
        }
    }

    VulkanFence(const VulkanFence&) = delete;
    VulkanFence& operator=(const VulkanFence&) = delete;

    VulkanFence(VulkanFence&& other) noexcept : device_(other.device_), fence_(other.fence_) {
        other.fence_ = VK_NULL_HANDLE;
    }

    VulkanFence& operator=(VulkanFence&& other) noexcept {
        if (this != &other) {
            if (fence_ != VK_NULL_HANDLE) {
                vkDestroyFence(device_, fence_, nullptr);
            }
            device_ = other.device_;
            fence_ = other.fence_;
            other.fence_ = VK_NULL_HANDLE;
        }
        return *this;
    }

    VkFence get() const { return fence_; }

private:
    VkDevice device_;
    VkFence fence_;
};

class VulkanSemaphore {
public:
    VulkanSemaphore(VkDevice device) : device_(device), semaphore_(VK_NULL_HANDLE) {
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        vkCreateSemaphore(device_, &semaphoreInfo, nullptr, &semaphore_);
    }

    ~VulkanSemaphore() {
        if (semaphore_ != VK_NULL_HANDLE) {
            vkDestroySemaphore(device_, semaphore_, nullptr);
        }
    }

    VulkanSemaphore(const VulkanSemaphore&) = delete;
    VulkanSemaphore& operator=(const VulkanSemaphore&) = delete;

    VulkanSemaphore(VulkanSemaphore&& other) noexcept : device_(other.device_), semaphore_(other.semaphore_) {
        other.semaphore_ = VK_NULL_HANDLE;
    }

    VulkanSemaphore& operator=(VulkanSemaphore&& other) noexcept {
        if (this != &other) {
            if (semaphore_ != VK_NULL_HANDLE) {
                vkDestroySemaphore(device_, semaphore_, nullptr);
            }
            device_ = other.device_;
            semaphore_ = other.semaphore_;
            other.semaphore_ = VK_NULL_HANDLE;
        }
        return *this;
    }

    VkSemaphore get() const { return semaphore_; }

private:
    VkDevice device_;
    VkSemaphore semaphore_;
};

class VulkanCommandPool {
public:
    VulkanCommandPool(VkDevice device, const VkCommandPoolCreateInfo& createInfo) : device_(device), commandPool_(VK_NULL_HANDLE) {
        vkCreateCommandPool(device_, &createInfo, nullptr, &commandPool_);
    }

    ~VulkanCommandPool() {
        if (commandPool_ != VK_NULL_HANDLE) {
            vkDestroyCommandPool(device_, commandPool_, nullptr);
        }
    }

    VulkanCommandPool(const VulkanCommandPool&) = delete;
    VulkanCommandPool& operator=(const VulkanCommandPool&) = delete;

    VulkanCommandPool(VulkanCommandPool&& other) noexcept : device_(other.device_), commandPool_(other.commandPool_) {
        other.commandPool_ = VK_NULL_HANDLE;
    }

    VulkanCommandPool& operator=(VulkanCommandPool&& other) noexcept {
        if (this != &other) {
            if (commandPool_ != VK_NULL_HANDLE) {
                vkDestroyCommandPool(device_, commandPool_, nullptr);
            }
            device_ = other.device_;
            commandPool_ = other.commandPool_;
            other.commandPool_ = VK_NULL_HANDLE;
        }
        return *this;
    }

    VkCommandPool get() const { return commandPool_; }

private:
    VkDevice device_;
    VkCommandPool commandPool_;
};

class VulkanAllocator {
public:
    VulkanAllocator(const VmaAllocatorCreateInfo& createInfo) : allocator_(VK_NULL_HANDLE) {
        vmaCreateAllocator(&createInfo, &allocator_);
    }

    ~VulkanAllocator() {
        if (allocator_ != VK_NULL_HANDLE) {
            vmaDestroyAllocator(allocator_);
        }
    }

    VulkanAllocator(const VulkanAllocator&) = delete;
    VulkanAllocator& operator=(const VulkanAllocator&) = delete;

    VulkanAllocator(VulkanAllocator&& other) noexcept : allocator_(other.allocator_) {
        other.allocator_ = VK_NULL_HANDLE;
    }

    VulkanAllocator& operator=(VulkanAllocator&& other) noexcept {
        if (this != &other) {
            if (allocator_ != VK_NULL_HANDLE) {
                vmaDestroyAllocator(allocator_);
            }
            allocator_ = other.allocator_;
            other.allocator_ = VK_NULL_HANDLE;
        }
        return *this;
    }

    VmaAllocator get() const { return allocator_; }

private:
    VmaAllocator allocator_;
};

class VulkanSwapchainKHR {
public:
    VulkanSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR& createInfo) : device_(device), swapchain_(VK_NULL_HANDLE) {
        vkCreateSwapchainKHR(device_, &createInfo, nullptr, &swapchain_);
    }

    // Constructor to wrap an existing handle
    VulkanSwapchainKHR(VkDevice device, VkSwapchainKHR swapchain) : device_(device), swapchain_(swapchain) {
        // Assumes ownership of the provided handle
    }

    ~VulkanSwapchainKHR() {
        if (swapchain_ != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(device_, swapchain_, nullptr);
        }
    }

    VulkanSwapchainKHR(const VulkanSwapchainKHR&) = delete;
    VulkanSwapchainKHR& operator=(const VulkanSwapchainKHR&) = delete;

    VulkanSwapchainKHR(VulkanSwapchainKHR&& other) noexcept : device_(other.device_), swapchain_(other.swapchain_) {
        other.swapchain_ = VK_NULL_HANDLE;
    }

    VulkanSwapchainKHR& operator=(VulkanSwapchainKHR&& other) noexcept {
        if (this != &other) {
            if (swapchain_ != VK_NULL_HANDLE) {
                vkDestroySwapchainKHR(device_, swapchain_, nullptr);
            }
            device_ = other.device_;
            swapchain_ = other.swapchain_;
            other.swapchain_ = VK_NULL_HANDLE;
        }
        return *this;
    }

    VkSwapchainKHR get() const { return swapchain_; }

private:
    VkDevice device_;
    VkSwapchainKHR swapchain_;
};

class VulkanImageView {
public:
    VulkanImageView(VkDevice device, const VkImageViewCreateInfo& createInfo) : device_(device), imageView_(VK_NULL_HANDLE) {
        vkCreateImageView(device_, &createInfo, nullptr, &imageView_);
    }

    // Constructor to wrap an existing handle
    VulkanImageView(VkDevice device, VkImageView imageView) : device_(device), imageView_(imageView) {
        // Assumes ownership of the provided handle
    }

    ~VulkanImageView() {
        if (imageView_ != VK_NULL_HANDLE) {
            vkDestroyImageView(device_, imageView_, nullptr);
        }
    }

    VulkanImageView(const VulkanImageView&) = delete;
    VulkanImageView& operator=(const VulkanImageView&) = delete;

    VulkanImageView(VulkanImageView&& other) noexcept : device_(other.device_), imageView_(other.imageView_) {
        other.imageView_ = VK_NULL_HANDLE;
    }

    VulkanImageView& operator=(VulkanImageView&& other) noexcept {
        if (this != &other) {
            if (imageView_ != VK_NULL_HANDLE) {
                vkDestroyImageView(device_, imageView_, nullptr);
            }
            device_ = other.device_;
            imageView_ = other.imageView_;
            other.imageView_ = VK_NULL_HANDLE;
        }
        return *this;
    }

    VkImageView get() const { return imageView_; }

private:
    VkDevice device_;
    VkImageView imageView_;
};

class VulkanSampler {
public:
    VulkanSampler(VkDevice device, const VkSamplerCreateInfo& createInfo) : device_(device), sampler_(VK_NULL_HANDLE) {
        vkCreateSampler(device_, &createInfo, nullptr, &sampler_);
    }

    ~VulkanSampler() {
        if (sampler_ != VK_NULL_HANDLE) {
            vkDestroySampler(device_, sampler_, nullptr);
        }
    }

    VulkanSampler(const VulkanSampler&) = delete;
    VulkanSampler& operator=(const VulkanSampler&) = delete;

    VulkanSampler(VulkanSampler&& other) noexcept : device_(other.device_), sampler_(other.sampler_) {
        other.sampler_ = VK_NULL_HANDLE;
    }

    VulkanSampler& operator=(VulkanSampler&& other) noexcept {
        if (this != &other) {
            if (sampler_ != VK_NULL_HANDLE) {
                vkDestroySampler(device_, sampler_, nullptr);
            }
            device_ = other.device_;
            sampler_ = other.sampler_;
            other.sampler_ = VK_NULL_HANDLE;
        }
        return *this;
    }

    VkSampler get() const { return sampler_; }

private:
    VkDevice device_;
    VkSampler sampler_;
};

class VulkanAllocatedBuffer {
public:
    VulkanAllocatedBuffer(VmaAllocator allocator, const VkBufferCreateInfo& bufferInfo, const VmaAllocationCreateInfo& allocInfo)
        : allocator_(allocator), buffer_(VK_NULL_HANDLE), allocation_(VK_NULL_HANDLE) {
        vmaCreateBuffer(allocator_, &bufferInfo, &allocInfo, &buffer_, &allocation_, &allocationInfo_);
    }

    ~VulkanAllocatedBuffer() {
        if (allocator_ != VK_NULL_HANDLE && buffer_ != VK_NULL_HANDLE && allocation_ != VK_NULL_HANDLE) {
            vmaDestroyBuffer(allocator_, buffer_, allocation_);
        }
    }

    VulkanAllocatedBuffer(const VulkanAllocatedBuffer&) = delete;
    VulkanAllocatedBuffer& operator=(const VulkanAllocatedBuffer&) = delete;

    VulkanAllocatedBuffer(VulkanAllocatedBuffer&& other) noexcept
        : allocator_(other.allocator_), buffer_(other.buffer_), allocation_(other.allocation_), allocationInfo_(other.allocationInfo_) {
        other.buffer_ = VK_NULL_HANDLE;
        other.allocation_ = VK_NULL_HANDLE;
    }

    VulkanAllocatedBuffer& operator=(VulkanAllocatedBuffer&& other) noexcept {
        if (this != &other) {
            if (allocator_ != VK_NULL_HANDLE && buffer_ != VK_NULL_HANDLE && allocation_ != VK_NULL_HANDLE) {
                vmaDestroyBuffer(allocator_, buffer_, allocation_);
            }
            allocator_ = other.allocator_;
            buffer_ = other.buffer_;
            allocation_ = other.allocation_;
            allocationInfo_ = other.allocationInfo_;

            other.buffer_ = VK_NULL_HANDLE;
            other.allocation_ = VK_NULL_HANDLE;
        }
        return *this;
    }

    VkBuffer getBuffer() const { return buffer_; } // Renamed from get()
    VmaAllocation getAllocation() const { return allocation_; }
    const VmaAllocationInfo& getInfo() const { return allocationInfo_; } // Renamed from getAllocationInfo
    void* getMappedData() const { return allocationInfo_.pMappedData; } // Added for convenience


private:
    VmaAllocator allocator_;
    VkBuffer buffer_;
    VmaAllocation allocation_;
    VmaAllocationInfo allocationInfo_{};
};

class VulkanAllocatedImage {
public:
    VulkanAllocatedImage(VkDevice device, VmaAllocator allocator, const VkImageCreateInfo& imageInfo, const VmaAllocationCreateInfo& allocInfo, const VkImageViewCreateInfo& imageViewInfo)
        : device_(device), allocator_(allocator), image_(VK_NULL_HANDLE), imageView_(VK_NULL_HANDLE), allocation_(VK_NULL_HANDLE), extent_(imageInfo.extent), format_(imageInfo.format) {
        vmaCreateImage(allocator_, &imageInfo, &allocInfo, &image_, &allocation_, nullptr);
        
        VkImageViewCreateInfo viewInfoCopy = imageViewInfo;
        viewInfoCopy.image = image_; 
        vkCreateImageView(device_, &viewInfoCopy, nullptr, &imageView_);
    }

    ~VulkanAllocatedImage() {
        if (imageView_ != VK_NULL_HANDLE) {
            vkDestroyImageView(device_, imageView_, nullptr);
        }
        if (allocator_ != VK_NULL_HANDLE && image_ != VK_NULL_HANDLE && allocation_ != VK_NULL_HANDLE) {
            vmaDestroyImage(allocator_, image_, allocation_);
        }
    }

    VulkanAllocatedImage(const VulkanAllocatedImage&) = delete;
    VulkanAllocatedImage& operator=(const VulkanAllocatedImage&) = delete;

    VulkanAllocatedImage(VulkanAllocatedImage&& other) noexcept
        : device_(other.device_), allocator_(other.allocator_), image_(other.image_), imageView_(other.imageView_),
          allocation_(other.allocation_), extent_(other.extent_), format_(other.format_) {
        other.image_ = VK_NULL_HANDLE;
        other.imageView_ = VK_NULL_HANDLE;
        other.allocation_ = VK_NULL_HANDLE;
    }

    VulkanAllocatedImage& operator=(VulkanAllocatedImage&& other) noexcept {
        if (this != &other) {
            if (imageView_ != VK_NULL_HANDLE) {
                vkDestroyImageView(device_, imageView_, nullptr);
            }
            if (allocator_ != VK_NULL_HANDLE && image_ != VK_NULL_HANDLE && allocation_ != VK_NULL_HANDLE) {
                vmaDestroyImage(allocator_, image_, allocation_);
            }

            device_ = other.device_;
            allocator_ = other.allocator_;
            image_ = other.image_;
            imageView_ = other.imageView_;
            allocation_ = other.allocation_;
            extent_ = other.extent_;
            format_ = other.format_;

            other.image_ = VK_NULL_HANDLE;
            other.imageView_ = VK_NULL_HANDLE;
            other.allocation_ = VK_NULL_HANDLE;
        }
        return *this;
    }

    VkImage getImage() const { return image_; } // Renamed from get()
    VkImageView getImageView() const { return imageView_; }
    VmaAllocation getAllocation() const { return allocation_; }
    VkExtent3D getExtent() const { return extent_; }
    VkFormat getFormat() const { return format_; }

private:
    VkDevice device_;
    VmaAllocator allocator_;
    VkImage image_;
    VkImageView imageView_;
    VmaAllocation allocation_;
    VkExtent3D extent_;
    VkFormat format_;
};

class VulkanPipelineLayout {
public:
    VulkanPipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo& createInfo) : device_(device), layout_(VK_NULL_HANDLE) {
        vkCreatePipelineLayout(device_, &createInfo, nullptr, &layout_);
    }

    ~VulkanPipelineLayout() {
        if (layout_ != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(device_, layout_, nullptr);
        }
    }

    VulkanPipelineLayout(const VulkanPipelineLayout&) = delete;
    VulkanPipelineLayout& operator=(const VulkanPipelineLayout&) = delete;

    VulkanPipelineLayout(VulkanPipelineLayout&& other) noexcept : device_(other.device_), layout_(other.layout_) {
        other.layout_ = VK_NULL_HANDLE;
    }

    VulkanPipelineLayout& operator=(VulkanPipelineLayout&& other) noexcept {
        if (this != &other) {
            if (layout_ != VK_NULL_HANDLE) {
                vkDestroyPipelineLayout(device_, layout_, nullptr);
            }
            device_ = other.device_;
            layout_ = other.layout_;
            other.layout_ = VK_NULL_HANDLE;
        }
        return *this;
    }

    VkPipelineLayout get() const { return layout_; }

private:
    VkDevice device_;
    VkPipelineLayout layout_;
};

class VulkanPipeline {
public:
    // Constructor for Graphics Pipelines
    VulkanPipeline(VkDevice device, const VkGraphicsPipelineCreateInfo& createInfo, VkPipelineCache pipelineCache = VK_NULL_HANDLE) 
        : device_(device), pipeline_(VK_NULL_HANDLE) {
        vkCreateGraphicsPipelines(device_, pipelineCache, 1, &createInfo, nullptr, &pipeline_);
    }

    // Constructor for Compute Pipelines
    VulkanPipeline(VkDevice device, const VkComputePipelineCreateInfo& createInfo, VkPipelineCache pipelineCache = VK_NULL_HANDLE)
        : device_(device), pipeline_(VK_NULL_HANDLE) {
        vkCreateComputePipelines(device_, pipelineCache, 1, &createInfo, nullptr, &pipeline_);
    }

    ~VulkanPipeline() {
        if (pipeline_ != VK_NULL_HANDLE) {
            vkDestroyPipeline(device_, pipeline_, nullptr);
        }
    }

    VulkanPipeline(const VulkanPipeline&) = delete;
    VulkanPipeline& operator=(const VulkanPipeline&) = delete;

    VulkanPipeline(VulkanPipeline&& other) noexcept : device_(other.device_), pipeline_(other.pipeline_) {
        other.pipeline_ = VK_NULL_HANDLE;
    }

    VulkanPipeline& operator=(VulkanPipeline&& other) noexcept {
        if (this != &other) {
            if (pipeline_ != VK_NULL_HANDLE) {
                vkDestroyPipeline(device_, pipeline_, nullptr);
            }
            device_ = other.device_;
            pipeline_ = other.pipeline_;
            other.pipeline_ = VK_NULL_HANDLE;
        }
        return *this;
    }

    VkPipeline get() const { return pipeline_; }

private:
    VkDevice device_;
    VkPipeline pipeline_;
};

class VulkanDescriptorPool {
public:
    VulkanDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo& createInfo) : device_(device), pool_(VK_NULL_HANDLE) {
        vkCreateDescriptorPool(device_, &createInfo, nullptr, &pool_);
    }

    ~VulkanDescriptorPool() {
        if (pool_ != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(device_, pool_, nullptr);
        }
    }

    VulkanDescriptorPool(const VulkanDescriptorPool&) = delete;
    VulkanDescriptorPool& operator=(const VulkanDescriptorPool&) = delete;

    VulkanDescriptorPool(VulkanDescriptorPool&& other) noexcept : device_(other.device_), pool_(other.pool_) {
        other.pool_ = VK_NULL_HANDLE;
    }

    VulkanDescriptorPool& operator=(VulkanDescriptorPool&& other) noexcept {
        if (this != &other) {
            if (pool_ != VK_NULL_HANDLE) {
                vkDestroyDescriptorPool(device_, pool_, nullptr);
            }
            device_ = other.device_;
            pool_ = other.pool_;
            other.pool_ = VK_NULL_HANDLE;
        }
        return *this;
    }

    VkDescriptorPool get() const { return pool_; }

private:
    VkDevice device_;
    VkDescriptorPool pool_;
};

class VulkanDescriptorSetLayout {
public:
    // Constructor to create a new layout
    VulkanDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo& createInfo) : device_(device), layout_(VK_NULL_HANDLE) {
        vkCreateDescriptorSetLayout(device_, &createInfo, nullptr, &layout_);
    }

    // Constructor to wrap an existing layout handle
    VulkanDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout layout) : device_(device), layout_(layout) {
        // Assumes ownership of the provided layout handle
    }

    ~VulkanDescriptorSetLayout() {
        if (layout_ != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(device_, layout_, nullptr);
        }
    }

    VulkanDescriptorSetLayout(const VulkanDescriptorSetLayout&) = delete;
    VulkanDescriptorSetLayout& operator=(const VulkanDescriptorSetLayout&) = delete;

    VulkanDescriptorSetLayout(VulkanDescriptorSetLayout&& other) noexcept : device_(other.device_), layout_(other.layout_) {
        other.layout_ = VK_NULL_HANDLE;
    }

    VulkanDescriptorSetLayout& operator=(VulkanDescriptorSetLayout&& other) noexcept {
        if (this != &other) {
            if (layout_ != VK_NULL_HANDLE) {
                vkDestroyDescriptorSetLayout(device_, layout_, nullptr);
            }
            device_ = other.device_;
            layout_ = other.layout_;
            other.layout_ = VK_NULL_HANDLE;
        }
        return *this;
    }

    VkDescriptorSetLayout get() const { return layout_; }

private:
    VkDevice device_;
    VkDescriptorSetLayout layout_;
};
