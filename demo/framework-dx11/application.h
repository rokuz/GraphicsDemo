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
#pragma warning(disable:4005)

#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#include <d3d11.h>
#include <string>
#include <list>

#include "CEGUI/CEGUI.h"

#include "matrix.h"
#include "logger.h"
#include "utils.h"
#include "timer.h"

#include "structs.h"
#include "window.h"
#include "autoreleasepool.h"
#include "outputd3d11.h"
#include "destroyable.h"
#include "pipelinestage.h"
#include "rasterizerstage.h"
#include "depthstencilstage.h"
#include "renderTarget.h"
//#include "geometry3D.h"
//#include "line3D.h"
//#include "texture.h"
//#include "gpuprogram.h"
//#include "standardGpuPrograms.h"
//#include "freeCamera.h"
//#include "lightManager.h"
//#include "uniformBuffer.h"

namespace framework
{

class Application
{
	friend class Destroyable;
	friend class PipelineStage;

public:
	Application();
	virtual ~Application(){}

	virtual void init(){}
	virtual void startup(CEGUI::DefaultWindow* root){}
	virtual void render(double elapsedTime){}
	virtual void shutdown(){}
	virtual void onResize(int width, int height){}
	virtual void onKeyButton(int key, int scancode, bool pressed){}
	virtual void onMouseButton(double xpos, double ypos, int button, bool pressed){}
	virtual void onMouseMove(double xpos, double ypos){}

	static Application* Instance();

	int run(Application* self);

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
                unsigned int    fullscreen  : 1;
                unsigned int    vsync       : 1;
                unsigned int    cursor      : 1;
                unsigned int    debug       : 1;
            };
            unsigned int        all;
        } flags;
		
		AppInfo();
    };

	AppInfo m_info;
	//LightManager m_lightManager;

	void exit();

	static std::string getGuiFullName(const std::string& name);
	void renderGui(double elapsedTime);
	void renderAxes(const matrix44& viewProjection);
	PipelineStageManager& getPipelineStageManager() { return m_pipelineManager; }
	const Device& getDevice() const { return m_device; }

	void resize();

private:
	static Application* m_self;
	utils::Timer m_timer;
	Window m_window;
	bool m_isRunning;
	double m_lastTime;
	double m_fpsStorage;
	CEGUI::Renderer* m_guiRenderer;
	CEGUI::DefaultWindow* m_rootWindow;
	CEGUI::Window* m_fpsLabel;
	double m_timeSinceLastFpsUpdate;
	double m_averageFps;
	size_t m_framesCounter;
	PipelineStageManager m_pipelineManager;

	IDXGIFactory* m_factory;
	Device m_device;
	D3D_DRIVER_TYPE m_driverType;
	unsigned int m_multisamplingQuality;

	std::list<std::weak_ptr<Destroyable> > m_destroyableList;
	//std::shared_ptr<Line3D> m_axisX;
	//std::shared_ptr<Line3D> m_axisY;
	//std::shared_ptr<Line3D> m_axisZ;

	std::shared_ptr<RasterizerStage> m_defaultRasterizer;
	std::shared_ptr<DepthStencilStage> m_defaultDepthStencil;
	std::shared_ptr<RenderTarget> m_defaultRenderTarget;

	void registerDestroyable(std::weak_ptr<Destroyable> ptr);
	void destroyAllDestroyable();

	bool initDevice(AuroreleasePool<IUnknown>& autorelease);
	bool isFeatureLevelSupported(D3D_FEATURE_LEVEL level);
	bool initSwapChain(Device& device, AuroreleasePool<IUnknown>& autorelease);
	void present();

	void initGui();
	void destroyGui();
	void initialiseResources();
	void initInput();
	//void initAxes();

	void mainLoop();
	void measureFps(double delta);

	//static void _onMouse(GLFWwindow* window, int button, int action, int mods);
	//static void _onCursor(GLFWwindow* window, double xpos, double ypos);
	//static void _onScroll(GLFWwindow* window, double xoffset, double yoffset);
};

}
#if defined _WIN32
#define DECLARE_MAIN(A)                             \
int CALLBACK WinMain(HINSTANCE hInstance,           \
                     HINSTANCE hPrevInstance,       \
                     LPSTR lpCmdLine,               \
                     int nCmdShow)                  \
{                                                   \
    A *app = new A();                               \
    int result = app->run(app);                     \
    delete app;                                     \
    return result;                                  \
}
#else
#error Undefined platform!
#endif