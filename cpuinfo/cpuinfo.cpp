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

	sb.AppendLine(L"Culture :\t" + System::Globalization::CultureInfo::CurrentCulture->DisplayName);
	sb.AppendLine(L"UI Culture :\t" + System::Globalization::CultureInfo::CurrentUICulture->DisplayName);

	UINT acp = GetACP();
	sb.AppendLine(L"ACP :\t" + acp.ToString());

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

	NotifyIcon^ ni = gcnew NotifyIcon();
	ni->BalloonTipText = sb.ToString();
	ni->Icon = System::Drawing::Icon::FromHandle((IntPtr)hIcon);// gcnew System::Drawing::Icon(
	ni->Text = L"CPU INFO";
	ni->Visible = true;
	ni->ShowBalloonTip(5000);
	::Sleep(10000);
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