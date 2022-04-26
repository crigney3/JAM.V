#include "AssetManager.h"
#include "CollisionManager.h"

// Effects that require multiple render target views
// are stored in the following order:
// 0 - Color minus ambient
// 1 - Only ambient
// 2 - Only normals
// 3 - Only depths
// 4 - Results of SSAO
// 5 - SSAO with blur fix
// 6 - Refraction Silhouette Render
// 7 - Render of pre-transparency composite
// 8 - Count: always the last one, tracks size
enum RTVTypes 
{
    COLORS_NO_AMBIENT,
    COLORS_AMBIENT,
    NORMALS,
    DEPTHS,
    SSAO_RAW,
    SSAO_BLUR,
    REFRACTION_SILHOUETTE,
    COMPOSITE,

    RTV_TYPE_COUNT
};

enum MiscEffectSRVTypes
{
    FLASHLIGHT_SHADOW,
    ENV_SHADOW,
    REFRACTION_SILHOUETTE_DEPTHS,
    TRANSPARENT_PREPASS_DEPTHS,
    RENDER_PREPASS_DEPTHS,

    MISC_EFFECT_SRV_COUNT
};

// These need to match the expected per-frame/object/material vertex shader data
struct VSPerFrameData
{
    DirectX::XMFLOAT4X4 ViewMatrix;
    DirectX::XMFLOAT4X4 ProjectionMatrix;
    DirectX::XMFLOAT4X4 LightViewMatrix;
    DirectX::XMFLOAT4X4 LightProjectionMatrix;
    DirectX::XMFLOAT4X4 EnvLightViewMatrix;
    DirectX::XMFLOAT4X4 EnvLightProjectionMatrix;
};

struct VSPerMaterialData
{
    DirectX::XMFLOAT4 ColorTint;
};

struct VSPerObjectData
{
    DirectX::XMFLOAT4X4 world;
};

// These need to match the expected per-frame/object/material pixel shader data
struct PSPerFrameData
{
    LightData Lights[MAX_LIGHTS];
    DirectX::XMFLOAT3 CameraPosition;
    unsigned int LightCount;
    int SpecIBLMipLevel;
};

struct PSPerMaterialData
{
    DirectX::XMFLOAT3 AmbientColor;
    float UvMult;
};

class Renderer
{
private:
    AssetManager& globalAssets = AssetManager::GetInstance();

    Microsoft::WRL::ComPtr<ID3D11Device> device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
    Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> backBufferRTV;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthBufferDSV;

    std::shared_ptr<Sky> currentSky;
    DirectX::XMFLOAT3 ambientColor;

    //General shaders
    std::shared_ptr<SimpleVertexShader> basicVS;
    std::shared_ptr<SimpleVertexShader> perFrameVS;
    std::shared_ptr<SimpleVertexShader> fullscreenVS;
    std::shared_ptr<SimplePixelShader> solidColorPS;
    std::shared_ptr<SimplePixelShader> perFramePS;
    std::shared_ptr<SimplePixelShader> textureSamplePS;

    //General meshes
    std::shared_ptr<Mesh> cubeMesh;
    std::shared_ptr<Mesh> sphereMesh;

    //components for shadows
    int shadowSize;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> miscEffectSRVs[MiscEffectSRVTypes::MISC_EFFECT_SRV_COUNT];
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> miscEffectDepthBuffers[MiscEffectSRVTypes::MISC_EFFECT_SRV_COUNT];
    Microsoft::WRL::ComPtr<ID3D11SamplerState> shadowSampler;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> shadowRasterizer;
    std::shared_ptr<SimpleVertexShader> VSShadow;

    //components for colliders
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> colliderRasterizer;

    // Offset and random values for SSAO blur and texture
    Microsoft::WRL::ComPtr<ID3D11Texture2D> ssaoRandomTex;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ssaoRandomSRV;

    Microsoft::WRL::ComPtr<ID3D11BlendState> particleBlendAdditive;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> particleDepthState;

    Microsoft::WRL::ComPtr<ID3D11RenderTargetView>      renderTargetRTVs[RTVTypes::RTV_TYPE_COUNT];
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>    renderTargetSRVs[RTVTypes::RTV_TYPE_COUNT];

    // Ambient Occlusion data
    std::shared_ptr<SimplePixelShader> ssaoPS;
    std::shared_ptr<SimplePixelShader> ssaoBlurPS;
    std::shared_ptr<SimplePixelShader> ssaoCombinePS;
    DirectX::XMFLOAT4 ssaoOffsets[64];
    const float ssaoRadius = 1.5f;
    const int ssaoSamples = 64;
    const int emptyRTV = 0;

    // Regardless of RTV count, SSAO needs 6 textures
    Microsoft::WRL::ComPtr<ID3D11Texture2D> ssaoTexture2D[6];

    // Composite and Silhouette also need textures
    Microsoft::WRL::ComPtr<ID3D11Texture2D> compositeTexture;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> silhouetteTexture;

    unsigned int windowHeight;
    unsigned int windowWidth;

    // Refraction data
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> refractionSilhouetteDepthState;

    // Depth pre-pass data
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> prePassDepthState;

    //Camera pointer
    std::shared_ptr<Camera> mainCamera;

    //Camera pointers for shadows
    std::shared_ptr<Camera> flashShadowCamera;
    std::shared_ptr<Camera> mainShadowCamera;

	// Conditional Drawing
    static bool drawColliders, drawColliderTransforms;

    //Temporary terrain variables
    std::shared_ptr<SimpleVertexShader> VSTerrain;
    std::shared_ptr<SimplePixelShader> PSTerrain;
    std::shared_ptr<Mesh> terrainMesh;
    std::shared_ptr<TerrainMaterial> terrainMat;

    void InitRenderTargetViews();

public:
    Renderer(
        unsigned int windowHeight,
        unsigned int windowWidth,
        Microsoft::WRL::ComPtr<ID3D11Device> device,
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
        Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain,
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> backBufferRTV,
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthBufferDSV
    );
    ~Renderer();

    void PostResize(
        unsigned int windowHeight,
        unsigned int windowWidth,
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> backBufferRTV,
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthBufferDSV
    );
    void PreResize();

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetRenderTargetSRV(RTVTypes type);
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetMiscEffectSRV(MiscEffectSRVTypes type);

    void DrawPointLights();
    void Draw(std::shared_ptr<Camera> camera);

    void SetActiveSky(std::shared_ptr<Sky> sky);

    void InitShadows();
    void RenderShadows(std::shared_ptr<Camera> shadowCam, MiscEffectSRVTypes type);
    void RenderDepths(std::shared_ptr<Camera> sourceCam, MiscEffectSRVTypes type);
    void RenderColliders(std::shared_ptr<Camera> cam);
    void RenderMeshBounds(std::shared_ptr<Camera> cam);

    static bool GetDrawColliderStatus();
    static void SetDrawColliderStatus(bool _newState);
	static bool GetDrawColliderTransformsStatus();
    static void SetDrawColliderTransformsStatus(bool _newState);
};