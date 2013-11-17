#include <wrl/client.h>
#include <memory>
#include <agile.h>

#include "corewrapper.h"

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;

// Helper class for basic timing.
ref class BasicTimer sealed
{
public:
	// Initializes internal timer values.
	BasicTimer()
	{
		if (!QueryPerformanceFrequency(&m_frequency))
		{
			throw ref new Platform::FailureException();
		}
		Reset();
	}
	
	// Reset the timer to initial values.
	void Reset()
	{
		Update();
		m_startTime = m_currentTime;
		m_total = 0.0f;
		m_delta = 1.0f / 60.0f;
	}
	
	// Update the timer's internal values.
	void Update()
	{
		if (!QueryPerformanceCounter(&m_currentTime))
		{
			throw ref new Platform::FailureException();
		}
		
		m_total = static_cast<float>(
			static_cast<double>(m_currentTime.QuadPart - m_startTime.QuadPart) /
			static_cast<double>(m_frequency.QuadPart)
			);
		
		if (m_lastTime.QuadPart == m_startTime.QuadPart)
		{
			// If the timer was just reset, report a time delta equivalent to 60Hz frame time.
			m_delta = 1.0f / 60.0f;
		}
		else
		{
			m_delta = static_cast<float>(
				static_cast<double>(m_currentTime.QuadPart - m_lastTime.QuadPart) /
				static_cast<double>(m_frequency.QuadPart)
				);
		}
		
		m_lastTime = m_currentTime;
	}
	
	// Duration in seconds between the last call to Reset() and the last call to Update().
	property float Total
	{
		float get() { return m_total; }
	}
	
	// Duration in seconds between the previous two calls to Update().
	property float Delta
	{
		float get() { return m_delta; }
	}

private:
	LARGE_INTEGER m_frequency;
	LARGE_INTEGER m_currentTime;
	LARGE_INTEGER m_startTime;
	LARGE_INTEGER m_lastTime;
	float m_total;
	float m_delta;
};

ref class WagicApp sealed : public Windows::ApplicationModel::Core::IFrameworkView
{
public:
	WagicApp();
	
	// IFrameworkView Methods.
	virtual void Initialize(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView);
	virtual void SetWindow(Windows::UI::Core::CoreWindow^ window);
	virtual void Load(Platform::String^ entryPoint);
	virtual void Run();
	virtual void Uninitialize();

protected:
	// Event Handlers.
	void OnWindowSizeChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::WindowSizeChangedEventArgs^ args);
	void OnLogicalDpiChanged(Platform::Object^ sender);
	void OnActivated(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView, Windows::ApplicationModel::Activation::IActivatedEventArgs^ args);
	void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ args);
	void OnResuming(Platform::Object^ sender, Platform::Object^ args);
	void OnWindowClosed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::CoreWindowEventArgs^ args);
	void OnVisibilityChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ args);
	void OnPointerPressed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);
	void OnPointerMoved(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);
	void OnKeyDown(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args);
	void OnKeyUp(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args);
	void OnPointerWheelChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);

private:
	WagicCore m_Wagic;
	bool m_windowClosed;
	bool m_windowVisible;
};

ref class WagicAppSource sealed : Windows::ApplicationModel::Core::IFrameworkViewSource
{
public:
	virtual Windows::ApplicationModel::Core::IFrameworkView^ CreateView();
};



WagicApp::WagicApp() :
	m_windowClosed(false),
	m_windowVisible(true)
{
}

void WagicApp::Initialize(CoreApplicationView^ applicationView)
{
	applicationView->Activated +=
        ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &WagicApp::OnActivated);

	CoreApplication::Suspending +=
        ref new EventHandler<SuspendingEventArgs^>(this, &WagicApp::OnSuspending);

	CoreApplication::Resuming +=
        ref new EventHandler<Platform::Object^>(this, &WagicApp::OnResuming);

}

void WagicApp::SetWindow(CoreWindow^ window)
{
	window->SizeChanged += 
        ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &WagicApp::OnWindowSizeChanged);

	window->VisibilityChanged +=
		ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &WagicApp::OnVisibilityChanged);

	window->Closed += 
        ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &WagicApp::OnWindowClosed);

	window->PointerCursor = ref new CoreCursor(CoreCursorType::Arrow, 0);

	window->PointerPressed +=
		ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &WagicApp::OnPointerPressed);

	window->PointerMoved +=
		ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &WagicApp::OnPointerMoved);

	window->KeyDown +=
		ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &WagicApp::OnKeyDown);

	window->KeyDown +=
		ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &WagicApp::OnKeyUp);

	window->PointerWheelChanged +=
		ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &WagicApp::OnPointerWheelChanged);

	m_Wagic.initApp();
	m_Wagic.onWindowResize((void*)window, window->Bounds.Width, window->Bounds.Height);
}

void WagicApp::Load(Platform::String^ entryPoint)
{
}

void WagicApp::Run()
{
	BasicTimer^ timer = ref new BasicTimer();

	while (!m_windowClosed)
	{
		if (m_windowVisible)
		{
			timer->Update();
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
			if(m_Wagic.onUpdate())
				m_Wagic.onRender();
			else
				m_windowClosed = true;
		}
		else
		{
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
		}
	}
}

void WagicApp::Uninitialize()
{
}

void WagicApp::OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
{
	m_Wagic.onWindowResize((void*)sender, args->Size.Width, args->Size.Height);
}

void WagicApp::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	m_windowVisible = args->Visible;
}

void WagicApp::OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
{
	m_windowClosed = true;
}

void WagicApp::OnPointerPressed(CoreWindow^ sender, PointerEventArgs^ args)
{
	WagicCore::PointerId pointerId = WagicCore::LEFT;
	if(args->CurrentPoint->Properties->IsLeftButtonPressed)
	{
		pointerId = WagicCore::LEFT;
	}
	else if (args->CurrentPoint->Properties->IsRightButtonPressed)
	{
		pointerId = WagicCore::RIGHT;
	}
	else if (args->CurrentPoint->Properties->IsMiddleButtonPressed)
	{
		pointerId = WagicCore::MIDLE;
	}

	m_Wagic.onPointerPressed(pointerId, (int)args->CurrentPoint->Position.X, (int)args->CurrentPoint->Position.Y);
}

void WagicApp::OnPointerMoved(CoreWindow^ sender, PointerEventArgs^ args)
{
	m_Wagic.onPointerMoved(WagicCore::LEFT, (int)args->CurrentPoint->Position.X, (int)args->CurrentPoint->Position.Y);
}

void WagicApp::OnKeyDown(CoreWindow^ sender, KeyEventArgs^ args)
{
	WPARAM param = (WPARAM) args->VirtualKey;

	m_Wagic.onKeyDown(param);
}

void WagicApp::OnKeyUp(CoreWindow^ sender, KeyEventArgs^ args)
{
	WPARAM param = (WPARAM) args->VirtualKey;

	m_Wagic.onKeyUp(param);
}

void WagicApp::OnPointerWheelChanged(CoreWindow^ sender, PointerEventArgs^ args)
{
	if(args->CurrentPoint->Properties->IsHorizontalMouseWheel)
		m_Wagic.onWheelChanged(0, 3*args->CurrentPoint->Properties->MouseWheelDelta);
	else
		m_Wagic.onWheelChanged(3*args->CurrentPoint->Properties->MouseWheelDelta, 0);
}

void WagicApp::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
{
	CoreWindow::GetForCurrentThread()->Activate();
}

void WagicApp::OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
{
	// Save app state asynchronously after requesting a deferral. Holding a deferral
	// indicates that the application is busy performing suspending operations. Be
	// aware that a deferral may not be held indefinitely. After about five seconds,
	// the app will be forced to exit.
	SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();
}
 
void WagicApp::OnResuming(Platform::Object^ sender, Platform::Object^ args)
{
	// Restore any data or state that was unloaded on suspend. By default, data
	// and state are persisted when resuming from suspend. Note that this event
	// does not occur if the app was previously terminated.
}

IFrameworkView^ WagicAppSource::CreateView()
{
    return ref new WagicApp();
}

[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
	auto wagicAppSource = ref new WagicAppSource();
	CoreApplication::Run(wagicAppSource);
	return 0;
}

bool JGEToggleFullscreen()
{
   return true;
}

