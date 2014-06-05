#include "framework.h"

// uniforms
DECLARE_UNIFORMS_BEGIN(PSSMAppUniforms)
	ENTITY_DATA,
	ONFRAME_DATA,
	LIGHTS_DATA,
	SHADOW_DATA,
	DIFFUSE_MAP,
	NORMAL_MAP,
	SHADOW_MAP,
	DEFAULT_SAMPLER,
	SHADOW_MAP_SAMPLER
DECLARE_UNIFORMS_END()
#define UF framework::UniformBase<PSSMAppUniforms>::Uniform

// constants
const std::string SHADERS_PATH = "data/shaders/dx11/pssm/";
const int MAX_SPLITS = 4;
#define PROFILING 0

// entity data
#pragma pack (push, 1)
struct EntityDataRaw
{
	matrix44 modelViewProjection;
	matrix44 model;
	int shadowIndices[MAX_SPLITS];
};
#pragma pack (pop)

// on frame data
#pragma pack (push, 1)
struct OnFrameDataRaw
{
	vector3 viewPosition;
	int splitCount;
};
#pragma pack (pop)

// shadow data
#pragma pack (push, 1)
struct ShadowDataRaw
{
	matrix44 shadowViewProjection[MAX_SPLITS];
};
#pragma pack (pop)

// application
class PSSMApp : public framework::Application
{
	struct Entity;
	struct EntityData;

public:
	PSSMApp()
	{
		m_shadowMapSize = 1024;
		m_splitCount = MAX_SPLITS;
		m_currentSplitCount = m_splitCount;
		m_splitLambda = 0.5f;
		m_farPlane = 0;
		m_splitShift = 10;

		m_renderDebug = false;
		m_furthestPointInCamera = 0;
	}

	virtual void init(const std::map<std::string, int>& params)
	{
		m_info.title = "Parallel-Split Shadow Mapping (DX11)";

		applyStandardParams(params);
		auto smSize = params.find("shadowmap");
		if (smSize != params.end())
		{
			m_shadowMapSize = smSize->second;
			if (m_shadowMapSize < 0) m_shadowMapSize = 1024;
			if (m_shadowMapSize > 8192) m_shadowMapSize = 8192;
		}
		auto split = params.find("split");
		if (split != params.end())
		{
			m_splitCount = split->second;
			if (m_splitCount < 1) m_splitCount = 1;
			if (m_splitCount > MAX_SPLITS) m_splitCount = MAX_SPLITS;
		}

		m_splitDistances.resize(m_splitCount + 1);
		if (m_splitCount > 1) m_maxSplitDistances.resize(m_splitCount - 1);
		m_currentSplitCount = m_splitCount;

		setLegend("WASD - move camera\nLeft mouse button - rotate camera\nF1 - debug info");

	#if PROFILING
		std::stringstream header;
		header << m_info.title << "\n" << m_info.windowWidth << "x" << m_info.windowHeight 
			<< ", samples = " << m_info.samples << ", fullscreen = " << (m_info.flags.fullscreen ? "yes" : "no") << "\n";
		utils::Profiler::instance().setHeader(header.str());
		utils::Profiler::instance().setFilename("profiler_" + utils::Utils::currentTimeDate(true) + ".txt");
	#endif
	}

	virtual void startup(gui::WidgetPtr_T root)
	{
		// camera
		m_camera.initWithPositionDirection(m_info.windowWidth, m_info.windowHeight, vector3(0, 50, -100), vector3());

		// overlays
		initOverlays(root);

		D3D11_TEXTURE2D_DESC desc = framework::RenderTarget::getDefaultDesc(m_shadowMapSize, m_shadowMapSize, DXGI_FORMAT_R32_FLOAT);
		desc.ArraySize = m_splitCount;
		m_shadowMap.reset(new framework::RenderTarget());
		m_shadowMap->initWithDescription(desc, true);
		if (!m_shadowMap->isValid()) exit();

		// gpu programs
		m_sceneRendering.reset(new framework::GpuProgram());
		m_sceneRendering->addShader(SHADERS_PATH + "scene.vsh.hlsl");
		m_sceneRendering->addShader(SHADERS_PATH + "scene.psh.hlsl");
		if (!m_sceneRendering->init()) exit();
		m_sceneRendering->bindUniform<PSSMAppUniforms>(UF::ONFRAME_DATA, "onFrameData");
		m_sceneRendering->bindUniform<PSSMAppUniforms>(UF::ENTITY_DATA, "entityData");
		m_sceneRendering->bindUniform<PSSMAppUniforms>(UF::LIGHTS_DATA, "lightsData");
		m_sceneRendering->bindUniform<PSSMAppUniforms>(UF::SHADOW_DATA, "shadowData");
		m_sceneRendering->bindUniform<PSSMAppUniforms>(UF::DIFFUSE_MAP, "diffuseMap");
		m_sceneRendering->bindUniform<PSSMAppUniforms>(UF::NORMAL_MAP, "normalMap");
		m_sceneRendering->bindUniform<PSSMAppUniforms>(UF::SHADOW_MAP, "shadowMap");
		m_sceneRendering->bindUniform<PSSMAppUniforms>(UF::DEFAULT_SAMPLER, "defaultSampler");
		m_sceneRendering->bindUniform<PSSMAppUniforms>(UF::SHADOW_MAP_SAMPLER, "shadowMapSampler");

		m_shadowMapRendering.reset(new framework::GpuProgram());
		m_shadowMapRendering->addShader(SHADERS_PATH + "shadowmap.vsh.hlsl");
		m_shadowMapRendering->addShader(SHADERS_PATH + "shadowmap.gsh.hlsl");
		m_shadowMapRendering->addShader(SHADERS_PATH + "shadowmap.psh.hlsl");
		if (!m_shadowMapRendering->init()) exit();
		m_shadowMapRendering->bindUniform<PSSMAppUniforms>(UF::SHADOW_DATA, "shadowData");
		m_shadowMapRendering->bindUniform<PSSMAppUniforms>(UF::ENTITY_DATA, "entityData");

		// entity
		m_entityGeometry = initEntity("data/media/house/house.geom");
		m_entityGeometry->bindToGpuProgram(m_sceneRendering);
		m_entityGeometry->bindToGpuProgram(m_shadowMapRendering);

		const int ENTITIES_IN_ROW = 2;
		const float HALF_ENTITIES_IN_ROW = float(ENTITIES_IN_ROW) * 0.5f;
		const float AREA_HALFLENGTH = 150.0f;
		m_entitiesData.resize(ENTITIES_IN_ROW * ENTITIES_IN_ROW);
		for (int i = 0; i < ENTITIES_IN_ROW; i++)
		{
			for (int j = 0; j < ENTITIES_IN_ROW; j++)
			{
				int index = i * ENTITIES_IN_ROW + j;
				float x = (float(i) - HALF_ENTITIES_IN_ROW) / HALF_ENTITIES_IN_ROW;
				float z = (float(j) - HALF_ENTITIES_IN_ROW) / HALF_ENTITIES_IN_ROW;

				m_entitiesData[index].model.set_translation(vector3(x * AREA_HALFLENGTH, 5.0f, z * AREA_HALFLENGTH));
			}
		}

		geom::PlaneGenerationInfo planeInfo;
		planeInfo.size = vector2(1000.0f, 1000.0f);
		planeInfo.uvSize = vector2(50.0f, 50.0f);
		m_planeGeometry = initEntity(planeInfo, "data/media/textures/grass.dds", "data/media/textures/grass_bump.dds");
		m_planeGeometry->bindToGpuProgram(m_sceneRendering);
		m_planeData.model.ident();
		m_planeData.isShadowCaster = false;
		if (m_planeData.isShadowCaster)
		{
			m_entityGeometry->bindToGpuProgram(m_shadowMapRendering);
		}

		// rasterizer stage for shadowmap rendering
		m_shadowMapRasterizer.reset(new framework::RasterizerStage());
		D3D11_RASTERIZER_DESC rasterizerDesc = framework::RasterizerStage::getDefault();
		m_shadowMapRasterizer->initWithDescription(rasterizerDesc);
		if (!m_shadowMapRasterizer->isValid()) exit();
		m_shadowMapRasterizer->getViewports().resize(1);
		m_shadowMapRasterizer->getViewports()[0] = framework::RasterizerStage::getDefaultViewport(m_shadowMapSize, m_shadowMapSize);

		// sampler for shadow mapping
		m_shadowMapSampler.reset(new framework::Sampler());
		D3D11_SAMPLER_DESC samplerDesc = framework::Sampler::getDefault();
		samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerDesc.BorderColor[0] = 1.0f;
		samplerDesc.BorderColor[1] = 1.0f;
		samplerDesc.BorderColor[2] = 1.0f;
		samplerDesc.BorderColor[3] = 1.0f;
		m_shadowMapSampler->initWithDescription(samplerDesc);
		if (!m_shadowMapSampler->isValid()) exit();

		// entity's data buffer
		m_entityDataBuffer.reset(new framework::UniformBuffer());
		if (!m_entityDataBuffer->initDefaultConstant<EntityDataRaw>()) exit();

		// on-frame data buffer
		m_onFrameDataBuffer.reset(new framework::UniformBuffer());
		if (!m_onFrameDataBuffer->initDefaultConstant<OnFrameDataRaw>()) exit();

		// shadow buffer
		m_shadowBuffer.reset(new framework::UniformBuffer());
		if (!m_shadowBuffer->initDefaultConstant<ShadowDataRaw>()) exit();

		// lights
		initLights();

		// skybox texture
		//m_skyboxTexture.reset(new framework::Texture());
		//if (!m_skyboxTexture->initWithDDS("data/media/textures/meadow.dds")) exit();
	}

	std::shared_ptr<framework::Geometry3D> initEntity(const geom::PlaneGenerationInfo& planeInfo, const std::string& diffuseMap, const std::string& normalMap)
	{
		std::shared_ptr<framework::Geometry3D> ent(new framework::Geometry3D());
		if (!ent->initAsPlane(planeInfo)) 
		{
			exit();
		}
		else
		{
			framework::MaterialManager::instance().initializeMaterial(ent, diffuseMap, normalMap, "");
		}
		
		return std::move(ent);
	}

	std::shared_ptr<framework::Geometry3D> initEntity(const std::string& geometry)
	{
		std::shared_ptr<framework::Geometry3D> ent(new framework::Geometry3D());
		if (!ent->init(geometry))
		{
			exit();
		}
		else
		{
			framework::MaterialManager::instance().initializeMaterial(ent);
		}

		return std::move(ent);
	}

	void initLights()
	{
		// directional light
		framework::LightSource source;
		source.type = framework::LightType::DirectLight;
		source.position = vector3(0, 15, 0);
		vector3 dir(1, -1, 1);
		dir.norm();
		source.orientation.set_from_axes(vector3(0, 0, 1), dir);
		source.diffuseColor = vector3(1.0f, 1.0f, 1.0f);
		source.specularColor = vector3(0.5f, 0.5f, 0.5f);
		source.ambientColor = vector3(0.3f, 0.3f, 0.3f);
		m_lightManager.addLightSource(source);

		// light buffer
		m_lightsBuffer.reset(new framework::UniformBuffer());
		if (!m_lightsBuffer->initDefaultConstant<framework::LightRawData>()) exit();
		m_lightsBuffer->setData(m_lightManager.getRawLightData(0));
		m_lightsBuffer->applyChanges();
	}

	void initOverlays(gui::WidgetPtr_T root)
	{
		m_debugLabel = framework::UIFactory::createLabel(gui::Coords(1.0f, -500.0f, 1.0f, -300.0f),
														 gui::Coords::Absolute(500.0f, 300.0f),
														 gui::RightAligned, gui::BottomAligned);
		root->addChild(m_debugLabel);

		m_debugLabel->setVisible(m_renderDebug);
	}

	virtual void shutdown()
	{
	#if PROFILING
		if (utils::Profiler::instance().isRun())
		{
			utils::Profiler::instance().stop();
		}
		utils::Profiler::instance().saveToFile();
	#endif
	}

	virtual void render(double elapsedTime)
	{
		m_camera.update(elapsedTime);
		update(elapsedTime);

		// render shadow map
		getPipeline().clearRenderTarget(m_shadowMap, vector4(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX));
		getPipeline().setRenderTarget(m_shadowMap);
		if (m_shadowMapRendering->use())
		{
			m_shadowMapRendering->setUniform<PSSMAppUniforms>(UF::SHADOW_DATA, m_shadowBuffer);

			m_shadowMapRasterizer->apply();
			for (size_t i = 0; i < m_entitiesData.size(); i++)
			{
				renderGeometry(true, m_entityGeometry, m_entitiesData[i]);
			}
			renderGeometry(true, m_planeGeometry, m_planeData);
			m_shadowMapRasterizer->cancel();
		}

		// set up on-frame data
		OnFrameDataRaw onFrameData;
		onFrameData.viewPosition = m_camera.getPosition();
		onFrameData.splitCount = m_currentSplitCount;
		m_onFrameDataBuffer->setData(onFrameData);
		m_onFrameDataBuffer->applyChanges();

		useDefaultRenderTarget();

		// render skybox
		//renderSkybox(m_camera, m_skyboxTexture);

		// render scene
		if (m_sceneRendering->use())
		{
			m_sceneRendering->setUniform<PSSMAppUniforms>(UF::ONFRAME_DATA, m_onFrameDataBuffer);
			m_sceneRendering->setUniform<PSSMAppUniforms>(UF::LIGHTS_DATA, m_lightsBuffer);
			m_sceneRendering->setUniform<PSSMAppUniforms>(UF::DEFAULT_SAMPLER, anisotropicSampler());

			m_sceneRendering->setUniform<PSSMAppUniforms>(UF::SHADOW_DATA, m_shadowBuffer);
			m_sceneRendering->setUniform<PSSMAppUniforms>(UF::SHADOW_MAP, m_shadowMap);

			for (size_t i = 0; i < m_entitiesData.size(); i++)
			{
				renderGeometry(false, m_entityGeometry, m_entitiesData[i]);
			}

			renderGeometry(false, m_planeGeometry, m_planeData);
		}

		renderDebug();
	}

	void renderGeometry(bool shadowmap, const std::shared_ptr<framework::Geometry3D>& geometry, const EntityData& entityData)
	{
		if (shadowmap && (!entityData.isShadowCaster || entityData.shadowInstancesCount == 0)) return;

		EntityDataRaw entityDataRaw;
		entityDataRaw.modelViewProjection = entityData.mvp;
		entityDataRaw.model = entityData.model;
		memcpy(entityDataRaw.shadowIndices, entityData.shadowIndices, sizeof(unsigned int) * MAX_SPLITS);
		m_entityDataBuffer->setData(entityDataRaw);
		m_entityDataBuffer->applyChanges();

		for (size_t i = 0; i < geometry->getMeshesCount(); i++)
		{
			if (shadowmap)
			{
				m_shadowMapRendering->setUniform<PSSMAppUniforms>(UF::ENTITY_DATA, m_entityDataBuffer);
			}
			else
			{
				auto diffMap = framework::MaterialManager::instance().getTexture(geometry, i, framework::MAT_DIFFUSE_MAP);
				auto normMap = framework::MaterialManager::instance().getTexture(geometry, i, framework::MAT_NORMAL_MAP);
				m_sceneRendering->setUniform<PSSMAppUniforms>(UF::DIFFUSE_MAP, diffMap);
				m_sceneRendering->setUniform<PSSMAppUniforms>(UF::NORMAL_MAP, normMap);

				m_sceneRendering->setUniform<PSSMAppUniforms>(UF::ENTITY_DATA, m_entityDataBuffer);
				m_sceneRendering->setUniform<PSSMAppUniforms>(UF::SHADOW_MAP_SAMPLER, m_shadowMapSampler);
			}
			geometry->renderMesh(i, shadowmap ? entityData.shadowInstancesCount : 1);
		}	
	}

	void renderDebug()
	{
		if (!m_renderDebug) return;

		matrix44 vp = m_camera.getView() * m_camera.getProjection();
		//renderAxes(vp);
		m_lightManager.renderDebugVisualization(vp);

		std::wstringstream stream;
		stream.precision(2);
		stream << "Furthest point = " << std::fixed << m_furthestPointInCamera << "\nDistances = ";
		for (int i = 0; i < m_currentSplitCount + 1; i++)
		{
			stream << std::fixed << m_splitDistances[i];
			if (i != m_currentSplitCount)
			{	
				if (i % 2 == 0 && i != 0) stream << ",\n"; else stream << ", ";
			}
		}

		int objectsCount = 0;
		int instancesCount = 0;
		for (size_t i = 0; i < m_entitiesData.size(); i++)
		{
			if (m_entitiesData[i].shadowInstancesCount > 0)
			{
				objectsCount++;
				instancesCount += (m_entitiesData[i].shadowInstancesCount - 1);
			}
		}
		if (m_planeData.shadowInstancesCount > 0)
		{
			objectsCount++;
			instancesCount += (m_planeData.shadowInstancesCount - 1);
		}
		stream << "\nRendered to SM objects = " << objectsCount << "\nRendered to SM instances = " << instancesCount;

		m_debugLabel->setText(stream.str());
	}

	virtual void onKeyButton(int key, int scancode, bool pressed)
	{
		if (key == InputKeys::F1 && pressed)
		{
			m_renderDebug = !m_renderDebug;
			m_debugLabel->setVisible(m_renderDebug);
			return;
		}
	#if PROFILING
		if (key == InputKeys::P && pressed)
		{
			if (!utils::Profiler::instance().isRun())
				utils::Profiler::instance().run();
			else
				utils::Profiler::instance().stop();
			return;
		}
	#endif
		m_camera.onKeyButton(key, scancode, pressed);
	}

	virtual void onMouseButton(double xpos, double ypos, int button, bool pressed)
	{
		m_camera.onMouseButton(xpos, ypos, button, pressed);
	}

	virtual void onMouseMove(double xpos, double ypos)
	{
		m_camera.onMouseMove(xpos, ypos);
	}

	void update(double elapsedTime)
	{
		matrix44 cameraView = m_camera.getView();
		matrix44 cameraProjection = m_camera.getProjection();
		matrix44 vp = cameraView * cameraProjection;
		for (size_t i = 0; i < m_entitiesData.size(); i++)
		{
			m_entitiesData[i].mvp = m_entitiesData[i].model * vp;
			m_entitiesData[i].shadowInstancesCount = 0;
		}

		m_planeData.mvp = m_planeData.model * vp;
		m_planeData.shadowInstancesCount = 0;

		// calculate shadow
		ShadowDataRaw shadowData;
		calculateMaxSplitDistances();
		m_furthestPointInCamera = calculateFurthestPointInCamera(cameraView);
		calculateSplitDistances();
		for (int i = 0; i < m_currentSplitCount; i++)
		{
			float n = m_splitDistances[i];
			float f = m_splitDistances[i + 1];
			bbox3 box = calculateFrustumBox(n, f);
			shadowData.shadowViewProjection[i] = calculateShadowViewProjection(box);
			updateShadowVisibilityMask(box, i);
		}

		m_shadowBuffer->setData(shadowData);
		m_shadowBuffer->applyChanges();
	}

	void calculateMaxSplitDistances()
	{
		float nearPlane = m_camera.getInternalCamera().GetNearPlane();
		float farPlane = m_camera.getInternalCamera().GetFarPlane();
		for (int i = 1; i < m_splitCount; i++)
		{
			float f = (float)i / (float)m_splitCount;
			float l = nearPlane * pow(farPlane / nearPlane, f);
			float u = nearPlane + (farPlane - nearPlane) * f;
			m_maxSplitDistances[i - 1] = l * m_splitLambda + u * (1.0f - m_splitLambda);
		}

		m_farPlane = farPlane + m_splitShift;
	}

	float calculateFurthestPointInCamera(const matrix44& cameraView)
	{
		bbox3 scenebox;
		scenebox.begin_extend();
		if (m_planeData.isShadowCaster)
		{
			bbox3 b = m_planeGeometry->getBoundingBox();
			b.transform(m_planeData.model);
			scenebox.extend(b);
		}
		for (size_t i = 0; i < m_entitiesData.size(); i++)
		{
			if (m_entitiesData[i].isShadowCaster)
			{
				bbox3 b = m_entityGeometry->getBoundingBox();
				b.transform(m_entitiesData[i].model);
				scenebox.extend(b);
			}
		}
		scenebox.end_extend();

		float maxZ = m_camera.getInternalCamera().GetNearPlane();
		for(int i = 0; i < 8; i++)
		{
			vector3 corner = scenebox.corner_point(i);
			float z = -cameraView.transform_coord(corner).z;
			if(z > maxZ) maxZ = z;
		}
		return std::min(maxZ, m_farPlane);
	}

	void calculateSplitDistances()
	{
		// calculate how many shadow maps do we really need
		m_currentSplitCount = 1;
		if (!m_maxSplitDistances.empty())
		{
			for (size_t i = 0; i < m_maxSplitDistances.size(); i++)
			{
				float d = m_maxSplitDistances[i] - m_splitShift;
				if (m_furthestPointInCamera >= d) m_currentSplitCount++;
			}
		}

		float nearPlane = m_camera.getInternalCamera().GetNearPlane();
		for (int i = 0; i < m_currentSplitCount; i++)
		{
			float f = (float)i / (float)m_currentSplitCount;
			float l = nearPlane * pow(m_furthestPointInCamera / nearPlane, f);
			float u = nearPlane + (m_furthestPointInCamera - nearPlane) * f;
			m_splitDistances[i] = l * m_splitLambda + u * (1.0f - m_splitLambda);
		}

		m_splitDistances[0] = nearPlane;
		m_splitDistances[m_currentSplitCount] = m_furthestPointInCamera;
	}

	matrix44 calculateShadowViewProjection(const bbox3& frustumBox)
	{
		const float LIGHT_SOURCE_HEIGHT = 200.0f;

		vector3 viewDir = m_camera.getOrientation().z_direction();
		vector3 size = frustumBox.size();
		vector3 center = frustumBox.center() - viewDir * m_splitShift;
		center.y = 0;

		auto lightSource = m_lightManager.getLightSource(0);
		vector3 lightDir = lightSource.orientation.z_direction();

		matrix44 shadowView;
		shadowView.pos_component() = center - lightDir * LIGHT_SOURCE_HEIGHT;
		shadowView.lookatRh(shadowView.pos_component() + lightDir, lightSource.orientation.y_direction());
		shadowView.invert_simple();

		matrix44 shadowProj;	
		float d = std::max(size.x, size.z);
		shadowProj.orthoRh(d, d, 0.1f, 1000.0f);

		return shadowView * shadowProj;
	}

	bbox3 calculateFrustumBox(float nearPlane, float farPlane)
	{
		vector3 eye = m_camera.getPosition();
		vector3 vZ = m_camera.getOrientation().z_direction();
		vector3 vX = m_camera.getOrientation().x_direction();
		vector3 vY = m_camera.getOrientation().y_direction();
		float fov = n_deg2rad(m_camera.getInternalCamera().GetAngleOfView());
		float aspect = m_camera.getInternalCamera().GetAspectRatio();

		float nearPlaneHeight = n_tan(fov * 0.5f) * nearPlane;
		float nearPlaneWidth = nearPlaneHeight * aspect;
		float farPlaneHeight = n_tan(fov * 0.5f) * farPlane;
		float farPlaneWidth = farPlaneHeight * aspect;
		vector3 nearPlaneCenter = eye + vZ * nearPlane;
		vector3 farPlaneCenter = eye + vZ * farPlane;

		bbox3 box;
		box.begin_extend();
		box.extend(vector3(nearPlaneCenter - vX * nearPlaneWidth - vY * nearPlaneHeight));
		box.extend(vector3(nearPlaneCenter - vX * nearPlaneWidth + vY * nearPlaneHeight));
		box.extend(vector3(nearPlaneCenter + vX * nearPlaneWidth + vY * nearPlaneHeight));
		box.extend(vector3(nearPlaneCenter + vX * nearPlaneWidth - vY * nearPlaneHeight));
		box.extend(vector3(farPlaneCenter - vX * farPlaneWidth - vY * farPlaneHeight));
		box.extend(vector3(farPlaneCenter - vX * farPlaneWidth + vY * farPlaneHeight));
		box.extend(vector3(farPlaneCenter + vX * farPlaneWidth + vY * farPlaneHeight));
		box.extend(vector3(farPlaneCenter + vX * farPlaneWidth - vY * farPlaneHeight));
		box.end_extend();
		return box;
	}

	void updateShadowVisibilityMask(const bbox3& frustumBox, int splitIndex)
	{
		vector3 viewDir = m_camera.getOrientation().z_direction();
		matrix44 boxOffset;
		boxOffset.set_translation(-viewDir * m_splitShift);
		bbox3 box = frustumBox;
		box.transform(boxOffset);

		if (m_planeData.isShadowCaster)
		{
			updateShadowVisibilityMask(box, m_planeGeometry, m_planeData, splitIndex);
		}
		for (size_t i = 0; i < m_entitiesData.size(); i++)
		{
			if (m_entitiesData[i].isShadowCaster)
			{
				updateShadowVisibilityMask(box, m_entityGeometry, m_entitiesData[i], splitIndex);
			}
		}
	}

	void updateShadowVisibilityMask(const bbox3& frustumBox, const std::shared_ptr<framework::Geometry3D>& entity, EntityData& entityData, int splitIndex)
	{
		bbox3 b = entity->getBoundingBox();
		b.transform(entityData.model);

		// shadow box computation
		auto lightSource = m_lightManager.getLightSource(0);
		vector3 lightDir = lightSource.orientation.z_direction();
		float shadowBoxL = fabs(lightDir.z) < 1e-5 ? 1000.0f : (b.size().y / -lightDir.z);
		bbox3 shadowBox;
		shadowBox.begin_extend();
		for (int i = 0; i < 8; i++)
		{
			shadowBox.extend(b.corner_point(i));
			shadowBox.extend(b.corner_point(i) + lightDir * shadowBoxL);
		}
		shadowBox.end_extend();
	
		if (frustumBox.clipstatus(shadowBox) != bbox3::Outside)
		{
			int i = entityData.shadowInstancesCount;
			entityData.shadowIndices[i] = splitIndex;
			entityData.shadowInstancesCount++;
		}
	}

private:
	// gpu program to render scene
	std::shared_ptr<framework::GpuProgram> m_sceneRendering;
	// gpu program to render shadow map
	std::shared_ptr<framework::GpuProgram> m_shadowMapRendering;

	// shadow map
	std::shared_ptr<framework::RenderTarget> m_shadowMap;

	// rasterizer stage for shadow map rendering
	std::shared_ptr<framework::RasterizerStage> m_shadowMapRasterizer;
	// sampler for shadow map rendering
	std::shared_ptr<framework::Sampler> m_shadowMapSampler;

	struct EntityData
	{
		matrix44 model;
		matrix44 mvp;
		unsigned int shadowInstancesCount;
		unsigned int shadowIndices[MAX_SPLITS];
		bool isShadowCaster;
		EntityData() : isShadowCaster(true), shadowInstancesCount(0)
		{
			memset(shadowIndices, 0, sizeof(unsigned int) * MAX_SPLITS);
		}
	};
	std::shared_ptr<framework::Geometry3D> m_entityGeometry;
	std::vector<EntityData> m_entitiesData;

	std::shared_ptr<framework::Geometry3D> m_planeGeometry;
	EntityData m_planeData;

	std::shared_ptr<framework::UniformBuffer> m_entityDataBuffer;
	std::shared_ptr<framework::UniformBuffer> m_onFrameDataBuffer;
	std::shared_ptr<framework::UniformBuffer> m_lightsBuffer;
	std::shared_ptr<framework::UniformBuffer> m_shadowBuffer;

	framework::FreeCamera m_camera;

	int m_shadowMapSize;

	std::vector<float> m_maxSplitDistances;
	float m_farPlane;
	float m_splitShift;
	int m_splitCount;
	int m_currentSplitCount;
	float m_splitLambda;
	std::vector<float> m_splitDistances;
	float m_furthestPointInCamera;

	bool m_renderDebug;
	gui::LabelPtr_T m_debugLabel;

	std::shared_ptr<framework::Texture> m_skyboxTexture;
};

DECLARE_MAIN(PSSMApp);