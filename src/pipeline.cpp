#include "pipeline.hpp"
#include "app.hpp"


Pipeline::Pipeline()
{

}

void Pipeline::Initalize(ShaderSettings vertSettings, ShaderSettings fragSettings)
{
    this->vertSettings = vertSettings;
    this->fragSettings = fragSettings;
}

void Pipeline::CreatePipeline(AppContext* context)
{
    // Create the shaders
	glslang_initialize_process();

    vertexShader.CompileShader(context->basePath, context->gpuDevice, vertSettings, true);
    fragmentShader.CompileShader(context->basePath, context->gpuDevice, fragSettings, true);

	glslang_finalize_process();

	// Create the pipeline
	SDL_GPUVertexBufferDescription vertexBufferDescriptions[1];
	vertexBufferDescriptions[0] = {};
	vertexBufferDescriptions[0].slot = 0;
	vertexBufferDescriptions[0].pitch = sizeof(Vertex);
	vertexBufferDescriptions[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
	vertexBufferDescriptions[0].instance_step_rate = 0;

	SDL_GPUVertexAttribute vertexAttributes[3];
	vertexAttributes[0] = {};
	vertexAttributes[0].location = 0;
	vertexAttributes[0].buffer_slot = 0;
	vertexAttributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
	vertexAttributes[0].offset = 0;

	vertexAttributes[1] = {};
	vertexAttributes[1].location = 1;
	vertexAttributes[1].buffer_slot = 0;
	vertexAttributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
	vertexAttributes[1].offset = offsetof(Vertex, normal);

	vertexAttributes[2] = {};
	vertexAttributes[2].location = 2;
	vertexAttributes[2].buffer_slot = 0;
	vertexAttributes[2].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
	vertexAttributes[2].offset = offsetof(Vertex, uv);

	SDL_GPUVertexInputState vertexInputState = {};
	vertexInputState.vertex_buffer_descriptions = vertexBufferDescriptions;
	vertexInputState.num_vertex_buffers = 1;
	vertexInputState.vertex_attributes = vertexAttributes;
	vertexInputState.num_vertex_attributes = 3;

	SDL_GPUColorTargetDescription colorTargetDescriptions[1];
	colorTargetDescriptions[0] = {};
	colorTargetDescriptions[0].format = SDL_GetGPUSwapchainTextureFormat(context->gpuDevice, context->window);

	SDL_GPUGraphicsPipelineTargetInfo targetInfo = {};
	targetInfo.color_target_descriptions = colorTargetDescriptions;
	targetInfo.num_color_targets = 1;
	targetInfo.has_depth_stencil_target = true;
	targetInfo.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D24_UNORM;

	SDL_GPUDepthStencilState depthStencil = {};
	depthStencil.enable_depth_test = true;
	depthStencil.enable_depth_write = true;
	depthStencil.enable_stencil_test = false;
	depthStencil.compare_op = SDL_GPU_COMPAREOP_LESS;
	depthStencil.write_mask = 0xFF;

	SDL_GPURasterizerState rasterizerState = {};
	rasterizerState.cull_mode = SDL_GPU_CULLMODE_NONE;
	rasterizerState.fill_mode = SDL_GPU_FILLMODE_FILL;
	rasterizerState.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;

	SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.vertex_shader = vertexShader.shader;
	pipelineCreateInfo.fragment_shader = fragmentShader.shader;
	pipelineCreateInfo.vertex_input_state = vertexInputState;
	pipelineCreateInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
	pipelineCreateInfo.target_info = targetInfo;
	pipelineCreateInfo.depth_stencil_state = depthStencil;
	pipelineCreateInfo.rasterizer_state = rasterizerState;

	pipeline = SDL_CreateGPUGraphicsPipeline(context->gpuDevice, &pipelineCreateInfo);
	if (!pipeline)
	{
		SDL_Log("Failed to create pipeline!");
        SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
		return;
	}


	SDL_ReleaseGPUShader(context->gpuDevice, vertexShader.shader);
	SDL_ReleaseGPUShader(context->gpuDevice, fragmentShader.shader);

    this->pipelineCreateInfo = pipelineCreateInfo;
}

void Pipeline::Bind(SDL_GPURenderPass* renderPass)
{
	SDL_BindGPUGraphicsPipeline(renderPass, pipeline);
}

void Pipeline::Delete(AppContext* context)
{
	if(pipeline != NULL)
		SDL_ReleaseGPUGraphicsPipeline(context->gpuDevice, pipeline);
}
