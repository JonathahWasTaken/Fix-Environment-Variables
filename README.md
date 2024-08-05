# Fix Environment Variables

This script is very simple and made for pre-installation of apps requiring use of Powershell or CMD.

Many users have these issues:
- Missing cmd.exe in %SystemRoot%\System32
- Missing crucial System Environment variables in PATH (Our best guess is general OS corruption), such as:
 - "%SystemRoot%\system32"
 - "%SystemRoot%\System32\\Wbem"
 - "%SystemRoot%"
 - "%SystemRoot%\System32\WindowsPowerShell\v1.0\"
 - "%SystemRoot%\System32\OpenSSH\"
- Missing/Corrupted "ComSpec" environment variable which should be set to "%SystemRoot%\system32\cmd.exe"

This C++ script fixes those environment variables whilst keeping the custom ones you may have, such as Python, Java and such.
It also checks for the existance of cmd.exe, and downloads it. The cmd.exe downloaded is from Windows version 11 22631.3257. cmd.exe also contains the same version number, it's been confirmed to work on Windows versions that are older or more recent. Whilst it's not downloading the exact cmd version of the user, I made this in 10 minutes and it does the job ¯\\_(ツ)_/¯

The CMD hash and digital signatures are authentic and original so don't yap about it.
