//
// Created by charl on 1/03/2021.
//
#pragma once
#include "OpenGLRenderer.hpp"
#include <glm/gtx/string_cast.hpp>
#include <Physics/PhysicsSystem.hpp>
void OpenGLRenderer::Init() 
{
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        SPDLOG_ERROR("Failed to initialize GLEW");
        return;
    }
    glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_CULL_FACE);
    SCREEN_SIZE.x = 1920;
    SCREEN_SIZE.y = 1080;
	float zFar = 300.0f;
	float zNear = 0.1f;
	unsigned int totalNumLights =  numClusters * 100; 
	depthShader = new Shader("content/Shaders/depth.vert", "content/Shaders/depth.frag");
	lightAccumulationShader = new Shader("content/Shaders/light_accumulation.vert", "content/Shaders/light_accumulation.frag");
	lightCullingShader = new Shader("content/Shaders/clusterLightCuller.comp");
	hdr = new Shader("content/Shaders/hdr.vert", "content/Shaders/hdr.frag");
	gridShader = new Shader("content/Shaders/clusterGenerator.comp");
    
	glm::mat4 perspective = glm::perspective(glm::radians(60.0f), 1920.0f / 1080.0f, zNear, zFar);
	depthShader->UseShader();
	depthShader->SetMat4("projection",perspective);

    lightAccumulationShader->UseShader();
	lightAccumulationShader->SetMat4("projection",perspective);
	lightAccumulationShader->SetFloat("zNear",zNear);
	lightAccumulationShader->SetFloat("zFar",zFar);
	
	hdr->UseShader();
	hdr->SetFloat("exposure", 0.5f);
    SPDLOG_INFO("OpenGL version: {}",glGetString(GL_VERSION));
	SPDLOG_INFO("GLSL version: {}",glGetString(GL_SHADING_LANGUAGE_VERSION));
	SPDLOG_INFO("Vendor: {}",glGetString(GL_VENDOR));
	SPDLOG_INFO("Renderer: {}", glGetString(GL_RENDERER));
    if (!GLEW_VERSION_4_3) 
    {
		SPDLOG_ERROR("OpenGL version be >= 4.3");
        throw std::runtime_error("OpenGL 4.3 API is not available.");
	}
	// Generate our shader storage buffers
	glGenBuffers(1, &lightBuffer);
	glGenBuffers(1, &visibleLightIndicesBuffer);
	glGenBuffers(1, &clusterAABB);
	glGenBuffers(1, &screenToViewSSBO);
	glGenBuffers(1, &lightIndexGlobalCountSSBO);
	glGenBuffers(1, &lightGridSSBO);

	// Bind ClusterAABB
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, clusterAABB);
	glBufferData(GL_SHADER_STORAGE_BUFFER, numClusters * sizeof(struct VolumeTileAABB), NULL, GL_STATIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, clusterAABB);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	// Bind visible light indices buffer
    
	//Setup screen2View SSBO
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, screenToViewSSBO);
	//Tile Size X/Y
	int sizeX =  (unsigned int)std::ceilf(SCREEN_SIZE.x / (float)gridSizeX);
    //Setting up contents of buffer
    screen2View.inverseProjectionMat = glm::inverse(perspective);
    screen2View.tileSizes[0] = gridSizeX;
    screen2View.tileSizes[1] = gridSizeY;
    screen2View.tileSizes[2] = gridSizeZ;
    screen2View.tileSizes[3] = sizeX;
    screen2View.screenWidth  = SCREEN_SIZE.x;
    screen2View.screenHeight = SCREEN_SIZE.y;
    //Basically reduced a log function into a simple multiplication an addition by pre-calculating these
    screen2View.sliceScalingFactor = (float)gridSizeZ / std::log2f(zFar / zNear) ;
    screen2View.sliceBiasFactor    = -((float)gridSizeZ * std::log2f(zNear) / std::log2f(zFar / zNear)) ;

    //Generating and copying data to memory in GPU
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(struct ScreenToView), &screen2View, GL_STATIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, screenToViewSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_LIGHTS * sizeof(PointLight), NULL, GL_DYNAMIC_DRAW);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, lightBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, visibleLightIndicesBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER,  totalNumLights * sizeof(unsigned int), NULL, GL_STATIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, visibleLightIndicesBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightGridSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, numClusters * 2 * sizeof(unsigned int), NULL, GL_STATIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, lightGridSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    

	//Every tile takes two unsigned ints one to represent the number of lights in that grid
    //Another to represent the offset 
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightIndexGlobalCountSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int), NULL, GL_STATIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, lightIndexGlobalCountSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	
 	gridShader->UseShader();
    gridShader->SetFloat("zNear", zNear);
    gridShader->SetFloat("zFar", zFar);
    glDispatchCompute(gridSizeX, gridSizeY, gridSizeZ);
	SetupDepthMap();
    SetupHDRBuffer();
	glClearColor(0.01f, 0.01f, 0.01f, 1.0f);
    SPDLOG_INFO("Renderer Initialised: Using OpenGL");

}

void OpenGLRenderer::DeInit() 
{
    ImGui_ImplOpenGL3_Shutdown();
}
void OpenGLRenderer::DrawQuad() 
{
	if (quadVAO == 0) 
    {
		GLfloat quadVertices[] = {
			-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};

		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	}

	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}
void OpenGLRenderer::SetupDepthMap()
{
    glGenFramebuffers(1, &depthMapFBO);

	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SCREEN_SIZE.x, SCREEN_SIZE.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	GLfloat borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLRenderer::SetupHDRBuffer()
{
    // Create a floating point HDR frame buffer and a floating point color buffer (as a texture)
	
	glGenFramebuffers(1, &hdrFBO);

	glGenTextures(1, &colorBuffer);
	glBindTexture(GL_TEXTURE_2D, colorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCREEN_SIZE.x, SCREEN_SIZE.y, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// It will also need a depth component as a render buffer, attached to the hdrFBO
	
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCREEN_SIZE.x, SCREEN_SIZE.y);

	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void OpenGLRenderer::Clear() 
{
    glClearColor(0.01f, 0.01f, 0.01f, 1.0f);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

}
void OpenGLRenderer::UpdateLights(std::vector<PointLight> &lightsArray) 
{
	size_t maxLights = std::min(lightsArray.size(),NUM_LIGHTS);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightBuffer);
	PointLight *pointLights = (PointLight*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);
	for (int i = 0; i < maxLights; i++) 
	{
		PointLight &light = pointLights[i];
		light.position = lightsArray[i].position;
		light.color = lightsArray[i].color;
		light.paddingAndRadius = lightsArray[i].paddingAndRadius;		
	}

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}
void OpenGLRenderer::ResetLightsBuffer()
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightBuffer);
	PointLight *pointLights = (PointLight*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);
	for (int i = 0; i < NUM_LIGHTS; i++) 
	{
		PointLight &light = pointLights[i];
		light.position = glm::vec4(0.0f);
		light.color = glm::vec4(0.0f);
		light.paddingAndRadius = glm::vec4(0.0f);		
	}

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void OpenGLRenderer::ToggleWireFrame() 
{
    if(isWireFrame)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        isWireFrame = false;
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        isWireFrame = true;
    }
}

void OpenGLRenderer::DrawArrays(VertexArrayBuffer& VAO, size_t indicesSize)
{
    VAO.Bind();
    glDrawElements(GL_TRIANGLES, indicesSize, GL_UNSIGNED_INT, 0);

}

void OpenGLRenderer::DrawScene()
{
    glm::mat4 view = EMS::getInstance().fire(ReturnMat4Event::getViewMatrix);
    glm::mat4 inverseview = glm::inverse(view);
    glm::vec3 viewpos = glm::vec3(inverseview[3].x,inverseview[3].y,inverseview[3].z);
	
	//DEPTH PASS
	depthShader->UseShader();
    depthShader->SetMat4("view",view);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(true);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	
	for(auto& drawItem : drawQueue)
    {
        depthShader->SetMat4("model",drawItem.transform);
        DrawMesh(depthShader,drawItem.mesh);
    }
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	//LIGHT CULL
    lightCullingShader->UseShader();
    lightCullingShader->SetMat4("view",view);
	//lightCullingShader->setInt("depthMap", 4);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glDispatchCompute(1, 1, 6);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	// Unbind the depth map
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	//DRAW & LIGHT SCENE
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	lightAccumulationShader->UseShader();
    lightAccumulationShader->SetMat4("view",view);
	lightAccumulationShader->SetVec3("viewPosition",viewpos);
    for(auto& drawItem : drawQueue)
    {
		lightAccumulationShader->SetBool("isAnimated",drawItem.isAnimated);
		lightAccumulationShader->SetVecMat4("boneTrans",drawItem.finalTransforms);
        lightAccumulationShader->SetMat4("model",drawItem.transform);
        DrawMesh(lightAccumulationShader,drawItem.mesh);
    }
	//Draw Physics Debug
	PhysicsSystem::getInstance().Draw();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	//POST PROCESS
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	hdr->UseShader();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, colorBuffer);
	DrawQuad();
	glBindTexture(GL_TEXTURE_2D, 0);
    drawQueue.clear();
}
void OpenGLRenderer::DrawGui() 
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
void OpenGLRenderer::SetDepthTesting(bool isEnabled)
{
    if(isEnabled)
    {
        glEnable(GL_DEPTH_TEST);
    }
    else
        {
        glDisable(GL_DEPTH_TEST);
    }
}
const void OpenGLRenderer::DrawMesh(Shader* shader,Mesh* mesh)
{
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNumber = 1;
	unsigned int heightNumber = 1;
    auto& textures = mesh->getTextures();
    for(unsigned int i = 0; i < textures.size(); i++)
    {
        textures[i].texture->Bind(i);
        std::string number;
        std::string name = textures[i].type;
        if(name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
         else if(name == "texture_specular")
        	number = std::to_string(specularNr++);
         else if(name == "texture_normal")
            number = std::to_string(normalNumber++);
         else if(name == "texture_height")
            number = std::to_string(heightNumber++);
    	shader->SetInt((name + number).c_str(), i);
    }
    DrawArrays(*mesh->GetVAO(),mesh->getIndices()->size());

    // Reset to defaults
	for (size_t i = 0; i < textures.size(); i++) 
	{
		textures[i].texture->UnBind(i);
	}    
}
void OpenGLRenderer::SubmitDraw(RENDER::DrawItem drawItem)
{
    drawQueue.push_back(drawItem);
}