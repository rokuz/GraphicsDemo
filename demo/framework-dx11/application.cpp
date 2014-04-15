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
#include "CEGUI/RendererModules/Direct3D11/Renderer.h"

namespace framework
{

static const std::string GUI_SKIN = "TaharezLook";

Application::AppInfo::AppInfo() : 
	title("Demo"), 
	windowWidth(1024), 
	windowHeight(768),
	samples(0),
	featureLevel(D3D_FEATURE_LEVEL_11_0)
{
	flags.all = 0;
	flags.fullscreen = 0;
	flags.vsync = 0;
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
	m_guiRenderer(0), 
	m_rootWindow(0), 
	m_fpsLabel(0), 
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

Application* Application::Instance()
{
	return m_self;
}

int Application::run(Application* self)
{
	m_self = self;

	if (!m_timer.init())
	{
		utils::Logger::toLog("Error: could not initialize a timer.\n");
		return EXIT_FAILURE;
	}

	if (!utils::Utils::exists("data/gui"))
	{
		utils::Logger::toLog("Error: could not find gui directory. Probably working directory has not been set correctly (especially if you are running from IDE).\n");
		return EXIT_FAILURE;
	}

	init();
	m_isRunning = true;

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
	if (!StandardGpuPrograms::init(m_device))
	{
		destroyDevice();
		destroyAllDestroyable();
		return EXIT_FAILURE;
	}
	initGui();
	initAxes(m_device);
	m_lightManager.init(m_device);

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
	do
	{
		// process events from the window
		m_window.pollEvents();

		// need to close?
		if (m_window.shouldClose())
		{
			m_isRunning = false;
		}

		// pre-render
		m_pipelineManager.beginFrame(m_device, m_defaultRenderTarget);
		m_defaultRasterizer->apply(m_device);
		m_defaultDepthStencil->apply(m_device);
		m_defaultBlending->apply(m_device);

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
		present();
		m_usingGpuProgram.reset();
		m_pipelineManager.endFrame(m_device);
	} 
	while (m_isRunning);
}

void Application::exit()
{
	m_isRunning = false;
}

void Application::resize()
{
	if (m_guiRenderer != 0)
	{
		m_guiRenderer->setDisplaySize(CEGUI::Sizef((float)m_info.windowWidth, (float)m_info.windowHeight));
	}

	if (m_defaultRasterizer.get() != 0)
	{
		m_defaultRasterizer->clearViewports();
		m_defaultRasterizer->addViewport(framework::RasterizerStage::getDefaultViewport(m_info.windowWidth, m_info.windowHeight));
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
		hr = m_device.device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, m_info.samples, &m_multisamplingQuality);
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

	// init default rasterizer
	m_defaultRasterizer.reset(new framework::RasterizerStage());
	D3D11_RASTERIZER_DESC rastDesc = framework::RasterizerStage::getDefault();
	rastDesc.MultisampleEnable = m_info.samples > 0 ? TRUE : FALSE;
	rastDesc.AntialiasedLineEnable = m_info.samples > 0 ? TRUE : FALSE;
	m_defaultRasterizer->initWithDescription(m_device, rastDesc);
	m_defaultRasterizer->addViewport(framework::RasterizerStage::getDefaultViewport(m_info.windowWidth, m_info.windowHeight));
	if (!m_defaultRasterizer->isValid())
	{
		return false;
	}
	m_defaultRasterizer->apply(m_device);

	// init default depth-stencil
	m_defaultDepthStencil.reset(new framework::DepthStencilStage());
	D3D11_DEPTH_STENCIL_DESC dsDesc = framework::DepthStencilStage::getDefault();
	m_defaultDepthStencil->initWithDescription(m_device, dsDesc);
	if (!m_defaultDepthStencil->isValid())
	{
		return false;
	}

	// init default blending
	m_defaultBlending.reset(new framework::BlendStage());
	D3D11_BLEND_DESC blendDesc = framework::BlendStage::getDefault();
	m_defaultBlending->initWithDescription(m_device, blendDesc);
	if (!m_defaultBlending->isValid())
	{
		return false;
	}

	return true;
}

bool Application::initSwapChain(Device& device)
{
	HRESULT hr = S_OK;

	// parameters
	DXGI_SWAP_CHAIN_DESC state;
	state.BufferDesc.Width = m_info.windowWidth;
	state.BufferDesc.Height = m_info.windowHeight;
	state.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	state.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	state.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	if ((bool)m_info.flags.vsync)
	{
		state.BufferDesc.RefreshRate.Numerator = 60;
		state.BufferDesc.RefreshRate.Denominator = 1;
	}
	else
	{
		state.BufferDesc.RefreshRate.Numerator = 0;
		state.BufferDesc.RefreshRate.Denominator = 1;
	}

	if (m_info.samples > 0)
	{
		state.SampleDesc.Count = m_info.samples;
		state.SampleDesc.Quality = m_multisamplingQuality;
	}
	else
	{
		state.SampleDesc.Count = 1;
		state.SampleDesc.Quality = 0;
	}

	state.BufferUsage = DXGI_USAGE_SHADER_INPUT | DXGI_USAGE_RENDER_TARGET_OUTPUT;
	state.BufferCount = 2;
	state.OutputWindow = m_window.getHandle();
	state.Windowed = ((bool)m_info.flags.fullscreen == false);
	state.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	state.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// initialization
	hr = m_factory->CreateSwapChain(device.device, &state, &device.swapChain);
	if (hr != S_OK)
	{
		utils::Logger::toLog("Error: could not create a swap chain.\n");
		return false;
	}

	// default render target from swap chain
	m_defaultRenderTarget.reset(new framework::RenderTarget());
	m_defaultRenderTarget->initWithSwapChain(device);
	if (!m_defaultRenderTarget->isValid())
	{
		return false;
	}

	return true;
}

void Application::destroyDevice()
{
	if (m_device.swapChain != 0) { m_device.swapChain->Release(); m_device.swapChain = 0; }
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
	if (m_device.swapChain != 0)
	{
		m_device.swapChain->Present(0, 0);
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
			static char buf[100];
			sprintf(buf, "%.2f fps", (float)m_averageFps);
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
	m_pipelineManager.setRenderTarget(m_device, m_defaultRenderTarget);
}

std::string Application::getGuiFullName(const std::string& name)
{
	return GUI_SKIN + name;
}

void Application::renderGui(double elapsedTime)
{
	useDefaultRenderTarget();

	CEGUI::System& gui_system(CEGUI::System::getSingleton());
	gui_system.injectTimePulse((float)elapsedTime);
	m_guiRenderer->beginRendering();
	gui_system.getDefaultGUIContext().draw();
	m_guiRenderer->endRendering();
	CEGUI::WindowManager::getSingleton().cleanDeadPool();
}

void Application::renderAxes(const Device& device, const matrix44& viewProjection)
{
	m_axisX->renderWithStandardGpuProgram(device, viewProjection, vector4(1, 0, 0, 1));
	m_axisY->renderWithStandardGpuProgram(device, viewProjection, vector4(0, 1, 0, 1));
	m_axisZ->renderWithStandardGpuProgram(device, viewProjection, vector4(0, 0, 1, 1));
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

void Application::initGui()
{
	m_guiRenderer = &CEGUI::Direct3D11Renderer::create(m_device.device, m_device.context);
	CEGUI::System::create(*m_guiRenderer);
	initialiseResources();

	CEGUI::System& gui_system(CEGUI::System::getSingleton());
	CEGUI::GUIContext* guiContext = &gui_system.getDefaultGUIContext();
	CEGUI::SchemeManager::getSingleton().createFromFile(getGuiFullName(".scheme").c_str());
	guiContext->getMouseCursor().setDefaultImage(getGuiFullName("/MouseArrow").c_str());
	guiContext->getMouseCursor().hide();

	CEGUI::WindowManager& winMgr = CEGUI::WindowManager::getSingleton();
	m_rootWindow = (CEGUI::DefaultWindow*)winMgr.createWindow("DefaultWindow", "Root");
	CEGUI::Font& defaultFont = CEGUI::FontManager::getSingleton().createFromFile("DejaVuSans-12.font");
	guiContext->setDefaultFont(&defaultFont);
	guiContext->setRootWindow(m_rootWindow);

	m_fpsLabel = (CEGUI::Window*)winMgr.createWindow(getGuiFullName("/Label").c_str());
	m_rootWindow->addChild(m_fpsLabel);

	m_fpsLabel->setPosition(CEGUI::UVector2(CEGUI::UDim(1.0f, -150.0f), cegui_reldim(0.0)));
	m_fpsLabel->setSize(CEGUI::USize(cegui_absdim(150.0f), cegui_absdim(25.0f)));
	m_fpsLabel->setProperty("HorzFormatting", "RightAligned");
	m_fpsLabel->setText("0 fps");
}

void Application::destroyGui()
{
	CEGUI::System::destroy();
	CEGUI::Direct3D11Renderer::destroy(static_cast<CEGUI::Direct3D11Renderer&>(*m_guiRenderer));
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
		CEGUI::Key::Scan ceguiKey = (CEGUI::Key::Scan)(key);
		if (CEGUI::System::getSingletonPtr())
		{
			CEGUI::System& gui_system(CEGUI::System::getSingleton());
			if (pressed)
			{
				gui_system.getDefaultGUIContext().injectKeyDown(ceguiKey);
			}
			else
			{
				gui_system.getDefaultGUIContext().injectKeyUp(ceguiKey);
			}
		}

		// hint to exit on ESC
		if (ceguiKey == CEGUI::Key::Escape) m_isRunning = false;

		onKeyButton(key, scancode, pressed);
	});

	m_window.setCharHandler([this](int codepoint)
	{
		if (CEGUI::System::getSingletonPtr())
		{
			CEGUI::System& gui_system(CEGUI::System::getSingleton());
			gui_system.getDefaultGUIContext().injectChar((CEGUI::String::value_type)codepoint);
		}
	});

	m_window.setMouseHandler([this](double xpos, double ypos, double zdelta, int button, bool pressed)
	{
		if (CEGUI::System::getSingletonPtr())
		{
			CEGUI::System& gui_system(CEGUI::System::getSingleton());
			if (button >= 0)
			{
				if (pressed)
				{
					gui_system.getDefaultGUIContext().injectMouseButtonDown((CEGUI::MouseButton)button);
				}
				else
				{
					gui_system.getDefaultGUIContext().injectMouseButtonUp((CEGUI::MouseButton)button);
				}
			}
			else
			{
				gui_system.getDefaultGUIContext().injectMousePosition((float)xpos, (float)ypos);
				if (fabs(zdelta) > 1e-4)
				{
					gui_system.getDefaultGUIContext().injectMouseWheelChange((float)zdelta);
				}
			}
		}

		if (button >= 0)
		{
			onMouseButton(xpos, ypos, button, pressed);
		}
		else
		{
			onMouseMove(xpos, ypos);
		}
	});
}

void Application::initAxes(const Device& device)
{	
	#define INIT_POINTS(name, point) std::vector<vector3> name; name.push_back(vector3()); name.push_back(point);
	INIT_POINTS(points_x, vector3(20, 0, 0));
	INIT_POINTS(points_y, vector3(0, 20, 0));
	INIT_POINTS(points_z, vector3(0, 0, 20));
	#undef INIT_POINTS

	m_axisX.reset(new Line3D());
	m_axisX->initWithArray(device, points_x);
		
	m_axisY.reset(new Line3D());
	m_axisY->initWithArray(device, points_y);

	m_axisZ.reset(new Line3D());
	m_axisZ->initWithArray(device, points_z);
}

void Application::initialiseResources()
{
	CEGUI::DefaultResourceProvider* rp = static_cast<CEGUI::DefaultResourceProvider*>(CEGUI::System::getSingleton().getResourceProvider());

	rp->setResourceGroupDirectory("schemes", "data/gui/schemes");
	rp->setResourceGroupDirectory("imagesets", "data/gui/imagesets");
	rp->setResourceGroupDirectory("fonts", "data/gui/fonts");
	rp->setResourceGroupDirectory("layouts", "data/gui/layouts");
	rp->setResourceGroupDirectory("looknfeels", "data/gui/looknfeel");
	rp->setResourceGroupDirectory("lua_scripts", "data/gui/lua_scripts");
	rp->setResourceGroupDirectory("schemas", "data/gui/xml_schemas");   
	rp->setResourceGroupDirectory("animations", "data/gui/animations");

	CEGUI::ImageManager::setImagesetDefaultResourceGroup("imagesets");
	CEGUI::Font::setDefaultResourceGroup("fonts");
	CEGUI::Scheme::setDefaultResourceGroup("schemes");
	CEGUI::WidgetLookManager::setDefaultResourceGroup("looknfeels");
	CEGUI::WindowManager::setDefaultResourceGroup("layouts");
	CEGUI::ScriptModule::setDefaultResourceGroup("lua_scripts");
	CEGUI::AnimationManager::setDefaultResourceGroup("animations");

	CEGUI::XMLParser* parser = CEGUI::System::getSingleton().getXMLParser();
	if (parser->isPropertyPresent("SchemaDefaultResourceGroup"))
	{
		parser->setProperty("SchemaDefaultResourceGroup", "schemas");
	}
}

}