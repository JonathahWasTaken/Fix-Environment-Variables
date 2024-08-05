#include <windows.h>
#include <iostream>
#include <string>
#include <urlmon.h>
#include <vector>
#pragma comment(lib, "urlmon.lib")

std::wstring GetEnvironmentVariableFromRegistry(const std::wstring& path, const std::wstring& valueName) {
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, path.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return L"";
    }

    DWORD bufferSize = 0;
    RegQueryValueEx(hKey, valueName.c_str(), nullptr, nullptr, nullptr, &bufferSize);
    if (bufferSize == 0) {
        RegCloseKey(hKey);
        return L"";
    }

    std::wstring value(bufferSize / sizeof(wchar_t), L'\0');
    if (RegQueryValueEx(hKey, valueName.c_str(), nullptr, nullptr, (LPBYTE)value.data(), &bufferSize) != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return L"";
    }

    RegCloseKey(hKey);
    return value;
}

void SetEnvironmentVariableInRegistry(const std::wstring& path, const std::wstring& valueName, const std::wstring& value) {
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, path.c_str(), 0, KEY_WRITE, &hKey) != ERROR_SUCCESS) {
        return;
    }

    if (RegSetValueEx(hKey, valueName.c_str(), 0, REG_EXPAND_SZ, (const BYTE*)value.c_str(), (value.size() + 1) * sizeof(wchar_t)) != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return;
    }

    RegCloseKey(hKey);
}

std::wstring ExpandEnvironmentVariables(const std::wstring& input) {
    wchar_t buffer[32767];
    ExpandEnvironmentStrings(input.c_str(), buffer, sizeof(buffer) / sizeof(wchar_t));
    return std::wstring(buffer);
}

bool fileExists(const std::wstring& path) {
    DWORD fileAttributes = GetFileAttributesW(path.c_str());
    return (fileAttributes != INVALID_FILE_ATTRIBUTES &&
        !(fileAttributes & FILE_ATTRIBUTE_DIRECTORY));
}

void setEnvironmentVariable(const std::wstring& varName, const std::wstring& varValue) {
    if (!SetEnvironmentVariableW(varName.c_str(), varValue.c_str())) {
        std::wcerr << L"Failed to set environment variable " << varName << std::endl;
    }
    else {
        std::wcout << L"Environment variable " << varName << L" set to " << varValue << std::endl;
    }
}

bool downloadFile(const std::wstring& url, const std::wstring& destination) {
    HRESULT hr = URLDownloadToFileW(nullptr, url.c_str(), destination.c_str(), 0, nullptr);
    return SUCCEEDED(hr);
}

int main() {
    std::vector<std::wstring> defaultPathEntries = {
        L"%SystemRoot%\\system32",
        L"%SystemRoot%\\System32\\Wbem",
        L"%SystemRoot%",
        L"%SystemRoot%\\System32\\WindowsPowerShell\\v1.0\\",
        L"%SystemRoot%\\System32\\OpenSSH\\"
    };

    std::wstring currentPathRaw = GetEnvironmentVariableFromRegistry(L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment", L"Path");

    std::wstring constructedPath;
    size_t pos = 0;
    std::wstring delimiter = L";";
    while ((pos = currentPathRaw.find(delimiter)) != std::wstring::npos) {
        std::wstring pathPart = currentPathRaw.substr(0, pos);
        for (const auto& entry : defaultPathEntries) {
            std::wstring expanded = ExpandEnvironmentVariables(entry);
            if (pathPart == expanded) {
                pathPart = entry;
                break;
            }
        }
        constructedPath += pathPart + delimiter;
        currentPathRaw.erase(0, pos + delimiter.length());
    }
    constructedPath += currentPathRaw;

    std::vector<std::wstring> currentPathArray;
    pos = 0;
    while ((pos = constructedPath.find(delimiter)) != std::wstring::npos) {
        currentPathArray.push_back(constructedPath.substr(0, pos));
        constructedPath.erase(0, pos + delimiter.length());
    }
    currentPathArray.push_back(constructedPath);

    std::vector<std::wstring> finalPathEntries;
    for (const auto& defaultPathEntry : defaultPathEntries) {
        std::wstring expandedPathEntry = ExpandEnvironmentVariables(defaultPathEntry);
        bool nonExpandedExists = std::find(currentPathArray.begin(), currentPathArray.end(), defaultPathEntry) != currentPathArray.end();
        bool expandedExists = std::find(currentPathArray.begin(), currentPathArray.end(), expandedPathEntry) != currentPathArray.end();

        if (nonExpandedExists) {
            finalPathEntries.push_back(expandedPathEntry);
        }
        if (expandedExists) {
            finalPathEntries.push_back(defaultPathEntry);
        }
        if (!nonExpandedExists && !expandedExists) {
            finalPathEntries.push_back(expandedPathEntry);
            finalPathEntries.push_back(defaultPathEntry);
        }
    }

    for (const auto& entry : currentPathArray) {
        if (std::find(finalPathEntries.begin(), finalPathEntries.end(), entry) == finalPathEntries.end()) {
            finalPathEntries.push_back(entry);
        }
    }

    std::wstring newPath;
    for (const auto& entry : finalPathEntries) {
        newPath += entry + delimiter;
    }
    if (!newPath.empty()) {
        newPath.pop_back();
    }

    SetEnvironmentVariableInRegistry(L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment", L"Path", newPath);

    // Refresh environment variables
    SetEnvironmentVariable(L"Path", newPath.c_str());

    wchar_t systemRoot[MAX_PATH];
    if (!GetEnvironmentVariableW(L"SystemRoot", systemRoot, MAX_PATH)) {
        std::wcerr << L"Failed to get SystemRoot environment variable." << std::endl;
        return 1;
    }

    std::wstring cmdPath = std::wstring(systemRoot) + L"\\system32\\cmd.exe";

    if (fileExists(cmdPath)) {
        std::wcout << cmdPath << L" exists." << std::endl;
    }
    else {
        std::wcerr << cmdPath << L" does not exist. Attempting to download..." << std::endl;
        std::wstring downloadUrl = L"https://static.hone.gg/cmd.exe"; // Replace with actual URL
        if (!downloadFile(downloadUrl, cmdPath)) {
            std::wcerr << L"Failed to download cmd.exe." << std::endl;
            return 1;
        }
        if (!fileExists(cmdPath)) {
            std::wcerr << L"cmd.exe still not found after download attempt." << std::endl;
            return 1;
        }
        std::wcout << L"cmd.exe downloaded successfully." << std::endl;
    }

    // Set the ComSpec environment variable
    setEnvironmentVariable(L"ComSpec", cmdPath);



    return 0;
}


