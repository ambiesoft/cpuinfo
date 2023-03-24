// cpuinfo.cpp : main project file.

#include "stdafx.h"

#include "../../lsMisc/HighDPI.h"
#include "../../lsMisc/CHandle.h"
#include "../../lsMisc/CreateSimpleWindow.h"

#include "resource.h"

#pragma comment(lib,"user32.lib")
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"Gdi32.lib")


using namespace System;
using namespace System::IO;
using namespace System::Net;
using namespace System::Windows::Forms;
using namespace Microsoft::Win32;

typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
bool Is64BitWindows()
{
#if defined(_WIN64)
	return true;  // 64-bit programs run only on Win64
#elif defined(_WIN32)

	// 32-bit programs run on both 32-bit and 64-bit Windows
	// so must sniff
	BOOL f64 = FALSE;
	LPFN_ISWOW64PROCESS fnIsWow64Process;

	fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(TEXT("kernel32")), "IsWow64Process");
	if (NULL != fnIsWow64Process)
	{
		return !!(fnIsWow64Process(GetCurrentProcess(), &f64) && f64);
	}

	return false;
#else
	return false; // Win64 does not support Win16
#endif
}

std::string GetGlVersion()
{
	//typedef const char* (WINAPI *FNglGetString)(int name);

	//Ambiesoft::CHModule module = LoadLibrary(L"Opengl32.dll");
	//if (!module)
	//	return std::string();

	//FNglGetString fnglGetString = (FNglGetString)GetProcAddress(module, "glGetString");
	//if (!fnglGetString)
	//	return std::string();
	Ambiesoft::CHWnd hwnd(Ambiesoft::CreateSimpleWindow());
	if (!hwnd)
		return std::string();

	std::unique_ptr<void, std::function<void(void*)>> dc((void*)GetDC(hwnd), [&](void* dc)
	{
		if (dc)
			ReleaseDC(hwnd, (HDC)dc);
	});
	PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory(&pfd, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;

	if (!SetPixelFormat((HDC)dc.get(), ChoosePixelFormat((HDC)dc.get(), &pfd), &pfd))
	{
		return std::string();
	}

	std::unique_ptr<void, std::function<void(void*)>> rc((void*)wglCreateContext((HDC)dc.get()), [](void* p)
	{
		if (p)
			wglDeleteContext((HGLRC)p);
	});
	if (!rc)
	{
		return std::string();
	}

	if (!wglMakeCurrent((HDC)dc.get(), (HGLRC)rc.get()))
	{
		return std::string();
	}
	const char* p = (const char*)glGetString(GL_VERSION);
	if (!p)
		return std::string();

	return p;
}

#define TABSPACE L""

String^ ToHumanString(System::Int64 value)
{
	System::Int64 available = value / 1024 / 1024;
	String^ unit = L"MB";
	if (available > 1024)
	{
		available /= 1024;
		unit = L"GB";
	}
	return available.ToString() + unit;
}

[STAThreadAttribute]
int mymain(array<System::String ^> ^args)
{
	Ambiesoft::InitHighDPISupport();

	// Enabling Windows XP visual effects before any controls are created
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false); 

	System::Text::StringBuilder sb;

	{
		sb.AppendLine(L"Machine: " + TABSPACE  + System::Environment::MachineName);
	}

	{
		sb.AppendLine(L"OS: " + TABSPACE  + System::Environment::OSVersion->VersionString + L" " + (Is64BitWindows()?L"64bit":L""));
	}

	{
		sb.AppendLine(L"User: " + TABSPACE  + System::Environment::UserName);
	}

	{
		String^ sid = System::DirectoryServices::AccountManagement::UserPrincipal::Current->Sid->ToString();
		sb.AppendLine(L"SID: " + TABSPACE + sid);
	}
	//	sb.AppendLine(L"Culture :" + TABSPACE  + System::Globalization::CultureInfo::CurrentCulture->DisplayName);
	//	sb.AppendLine(L"UI Culture :" + TABSPACE  + System::Globalization::CultureInfo::CurrentUICulture->DisplayName);

	// Number of CPU
	{
		SYSTEM_INFO sysinfo;
		GetSystemInfo( &sysinfo );
		sb.AppendLine(L"Number of CPU: " + TABSPACE  + sysinfo.dwNumberOfProcessors.ToString());
	}

	// memory
	{
		MEMORYSTATUSEX msx = {0};
		msx.dwLength = sizeof(msx);
		if(!GlobalMemoryStatusEx(&msx))
		{
			sb.AppendLine(gcnew String(_T("Function fails")));
		}
		else
		{
			TCHAR buff[256];
			_stprintf(buff,
				_T("%I64d MB of physical memory."),
				msx.ullTotalPhys/(1024*1024));
			sb.AppendLine(L"Ram: " + TABSPACE  + gcnew String(buff));
		}
	}

	// IP Address
	{
		try
		{
			for each (IPAddress ^ ip in Dns::GetHostEntry(System::Net::Dns::GetHostName())->AddressList)
			{
				if (ip->AddressFamily == Sockets::AddressFamily::InterNetwork)
				{
					sb.AppendLine(L"IP: " + ip->ToString());
				}
			}
		}
		catch (Exception^ ex)
		{
			sb.AppendLine(ex->ToString());
		}
	}

	// Installed dotnet version
	{
		sb.AppendLine(L".NET Framework:");
		String^ t = Ambiesoft::AmbLib::GetInstalledDotNetVersionFromRegistry();
		StringReader sr(t);
		System::Text::StringBuilder sbt;
		String^ tt;
		while( (tt=sr.ReadLine()) != nullptr )
		{
			sbt.Append(L" ");
			sbt.Append(tt);
			sbt.AppendLine();
		}
		sb.AppendLine(sbt.ToString());
	}

	// OpenGL
	{
		std::string glversion = GetGlVersion();
		sb.AppendLine(L"OpenGL: " + TABSPACE + gcnew String(glversion.c_str()));
	}

	// ACP
	{
		CPINFOEX cpinfoex;
		GetCPInfoEx(CP_ACP,
			0,
			&cpinfoex);
		sb.AppendLine(L"ACP: " + TABSPACE  + gcnew String(cpinfoex.CodePageName));
	}


	// Locale
	{
		{
			TCHAR szLI[128];
			szLI[0] = 0;
			GetLocaleInfo(
				LOCALE_SYSTEM_DEFAULT,
				LOCALE_SENGLANGUAGE,
				szLI,
				sizeof(szLI) / sizeof(szLI[0]));
			sb.AppendLine(L"Locale(LOCALE_SENGLANGUAGE) :" + TABSPACE + gcnew String(szLI));
		}
		{
			TCHAR szLL[128]; szLL[0] = 0;
			GetLocaleInfo(
				LOCALE_SYSTEM_DEFAULT,
				LOCALE_SABBREVLANGNAME,
				szLL,
				128);
			sb.AppendLine(L"Locale(LOCALE_SABBREVLANGNAME) :" + TABSPACE + gcnew String(szLL));
		}
	}

	
	// Priority
	{
		DWORD dwPriorityClass = GetPriorityClass(GetCurrentProcess());
		sb.AppendLine(L"Priority Class: " + TABSPACE  + dwPriorityClass.ToString());
	}


	// resolution
	{
		sb.AppendLine(L"Resolution: " + Screen::PrimaryScreen->Bounds.Width.ToString() + L"x" + Screen::PrimaryScreen->Bounds.Height.ToString());
	}

	// perf counter
	{
		String^ pefcounter;
		sb.Append(L"QueryPerformanceFrequency: " + TABSPACE );
		LARGE_INTEGER li={0};
		if(!QueryPerformanceFrequency(&li))
		{
			pefcounter = L"Failed";
		}
		else
		{
			pefcounter = li.QuadPart.ToString();
		}
		sb.AppendLine(pefcounter);
	}


	// drive free space
	{
		for each(System::IO::DriveInfo^ di in System::IO::DriveInfo::GetDrives())
		{
			if( di->DriveType==System::IO::DriveType::Fixed)
			{
				if(di->IsReady)
				{
					sb.Append(di->Name->TrimEnd('\\'));
					sb.Append(L" " + TABSPACE );

					sb.Append(ToHumanString(di->AvailableFreeSpace));


					sb.Append(L" / ");

					sb.Append(ToHumanString(di->TotalSize));

					sb.Append(L" (");
					sb.Append(((100 - (100 * di->AvailableFreeSpace) / di->TotalSize)).ToString());
					sb.Append(L"% used)");

					sb.AppendLine();
				}
			}
		}
		sb.AppendLine();
	}

	// page file
	{
		// HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Session Manager\Memory Management
		// HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management
		array<Object^>^ oPageFile = (array<Object^>^)Registry::GetValue(L"HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management",
			L"PagingFiles",
			nullptr);
		if(oPageFile != nullptr)
		{
			sb.AppendLine(L"PageFile: " + TABSPACE  + oPageFile[0]->ToString());
		}
	}


	//--------------------------------------------------

	if(false)
	{
		// showballoon.exe [/title:STRING] [/icon:EXE or DLL for ICON] [/iconindex:i] [/duration:MILLISEC] [/waitpid:PID] STRING
		String^ exe = L"C:\\Linkout\\CommonExe\\showballoon.exe";
		String^ arg = String::Format(L"/title:{0} /duration:5000 {1}",
			L"balloonTest",
			System::Web::HttpUtility::UrlEncode(sb.ToString()));

		System::Diagnostics::Process::Start(exe,arg);
	}

	sb.AppendLine();
	
	// DNS server
	{
		sb.AppendLine("DNS: " + TABSPACE + Ambiesoft::AmbLib::GetDnsAdress());
	}

	// Lookup
	{
		String^ strip = String::Empty;
		try
		{
			IPHostEntry^ entry = Dns::GetHostEntry("mysqlserverhost");
			if(entry && entry->AddressList->Length > 0)
			{
				IPAddress^ ip = entry->AddressList[0];
				strip = ip->ToString();
			}
		}
		catch(Exception^) {}
		sb.AppendLine("Lookup: " + TABSPACE + "mysqlserverhost=" + strip);
	}

	System::Windows::Forms::MessageBox::Show(sb.ToString(),
		Application::ProductName + " ver" + Ambiesoft::AmbLib::getAssemblyVersion(System::Reflection::Assembly::GetExecutingAssembly(), 3),
		MessageBoxButtons::OK,
		MessageBoxIcon::Information);

	//Ambiesoft::CountdownMessageBox::Show(nullptr,
	//	sb.ToString(),
	//	Application::ProductName);


	if(false)
	{
		HICON hIcon = ::LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON_MAIN));

		int waitspan = 30*1000;
		NotifyIcon^ ni = gcnew NotifyIcon();
		ni->BalloonTipText = sb.ToString();
		ni->Icon = System::Drawing::Icon::FromHandle((IntPtr)hIcon);// gcnew System::Drawing::Icon(
		ni->Text = L"CPU INFO";
		ni->Visible = true;
		ni->ShowBalloonTip(waitspan);
		::Sleep(waitspan);
		delete ni;
	}

	return 0;
}

int main(array<System::String ^> ^args)
{
	try
	{
		return mymain(args);
	}
	catch(System::Exception^ ex)
	{
		MessageBox::Show(ex->Message);
	}
	return -1;
}