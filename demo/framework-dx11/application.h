/*
 * Copyright (c) 2014 Roman Kuznetsov 
 *
 * Thanks to Graham Sellers (and his book "OpenGL SuperBible, 6th Edition") for inspiration and some code
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#pragma once

namespace framework
{

class Application
{
	friend class Destroyable;
	friend class PipelineStage;
	friend class GpuProgram;

public:
	Application();
	virtual ~Application(){}

	virtual void init(const std::map<std::string, int>& params){}
	virtual void startup(gui::WidgetPtr_T root){}
	virtual void render(double elapsedTime){}
	virtual void shutdown(){}
	virtual void onResize(int width, int height){}
	virtual void onKeyButton(int key, int scancode, bool pressed){}
	virtual void onMouseButton(double xpos, double ypos, int button, bool pressed){}
	virtual void onMouseMove(double xpos, double ypos){}

	static Application* instance();
	int run(Application* self, const std::string& commandLine);
	void exit();
	void resize();
	void applyStandardParams(const std::map<std::string, int>& params);
	void setLegend(const std::string& legend);

	const Device& getDevice() const { return m_device; }
	std::weak_ptr<GpuProgram> getUsingGpuProgram() const { return m_usingGpuProgram; }
	Pipeline& getPipeline() { return m_pipeline; }
	vector2 getScreenSize() const { return vector2((float)m_info.windowWidth, (float)m_info.windowHeight); }
	
	void useDefaultRenderTarget();
	const std::shared_ptr<RenderTarget>& defaultRenderTarget() const;
	const std::shared_ptr<RasterizerStage>& defaultRasterizer() const { return m_defaultRasterizer; }
	const std::shared_ptr<Sampler>& anisotropicSampler() const { return m_anisotropicSampler; }
	const std::shared_ptr<Sampler>& linearSampler() const { return m_linearSampler; }
	const std::shared_ptr<DepthStencilStage>& disableDepthTest() const { return m_disableDepthTest; }
	const std::shared_ptr<BlendStage>& defaultAlphaBlending() const { return m_defaultAlphaBlending; }
	
	void saveTextureToFile(std::shared_ptr<Texture> texture, const std::string& filename);

protected:
	struct AppInfo
    {
        std::string title;
        int windowWidth;
        int windowHeight;
        int samples;
		D3D_FEATURE_LEVEL featureLevel;
        union
        {
            struct
            {
                unsigned int fullscreen  : 1;
                unsigned int cursor      : 1;
                unsigned int debug       : 1;
            };
            unsigned int all;
        } flags;
		
		AppInfo();
    };

	AppInfo m_info;
	LightManager m_lightManager;
	gui::LabelPtr_T m_fpsLabel;
	gui::LabelPtr_T m_legendLabel;

	void renderGui(double elapsedTime);
	void renderAxes(const matrix44& viewProjection);
	void renderSkybox(Camera& camera, std::shared_ptr<Texture> texture);

private:
	static Application* m_self;
	utils::Timer m_timer;
	Window m_window;
	bool m_isRunning;
	double m_lastTime;
	utils::FpsCounter m_fpsCounter;
	std::string m_legend;
	Pipeline m_pipeline;
	std::weak_ptr<GpuProgram> m_usingGpuProgram;

	IDXGIFactory_T* m_factory;
	IDXGIAdapter_T* m_adapter;
	IDXGIOutput_T* m_output;
	DXGI_MODE_DESC_T m_displayDesc;

	Device m_device;
	D3D_DRIVER_TYPE m_driverType;
	unsigned int m_multisamplingQuality;
	
	std::list<std::weak_ptr<Destroyable> > m_destroyableList;
	
	std::shared_ptr<Line3D> m_axisX;
	std::shared_ptr<Line3D> m_axisY;
	std::shared_ptr<Line3D> m_axisZ;

	std::shared_ptr<RasterizerStage> m_defaultRasterizer;
	std::shared_ptr<DepthStencilStage> m_defaultDepthStencil;
	std::shared_ptr<BlendStage> m_defaultBlending;
	std::shared_ptr<RenderTarget> m_defaultRenderTarget;
	std::shared_ptr<RenderTarget> m_multisamplingRenderTarget;
	std::shared_ptr<Sampler> m_anisotropicSampler;
	std::shared_ptr<Sampler> m_linearSampler;
	std::shared_ptr<DepthStencilStage> m_disableDepthTest;
	std::shared_ptr<BlendStage> m_defaultAlphaBlending;

	void registerDestroyable(std::weak_ptr<Destroyable> ptr);
	void destroyAllDestroyable();

	bool initD3D11();
	bool initFactoryAndAdapter();
	bool isFeatureLevelSupported(D3D_FEATURE_LEVEL level);
	bool findDisplayMode();
	bool initSwapChain(Device& device);
	void present();
	void destroyD3D11();

	bool initGui();
	void destroyGui();
	void initInput();
	void initAxes();

	void setUsingGpuProgram(std::weak_ptr<GpuProgram> program) { m_usingGpuProgram = program; }

	void mainLoop();
};

}
#if defined _WIN32
#define DECLARE_MAIN(A)										 \
int CALLBACK WinMain(HINSTANCE hInstance,					 \
					HINSTANCE hPrevInstance,				 \
					LPSTR lpCmdLine,						 \
					int nCmdShow)							 \
{															 \
	utils::Utils::init();									 \
	utils::Logger::start(utils::Logger::IDE_OUTPUT |		 \
						 utils::Logger::FILE);				 \
    A *app = new A();										 \
	int result = app->run(app, lpCmdLine);					 \
    delete app;												 \
	utils::Logger::finish();								 \
    return result;											 \
}
#else
#error Undefined platform!
#endif