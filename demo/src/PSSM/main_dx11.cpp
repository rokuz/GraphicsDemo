#include "framework.h"

// uniforms
DECLARE_UNIFORMS_BEGIN(PSSMAppUniforms)
	SPACE_DATA,
	LIGHTS_DATA,
	DIFFUSE_MAP,
	NORMAL_MAP,
	SPECULAR_MAP,
	DEFAULT_SAMPLER
DECLARE_UNIFORMS_END()
#define UF framework::UniformBase<PSSMAppUniforms>::Uniform

// spatial data
#pragma pack (push, 1)
struct SpaceData
{
	matrix44 modelViewProjection;
	matrix44 model;
	vector3 viewPosition;
	unsigned int : 32;
};
#pragma pack (pop)

// constants
const int MAX_LIGHTS_COUNT = 16;
const std::string SHADERS_PATH = "data/shaders/dx11/pssm/";

// application
class PSSMApp : public framework::Application
{
public:
	PSSMApp(){}
	virtual ~PSSMApp(){}

	virtual void init(const std::map<std::string, int>& params)
	{
		m_info.title = "Parallel-Split Shadow Mapping (DX11)";

		applyStandardParams(params);
		m_info.flags.fullscreen = 0;
	}

	virtual void startup(gui::WidgetPtr_T root)
	{
		m_rotation = 0.0f;
		m_pause = false;

		m_camera.initWithPositionDirection(m_info.windowWidth, m_info.windowHeight, vector3(0, 50, -100), vector3());

		// gpu program
		m_program.reset(new framework::GpuProgram());
		m_program->addShader(SHADERS_PATH + "scene.vsh.hlsl");
		m_program->addShader(SHADERS_PATH + "scene.psh.hlsl");
		if (!m_program->init()) exit();
		m_program->bindUniform<PSSMAppUniforms>(UF::SPACE_DATA, "spaceData");
		m_program->bindUniform<PSSMAppUniforms>(UF::LIGHTS_DATA, "lightsData");
		m_program->bindUniform<PSSMAppUniforms>(UF::DIFFUSE_MAP, "diffuseMap");
		m_program->bindUniform<PSSMAppUniforms>(UF::NORMAL_MAP, "normalMap");
		m_program->bindUniform<PSSMAppUniforms>(UF::SPECULAR_MAP, "specularMap");
		m_program->bindUniform<PSSMAppUniforms>(UF::DEFAULT_SAMPLER, "defaultSampler");

		m_plane.reset(new framework::Geometry3D());
		geom::PlaneGenerationInfo planeInfo;
		planeInfo.size = vector2(1000.0f, 1000.0f);
		planeInfo.uvSize = vector2(50.0f, 50.0f);
		if (!m_plane->initAsPlane(planeInfo)) exit();
		m_plane->bindToGpuProgram(m_program);

		m_grassTexture.reset(new framework::Texture());
		m_grassTexture->initWithDDS("data/media/textures/grass.dds");

		// geometry
		m_geometry.reset(new framework::Geometry3D());
		if (!m_geometry->init("data/media/spaceship/spaceship.geom")) exit();
		m_geometry->bindToGpuProgram(m_program);

		// textures
		m_texture.reset(new framework::Texture());
		m_texture->initWithDDS("data/media/spaceship/spaceship_diff.dds");

		m_specularTexture.reset(new framework::Texture());
		m_specularTexture->initWithDDS("data/media/spaceship/spaceship_specular.dds");

		m_normalTexture.reset(new framework::Texture());
		m_normalTexture->initWithDDS("data/media/spaceship/spaceship_normal.dds");

		// space info buffer
		m_spaceBuffer.reset(new framework::UniformBuffer());
		if (!m_spaceBuffer->initDefaultConstant<SpaceData>()) exit();

		// lights
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

		m_lightsBuffer.reset(new framework::UniformBuffer());
		if (!m_lightsBuffer->initDefaultStructured<framework::LightRawData>((size_t)MAX_LIGHTS_COUNT)) exit();

		int lightsCount = std::min((int)m_lightManager.getLightSourcesCount(), MAX_LIGHTS_COUNT);
		for (int i = 0; i < lightsCount; i++)
		{
			m_lightsBuffer->setElement(i, m_lightManager.getRawLightData(i));
		}
		m_lightsBuffer->applyChanges();
	}

	virtual void shutdown()
	{
	}

	virtual void onResize(int width, int height)
	{
		m_camera.updateResolution(width, height);
	}

	virtual void render(double elapsedTime)
	{
		m_camera.update(elapsedTime);

		quaternion quat;
		quat.set_rotate_x(n_deg2rad(-90.0f));
		quaternion quat2;
		quat2.set_rotate_z(-n_deg2rad(m_rotation));
		matrix44 model(quat * quat2);
		quaternion quat3;
		quat3.set_rotate_y(-n_deg2rad(m_rotation));
		model.set_translation(quat3.z_direction() * 30.0f);

		m_mvp = (model * m_camera.getView()) * m_camera.getProjection();

		m_rotation += (m_pause? 0 : (float)elapsedTime * 70.0f);

		useDefaultRenderTarget();

		if (m_program->use())
		{
			SpaceData spaceData;
			spaceData.modelViewProjection = m_mvp;
			spaceData.model = model;
			spaceData.viewPosition = m_camera.getPosition();
			m_spaceBuffer->setData(spaceData);
			m_spaceBuffer->applyChanges();

			m_program->setUniform<PSSMAppUniforms>(UF::SPACE_DATA, m_spaceBuffer);
			m_program->setUniform<PSSMAppUniforms>(UF::LIGHTS_DATA, m_lightsBuffer);
			m_program->setUniform<PSSMAppUniforms>(UF::DIFFUSE_MAP, m_texture);
			m_program->setUniform<PSSMAppUniforms>(UF::NORMAL_MAP, m_normalTexture);
			m_program->setUniform<PSSMAppUniforms>(UF::SPECULAR_MAP, m_specularTexture);
			m_program->setUniform<PSSMAppUniforms>(UF::DEFAULT_SAMPLER, anisotropicSampler());

			m_geometry->renderAllMeshes();

			spaceData.modelViewProjection = m_camera.getView() * m_camera.getProjection();
			spaceData.model = matrix44();
			spaceData.viewPosition = m_camera.getPosition();
			m_spaceBuffer->setData(spaceData);
			m_spaceBuffer->applyChanges();
			m_program->setUniform<PSSMAppUniforms>(UF::SPACE_DATA, m_spaceBuffer);
			m_program->setUniform<PSSMAppUniforms>(UF::LIGHTS_DATA, m_lightsBuffer);
			m_program->setUniform<PSSMAppUniforms>(UF::DIFFUSE_MAP, m_grassTexture);
			m_program->setUniform<PSSMAppUniforms>(UF::NORMAL_MAP, m_normalTexture);
			m_program->setUniform<PSSMAppUniforms>(UF::SPECULAR_MAP, m_specularTexture);
			m_program->setUniform<PSSMAppUniforms>(UF::DEFAULT_SAMPLER, anisotropicSampler());

			m_plane->renderAllMeshes();
		}

		//m_geometry->renderBoundingBox(m_mvp);
		//m_plane->renderBoundingBox(m_camera.getView() * m_camera.getProjection());

		matrix44 vp = m_camera.getView() * m_camera.getProjection();
		renderAxes(vp);
		m_lightManager.renderDebugVisualization(vp);
	}

	virtual void onKeyButton(int key, int scancode, bool pressed)
	{
		if (key == InputKeys::Space && pressed)
		{
			m_pause = !m_pause;
			return;
		}
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

private:

	std::shared_ptr<framework::Geometry3D> m_plane;
	std::shared_ptr<framework::Texture> m_grassTexture;

	std::shared_ptr<framework::GpuProgram> m_program;
	std::shared_ptr<framework::Geometry3D> m_geometry;
	std::shared_ptr<framework::Texture> m_texture;
	std::shared_ptr<framework::Texture> m_normalTexture;
	std::shared_ptr<framework::Texture> m_specularTexture;

	std::shared_ptr<framework::UniformBuffer> m_spaceBuffer;
	std::shared_ptr<framework::UniformBuffer> m_lightsBuffer;

	framework::FreeCamera m_camera;
	matrix44 m_mvp;

	float m_rotation;
	bool m_pause;
};

DECLARE_MAIN(PSSMApp);