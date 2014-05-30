#include "stdafx.h"
#include "ZenDiaryApp.h"

namespace ZenDiary
{
	namespace App
	{
		ZenDiaryApp::ZenDiaryApp() : 
			m_core(nullptr),
			m_window(nullptr)
		{
			m_terminate.store(false);
		}

		ZenDiaryApp::~ZenDiaryApp()
		{

		}

		ZD_STATUS ZenDiaryApp::Initialize(const std::string &argv)
		{
			Helpers::Serialization::FromFile(m_settings, m_settings_path);			

			srand(static_cast<uint_t>(time(nullptr)));

			ZD_RETURN_IF_FAILED(InitializeDirectories());
			ZD_RETURN_IF_FAILED(InitializeJsHandlers());
			ZD_RETURN_IF_FAILED(InitializeWindow());
			
			return ZD_NOERROR;
		}

		ZD_STATUS ZenDiaryApp::Deinitialize()
		{
			Helpers::Serialization::ToFile(m_settings, m_settings_path);
			FreeWindow();
			return ZD_NOERROR;
		}

		ZD_STATUS ZenDiaryApp::Run()
		{
			MSG msg = { 0 };

			while (!m_terminate.load())
			{
				while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
				{
					if (msg.message == WM_QUIT)
					{
						Terminate();
					}

					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}

				ZD_SAFE_CALL(m_core)->Update();
				Sleep(1);
			}
			return ZD_NOERROR;
		}

		ZD_STATUS ZenDiaryApp::Terminate()
		{
			m_terminate.store(true);
			return ZD_NOERROR;
		}

		ZD_STATUS ZenDiaryApp::InitializeWindow()
		{
			Awesomium::WebConfig config;

			config.log_level = Awesomium::kLogLevel_Verbose;

#ifdef _DEBUG
			config.remote_debugging_port = m_remote_debugging_port;
			config.remote_debugging_host = Awesomium::WSLit("127.0.0.1");
#endif

			const WindowSettings &window_settings = m_settings.GetGuiSettings().GetWindowSettings();

			m_core = Awesomium::WebCore::Initialize(config);
			m_window = WebWindow::Create(window_settings.GetTitle(), 
				window_settings.GetWidth(), window_settings.GetHeight());

			Awesomium::WebView *view = m_window->GetWebView();

			BindMethods();

			view->set_menu_listener(&m_menu_handler);
			view->set_js_method_handler(&m_method_handler);

			view->LoadURL(Awesomium::WebURL(Awesomium::WSLit(
				Helpers::String::FilenameToUrl(GetFullname("signin.html")).c_str())));
			return ZD_NOERROR;
		}

		ZD_STATUS ZenDiaryApp::FreeWindow()
		{				
			ZD_SAFE_DELETE(m_window);
			Awesomium::WebCore::Shutdown();

			return ZD_NOERROR;
		}

		ZD_STATUS ZenDiaryApp::InitializeDirectories()
		{
			const std::deque<std::string> directories = {
				Helpers::String::ExtractPath(m_settings_path), Helpers::String::ExtractPath(m_database_path)
			};

			for (auto &path : directories)
			{				
				boost::filesystem::create_directories(boost::filesystem::path(path));
			}

			return ZD_NOERROR;
		}

		ZD_STATUS ZenDiaryApp::InitializeJsHandlers()
		{
			m_js_handlers.SetGlobalSettings(&m_settings);
			return ZD_NOERROR;
		}

		ZD_STATUS ZenDiaryApp::BindMethods()
		{
			if (!m_window)
			{
				return ZD_ERROR_NOT_INITIALIZED;
			}

			Awesomium::WebView *view = m_window->GetWebView();
			if (!view)
			{
				return ZD_ERROR_NOT_INITIALIZED;
			}

			Awesomium::JSValue result = view->CreateGlobalJavascriptObject(Awesomium::WSLit("zen"));
			if (!result.IsObject())
			{
				return ZD_ERROR_FAILED_TO_CREATE;
			}

			Awesomium::JSObject &zen_diary = result.ToObject();

			ZD_BIND_JS_HANDLER("alert", &JSHandlers::OnAlert);
			ZD_BIND_JS_HANDLER("isFirstRun", &JSHandlers::OnIsFirstRun);

			ZD_BIND_JS_HANDLER("registerUser", &JSHandlers::OnRegisterUser);
			return ZD_NOERROR;
		}

		std::string ZenDiaryApp::GetFullname(const std::string &filename)
		{
			const size_t buf_size = MAX_PATH;
			char buf[buf_size];

			GetModuleFileName(GetModuleHandle(nullptr), buf, buf_size);

			std::string path = Helpers::String::ExtractPath(buf);

			std::stringstream stream;
			stream << path << m_httpdocs_path << filename;

			return stream.str();
		}
	}
}