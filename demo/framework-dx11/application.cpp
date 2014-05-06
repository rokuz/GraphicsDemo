/*
 * Copyright (c) 2014 Roman Kuznetsov 
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

#include "application.h"
#include <algorithm>
#include "guirenderer.h"

namespace framework
{

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

const DXGI_FORMAT DISPLAY_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;
const UINT BACK_BUFFERS_COUNT = 2;

Application::AppInfo::AppInfo() : 
	title("Demo"), 
	windowWidth(1024), 
	windowHeight(768),
	samples(0),
	featureLevel(D3D_FEATURE_LEVEL_11_0)
{
	flags.all = 0;
	flags.fullscreen = 0;
	flags.cursor = 1;
	#ifdef _DEBUG
	flags.debug = 1;
	#else
	flags.debug = 0;
	#endif
}

Application* Application::m_self = 0;

Application::Application() : 
	m_isRunning(false), 
	m_lastTime(0), 
	m_fpsStorage(0), 
	m_timeSinceLastFpsUpdate(0),
	m_averageFps(0), 
	m_framesCounter(0),
	m_factory(0),
	m_adapter(0),
	m_driverType(D3D_DRIVER_TYPE_HARDWARE),
	m_multisamplingQuality(0)
{
}

Application* Application::instance()
{
	return m_self;
}

int Application::run(Application* self, const std::string& commandLine)
{
	m_self = self;

	if (!m_timer.init())
	{
		utils::Logger::toLog("Error: could not initialize a timer.\n");
		return EXIT_FAILURE;
	}

	if (!utils::Utils::exists("data"))
	{
		utils::Logger::toLog("Error: could not find data directory. Probably working directory has not been set correctly (especially if you are running from IDE).\n");
		return EXIT_FAILURE;
	}

	// initialize app
	m_isRunning = true;
	auto params = utils::Utils::parseCommandLine(commandLine);
	init(params);
	if (!m_isRunning) { return EXIT_SUCCESS; }

	// create a window
	if (!m_window.init(m_info.windowWidth, m_info.windowHeight, m_info.title))
	{
		utils::Logger::toLog("Error: could not create window.\n");
		return EXIT_FAILURE;
	}
	m_window.setCursorVisibility((bool)m_info.flags.cursor);

	initInput();

	// init D3D device
	if (!initDevice())
	{
		destroyDevice();
		destroyAllDestroyable();
		return EXIT_FAILURE;
	}

	// init other subsystems
	if (!StandardGpuPrograms::init() || !initGui())
	{
		destroyDevice();
		destroyAllDestroyable();
		return EXIT_FAILURE;
	}
	initAxes();
	m_lightManager.init();

	// user-defined initialization
	startup(m_rootWindow);

	mainLoop();

	// destroy everything
	shutdown();
	destroyAllDestroyable();
	destroyGui();
	destroyDevice();
	m_window.destroy();
	
	return EXIT_SUCCESS;
}

void Application::mainLoop()
{
	if (!m_isRunning) return;

	do
	{
		TRACE_BLOCK("_Frame");

		// process events from the window
		m_window.pollEvents();

		// need to close?
		if (m_window.shouldClose())
		{
			m_isRunning = false;
		}

		// pre-render
		m_pipeline.beginFrame(defaultRenderTarget());
		m_defaultRasterizer->apply();
		m_defaultDepthStencil->apply();
		m_defaultBlending->apply();

		// render frame
		if (fabs(m_lastTime) < 1e-7)
		{
			// the first frame
			render(0);
			renderGui(0);

			m_lastTime = m_timer.getTime();
		}
		else
		{
			double curTime = m_timer.getTime();
			double delta = curTime - m_lastTime;

			// fps counter
			measureFps(delta);

			// rendering
			render(delta);
			renderGui(delta);

			m_lastTime = curTime;
		}

		// post-render
		m_usingGpuProgram.reset();
		m_pipeline.endFrame();
		present();
	} 
	while (m_isRunning);
}

void Application::exit()
{
	m_isRunning = false;
}

void Application::resize()
{
	gui::UIManager::instance().setScreenSize((size_t)m_info.windowWidth, (size_t)m_info.windowHeight);

	if (m_defaultRasterizer.get() != 0)
	{
		m_defaultRasterizer->getViewports().clear();
		m_defaultRasterizer->getViewports().push_back(framework::RasterizerStage::getDefaultViewport(m_info.windowWidth, m_info.windowHeight));
	}

	onResize(m_info.windowWidth, m_info.windowHeight);
}

bool Application::initDevice()
{
	HRESULT hr = S_OK;

	// dxgi factory
	IDXGIFactory* factory = 0;
	hr = CreateDXGIFactory1(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&factory));
	if (hr != S_OK)
	{
		utils::Logger::toLog("Error: could not create DXGI factory.\n");
		return false;
	}
	m_factory = factory;
	hr = m_factory->MakeWindowAssociation(m_window.getHandle(), DXGI_MWA_NO_ALT_ENTER);

	// adapter
	IDXGIAdapter* adapter = 0;
	if (factory->EnumAdapters(0, &adapter) == DXGI_ERROR_NOT_FOUND)
	{
		utils::Logger::toLog("Error: could not find a graphics adapter.\n");
		return false;
	}
	m_adapter = adapter;

	// adapter description
	DXGI_ADAPTER_DESC desc;
	if (adapter->GetDesc(&desc) != S_OK)
	{
		utils::Logger::toLog("Error: could not find a graphics adapter.\n");
		return false;
	}
	utils::Logger::toLog(L"Video adapter: " + std::wstring(desc.Description) + L".\n");

	// check for feature level
	if (!isFeatureLevelSupported(m_info.featureLevel))
	{
		utils::Logger::toLog("Error: chosen feature level is unsupported.\n");
		return false;
	}

	// get display modes
	if (!findDisplayMode())
	{
		return false;
	}

	UINT CreateDeviceFlags = 0;
	if (m_info.flags.debug)
	{
		CreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	}

	D3D_FEATURE_LEVEL level[] = { m_info.featureLevel };
	D3D_FEATURE_LEVEL createdLevel;
	hr = D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr, CreateDeviceFlags, level, 1, D3D11_SDK_VERSION, &m_device.device, &createdLevel, &m_device.context);
	if (hr != S_OK)
	{
		utils::Logger::toLog("Error: could not create D3D device.\n");
		return false;
	}
	m_device.featureLevel = createdLevel;
	utils::Logger::toLogWithFormat("Feature level: %s.\n", toString(m_info.featureLevel).c_str());

	if (m_info.flags.debug)
	{
		hr = m_device.device->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&m_device.debugger));
		if (hr != S_OK)
		{
			utils::Logger::toLog("Error: could not get D3D debugger interface.\n");
			return false;
		}
	}

	// check for multisampling level
	if (m_info.samples != 0)
	{
		m_multisamplingQuality = 0;
		hr = m_device.device->CheckMultisampleQualityLevels(DISPLAY_FORMAT, m_info.samples, &m_multisamplingQuality);
		if (hr != S_OK || m_multisamplingQuality == 0)
		{
			utils::Logger::toLogWithFormat("Error: %dx-multisampling is not supported.\n", m_info.samples);
			return false;
		}
		m_multisamplingQuality--;
	}

	// swap chain
	if (!initSwapChain(m_device))
	{
		return false;
	}

	// disable alt+enter
	m_factory->MakeWindowAssociation(m_window.getHandle(), DXGI_MWA_NO_ALT_ENTER);

	// init default rasterizer
	m_defaultRasterizer.reset(new framework::RasterizerStage());
	D3D11_RASTERIZER_DESC rastDesc = framework::RasterizerStage::getDefault();
	rastDesc.MultisampleEnable = m_info.samples > 0 ? TRUE : FALSE;
	rastDesc.AntialiasedLineEnable = m_info.samples > 0 ? TRUE : FALSE;
	m_defaultRasterizer->initWithDescription(rastDesc);
	m_defaultRasterizer->getViewports().push_back(framework::RasterizerStage::getDefaultViewport(m_info.windowWidth, m_info.windowHeight));
	if (!m_defaultRasterizer->isValid())
	{
		return false;
	}
	m_defaultRasterizer->apply();

	// init default depth-stencil
	m_defaultDepthStencil.reset(new framework::DepthStencilStage());
	D3D11_DEPTH_STENCIL_DESC dsDesc = framework::DepthStencilStage::getDefault();
	m_defaultDepthStencil->initWithDescription(dsDesc);
	if (!m_defaultDepthStencil->isValid())
	{
		return false;
	}

	// init default blending
	m_defaultBlending.reset(new framework::BlendStage());
	D3D11_BLEND_DESC blendDesc = framework::BlendStage::getDefault();
	m_defaultBlending->initWithDescription(blendDesc);
	if (!m_defaultBlending->isValid())
	{
		return false;
	}

	// init default samplers
	m_anisotropicSampler.reset(new framework::Sampler());
	D3D11_SAMPLER_DESC anisoSamplerDesc = framework::Sampler::getDefault();
	anisoSamplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	anisoSamplerDesc.MaxAnisotropy = 16;
	m_anisotropicSampler->initWithDescription(anisoSamplerDesc);
	if (!m_anisotropicSampler->isValid())
	{
		return false;
	}

	m_linearSampler.reset(new framework::Sampler());
	D3D11_SAMPLER_DESC linearSamplerDesc = framework::Sampler::getDefault();
	linearSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	m_linearSampler->initWithDescription(linearSamplerDesc);
	if (!m_linearSampler->isValid())
	{
		return false;
	}

	return true;
}

bool Application::findDisplayMode()
{
	HRESULT hr = S_OK;

	IDXGIOutput* output = NULL; 
	hr = m_adapter->EnumOutputs(0, &output);
	if (hr != S_OK)
	{
		utils::Logger::toLog("Error: could not enumarate outputs.\n");
		return false;
	}
	UINT numModes = 0;
	DXGI_FORMAT format = DISPLAY_FORMAT;
	hr = output->GetDisplayModeList(format, 0, &numModes, NULL);
	if (hr != S_OK)
	{
		utils::Logger::toLog("Error: could not get display modes number.\n");
		return false;
	}
	std::vector<DXGI_MODE_DESC> displayModes;
	displayModes.resize(numModes);
	hr = output->GetDisplayModeList(format, 0, &numModes, displayModes.data());
	if (hr != S_OK)
	{
		utils::Logger::toLog("Error: could not get display modes number.\n");
		return false;
	}
	output->Release();
	
	auto it = std::find_if(displayModes.begin(), displayModes.end(), [&](const DXGI_MODE_DESC& desc)
	{
		return desc.Width == (UINT)m_info.windowWidth && desc.Height == (UINT)m_info.windowHeight;
	});
	if (it == displayModes.end())
	{
		utils::Logger::toLog("Error: could not find appropriate display mode.\n");
		return false;
	}
	m_displayDesc = *it;
	m_displayDesc.Format = DXGI_FORMAT_UNKNOWN;
	m_displayDesc.RefreshRate.Numerator = 0;
	m_displayDesc.RefreshRate.Denominator = 0;

	return true;
}

bool Application::initSwapChain(Device& device)
{
	HRESULT hr = S_OK;

	// parameters
	DXGI_SWAP_CHAIN_DESC state;
	state.BufferDesc.Width = m_displayDesc.Width;
	state.BufferDesc.Height = m_displayDesc.Height;
	state.BufferDesc.Format = DISPLAY_FORMAT;
	state.BufferDesc.ScanlineOrdering = m_displayDesc.ScanlineOrdering;
	state.BufferDesc.Scaling = m_displayDesc.Scaling;
	state.BufferDesc.RefreshRate.Numerator = m_displayDesc.RefreshRate.Numerator;
	state.BufferDesc.RefreshRate.Denominator = m_displayDesc.RefreshRate.Denominator;
	state.SampleDesc.Count = 1;
	state.SampleDesc.Quality = 0;
	state.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	state.BufferCount = BACK_BUFFERS_COUNT;
	state.OutputWindow = m_window.getHandle();
	state.Windowed = TRUE;
	state.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	state.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// initialization
	hr = m_factory->CreateSwapChain(device.device, &state, &device.swapChain);
	if (hr != S_OK)
	{
		utils::Logger::toLog("Error: could not create a swap chain.\n");
		return false;
	}

	// set fullscreen
	if (m_info.flags.fullscreen)
	{
		hr = m_device.swapChain->ResizeTarget(&m_displayDesc);
		if (hr != S_OK)
		{
			utils::Logger::toLog("Error: failed ResizeTarget in swap chain.\n");
			return false;
		}
		
		hr = m_device.swapChain->SetFullscreenState(TRUE, NULL);
		if (hr != S_OK)
		{
			utils::Logger::toLog("Error: failed SetFullscreenState in swap chain.\n");
			return false;
		}

		hr = m_device.swapChain->ResizeBuffers(BACK_BUFFERS_COUNT, m_displayDesc.Width, m_displayDesc.Height, m_displayDesc.Format, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
		if (hr != S_OK)
		{
			utils::Logger::toLog("Error: failed ResizeBuffers in swap chain.\n");
			return false;
		}
	}

	// default render target from swap chain
	m_defaultRenderTarget.reset(new framework::RenderTarget());
	m_defaultRenderTarget->initWithSwapChain(device, m_info.samples == 0);
	if (!m_defaultRenderTarget->isValid())
	{
		return false;
	}

	// multisampling render target
	if (m_info.samples > 0)
	{
		auto desc = RenderTarget::getDefaultDesc(m_displayDesc.Width, m_displayDesc.Height, DISPLAY_FORMAT);
		desc.SampleDesc.Count = m_info.samples;
		desc.SampleDesc.Quality = m_multisamplingQuality;

		m_multisamplingRenderTarget.reset(new framework::RenderTarget());
		m_multisamplingRenderTarget->initWithDescription(desc, true);
		if (!m_multisamplingRenderTarget->isValid())
		{
			return false;
		}
	}

	return true;
}

void Application::destroyDevice()
{
	if (m_device.swapChain != 0) 
	{ 
		m_device.swapChain->SetFullscreenState(FALSE, NULL);
		m_device.swapChain->Release(); 
		m_device.swapChain = 0; 
	}
	if (m_device.context != 0) 
	{ 
		m_device.context->ClearState();
		m_device.context->Flush();
		m_device.context->Release(); 
		m_device.context = 0; 
	}
	if (m_device.debugger != 0) 
	{ 
		//m_device.debugger->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
		m_device.debugger->Release(); 
		m_device.debugger = 0; 
	}
	
	if (m_device.device != 0) { m_device.device->Release(); m_device.device = 0; }
	if (m_adapter != 0) { m_adapter->Release(); m_adapter = 0; }
	if (m_factory != 0) { m_factory->Release(); m_factory = 0; }
}

void Application::present()
{
	TRACE_FUNCTION

	if (m_device.context != 0 && m_device.swapChain != 0)
	{
		// resolve subresource in case of multisampling
		if (m_info.samples > 0)
		{
			m_device.context->ResolveSubresource(m_defaultRenderTarget->getTexture(0), 0, m_multisamplingRenderTarget->getTexture(0), 0, DISPLAY_FORMAT);
		}

		HRESULT hr = m_device.swapChain->Present(0, 0);
		if (hr != S_OK && hr != DXGI_STATUS_OCCLUDED)
		{
			utils::Logger::toLog("Error: application was stopped bacause of Present failure.\n");
			m_isRunning = false;
		}
	}
}

void Application::measureFps(double delta)
{
	m_timeSinceLastFpsUpdate += delta;
	if (m_timeSinceLastFpsUpdate >= 1.0)
	{
		m_averageFps = m_fpsStorage / (m_framesCounter > 0 ? (double)m_framesCounter : 1.0);
		if (m_fpsLabel != 0)
		{
			static wchar_t buf[100];
			swprintf(buf, L"%.2f fps", (float)m_averageFps);
			m_fpsLabel->setText(buf);
		}

		m_timeSinceLastFpsUpdate -= 1.0;
		m_framesCounter = 0;
		m_fpsStorage = 0;
	}
	else
	{
		m_framesCounter++;
		m_fpsStorage += (1.0 / delta);
	}
}

void Application::useDefaultRenderTarget()
{
	m_pipeline.setRenderTarget(defaultRenderTarget());
}

void Application::renderGui(double elapsedTime)
{
	//TRACE_FUNCTION
	//useDefaultRenderTarget();

	//TODO

	gui::UIManager::instance().injectFrameTime(elapsedTime);
}

void Application::renderAxes(const matrix44& viewProjection)
{
	m_axisX->renderWithStandardGpuProgram(viewProjection, vector4(1, 0, 0, 1));
	m_axisY->renderWithStandardGpuProgram(viewProjection, vector4(0, 1, 0, 1));
	m_axisZ->renderWithStandardGpuProgram(viewProjection, vector4(0, 0, 1, 1));
}

void Application::registerDestroyable(std::weak_ptr<Destroyable> ptr)
{
	if (!ptr.expired())
	{
		auto sptr = ptr.lock();
		auto fptr = std::find_if(m_destroyableList.begin(), m_destroyableList.end(), [=](std::weak_ptr<Destroyable> ptr2)
		{
			if (ptr2.expired()) return false;
			return sptr == ptr2.lock();
		});

		if (fptr == m_destroyableList.end())
		{
			m_destroyableList.push_back(ptr);
		}
	}
}

void Application::destroyAllDestroyable()
{
	for (auto it = m_destroyableList.begin(); it != m_destroyableList.end(); ++it)
	{
		if (!(*it).expired()) it->lock()->destroy();
	}
	m_destroyableList.clear();
}

bool Application::initGui()
{
	std::shared_ptr<UIResourcesFactoryD3D11> factory(new UIResourcesFactoryD3D11());
	if (!gui::UIManager::instance().init((size_t)m_info.windowWidth, (size_t)m_info.windowHeight, factory))
	{
		return false;
	}
	
	m_rootWindow = gui::UIManager::instance().root();

	// create a label to show fps statistics
	m_fpsLabel = gui::UIManager::instance().createLabel(gui::Coords::Coords(1.0f, -150.0f, 0.0f, 0.0f),
														gui::Coords::Absolute(150.0f, 25.0f),
														gui::RightAligned, gui::TopAligned, L"0 fps");
	m_rootWindow->addChild(m_fpsLabel);

	// create a label to show the legend
	m_legendLabel = gui::UIManager::instance().createLabel(gui::Coords::Coords(0.0f, 0.0f, 0.0f, 0.0f),
														   gui::Coords::Absolute(500.0f, 500.0f),
														   gui::LeftAligned, gui::TopAligned,
														   utils::Utils::toUnicode(m_legend));
	m_rootWindow->addChild(m_legendLabel);

	return true;
}

void Application::destroyGui()
{
	gui::UIManager::instance().cleanup();
}

void Application::setLegend(const std::string& legend)
{
	m_legend = legend;
	if (m_legendLabel != 0)
	{
		m_legendLabel->setVisible(!m_legend.empty());
		m_legendLabel->setText(utils::Utils::toUnicode(m_legend.c_str()));
	}
}

bool Application::isFeatureLevelSupported(D3D_FEATURE_LEVEL level)
{
	D3D_FEATURE_LEVEL FeatureLevel;
	HRESULT hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, nullptr, &FeatureLevel, nullptr);
	if (FAILED(hr)) return false;

	return level <= FeatureLevel;
}

void Application::initInput()
{
	m_window.setKeyboardHandler([this](int key, int scancode, bool pressed)
	{
		InputKeys::Scan iKey = (InputKeys::Scan)(key);
		if (pressed)
		{
			gui::UIManager::instance().injectKeyDown(iKey);
		}
		else
		{
			gui::UIManager::instance().injectKeyUp(iKey);
		}

		// hint to exit on ESC
		if (iKey == InputKeys::Escape) m_isRunning = false;

		onKeyButton(key, scancode, pressed);
	});

	m_window.setCharHandler([this](int codepoint)
	{
		gui::UIManager::instance().injectChar(codepoint);
	});

	m_window.setMouseHandler([this](double xpos, double ypos, double zdelta, int button, bool pressed)
	{
		if (button >= 0)
		{
			if (pressed)
			{
				gui::UIManager::instance().injectMouseButtonDown((InputKeys::MouseButton)button);
			}
			else
			{
				gui::UIManager::instance().injectMouseButtonUp((InputKeys::MouseButton)button);
			}

			onMouseButton(xpos, ypos, button, pressed);
		}
		else
		{
			gui::UIManager::instance().injectMousePosition((float)xpos, (float)ypos);
			if (fabs(zdelta) > 1e-4)
			{
				gui::UIManager::instance().injectMouseWheelChange((float)zdelta);
			}

			onMouseMove(xpos, ypos);
		}
	});
}

void Application::initAxes()
{	
	#define INIT_POINTS(name, point) std::vector<vector3> name; name.push_back(vector3()); name.push_back(point);
	INIT_POINTS(points_x, vector3(20, 0, 0));
	INIT_POINTS(points_y, vector3(0, 20, 0));
	INIT_POINTS(points_z, vector3(0, 0, 20));
	#undef INIT_POINTS

	m_axisX.reset(new Line3D());
	m_axisX->initWithArray(points_x);
		
	m_axisY.reset(new Line3D());
	m_axisY->initWithArray(points_y);

	m_axisZ.reset(new Line3D());
	m_axisZ->initWithArray(points_z);
}

const std::shared_ptr<RenderTarget>& Application::defaultRenderTarget() const
{
	if (m_multisamplingRenderTarget.get() != 0) 
		return m_multisamplingRenderTarget;

	return m_defaultRenderTarget;
}

}