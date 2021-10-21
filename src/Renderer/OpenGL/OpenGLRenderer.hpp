#pragma once
#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <spdlog/spdlog.h>
#include <backends/imgui_impl_opengl3.h>
#include "Renderer/RenderAPI.hpp"
#include "Renderer/DataTypes.hpp"
#include "GLFW/glfw3.h"
#include "Engine/EventManager.hpp"
/**
 * @class OpenGLRenderer
 * @brief OpenGL rendering api implementation
 */
class OpenGLRenderer : public RenderAPI 
{
  public:
    /**
     * @brief Constructor for OpenGLRenderer
     */
    OpenGLRenderer() = default;
    /**
     * @brief Destructor for OpenGLRenderer
     */
    ~OpenGLRenderer()override = default;
    /**
     * @brief Initialises OpenGL
     */
    void Init() override;
    /**
     * @brief Deinitialises OpenGL
     */
    void DeInit() override;
    /**
     * @brief Toggles Wireframe
     */
    void ToggleWireFrame() override;
    /**
     * @brief Draws a mesh
     * @param VertexArrayBuffer& - VAO
     * @param size_t - indiceSize
     */
    void DrawArrays(VertexArrayBuffer& VAO,size_t indiceSize) override;
    /**
     * @brief Clears the Screen after Rendering
     */
    void Clear() override;
    /**
     * @brief Draws the GUI
     */
    void DrawGui() override;
    /**
     * @param Set Depth Testing
     * @param bool - isEnabled
     */
    void SetDepthTesting(bool isEnabled) override;
    void DrawScene() override;
    void SubmitDraw(RENDER::DrawItem drawItem) override; 
    void UpdateLights(std::vector<PointLight> &lightsArray);
  private:
    ///Boolean to check whether it is in wireframe
    glm::ivec2 SCREEN_SIZE;
    size_t NUM_LIGHTS = 4000;
    bool isWireFrame = false;
    void DrawQuad();
    void SetupDepthMap();
    void SetupHDRBuffer();
    const void DrawMesh(Shader* shader, Mesh* mesh);
    Shader* depthShader;
    Shader* lightAccumulationShader;
    Shader* lightCullingShader;
    Shader* hdr;
    Shader* gridShader;
    std::vector<RENDER::DrawItem> drawQueue;
    //Shared SSBO'S
    GLuint screenToViewSSBO;
    GLuint lightBuffer = 0;
    //For Light culling
    GLuint visibleLightIndicesBuffer = 0;
    GLuint clusterAABB = 0;
    //Quad For post Processing effects
    GLuint quadVAO = 0;
    GLuint quadVBO = 0;
    //Final Scene FrameBuffer
    GLuint hdrFBO;
    //Render Buffer attatch to HDR Frame Buffer
    GLuint rboDepth = 0;
    GLuint colorBuffer = 0;
    //Depth map for Z fighting pass
    GLuint depthMapFBO = 0;
    GLuint depthMap = 0;

    const int numClusters = 3456;
    const int gridSizeX = 16;
    const int gridSizeY = 9;
    const int gridSizeZ = 24;
    GLuint lightGridSSBO;
    GLuint lightIndexGlobalCountSSBO;
    ScreenToView screen2View;
};
