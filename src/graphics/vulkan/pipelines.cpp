#include "graphics/vulkan/pipelines.h"
#include "graphics/vulkan/vk_engine.h" // For VulkanEngine, VulkanPipelineLayout, VulkanPipeline wrappers

// Default constructor (if added in header)
GLTFMetallic_Roughness::GLTFMetallic_Roughness() {}

// Destructor
GLTFMetallic_Roughness::~GLTFMetallic_Roughness() {} // Empty, unique_ptrs handle cleanup

void GLTFMetallic_Roughness::build_pipelines(VulkanEngine* engine) {
    VkShaderModule meshFragShader =
            load_shader(engine, "./shaders/mesh.frag.spv", "fragment");
    VkShaderModule meshVertexShader =
            load_shader(engine, "./shaders/mesh.vert.spv", "vertex");

    create_material_layout(engine); // Creates this->materialLayout (VkDescriptorSetLayout)

    // Create pipeline layouts - they must be separate unique_ptr objects now
    // The create_pipeline_layout helper will be modified or its logic inlined/called twice
    // For now, let's assume create_pipeline_layout is modified to return the CreateInfo
    // and we make_unique from that. Or, it returns a unique_ptr directly.

    // Modifying create_pipeline_layout to return unique_ptr for simplicity here.
    // If create_pipeline_layout is called twice, it creates two distinct Vulkan objects.
    opaquePipeline.layout = this->create_pipeline_layout_unique_ptr(engine); // New helper or modified
    transparentPipeline.layout = this->create_pipeline_layout_unique_ptr(engine); // New helper or modified

    // Pass the raw layout handle to build_..._pipeline methods
    build_opaque_pipeline(engine, meshVertexShader, meshFragShader, opaquePipeline.layout->get());
    build_transparent_pipeline(engine, meshVertexShader, meshFragShader, transparentPipeline.layout->get());

    vkDestroyShaderModule(engine->_device, meshFragShader, nullptr);
    vkDestroyShaderModule(engine->_device, meshVertexShader, nullptr);
}

VkShaderModule GLTFMetallic_Roughness::load_shader(VulkanEngine* engine,
                                                   const char* relative_path,
                                                   const char* type) {
    VkShaderModule shaderModule;
    if (!vkutil::load_shader_module(relative_path, engine->_device,
                                    &shaderModule)) {
        fmt::println("Error when building the {} shader module", type);
    }
    return shaderModule;
}

void GLTFMetallic_Roughness::create_material_layout(VulkanEngine* engine) {
    DescriptorLayoutBuilder layoutBuilder;
    layoutBuilder.add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    layoutBuilder.add_binding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    layoutBuilder.add_binding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

    materialLayout = layoutBuilder.build(
            engine->_device,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
}

// Option 1: Modify create_pipeline_layout to return a unique_ptr
// This helper is illustrative; the actual create_pipeline_layout would be modified
std::unique_ptr<VulkanPipelineLayout> GLTFMetallic_Roughness::create_pipeline_layout_unique_ptr(VulkanEngine* engine) {
    VkPushConstantRange matrixRange{};
    matrixRange.offset = 0;
    matrixRange.size = sizeof(GPUDrawPushConstants);
    matrixRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayout layouts[] = {engine->_gpuSceneDataDescriptorLayout,
                                       materialLayout};
    VkPipelineLayoutCreateInfo mesh_layout_info =
            vkinit::pipeline_layout_create_info();
    mesh_layout_info.setLayoutCount = 2;
    mesh_layout_info.pSetLayouts = layouts;
    mesh_layout_info.pPushConstantRanges = &matrixRange;
    mesh_layout_info.pushConstantRangeCount = 1;

    // VkPipelineLayout newLayout;
    // VK_CHECK(vkCreatePipelineLayout(engine->_device, &mesh_layout_info, nullptr,
    //                                 &newLayout));
    // return newLayout;
    return std::make_unique<VulkanPipelineLayout>(engine->_device, mesh_layout_info);
}


// Original create_pipeline_layout, now perhaps unused or for internal logic if needed
// VkPipelineLayout GLTFMetallic_Roughness::create_pipeline_layout(
//         VulkanEngine* engine) {
//     VkPushConstantRange matrixRange{};
//     matrixRange.offset = 0;
//     matrixRange.size = sizeof(GPUDrawPushConstants);
//     matrixRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

//     VkDescriptorSetLayout layouts[] = {engine->_gpuSceneDataDescriptorLayout->get(), // Ensure ->get() if these are unique_ptr
//                                        materialLayout};
//     VkPipelineLayoutCreateInfo mesh_layout_info =
//             vkinit::pipeline_layout_create_info();
//     mesh_layout_info.setLayoutCount = 2;
//     mesh_layout_info.pSetLayouts = layouts;
//     mesh_layout_info.pPushConstantRanges = &matrixRange;
//     mesh_layout_info.pushConstantRangeCount = 1;

//     VkPipelineLayout newLayout_raw;
//     VK_CHECK(vkCreatePipelineLayout(engine->_device, &mesh_layout_info, nullptr,
//                                     &newLayout_raw));
//     return newLayout_raw;
// }


void GLTFMetallic_Roughness::build_opaque_pipeline(VulkanEngine* engine,
                                                   VkShaderModule vertexShader,
                                                   VkShaderModule fragShader,
                                                   VkPipelineLayout layout_handle) { // Takes raw handle
    PipelineBuilder pipelineBuilder;
    pipelineBuilder.set_shaders(vertexShader, fragShader);
    pipelineBuilder.set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    pipelineBuilder.set_polygon_mode(VK_POLYGON_MODE_FILL);
    pipelineBuilder.set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
    pipelineBuilder.set_multisampling_none();
    pipelineBuilder.disable_blending();
    pipelineBuilder.enable_depthtest(true, VK_COMPARE_OP_GREATER_OR_EQUAL);
    pipelineBuilder.set_color_attachment_format(engine->_drawImage->getFormat()); // Access via getter
    pipelineBuilder.set_depth_format(engine->_depthImage ? engine->_depthImage->getFormat() : VK_FORMAT_UNDEFINED); // Access via getter, check for null
    pipelineBuilder._pipelineLayout = layout_handle;

    // opaquePipeline.pipeline = pipelineBuilder.build_pipeline(engine->_device);
    VkPipeline raw_pipeline = pipelineBuilder.build_pipeline(engine->_device);
    if (raw_pipeline != VK_NULL_HANDLE) {
        opaquePipeline.pipeline = std::make_unique<VulkanPipeline>(engine->_device, raw_pipeline); // Wrap raw pipeline
    }
}

void GLTFMetallic_Roughness::build_transparent_pipeline(
        VulkanEngine* engine, VkShaderModule vertexShader,
        VkShaderModule fragShader, VkPipelineLayout layout_handle) { // Takes raw handle
    PipelineBuilder pipelineBuilder;
    pipelineBuilder.set_shaders(vertexShader, fragShader);
    // Assuming color and depth formats are set similarly for transparent pipeline if needed
    pipelineBuilder.set_color_attachment_format(engine->_drawImage->getFormat()); 
    pipelineBuilder.set_depth_format(engine->_depthImage ? engine->_depthImage->getFormat() : VK_FORMAT_UNDEFINED);
    pipelineBuilder.enable_blending_additive();
    pipelineBuilder.enable_depthtest(false, VK_COMPARE_OP_GREATER_OR_EQUAL); // Typically false for transparent
    pipelineBuilder._pipelineLayout = layout_handle;

    // transparentPipeline.pipeline =
    //         pipelineBuilder.build_pipeline(engine->_device);
    VkPipeline raw_pipeline = pipelineBuilder.build_pipeline(engine->_device);
    if (raw_pipeline != VK_NULL_HANDLE) {
         transparentPipeline.pipeline = std::make_unique<VulkanPipeline>(engine->_device, raw_pipeline); // Wrap raw pipeline
    }
}

MaterialInstance GLTFMetallic_Roughness::write_material(
        VkDevice device, MaterialPass pass, const MaterialResources& resources,
        DescriptorAllocatorGrowable& descriptorAllocator) {
    MaterialInstance matData{};
    matData.passType = pass;
    matData.pipeline = (pass == MaterialPass::Transparent)
                               ? &transparentPipeline
                               : &opaquePipeline;
    matData.materialSet = descriptorAllocator.allocate(device, materialLayout);

    writer.clear();
    writer.write_buffer(0, resources.dataBuffer, sizeof(MaterialConstants),
                        resources.dataBufferOffset,
                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    writer.write_image(1, resources.colorImage.imageView,
                       resources.colorSampler,
                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                       VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    writer.write_image(2, resources.metalRoughImage.imageView,
                       resources.metalRoughSampler,
                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                       VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

    writer.update_set(device, matData.materialSet);
    writer.update_set(device, matData.materialSet);

    return matData;
}

// Pipelines Implementation
Pipelines::~Pipelines() {} // Destructor (already present from Turn 52 diff)

bool Pipelines::init(VkDevice device, 
                     VkDescriptorSetLayout singleImageDescriptorLayout,
                     VkDescriptorSetLayout drawImageDescriptorLayout,
                     const VulkanAllocatedImage& drawImage) { // Signature matches header
    _device = device;
    _singleImageDescriptorLayout = singleImageDescriptorLayout;
    _drawImageDescriptorLayout = drawImageDescriptorLayout;
    // _drawImage member was removed in pipelines.h (Turn 54), so no assignment here.

    GraphicsPipeline::GraphicsPipelineConfig triangleConfig;
    triangleConfig.vertexShaderPath = "./shaders/colored_triangle.vert.spv";
    triangleConfig.fragmentShaderPath = "./shaders/colored_triangle.frag.spv";
    triangleConfig.colorFormat = drawImage.getFormat(); // Use parameter.getFormat()
    triangleConfig.depthFormat = VK_FORMAT_UNDEFINED; // Assuming no depth for this simple pipeline
    triangleConfig.depthTest = false;
    triangleConfig.cullMode = VK_CULL_MODE_NONE;
    triangleConfig.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    trianglePipeline = std::make_unique<GraphicsPipeline>(triangleConfig);
    trianglePipeline->init(device);
    
    GraphicsPipeline::GraphicsPipelineConfig meshConfig;
    meshConfig.vertexShaderPath = "./shaders/colored_triangle_mesh.vert.spv";
    meshConfig.fragmentShaderPath = "./shaders/tex_image.frag.spv";
    meshConfig.colorFormat = drawImage.getFormat(); // Use parameter.getFormat()
    meshConfig.depthFormat = VK_FORMAT_UNDEFINED; // Assuming no depth, or use drawImage.getFormat() if appropriate
    meshConfig.depthTest = true;
    meshConfig.depthCompareOp = VK_COMPARE_OP_GREATER;
    meshConfig.cullMode = VK_CULL_MODE_NONE;
    meshConfig.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    
    VkPushConstantRange bufferRange{};
    bufferRange.offset = 0;
    bufferRange.size = sizeof(GPUDrawPushConstants);
    bufferRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    meshConfig.pushConstants.push_back(bufferRange);
    
    meshConfig.descriptorSetLayouts.push_back(_singleImageDescriptorLayout);

    meshPipeline = std::make_unique<GraphicsPipeline>(meshConfig);
    meshPipeline->init(device);
    
    // Initialize gradient pipeline
    ComputePipeline::ComputePipelineConfig gradientConfig;
    gradientConfig.descriptorSetLayout = drawImageDescriptorLayout; 
    gradientConfig.shaderPath = "./shaders/gradient.comp.spv";
    
    gradientPipeline = std::make_unique<ComputePipeline>(gradientConfig);
    if (!gradientPipeline->init(device, drawImageDescriptorLayout, drawImage)) { 
        fmt::println("Failed to initialize gradient pipeline");
        return false;
    }

    fmt::println("Pipelines initialized successfully");
    return true;
}

// void Pipelines::destroy() {} // Removed by commenting out or deleting the whole function
}
