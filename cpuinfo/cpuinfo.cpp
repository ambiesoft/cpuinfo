// cpuinfo.cpp : main project file.

#include "stdafx.h"
#include "resource.h"

#pragma comment(lib,"user32.lib")

using namespace System;
using namespace System::Windows::Forms;
using namespace Microsoft::Win32;

[STAThreadAttribute]
int mymain(array<System::String ^> ^args)
{
	// Enabling Windows XP visual effects before any controls are created
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false); 

	System::Text::StringBuilder sb;

	sb.AppendLine(L"Machine :\t" + System::Environment::MachineName);
	sb.AppendLine(L"OS :\t" + System::Environment::OSVersion->VersionString);
	sb.AppendLine(L"User :\t" + System::Environment::UserName);



//	sb.AppendLine(L"Culture :\t" + System::Globalization::CultureInfo::CurrentCulture->DisplayName);
//	sb.AppendLine(L"UI Culture :\t" + System::Globalization::CultureInfo::CurrentUICulture->DisplayName);

	CPINFOEX cpinfoex;
	GetCPInfoEx(CP_ACP,
		0,
		&cpinfoex);
	sb.AppendLine(L"ACP :\t" + gcnew String(cpinfoex.CodePageName));

	TCHAR szLI[128];
	szLI[0]=0;
	GetLocaleInfo(
		LOCALE_SYSTEM_DEFAULT,
		LOCALE_SENGLANGUAGE,
		szLI,
		sizeof(szLI)/sizeof(szLI[0]));


	DWORD dwPriorityClass = GetPriorityClass(GetCurrentProcess());
	sb.AppendLine(L"Priority Class :\t" + dwPriorityClass.ToString());


	SYSTEM_INFO sysinfo;
	GetSystemInfo( &sysinfo );
	sb.AppendLine(L"Number of CPU :\t" + sysinfo.dwNumberOfProcessors.ToString());

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
		sb.AppendLine(L"Ram :\t" + gcnew String(buff));
	}

	for each(System::IO::DriveInfo^ di in System::IO::DriveInfo::GetDrives())
	{
		if( di->DriveType==System::IO::DriveType::Fixed)
		{
			if(di->IsReady)
			{
				sb.Append(di->Name);
				sb.Append(L"\t");

				System::Int64 available = di->AvailableFreeSpace / 1024 / 1024;
				String^ availablestring = available.ToString() + L"MB";
				sb.Append(availablestring);

				
				sb.Append(L" / ");
				
				System::Int64 total = di->TotalSize / 1024 / 1024;
				sb.Append(total.ToString());
				sb.Append(L"MB");

				sb.AppendLine();
			}
		}
	}

	// HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Session Manager\Memory Management
	// HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management
	array<Object^>^ oPageFile = (array<Object^>^)Registry::GetValue(L"HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management",
		L"PagingFiles",
		nullptr);
	if(oPageFile != nullptr)
	{
		sb.AppendLine(L"PageFile:\t" + oPageFile[0]->ToString());
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

	//System::Windows::Forms::MessageBox::Show(sb.ToString(),
	//	Application::ProductName,
	//	MessageBoxButtons::OK,
	//	MessageBoxIcon::Information);

	Ambiesoft::CountdownMessageBox::Show(nullptr,
		sb.ToString(),
		Application::ProductName);

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