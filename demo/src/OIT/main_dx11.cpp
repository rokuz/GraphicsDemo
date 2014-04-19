#include "application.h"

// uniforms
DECLARE_UNIFORMS_BEGIN(TestAppUniforms)
	SPACE_DATA,
	LIGHTS_DATA,
	DIFFUSE_MAP,
	NORMAL_MAP,
	SPECULAR_MAP,
	DEFAULT_SAMPLER
DECLARE_UNIFORMS_END()
#define UF framework::UniformBase<TestAppUniforms>::Uniform

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

// fragment data
//#pragma pack (push, 1)
//struct FragmentData
//{
//	unsigned int packedColor;
//	float depth;
//	unsigned int next;
//};
//#pragma pack (pop)

const int MAX_LIGHTS_COUNT = 16;

class OITApp : public framework::Application
{
public:
	OITApp(){}
	virtual ~OITApp(){}

	virtual void init()
	{
		m_info.title = "Order Independent Transparency (DX11)";
		m_info.samples = 4;
		m_info.flags.fullscreen = 0;
	}

	virtual void startup(CEGUI::DefaultWindow* root)
	{
		m_rotation = 0.0f;
		m_pause = false;

		// camera
		m_camera.initWithPositionDirection(m_info.windowWidth, m_info.windowHeight, vector3(0, 50, -100), vector3());

		// head buffer
		m_headBuffer.reset(new framework::RenderTarget());
		auto headBufferDesc = framework::RenderTarget::getDefaultDesc(m_info.windowWidth, 
																	  m_info.windowHeight, 
																	  DXGI_FORMAT_R32_UINT);
		headBufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		m_headBuffer->initWithDescription(headBufferDesc, false);
		if (!m_headBuffer->isValid()) exit();

		// fragments buffer
		unsigned int fragmentsBufferSize = (unsigned int)m_info.windowWidth * m_info.windowHeight * 8;
		unsigned int fragmentSize = 4 + 4 + 4; // color + depth + next
		unsigned int fragmentsBufferFlags = D3D11_BUFFER_UAV_FLAG::D3D11_BUFFER_UAV_FLAG_COUNTER;
		m_fragmentsBuffer.reset(new framework::UnorderedAccessBuffer());
		if (!m_fragmentsBuffer->initDefaultUnorderedAccess(fragmentsBufferSize, fragmentSize, fragmentsBufferFlags)) exit();

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
		update(elapsedTime);

		// clear only head buffer
		framework::UnorderedAccessibleBatch clearbatch;
		clearbatch.add(m_headBuffer);
		getPipeline().clearUnorderedAccessBatch(clearbatch);
		
		// set render target and head/fragments buffers
		framework::UnorderedAccessibleBatch batch;
		batch.add(m_headBuffer);
		batch.add(m_fragmentsBuffer);
		getPipeline().setRenderTarget(defaultRenderTarget(), batch);

		if (m_program->use())
		{
			SpaceData spaceData;
			spaceData.modelViewProjection = m_mvp;
			spaceData.model = m_model;
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

	void update(double elapsedTime)
	{
		quaternion quat;
		quat.set_rotate_x(n_deg2rad(-90.0f));
		quaternion quat2;
		quat2.set_rotate_z(-n_deg2rad(m_rotation));
		m_model = matrix44(quat * quat2);
		quaternion quat3;
		quat3.set_rotate_y(-n_deg2rad(m_rotation));
		m_model.set_translation(quat3.z_direction() * 30.0f);

		m_mvp = (m_model * m_camera.getView()) * m_camera.getProjection();
		m_rotation += (m_pause ? 0 : (float)elapsedTime * 70.0f);
	}

private:
	// texture using to store heads of dynamic lists
	std::shared_ptr<framework::RenderTarget> m_headBuffer;
	// buffer using to store dynamic lists
	std::shared_ptr<framework::UnorderedAccessBuffer> m_fragmentsBuffer;
	
	std::shared_ptr<framework::GpuProgram> m_program;
	std::shared_ptr<framework::Geometry3D> m_geometry;
	std::shared_ptr<framework::Texture> m_texture;
	std::shared_ptr<framework::Texture> m_normalTexture;
	std::shared_ptr<framework::Texture> m_specularTexture;

	std::shared_ptr<framework::UniformBuffer> m_spaceBuffer;
	std::shared_ptr<framework::UniformBuffer> m_lightsBuffer;

	framework::FreeCamera m_camera;
	matrix44 m_model;
	matrix44 m_mvp;

	float m_rotation;
	bool m_pause;
};

DECLARE_MAIN(OITApp);