#pragma once

// Used to show where out-parameter logic occurs
// Removed on compile
#define OUT

#include "Light.h"
#include "Sky.h"
#include "SimpleShader.h"
#include "GameEntity.h"
#include "ParticleSystem.h"
#include "Terrain.h"
#include "WICTextureLoader.h"
#include <assimp/Importer.hpp>
#include <assimp/types.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <map>
#include <random>
#include "DXCore.h"
#include "experimental\filesystem"
#include <locale>
#include <codecvt>
#include "AudioHandler.h"
#include <thread>
#include <mutex>
#include <exception>
#include "SpriteBatch.h"
#include "SpriteFont.h"
#include "CollisionManager.h"

#define RandomRange(min, max) (float)rand() / RAND_MAX * (max - min) + min

struct LoadingNotifications {
	std::string category;
	std::string object;
	std::exception_ptr errorCode;
};

// State machine used to track what type of load
// AssetManager is doing on calling any Create() function
enum AMLoadState {
	// Used when SHOE first loads
	INITIALIZING,
	// Used when something calls a Create() function
	SINGLE_CREATION,
	// In the future, used for complex asset imports
	COMPLEX_CREATION,
	// In the future, used for loading a scene with
	// a loading screen running parallel
	SCENE_LOAD
};

enum ComponentTypes {
	// While Transform is tracked here, it is often skipped or handled uniquely
	// when assessing all components, as it cannot be removed or doubled
	TRANSFORM,
	MESH_RENDERER,
	PARTICLE_SYSTEM,
	COLLIDER,
	TERRAIN,
	LIGHT,
	// Must always be the final enum
	COMPONENT_TYPE_COUNT
};

class AssetManager
{
#pragma region Singleton
public:
	// Gets the one and only instance of this class
	static AssetManager& GetInstance()
	{
		if (!instance)
		{
			instance = new AssetManager();
		}

		return *instance;
	}

	// Remove these functions (C++ 11 version)
	AssetManager(AssetManager const&) = delete;
	void operator=(AssetManager const&) = delete;

private:
	static AssetManager* instance;
	AssetManager() 
	{
		
	};
#pragma endregion

private:
	DXCore* dxInstance;
	AudioHandler& audioInstance = AudioHandler::GetInstance();
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> textureState;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> clampState;

	DirectX::XMFLOAT4 redTint = DirectX::XMFLOAT4(0.2f, 0.0f, 0.0f, 1.0f);
	DirectX::XMFLOAT4 greenTint = DirectX::XMFLOAT4(0.0f, 0.2f, 0.0f, 1.0f);
	DirectX::XMFLOAT4 blueTint = DirectX::XMFLOAT4(0.0f, 0.0f, 0.2f, 1.0f);
	DirectX::XMFLOAT4 whiteTint = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	DirectX::XMFLOAT4 grayTint = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	DirectX::XMFLOAT4 brownTint = DirectX::XMFLOAT4(1.0f, 0.4f, 0.0f, 1.0f);

	// Helper for creating a cubemap from 6 individual textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateCubemap(
		const wchar_t* right,
		const wchar_t* left,
		const wchar_t* up,
		const wchar_t* down,
		const wchar_t* front,
		const wchar_t* back);
	std::shared_ptr<Mesh> LoadTerrain(const char* filename, unsigned int mapWidth, unsigned int mapHeight, float heightScale);

	void CreateComplexGeometry();
	void ProcessComplexModel(aiNode* node, const aiScene* scene);
	std::shared_ptr<Mesh> ProcessComplexMesh(aiMesh* mesh, const aiScene* scene);
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> LoadParticleTexture(std::wstring textureNameToLoad, bool isMultiParticle);

	void InitializeMeshes();
	void InitializeMaterials();
	void InitializeShaders();
	void InitializeGameEntities();
	void InitializeColliders();
	void InitializeTerrainMaterials();
	void InitializeCameras();
	void InitializeLights();
	void InitializeSkies();
	void InitializeEmitters();
	void InitializeAudio();
	void InitializeFonts();
	void InitializeIMGUI(HWND hwnd);

	std::vector<std::shared_ptr<SimplePixelShader>> pixelShaders;
	std::vector<std::shared_ptr<SimpleVertexShader>> vertexShaders;
	std::vector<std::shared_ptr<SimpleComputeShader>> computeShaders;
	std::vector<std::shared_ptr<Sky>> skies;
	std::vector<std::shared_ptr<Camera>> globalCameras;
	std::vector<std::shared_ptr<Mesh>> globalMeshes;
	std::vector<std::shared_ptr<Material>> globalMaterials;
	std::vector<std::shared_ptr<GameEntity>> globalEntities;
	std::vector<std::shared_ptr<TerrainMats>> globalTerrainMaterials;
	std::vector<FMOD::Sound*> globalSounds;
	std::map<std::string, std::shared_ptr<DirectX::SpriteFont>> globalFonts;

	std::condition_variable* threadNotifier;
	std::mutex* threadLock;

	// Most recently loaded object from category
	LoadingNotifications loaded;
	// Helper functions for threads
	void SetLoadedAndWait(std::string category, std::string object, std::exception_ptr error = NULL);
	AMLoadState assetManagerLoadState;
	bool singleLoadComplete;

	ComponentTypes allCurrentComponentTypes;

public:
	static bool materialSortDirty;

	~AssetManager();

	Microsoft::WRL::ComPtr<ID3D11Device> GetDevice();
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> GetContext();

	void Initialize(Microsoft::WRL::ComPtr<ID3D11Device> device, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, std::condition_variable* threadNotifier, std::mutex* threadLock, HWND hwnd);

	std::string GetLastLoadedCategory();
	std::string GetLastLoadedObject();
	std::exception_ptr GetLoadingException();
	AMLoadState GetAMLoadState();
	bool GetSingleLoadComplete();
	void SetAMLoadState(AMLoadState state);
	void SetSingleLoadComplete(bool loadComplete);

	ComponentTypes GetAllCurrentComponentTypes();

	// Methods to create new assets

	std::shared_ptr<GameEntity> CreateGameEntity(std::string name = "GameEntity");
	std::shared_ptr<GameEntity> CreateGameEntity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> mat, std::string name = "GameEntity");
	std::shared_ptr<Sky> CreateSky(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> skyTexture, std::string name);
	std::shared_ptr<SimpleVertexShader> CreateVertexShader(std::string id, std::wstring nameToLoad);
	std::shared_ptr<SimplePixelShader> CreatePixelShader(std::string id, std::wstring nameToLoad);
	std::shared_ptr<SimpleComputeShader> CreateComputeShader(std::string id, std::wstring nameToLoad);
	std::shared_ptr<Mesh> CreateMesh(std::string id, std::string nameToLoad);
	std::shared_ptr<Camera> CreateCamera(std::string id, DirectX::XMFLOAT3 pos, float aspectRatio, int type);
	std::shared_ptr<Light> CreateDirectionalLight(std::string name, DirectX::XMFLOAT3 direction, DirectX::XMFLOAT3 color = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), float intensity = 1.0f);
	std::shared_ptr<Light> CreatePointLight(std::string name, float range, DirectX::XMFLOAT3 color = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), float intensity = 1.0f);
	std::shared_ptr<Light> CreateSpotLight(std::string name, DirectX::XMFLOAT3 direction, float range, DirectX::XMFLOAT3 color = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), float intensity = 1.0f);
	std::shared_ptr<Material> CreatePBRMaterial(std::string id,
											    std::wstring albedoNameToLoad,
											    std::wstring normalNameToLoad,
											    std::wstring metalnessNameToLoad,
											    std::wstring roughnessNameToLoad);
	std::shared_ptr<Terrain> CreateTerrainEntity(std::string name = "Terrain");
	std::shared_ptr<ParticleSystem> CreateParticleEmitter(std::string name,
													std::wstring textureNameToLoad,
													bool isMultiParticle);
	std::shared_ptr<ParticleSystem> CreateParticleEmitter(std::string name,
												   std::wstring textureNameToLoad,
												   int maxParticles,
												   float particleLifeTime,
												   float particlesPerSecond,
												   bool isMultiParticle = false,
												   bool additiveBlendState = true);
	FMOD::Sound* CreateSound(std::string filePath, FMOD_MODE mode);
	std::shared_ptr<DirectX::SpriteFont> CreateSHOEFont(std::string name, std::wstring filePath, bool preInitializing = false);

	// Helper methods to add components to objects

	std::shared_ptr<Collider> AddColliderToGameEntity(OUT std::shared_ptr<GameEntity> entity);
	std::shared_ptr<Collider> AddTriggerBoxToGameEntity(OUT std::shared_ptr<GameEntity> entity);

	// Methods to remove assets

	void RemoveGameEntity(std::string name);
	void RemoveGameEntity(int id);
	void RemoveSky(std::string name);
	void RemoveSky(int id);
	void RemoveVertexShader(std::string name);
	void RemoveVertexShader(int id);
	void RemovePixelShader(std::string name);
	void RemovePixelShader(int id);
	void RemoveMesh(std::string name);
	void RemoveMesh(int id);
	void RemoveCamera(std::string name);
	void RemoveCamera(int id);
	void RemoveMaterial(std::string name);
	void RemoveMaterial(int id);
	void RemoveTerrainMaterial(std::string name);
	void RemoveTerrainMaterial(int id);

	// Methods to disable and enable assets for rendering
	// Currently not implemented except for lights
	
	void EnableDisableSky(std::string name, bool value);
	void EnableDisableSky(int id, bool value);
	void EnableDisableCamera(std::string name, bool value);
	void EnableDisableCamera(int id, bool value);

	// Asset search-by-name methods

	std::shared_ptr<GameEntity> GetGameEntityByName(std::string name);
	std::shared_ptr<Sky> GetSkyByName(std::string name);
	std::shared_ptr<SimpleVertexShader> GetVertexShaderByName(std::string name);
	std::shared_ptr<SimplePixelShader> GetPixelShaderByName(std::string name);
	std::shared_ptr<SimpleComputeShader> GetComputeShaderByName(std::string name);
	std::shared_ptr<Mesh> GetMeshByName(std::string name);
	std::shared_ptr<Camera> GetCameraByName(std::string name);
	std::shared_ptr<Material> GetMaterialByName(std::string name);
	std::shared_ptr<TerrainMats> GetTerrainMaterialByName(std::string name);
	FMOD::Sound* GetSoundByName();
	std::shared_ptr<DirectX::SpriteFont> GetFontByName(std::string name);

	int GetGameEntityIDByName(std::string name);
	int GetSkyIDByName(std::string name);
	int GetVertexShaderIDByName(std::string name);
	int GetPixelShaderIDByName(std::string name);
	int GetComputeShaderIDByName(std::string name);
	int GetMeshIDByName(std::string name);
	int GetCameraIDByName(std::string name);
	int GetMaterialIDByName(std::string name);
	//int GetTerrainMaterialIDByName(std::string name);

	// Relevant Get methods
	
	Microsoft::WRL::ComPtr<ID3D11InputLayout> GetInputLayout();
	size_t GetPixelShaderArraySize();
	size_t GetVertexShaderArraySize();
	size_t GetComputeShaderArraySize();
	size_t GetSkyArraySize();
	size_t GetCameraArraySize();
	size_t GetMeshArraySize();
	size_t GetMaterialArraySize();
	size_t GetGameEntityArraySize();
	size_t GetTerrainMaterialArraySize();
	size_t GetSoundArraySize();
	std::vector<std::shared_ptr<GameEntity>>* GetActiveGameEntities();
	std::vector<std::shared_ptr<Sky>>* GetSkyArray();
	Light* GetFlashlight();

	FMOD::Sound* GetSoundAtID(int id);
	std::shared_ptr<Camera> GetCameraAtID(int id);
	std::shared_ptr<Material> GetMaterialAtID(int id);
	std::shared_ptr<Mesh> GetMeshAtID(int id);
	std::shared_ptr<SimpleVertexShader> GetVertexShaderAtID(int id);
	std::shared_ptr<SimplePixelShader> GetPixelShaderAtID(int id);
	std::shared_ptr<SimpleComputeShader> GetComputeShaderAtID(int id);
	std::shared_ptr<GameEntity> GetGameEntityByID(int id);
	std::shared_ptr<Sky> GetSkyAtID(int id);

	inline std::wstring ConvertToWide(const std::string& as);

	std::shared_ptr<Sky> currentSky;
};