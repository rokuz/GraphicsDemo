#include "application.h"

DECLARE_UNIFORMS_BEGIN(TestAppUniforms)
	SPACE_DATA,
	LIGHTS_DATA,
	DIFFUSE_MAP,
	NORMAL_MAP,
	SPECULAR_MAP,
	DEFAULT_SAMPLER
DECLARE_UNIFORMS_END()
#define UF framework::UniformBase<TestAppUniforms>::Uniform

#pragma pack (push, 1)
struct SpaceData
{
	matrix44 modelViewProjection;
	matrix44 model;
	vector3 viewPosition;
	unsigned int : 32;
};
#pragma pack (pop)

const int MAX_LIGHTS_COUNT = 16;

class TestApp : public framework::Application
{
public:
	TestApp(){}
	virtual ~TestApp(){}

	virtual void init()
	{
		m_info.title = "Template application (DX11)";
		m_info.samples = 4;
		m_info.flags.fullscreen = 0;
	}

	virtual void startup(CEGUI::DefaultWindow* root)
	{
		m_rotation = 0.0f;
		m_pause = false;

		m_camera.initWithPositionDirection(m_info.windowWidth, m_info.windowHeight, vector3(0, 50, -100), vector3());

		// gpu program
		m_program.reset(new framework::GpuProgram());
		m_program->addShader("data/shaders/dx11/template/shader.vsh");
		m_program->addShader("data/shaders/dx11/template/shader.psh");
		if (!m_program->init()) exit();
		m_program->bindUniform<TestAppUniforms>(UF::SPACE_DATA, "spaceData");
		m_program->bindUniform<TestAppUniforms>(UF::LIGHTS_DATA, "lightsData");
		m_program->bindUniform<TestAppUniforms>(UF::DIFFUSE_MAP, "diffuseMap");
		m_program->bindUniform<TestAppUniforms>(UF::NORMAL_MAP, "normalMap");
		m_program->bindUniform<TestAppUniforms>(UF::SPECULAR_MAP, "specularMap");
		m_program->bindUniform<TestAppUniforms>(UF::DEFAULT_SAMPLER, "defaultSampler");

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

			m_program->setUniform<TestAppUniforms>(UF::SPACE_DATA, m_spaceBuffer);
			m_program->setUniform<TestAppUniforms>(UF::LIGHTS_DATA, m_lightsBuffer);
			m_program->setUniform<TestAppUniforms>(UF::DIFFUSE_MAP, m_texture);
			m_program->setUniform<TestAppUniforms>(UF::NORMAL_MAP, m_normalTexture);
			m_program->setUniform<TestAppUniforms>(UF::SPECULAR_MAP, m_specularTexture);
			m_program->setUniform<TestAppUniforms>(UF::DEFAULT_SAMPLER, anisotropicSampler());

			m_geometry->renderAllMeshes();
		}

		//m_geometry->renderBoundingBox(m_mvp);

		matrix44 vp = m_camera.getView() * m_camera.getProjection();
		renderAxes(vp);
		m_lightManager.renderDebugVisualization(vp);
	}

	virtual void onKeyButton(int key, int scancode, bool pressed)
	{
		if (key == CEGUI::Key::Space && pressed)
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

DECLARE_MAIN(TestApp);