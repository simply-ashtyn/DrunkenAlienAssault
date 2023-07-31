// This is a sample of how to load a level in a object oriented fashion.
// Feel free to use this code as a base and tweak it for your needs.
// This reads .h2b files which are optimized binary .obj+.mtl files

#include "h2bParser.h"

class Model {
	// debugging tool
	HRESULT r;
	
	// Name of the Model in the GameLevel (useful for debugging)
	std::string name;
	// Loads and stores CPU model data from .h2b file
	H2B::Parser cpuModel; // reads the .h2b format
	// Shader variables needed by this model. 
	GW::MATH::GMATRIXF world;// TODO: Add matrix/light/etc vars..
	// TODO: API Rendering vars here (unique to this model)
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	vertexFormat;
	// Vertex Buffer
	Microsoft::WRL::ComPtr<ID3D11Buffer>		vertexBuffer;
	// Index Buffer
	Microsoft::WRL::ComPtr<ID3D11Buffer>		indexBuffer;
	// Pipeline/State Objects
	
	// Uniform/ShaderVariable Buffer
	Microsoft::WRL::ComPtr<ID3D11Buffer> meshBuffer;

	// Vertex/Pixel Shaders
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	pixelShader;
	std::chrono::steady_clock clock;
	std::chrono::steady_clock::time_point start;
	GW::MATH::GMatrix mProxy;
public:
	//GW::MATH::GVECTORF collisionBox[8];
// scene buffer
//Microsoft::WRL::ComPtr<ID3D11Buffer> sceneBuffer;

void PrintLabeledDebugString(const char* label, const char* toPrint)
{
	std::cout << label << toPrint << std::endl;
#if defined WIN32 //OutputDebugStringA is a windows-only function 
	OutputDebugStringA(label);
	OutputDebugStringA(toPrint);
#endif
}

// Reads a file into an std::string 
std::string ReadFileIntoString(const char* filePath)
{
	std::string output;
	unsigned int stringLength = 0;
	GW::SYSTEM::GFile file;

	file.Create();
	file.GetFileSize(filePath, stringLength);

	if (stringLength > 0 && +file.OpenBinaryRead(filePath))
	{
		output.resize(stringLength);
		file.Read(&output[0], stringLength);
	}
	else
		std::cout << "ERROR: File \"" << filePath << "\" Not Found!" << std::endl;

	return output;
}

struct OBJ_ATTRIBUTES
{
	GW::MATH::GVECTORF Kd; float d;
	GW::MATH::GVECTORF Ks; float Ns;
	GW::MATH::GVECTORF Ka; float sharpness;
	GW::MATH::GVECTORF Tf; float Ni;
	GW::MATH::GVECTORF Ke; unsigned int illum;
};

struct PipelineHandles
{
	ID3D11DeviceContext* context;
	ID3D11RenderTargetView* targetView;
	ID3D11DepthStencilView* depthStencil;
};

struct meshData
{
	GW::MATH::GMATRIXF worldMatrix;
	H2B::ATTRIBUTES material;
};

	meshData MD;
ID3D11RasterizerState* rasterizerState;


	inline void SetName(std::string modelName) {
		name = modelName;
	}
	inline std::string GetName() {
		return name;
	}
	inline GW::MATH::GMATRIXF GetTransform() {
		return world;
	}
	inline void SetWorldMatrix(GW::MATH::GMATRIXF worldMatrix) {
		world = worldMatrix;
		MD.worldMatrix = world;
	}
	bool LoadModelDataFromDisk(const char* h2bPath) {
		// if this succeeds "cpuModel" should now contain all the model's info
		return cpuModel.Parse(h2bPath);
	}
	bool UploadModelData2GPU(ID3D11Device* gpuDevice) {
		// TODO: Use chosen API to upload this model's graphics data to GPU

		UINT compilerFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if _DEBUG
		compilerFlags |= D3DCOMPILE_DEBUG;
#endif
		Microsoft::WRL::ComPtr<ID3DBlob> vsBlob = CompileVertexShader(gpuDevice, compilerFlags);
		Microsoft::WRL::ComPtr<ID3DBlob> psBlob = CompilePixelShader(gpuDevice, compilerFlags);

		CreateVertexInputLayout(gpuDevice, vsBlob);

		/*D3D11_RASTERIZER_DESC rastDesc;
		ZeroMemory(&rastDesc, sizeof(D3D11_RASTERIZER_DESC));
		rastDesc.FillMode = D3D11_FILL_WIREFRAME;
		rastDesc.CullMode = D3D11_CULL_NONE;
		rastDesc.DepthClipEnable = true;
		gpuDevice->CreateRasterizerState(&rastDesc, &rasterizerState);*/

		/*D3D11_SUBRESOURCE_DATA bData = { cpuModel.vertices.data(), 0, 0 };
		CD3D11_BUFFER_DESC bDesc(sizeof(H2B::VERTEX) * cpuModel.vertexCount, D3D11_BIND_VERTEX_BUFFER);
		gpuDevice->CreateBuffer(&bDesc, &bData, vertexBuffer.GetAddressOf());*/

		CreateVertexBuffer(gpuDevice, cpuModel.vertices.data(), sizeof(H2B::VERTEX) * cpuModel.vertexCount);

		//D3D11_SUBRESOURCE_DATA iData = { cpuModel.indices.data(), 0, 0 };
		//CD3D11_BUFFER_DESC iDesc(sizeof(unsigned) * cpuModel.indexCount, D3D11_BIND_INDEX_BUFFER);
		//gpuDevice->CreateBuffer(&iDesc, &iData, indexBuffer.GetAddressOf());

		CreateIndexBuffer(gpuDevice, cpuModel.indices.data(), sizeof(unsigned int) * cpuModel.indexCount);


		//constant buffer here
		/*	D3D11_SUBRESOURCE_DATA bData2 = { &MD, 0, 0 };
		CD3D11_BUFFER_DESC bDesc2(sizeof(MD), D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE, 0);
		gpuDevice->CreateBuffer(&bDesc2, &bData2, constantBuffer2.GetAddressOf());*/

		CreateMeshBuffer(gpuDevice);


		return true;
	}

	bool DrawModel(GW::GRAPHICS::GDirectX11Surface d3dSurface, GW::SYSTEM::GWindow win, D3D11_VIEWPORT view, GW::MATH::GMATRIXF worldENT) {
		// TODO: Use chosen API to setup the pipeline for this model and draw it

		PipelineHandles curHandles = GetCurrentPipelineHandles(d3dSurface);
		SetUpPipeline(curHandles);
		if (worldENT.row1.x != -107374176)
			MD.worldMatrix = worldENT;
		else
			MD.worldMatrix = world;
		curHandles.context->RSSetViewports(1, &view);

		D3D11_MAPPED_SUBRESOURCE subRes1;
		for (int i = 0; i < cpuModel.meshCount; i++)
		{
			//curHandles.context->Map(constantBuffer2.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subRes1);
			//memcpy(subRes1.pData, &MD, sizeof(MD));
			//curHandles.context->Unmap(constantBuffer2.Get(), 0);

			//MD.material = cpuModel.materials[i].attrib;
			MD.material = cpuModel.materials[cpuModel.meshes[i].materialIndex].attrib;

			curHandles.context->UpdateSubresource(meshBuffer.Get(), 0, nullptr, &MD, 0, 0);

			curHandles.context->DrawIndexed(cpuModel.meshes[i].drawInfo.indexCount, cpuModel.meshes[i].drawInfo.indexOffset, 0);
		}

		return false;
	}

	// vertex buffer
	void CreateVertexBuffer(ID3D11Device* creator, const void* data, unsigned int sizeInBytes)
	{
		D3D11_BUFFER_DESC bufferVert;
		bufferVert.Usage = D3D11_USAGE_IMMUTABLE;
		bufferVert.ByteWidth = sizeInBytes;
		bufferVert.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferVert.CPUAccessFlags = 0;
		bufferVert.MiscFlags = 0;
		bufferVert.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA bData = { data, 0, 0 };

		creator->CreateBuffer(&bufferVert, &bData, vertexBuffer.GetAddressOf());

		//CD3D11_BUFFER_DESC bDesc(sizeInBytes, D3D11_BIND_VERTEX_BUFFER);
		//creator->CreateBuffer(&bDesc, &bData, vertexBuffer.GetAddressOf());
	}
	// index buffer
	void CreateIndexBuffer(ID3D11Device* creator, const void* data, unsigned int sizeInBytes)
	{
		D3D11_BUFFER_DESC bufferIndex;
		bufferIndex.Usage = D3D11_USAGE_IMMUTABLE;
		bufferIndex.ByteWidth = sizeInBytes;
		bufferIndex.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferIndex.CPUAccessFlags = 0;
		bufferIndex.MiscFlags = 0;
		bufferIndex.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA bData = { data, 0, 0 };

		r = creator->CreateBuffer(&bufferIndex, &bData, indexBuffer.GetAddressOf());

		//CD3D11_BUFFER_DESC bDesc(sizeInBytes, D3D11_BIND_INDEX_BUFFER);
		//creator->CreateBuffer(&bDesc, &bData, microsoftIndexBuffer.GetAddressOf());
	}
	// mesh buffer
	void CreateMeshBuffer(ID3D11Device* creator) {

		D3D11_BUFFER_DESC bufferMesh = { 0 };
		bufferMesh.Usage = D3D11_USAGE_DEFAULT;
		bufferMesh.ByteWidth = sizeof(MD);
		bufferMesh.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferMesh.CPUAccessFlags = 0;
		bufferMesh.MiscFlags = 0;
		bufferMesh.StructureByteStride = 0;
		creator->CreateBuffer(&bufferMesh, nullptr, meshBuffer.GetAddressOf());

	}

	//Render helper functions
	PipelineHandles GetCurrentPipelineHandles(GW::GRAPHICS::GDirectX11Surface d3dSurface)
	{
		PipelineHandles retval;
		d3dSurface.GetImmediateContext((void**)&retval.context);
		d3dSurface.GetRenderTargetView((void**)&retval.targetView);
		d3dSurface.GetDepthStencilView((void**)&retval.depthStencil);
		return retval;
	}

	void ReleasePipelineHandles(PipelineHandles toRelease)
	{
		toRelease.depthStencil->Release();
		toRelease.targetView->Release();
		toRelease.context->Release();
	}
private:

	Microsoft::WRL::ComPtr<ID3DBlob> CompileVertexShader(ID3D11Device* creator, UINT compilerFlags)
	{
		std::string vertexShaderSource = ReadFileIntoString("../Shaders/VertexShader.hlsl");

		Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, errors;

		HRESULT compilationResult =
			D3DCompile(vertexShaderSource.c_str(), vertexShaderSource.length(),
				nullptr, nullptr, nullptr, "main", "vs_4_0", compilerFlags, 0,
				vsBlob.GetAddressOf(), errors.GetAddressOf());

		if (SUCCEEDED(compilationResult))
		{
			creator->CreateVertexShader(vsBlob->GetBufferPointer(),
				vsBlob->GetBufferSize(), nullptr, vertexShader.GetAddressOf());
		}
		else
		{
			PrintLabeledDebugString("Vertex Shader Errors:\n", (char*)errors->GetBufferPointer());
			abort();
			return nullptr;
		}

		return vsBlob;
	}

	Microsoft::WRL::ComPtr<ID3DBlob> CompilePixelShader(ID3D11Device* creator, UINT compilerFlags)
	{
		std::string pixelShaderSource = ReadFileIntoString("../Shaders/PixelShader.hlsl");

		Microsoft::WRL::ComPtr<ID3DBlob> psBlob, errors;

		HRESULT compilationResult =
			D3DCompile(pixelShaderSource.c_str(), pixelShaderSource.length(),
				nullptr, nullptr, nullptr, "main", "ps_4_0", compilerFlags, 0,
				psBlob.GetAddressOf(), errors.GetAddressOf());

		if (SUCCEEDED(compilationResult))
		{
			creator->CreatePixelShader(psBlob->GetBufferPointer(),
				psBlob->GetBufferSize(), nullptr, pixelShader.GetAddressOf());
		}
		else
		{
			PrintLabeledDebugString("Pixel Shader Errors:\n", (char*)errors->GetBufferPointer());
			abort();
			return nullptr;
		}

		return psBlob;

	}

	void CreateVertexInputLayout(ID3D11Device* creator, Microsoft::WRL::ComPtr<ID3DBlob>& vsBlob)
	{
		// TODO: Part 1C DONE
		D3D11_INPUT_ELEMENT_DESC attributes[3];

		attributes[0].SemanticName = "POS";
		attributes[0].SemanticIndex = 0;
		attributes[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		attributes[0].InputSlot = 0;
		attributes[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		attributes[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		attributes[0].InstanceDataStepRate = 0;

		attributes[1].SemanticName = "UVW";
		attributes[1].SemanticIndex = 0;
		attributes[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		attributes[1].InputSlot = 0;
		attributes[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		attributes[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		attributes[1].InstanceDataStepRate = 0;

		attributes[2].SemanticName = "NRM";
		attributes[2].SemanticIndex = 0;
		attributes[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		attributes[2].InputSlot = 0;
		attributes[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		attributes[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		attributes[2].InstanceDataStepRate = 0;

		creator->CreateInputLayout(attributes, ARRAYSIZE(attributes),
			vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
			vertexFormat.GetAddressOf());
	}


	void SetUpPipeline(PipelineHandles handles)
	{
		//SetRenderTargets(handles);

		SetVertexAndIndexBuffers(handles);

		SetShaders(handles);

		handles.context->IASetInputLayout(vertexFormat.Get());
		handles.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//handles.context->VSSetConstantBuffers(0, 1, sceneBuffer.GetAddressOf());
		//handles.context->PSSetConstantBuffers(0, 1, sceneBuffer.GetAddressOf());
		handles.context->VSSetConstantBuffers(1, 1, meshBuffer.GetAddressOf());
		handles.context->PSSetConstantBuffers(1, 1, meshBuffer.GetAddressOf());

		//handles.context->RSSetState(rasterizerState);
	}

	void SetRenderTargets(PipelineHandles handles)
	{
		ID3D11RenderTargetView* const views[] = { handles.targetView };
		handles.context->OMSetRenderTargets(ARRAYSIZE(views), views, handles.depthStencil);
	}

	//void SetVertexBuffers(PipelineHandles handles)
	//{
	//	// TODO: Part 1C DONE
	//	const UINT strides[] = { sizeof(H2B::VERTEX) };
	//	const UINT offsets[] = { 0 };
	//	ID3D11Buffer* const buffs[] = { vertexBuffer.Get() };
	//	handles.context->IASetVertexBuffers(0, ARRAYSIZE(buffs), buffs, strides, offsets);
	//}
	//void setIndexBuffer(PipelineHandles handles)
	//{
	//	handles.context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	//}

	void SetShaders(PipelineHandles handles)
	{
		handles.context->VSSetShader(vertexShader.Get(), nullptr, 0);
		handles.context->PSSetShader(pixelShader.Get(), nullptr, 0);
	}



	void SetVertexAndIndexBuffers(PipelineHandles handles)
	{
		const UINT strides[] = { sizeof(H2B::VERTEX) };
		const UINT offsets[] = { 0 };
		ID3D11Buffer* const buffs[] = { vertexBuffer.Get() };
		handles.context->IASetVertexBuffers(0, ARRAYSIZE(buffs), buffs, strides, offsets);
		handles.context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	}

};


class Level_Objects {

public:

	// store all our models
	std::list<Model> allObjectsInLevel;
	// TODO: This could be a good spot for any global data like cameras or lights
	std::vector<GW::MATH::GVECTORF> lightsInLevel;
	GW::MATH::GMatrix mProxy;
	GW::MATH::GMATRIXF proj = GW::MATH::GIdentityMatrixF;
	float ratio = 1.33333f;

	std::vector<GW::MATH::GMATRIXF> camerasInLevel;
	GW::MATH::GMATRIXF projectionMatrix;
	std::shared_ptr<flecs::world> _game;
	//GW::MATH2D::GVECTOR3F boundry[8];

	//GW::MATH::GOBBF ComputeOBB() const
	//{
	//	GW::MATH::GOBBF out = {
	//		GW::MATH::GIdentityVectorF,
	//		GW::MATH::GIdentityVectorF,
	//		GW::MATH::GIdentityQuaternionF // initally unrotated (local space)
	//	};
	//	out.center.x = (boundry[0].x + boundry[4].x) * 0.5f;
	//	out.center.y = (boundry[0].y + boundry[1].y) * 0.5f;
	//	out.center.z = (boundry[0].z + boundry[2].z) * 0.5f;
	//	out.extent.x = std::fabsf(boundry[0].x - boundry[4].x) * 0.5f;
	//	out.extent.y = std::fabsf(boundry[0].y - boundry[1].y) * 0.5f;
	//	out.extent.z = std::fabsf(boundry[0].z - boundry[2].z) * 0.5f;
	//	return out;
	//}

	// Imports the default level txt format and creates a Model from each .h2b
	bool LoadLevel(const char* gameLevelPath,
		const char* h2bFolderPath,
		GW::SYSTEM::GLog log, std::shared_ptr<flecs::world> _gameWorld) {

		// What this does:
		// Parse GameLevel.txt 
		// For each model found in the file...
			// Create a new Model class on the stack.
				// Read matrix transform and add to this model.
				// Load all CPU rendering data for this model from .h2b
			// Move the newly found Model to our list of total models for the level 

		_game = _gameWorld;

		log.LogCategorized("EVENT", "LOADING GAME LEVEL [OBJECT ORIENTED]");
		log.LogCategorized("MESSAGE", "Begin Reading Game Level Text File.");

		UnloadLevel();// clear previous level data if there is any
		GW::SYSTEM::GFile file;
		file.Create();
		if (-file.OpenTextRead(gameLevelPath)) {
			log.LogCategorized(
				"ERROR", (std::string("Game level not found: ") + gameLevelPath).c_str());
			return false;
		}
		char linebuffer[1024];
		while (+file.ReadLine(linebuffer, 1024, '\n'))
		{
			// having to have this is a bug, need to have Read/ReadLine return failure at EOF
			if (linebuffer[0] == '\0')
				break;
			if (std::strcmp(linebuffer, "MESH") == 0)
			{
				Model newModel;
				file.ReadLine(linebuffer, 1024, '\n');
				log.LogCategorized("INFO", (std::string("Model Detected: ") + linebuffer).c_str());
				// create the model file name from this (strip the .001)
				newModel.SetName(linebuffer);
				std::string modelFile = linebuffer;
				modelFile = modelFile.substr(0, modelFile.find_last_of("."));
				modelFile += ".h2b";

				// now read the transform data as we will need that regardless
				GW::MATH::GMATRIXF transform;
				for (int i = 0; i < 4; ++i) {
					file.ReadLine(linebuffer, 1024, '\n');
					// read floats
					std::sscanf(linebuffer + 13, "%f, %f, %f, %f",
						&transform.data[0 + i * 4], &transform.data[1 + i * 4],
						&transform.data[2 + i * 4], &transform.data[3 + i * 4]);
				}
				//// read the collision box 
				//file.ReadLine(linebuffer, 1024, '\n');
				//if (std::strcmp(linebuffer, "Collision Box") == 0)
				//{
				//	for (int i = 0; i < 8; i++)
				//	{
				//		file.ReadLine(linebuffer, 1024, '\n');
				//		GW::MATH::GVECTORF vector;
				//		std::sscanf(linebuffer + 8, "(%f, %f, %f)", &vector.x, &vector.y, &vector.z);
				//		newModel.collisionBox[i] = vector;
				//	}
				//}

				std::string loc = "Location: X ";
				loc += std::to_string(transform.row4.x) + " Y " +
					std::to_string(transform.row4.y) + " Z " + std::to_string(transform.row4.z);
				log.LogCategorized("INFO", loc.c_str());

				// Add new model to list of all Models
				log.LogCategorized("MESSAGE", "Begin Importing .H2B File Data.");
				modelFile = std::string(h2bFolderPath) + "/" + modelFile;
				newModel.SetWorldMatrix(transform);

				///add models to entity struct
				_game->entity(newModel.GetName().c_str())
					.set<ModelName>({ newModel.GetName() })
					.set<Transform>({ newModel.GetTransform() })
					.add<Collidable>();

				///*debug* print objects added to flecs
				auto f = _gameWorld->filter<ModelName, Transform>();
				f.each([&log](ModelName n, Transform t) {
					std::string obj = "FLECS Entity ";
					obj += n.name + " located at X " + std::to_string(t.transform.row4.x) +
						" Y " + std::to_string(t.transform.row4.y) + " Z " + std::to_string(t.transform.row4.z);
					log.LogCategorized("GAMEPLAY", obj.c_str());
					});

				// If we find and load it add it to the level
				if (newModel.LoadModelDataFromDisk(modelFile.c_str())) {
					// add to our level objects, we use std::move since Model::cpuModel is not copy safe.
					allObjectsInLevel.push_back(std::move(newModel));
					log.LogCategorized("INFO", (std::string("H2B Imported: ") + modelFile).c_str());
				}
				else {
					// notify user that a model file is missing but continue loading
					log.LogCategorized("ERROR",
						(std::string("H2B Not Found: ") + modelFile).c_str());
					log.LogCategorized("WARNING", "Loading will continue but model(s) are missing.");
				}
				log.LogCategorized("MESSAGE", "Importing of .H2B File Data Complete.");
			}
			if (std::strcmp(linebuffer, "CAMERA") == 0)
			{
				file.ReadLine(linebuffer, 1024, '\n');
				log.LogCategorized("INFO", (std::string("Camera Detected: ") + linebuffer).c_str());

				//get the information to pass it to our scene data
				GW::MATH::GMATRIXF transform;
				for (int i = 0; i < 4; ++i) {
					file.ReadLine(linebuffer, 1024, '\n');
					// read floats
					std::sscanf(linebuffer + 13, "%f, %f, %f, %f",
						&transform.data[0 + i * 4], &transform.data[1 + i * 4],
						&transform.data[2 + i * 4], &transform.data[3 + i * 4]);
				}

				//Pass Location to the Debug Log
				std::string loc = "Location: X ";
				loc += std::to_string(transform.row4.x) + " Y " +
					std::to_string(transform.row4.y) + " Z " + std::to_string(transform.row4.z);
				log.LogCategorized("INFO", loc.c_str());

				//Pass Location to camerasInLevel array
				log.LogCategorized("MESSAGE", "Begin Importing Camera Location Data.");
				GW::I::GMatrixInterface::InverseF(transform, transform);

				camerasInLevel.push_back(transform);

				mProxy.ProjectionDirectXLHF(G_DEGREE_TO_RADIAN_F(65), ratio, 0.1, 1000, proj);
			}
			if (std::strcmp(linebuffer, "LIGHT") == 0)
			{
				file.ReadLine(linebuffer, 1024, '\n');
				log.LogCategorized("INFO", (std::string("Light Detected: ") + linebuffer).c_str());

				//get the matrix from which we will pull out the position
				GW::MATH::GMATRIXF transform;
				for (int i = 0; i < 4; ++i) {
					file.ReadLine(linebuffer, 1024, '\n');
					// read floats
					std::sscanf(linebuffer + 13, "%f, %f, %f, %f",
						&transform.data[0 + i * 4], &transform.data[1 + i * 4],
						&transform.data[2 + i * 4], &transform.data[3 + i * 4]);
				}

				GW::MATH::GVECTORF lightPos = transform.row4;
				lightsInLevel.push_back(lightPos);

				//Pass Location to the Debug Log
				std::string loc = "Location: X ";
				loc += std::to_string(transform.row4.x) + " Y " +
					std::to_string(transform.row4.y) + " Z " + std::to_string(transform.row4.z);
				log.LogCategorized("INFO", loc.c_str());
			}
		}

		///Create Player One
		auto e = _game->lookup("craft_speederD");
		auto transformInfo = e.get<Transform>();

		e = e.set_name("Player");
		e.set<Player, Transform>({ transformInfo->transform });
		//e.set([&](Transform t){t = *transformInfo;});
		e.add<Player>();

		///look for all enemies
		if (_game->lookup("craft_speederB.003").is_valid())
		{
			e = _game->lookup("craft_speederB.003");
			transformInfo = e.get<Transform>();
			e = e.set_name("Boss");
			e.set([&](Transform t) {t = *transformInfo; });
			e.add<Boss>();
		}
		else if (_game->lookup("craft_cargoB.003").is_valid())
		{
			e = _game->lookup("craft_cargoB.003");
			transformInfo = e.get<Transform>();
			e = e.set_name("Boss");
			e.set([&](Transform t) {t = *transformInfo; });
			e.add<Boss>();
		}    
		else if (_game->lookup("craft_racer.003").is_valid())
		{
			e = _game->lookup("craft_racer.003");
			transformInfo = e.get<Transform>();
			e = e.set_name("Boss");
			e.set([&](Transform t) {t = *transformInfo; });
			e.add<Boss>();
		}
		else if (_game->lookup("craft_racer.h2b").is_valid())
		{
			e = _game->lookup("craft_racer.h2b");
			transformInfo = e.get<Transform>();
			e = e.set_name("Enemy");
			e.set([&](Transform t) {t = *transformInfo; });
			e.add<Enemy>();
		}
		else if (_game->lookup("craft_cargoB.h2b").is_valid())
		{
			e = _game->lookup("craft_cargoB.h2b");
			transformInfo = e.get<Transform>();
			e = e.set_name("Enemy");
			e.set([&](Transform t) {t = *transformInfo; });
			e.add<Enemy>();
		}
		else if (_game->lookup("craft_speederB.h2b").is_valid())
		{
			e = _game->lookup("craft_speederB.h2b");
			transformInfo = e.get<Transform>();
			e = e.set_name("Enemy");
			e.set([&](Transform t) {t = *transformInfo; });
			e.add<Enemy>();
		}

		log.LogCategorized("MESSAGE", "Game Level File Reading Complete.");
		// level loaded into CPU ram
		log.LogCategorized("EVENT", "GAME LEVEL WAS LOADED TO CPU [OBJECT ORIENTED]");
		return true;
	}
	// Upload the CPU level to GPU
	void UploadLevelToGPU(ID3D11Device* gpuDevice) {
		// iterate over each model and tell it to draw itself
		for (auto& e : allObjectsInLevel) {
			e.UploadModelData2GPU(gpuDevice);
		}
	}
	//Gets Camera Matrices information
	std::vector<GW::MATH::GMATRIXF> GetCameras()
	{
		return camerasInLevel;
	}
	// Draws all objects in the level
	void RenderLevel(GW::GRAPHICS::GDirectX11Surface d3dSurface, GW::SYSTEM::GWindow win, D3D11_VIEWPORT view) {
		// iterate over each model and tell it to draw itself
		for (auto& e : allObjectsInLevel) {
			if (e.GetName() == "craft_speederD")
			{
				auto playerRef = _game->lookup("Player");
				e.MD.worldMatrix = playerRef.get_ref<Transform>().get()->transform;
			}

			e.DrawModel(d3dSurface, win, view, e.MD.worldMatrix);
		}
	}
	// used to wipe CPU & GPU level data between levels
	void UnloadLevel() {
		allObjectsInLevel.clear();
	}
	// *THIS APPROACH COMBINES DATA & LOGIC* 
	// *WITH THIS APPROACH THE CURRENT RENDERER SHOULD BE JUST AN API MANAGER CLASS*
	// *ALL ACTUAL GPU LOADING AND RENDERING SHOULD BE HANDLED BY THE MODEL CLASS* 
	// For example: anything that is not a global API object should be encapsulated.
};

