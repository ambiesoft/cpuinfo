// cpuinfo.cpp : main project file.

#include "stdafx.h"
#include "resource.h"

#pragma comment(lib,"user32.lib")

using namespace System;
using namespace System::Windows::Forms;

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



	sb.AppendLine(L"Culture :\t" + System::Globalization::CultureInfo::CurrentCulture->DisplayName);
	sb.AppendLine(L"UI Culture :\t" + System::Globalization::CultureInfo::CurrentUICulture->DisplayName);



	CPINFOEX cpinfoex;
	GetCPInfoEx(CP_ACP,
		0,
		&cpinfoex);
	sb.AppendLine(L"ACP :\t" + gcnew String(cpinfoex.CodePageName));

	//TCHAR szLI[128];
	//szLI[0]=0;
	//GetLocaleInfo(
	//	LOCALE_SYSTEM_DEFAULT,
	//	LOCALE_SENGLANGUAGE,
	//	szLI,
	//	sizeof(szLI)/sizeof(szLI[0]));

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
			_T("%I64d MB of physical memory.\n"),
            msx.ullTotalPhys/(1024*1024));
		sb.AppendLine(L"Ram :\t" + gcnew String(buff));
	}

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