#include "config.h"

[Setup]
;AlwaysShowDirOnReadyPage=yes
AppId={{B7FB40FA-634A-41ED-BA6F-20D9A3F4AC6D}
AppName={#PACKAGE_NAME}
AppPublisher=Nick Dowell
AppPublisherURL=https://github.com/amsynth/amsynth
AppSupportURL=https://github.com/amsynth/amsynth/issues
AppUpdatesURL=https://github.com/amsynth/amsynth/releases
AppVersion={#PACKAGE_VERSION}
Compression=lzma
DefaultDirName={commonpf64}\Steinberg\VstPlugins
DirExistsWarning=no
DisableDirPage=no
DisableWelcomePage=no
LicenseFile=..\COPYING
OutputBaseFilename=amsynth_{#PACKAGE_VERSION}
OutputDir=.\
SolidCompression=yes
UninstallDisplayName={#PACKAGE_NAME}
UninstallFilesDir={commonappdata}\amsynth\uninst
WizardStyle=modern

[Files]
Source: ..\data\banks\*; DestDir: {commonappdata}\amsynth\banks\; Flags: recursesubdirs
Source: ..\data\skins\*; DestDir: {commonappdata}\amsynth\skins\; Excludes: "*.knob,*.xcf"; Flags: recursesubdirs
Source: x64\Release\amsynth.dll; DestDir: "{app}"; Flags: ignoreversion

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
