#ifndef _TAMA_RUNCMDUTIL_
#define _TAMA_RUNCMDUTIL_

#include <string.h>

#if defined _WIN32 || defined __CYGWIN__
#pragma comment(lib, "shlwapi.lib")
#endif

#if defined _WIN32 || defined __CYGWIN__
bool GetDllDirA(char *path, int size)
{
  //char path[_MAX_PATH+16];
  HMODULE hModule = NULL;
  if(!GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,  (LPTSTR)&GetDllDirA, &hModule))return false;
  if(!GetModuleFileNameA(hModule, path, size))return false;
  PathRemoveFileSpecA(path);
  return true;
}
bool GetDllDirW(wchar_t *path, int size)
{
  //char path[_MAX_PATH+16];
  HMODULE hModule = NULL;
  if(!GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,  (LPTSTR)&GetDllDirW, &hModule))return false;
  if(!GetModuleFileNameW(hModule, path, size))return false;
  PathRemoveFileSpecW(path);
  return true;
}
#else
bool GetMacOSDirInPlugin(char *path, int maxBufLen, CFStringRef bundleid)
{
  CFBundleRef bundle = CFBundleGetBundleWithIdentifier(bundleid);
  if(bundle==NULL)
  {
    strcpy(path, "");
    return false;
  }
  CFURLRef bundleUrl = CFBundleCopyBundleURL(bundle);
  CFStringRef dirPathRef = CFURLCopyFileSystemPath(bundleUrl, kCFURLPOSIXPathStyle);
  const char* filePath = CFStringGetCStringPtr(dirPathRef, kCFStringEncodingUTF8);
  strcpy(path, filePath);
  
  CFRelease(dirPathRef);
  CFRelease(bundleUrl);
  return true;
}
#endif

std::string MyGetTempFilePathA()
{
  boost::filesystem::path path = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  return path.string();
}

#if defined _WIN32 || defined __CYGWIN__
std::wstring MyGetTempFilePathW()
{
  boost::filesystem::path path = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  return path.native();
}
#endif

#if defined _WIN32 || defined __CYGWIN__
DWORD RunCmdA(std::string &cmd)
{
  STARTUPINFOA si = {0};
  si.cb = sizeof(si);
  PROCESS_INFORMATION pi = {0};
  
  if(!CreateProcessA(NULL, &cmd[0], NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
  {
    return -1;
  }

  DWORD result=-1;
  WaitForSingleObject(pi.hProcess, INFINITE);
  GetExitCodeProcess(pi.hProcess, &result);

  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
  return result;
}
#else
DWORD RunCmdA(std::string &cmd)
{
  MQWindow win = MQWindow::GetMainWindow();
  //MQDialog::MessageInformationBox(win, MQEncoding::AnsiToWide(cmd.c_str()), L"RunCmdA");
  return system(cmd.c_str());
}
#endif

#if defined _WIN32 || defined __CYGWIN__
DWORD RunCmdW(std::wstring &cmd)
{
  STARTUPINFOW si = {0};
  si.cb = sizeof(si);
  PROCESS_INFORMATION pi = {0};
  
  if(!CreateProcessW(NULL, &cmd[0], NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
  {
    return false;
  }

  DWORD result=-1;
  WaitForSingleObject(pi.hProcess, INFINITE);
  GetExitCodeProcess(pi.hProcess, &result);

  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
  return result;
}
#endif

#endif //_TAMA_RUNCMDUTIL_
