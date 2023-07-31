#define GATEWARE_ENABLE_CORE
#define GATEWARE_ENABLE_SYSTEM
#define GATEWARE_ENABLE_GRAPHICS
#define GATEWARE_DISABLE_GDIRECTX12SURFACE
#define GATEWARE_DISABLE_GRASTERSURFACE
#define GATEWARE_DISABLE_GOPENGLSURFACE
#define GATEWARE_DISABLE_GVULKANSURFACE
#define GATEWARE_ENABLE_MATH 
#define GATEWARE_ENABLE_INPUT

#include <d3dcompiler.h>
#include <chrono>
#include "load_object_oriented.h"
#pragma comment(lib, "d3dcompiler.lib") //needed for runtime shader compilation. Consider compiling shaders before runtime 

class Renderer
{
	// proxy handles
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GDirectX11Surface d3d;
	GW::SYSTEM::GLog log;
	std::shared_ptr<flecs::world> world;

	GW::INPUT::GInput input;
	Level_Objects modelsToDraw;

	std::chrono::steady_clock clock;
	std::chrono::steady_clock::time_point start;
	GW::MATH::GMatrix mProxy;

	Model model;

	const float camSpeed = 20.0f;

	float aspectRatio = 0;
	float FOV = G_DEGREE_TO_RADIAN_F(65);

	// timer
	std::chrono::high_resolution_clock::time_point lastTime;
	float deltaTime;

	std::chrono::high_resolution_clock::time_point progTime; //Its going to be used to start a timer. 
	float startTime = 3;

	Microsoft::WRL::ComPtr<ID3D11Buffer> sceneBuffer;

public:

	struct vertex
	{
		float x, y, z, w;
	};

	struct sceneData
	{
		GW::MATH::GVECTORF lightDirection, lightColor, lightAmbient, cameraPos;
		GW::MATH::GMATRIXF viewMatrix, ProjectionMatrix;
	};

	sceneData SD;

	GW::MATH::GMATRIXF camera = GW::MATH::GIdentityMatrixF;
	GW::MATH::GMATRIXF miniMap = GW::MATH::GIdentityMatrixF;

	// level select
	bool isLevelSwaped = false;
	bool whichLevel = true;
	float _NumPad1 = 0.0f;
	float _NumPad2 = 0.0f;
	float _NumPad3 = 0.0f;

	Renderer()
	{

	}


	Renderer(int level, GW::SYSTEM::GWindow _win, GW::GRAPHICS::GDirectX11Surface _d3d, std::shared_ptr<flecs::world> gameWorld, GW::SYSTEM::GLog _log, Level_Objects objs, GW::INPUT::GInput _input)
	{
		win = _win;
		d3d = _d3d;
		world = gameWorld;
		log = _log;
		modelsToDraw = objs;
		input = _input;

		ID3D11Device* creator;
		d3d.GetDevice((void**)&creator);

		log.Create("logfile.txt");
		log.EnableConsoleLogging(true);


		if (level == 1)
			modelsToDraw.LoadLevel(level1txt, level1M, log.Relinquish(), world);
		if (level == 2)
			modelsToDraw.LoadLevel(level2txt, level2M, log.Relinquish(), world);
		if (level == 3)
			modelsToDraw.LoadLevel(level3txt, level3M, log.Relinquish(), world);

		modelsToDraw.UploadLevelToGPU(creator);

		// AJ's code to create a stantionary view matrix
		ViewMatrixBuilder();

		// Aj's Projection matrix
		ProjectionMatrixBuilder();

		// AJ's code to create light
		LightVecBuilder();

		// Aj's scene buffer
		CreateSceneBuffer(creator);

		creator->Release();

		progTime = std::chrono::high_resolution_clock::now();
		lastTime = progTime;
	}

public:


	Renderer& operator=(const Renderer& other)
	{
		if (this != &other)
		{
			win = other.win;
			d3d = other.d3d;
			log = other.log;
			world = other.world;
			modelsToDraw = other.modelsToDraw;
			clock = other.clock;
			start = other.start;
			mProxy = other.mProxy;
			model = other.model;

			aspectRatio = other.aspectRatio;
			lastTime = other.lastTime;
			deltaTime = other.deltaTime;
			progTime = other.progTime;
			sceneBuffer = other.sceneBuffer;

			SD = other.SD;
		}
		return *this;
	}

	D3D11_VIEWPORT screen{ 0, 0, 800, 600, 0, 1 };
	D3D11_VIEWPORT minimap{ 646, 20, 133, 100, 0, 1 };

	void RenderMain()
	{
		Model::PipelineHandles curHandles = model.GetCurrentPipelineHandles(d3d);

		SetRenderTargets(curHandles);

		D3D11_MAPPED_SUBRESOURCE sceneMap = { 0 };
		curHandles.context->Map(sceneBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &sceneMap);
		memcpy(sceneMap.pData, &SD, sizeof(SD));
		curHandles.context->Unmap(sceneBuffer.Get(), 0);

		curHandles.context->VSSetConstantBuffers(0, 1, sceneBuffer.GetAddressOf());
		curHandles.context->PSSetConstantBuffers(0, 1, sceneBuffer.GetAddressOf());

		modelsToDraw.RenderLevel(d3d, win, screen);

		model.ReleasePipelineHandles(curHandles);
		start = clock.now();
	}

	void SetRenderTargets(Model::PipelineHandles handles)
	{

		ID3D11RenderTargetView* const views[] = { handles.targetView };
		handles.context->OMSetRenderTargets(ARRAYSIZE(views), views, handles.depthStencil);
	}

	//Camera Trolley
	void ScrollingCam(float _cameraSpeed, bool* lostALife)
	{

		std::chrono::high_resolution_clock::time_point timerFunc = std::chrono::high_resolution_clock::now();
		float elapsedSecsApp = std::chrono::duration_cast<std::chrono::microseconds> (timerFunc - progTime).count() / 1000000.0f;;


		if (elapsedSecsApp > startTime)
		{
			std::chrono::high_resolution_clock::time_point _now = std::chrono::high_resolution_clock::now();

			deltaTime = std::chrono::duration_cast<std::chrono::microseconds> (timerFunc - lastTime).count() / 1000000.0f;
			lastTime = timerFunc;

			GW::MATH::GMATRIXF _view;
			GW::MATH::GMatrix::InverseF(SD.viewMatrix, _view);

			float FPS = _cameraSpeed * deltaTime;


			float changeX = FPS * .5f;

			GW::MATH::GVECTORF translationVec = { changeX , 0, 0 };

			GW::MATH::GMatrix::TranslateLocalF(_view, translationVec, _view);

			ID3D11DeviceContext* con;
			d3d.GetImmediateContext((void**)&con);

			D3D11_MAPPED_SUBRESOURCE subRes;
			con->Map(sceneBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subRes);
			SD.cameraPos = _view.row4;
			memcpy(subRes.pData, &SD, sizeof(SD));
			con->Unmap(sceneBuffer.Get(), 0);

			GW::MATH::GMatrix::InverseF(_view, SD.viewMatrix);
		}
		if (*lostALife)
		{
			ViewMatrixBuilder();
			*lostALife = false;
		}

	}

	// camera controls
	void DebugCamera(GW::INPUT::GInput inputProxy, GW::INPUT::GController controllerProxy) {

		std::chrono::high_resolution_clock::time_point _now = std::chrono::high_resolution_clock::now();
		deltaTime = std::chrono::duration_cast<std::chrono::microseconds> (_now - lastTime).count() / 1000000.0f;
		lastTime = _now;

		GW::MATH::GMATRIXF _view;
		GW::MATH::GMatrix::InverseF(SD.viewMatrix, _view);

		// camera speed
		float cameraSpeed = 3.0f;

		float FPS = cameraSpeed * deltaTime;

		// keyboard controls

		// space
		float spaceBar = 0.0f;
		inputProxy.GetState(G_KEY_SPACE, spaceBar);

		// left shift
		float leftShiftBtn = 0.0f;
		inputProxy.GetState(G_KEY_LEFTSHIFT, leftShiftBtn);

		// WASD Key

		// w btn
		float _WKey = 0.0f;
		inputProxy.GetState(G_KEY_W, _WKey);

		// a btn
		float _aKey = 0.0f;
		inputProxy.GetState(G_KEY_A, _aKey);

		// s btn
		float _sKey = 0.0f;
		inputProxy.GetState(G_KEY_S, _sKey);

		// d btn
		float _dKey = 0.0f;
		inputProxy.GetState(G_KEY_D, _dKey);

		// rotation controls
		float mouseX = 0.0f;
		float mouseY = 0.0f;
		inputProxy.GetMousePosition(mouseX, mouseY);

		float changeMouseX = 0.0f;
		float changeMouseY = 0.0f;
		GW::GReturn result = inputProxy.GetMouseDelta(changeMouseX, changeMouseY);

		float changeX = _dKey - _aKey;
		float changeY = spaceBar - leftShiftBtn;
		float changeZ = _WKey - _sKey;

		// window height
		unsigned int height = 0;
		win.GetHeight(height);

		// window width
		unsigned int width = 0;
		win.GetWidth(width);

		// pitch 
		float pitch = (65.0f * changeMouseY / (float)height) + (3.14159f * deltaTime); // inverse controls mul by -1

		// aspect ratio
		float aspectRatio = 0.0f;
		d3d.GetAspectRatio(aspectRatio);

		// yaw
		float yaw = (65.0f * changeMouseX / (float)width) + (3.14159f * deltaTime); // no inverse controls


		GW::MATH::GVECTORF translationVec = { changeX * FPS, changeY * FPS, changeZ * FPS };

		GW::MATH::GMatrix::TranslateLocalF(_view, translationVec, _view);

		if (G_PASS(result) && result != GW::GReturn::REDUNDANT)
		{
			// local rotation on x
			GW::MATH::GMatrix::RotateXLocalF(_view, G_DEGREE_TO_RADIAN(pitch), _view);

			// store the position of the camera
			GW::MATH::GVECTORF pos = _view.row4;

			// global on y
			GW::MATH::GMatrix::RotateYGlobalF(_view, G_DEGREE_TO_RADIAN(yaw), _view);

			// restore the position to the camera
			_view.row4 = pos;
		}

		// update the camera's position in the _sceneData variable
		ID3D11DeviceContext* con;
		d3d.GetImmediateContext((void**)&con);

		D3D11_MAPPED_SUBRESOURCE subRes;
		con->Map(sceneBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subRes);
		SD.cameraPos = _view.row4;
		memcpy(subRes.pData, &SD, sizeof(SD));
		con->Unmap(sceneBuffer.Get(), 0);

		GW::MATH::GMatrix::InverseF(_view, SD.viewMatrix);
	}

	void CreateSceneBuffer(ID3D11Device* creator) {
		D3D11_BUFFER_DESC bufferScene;
		bufferScene.Usage = D3D11_USAGE_DYNAMIC;
		bufferScene.ByteWidth = sizeof(SD);
		bufferScene.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferScene.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferScene.MiscFlags = 0;
		creator->CreateBuffer(&bufferScene, nullptr, sceneBuffer.GetAddressOf());
	}

	void ProjectionMatrixBuilder() {
		// field of view
		float FOV = G_DEGREE_TO_RADIAN(65.0f);
		// near plane
		float nPlane = 0.1f;
		// far plane
		float fPlane = 100.0f;
		// aspect ratio
		float aspectRatio = 0.0f;
		d3d.GetAspectRatio(aspectRatio);
		GW::MATH::GMatrix::ProjectionDirectXLHF(FOV, aspectRatio, nPlane, fPlane, SD.ProjectionMatrix);
	}

	// helper functions for view
	void ViewMatrixBuilder() {
		// camera vector
		GW::MATH::GVECTORF camera;
		camera.x = 5.0f;
		camera.y = 2.5f;
		camera.z = -20.0f;
		camera.w = 0.0f;
		SD.cameraPos = camera;
		// target position of the camera
		GW::MATH::GVECTORF targetPos;
		targetPos.x = 0.0f;
		targetPos.y = 2.5f;
		targetPos.z = -20.0f;
		targetPos.w = 0.0f;
		// Orientation of vector
		GW::MATH::GVECTORF orientation;
		orientation.x = 0.0f;
		orientation.y = 1.0f;
		orientation.z = 0.0f;
		orientation.w = 0.0f;
		GW::MATH::GMatrix::LookAtLHF(camera, targetPos, orientation, SD.viewMatrix);
		//_sceneData.vMatrix = view;
	}

	void LightVecBuilder() {
		SD.lightColor = { 0.9f,0.9f,1.0f,1.0f };
		SD.lightDirection = { -1.0f, -1.0f, 2.0f, 0.0f };
		GW::MATH::GVector::NormalizeF(SD.lightDirection, SD.lightDirection);
		SetAmbientLight();
	}

	//  setting sun ambient
	void SetAmbientLight() {
		SD.lightAmbient = { 0.25f, 0.25f, 0.35f };
	}

	int SelectLevel(GW::INPUT::GInput immediateInput) {
		GW::SYSTEM::GLog _glog;
		immediateInput.GetState(G_KEY_1, _NumPad1);
		immediateInput.GetState(G_KEY_2, _NumPad2);
		immediateInput.GetState(G_KEY_3, _NumPad3);
		_glog.Create("errorLog.txt");
		_glog.EnableConsoleLogging(true); // shows all loaded items

		if (_NumPad1 > 0.0f)
		{
			return 1;
		}
		if (_NumPad2 > 0.0f)
		{
			return 2;
		}
		if (_NumPad3 > 0.0f)
		{
			return 3;
		}
		return 0;
	}

	void UnloadEntities()
	{
		world.reset();
		world = std::make_shared<flecs::world>();
	}

	~Renderer()
	{
		// ComPtr will auto release so nothing to do here yet
	}
};