@echo off
setlocal

for %%I in ("%~dp0..") do set "PROJECT_ROOT=%%~fI"
set "JAVA_HOME=%PROJECT_ROOT%\.android-tools\jdk\jdk-17.0.20+8"
set "ANDROID_HOME=%PROJECT_ROOT%\.android-tools\sdk"
set "GRADLE_USER_HOME=%PROJECT_ROOT%\.android-tools\gradle-home"
set "PATH=%JAVA_HOME%\bin;%ANDROID_HOME%\platform-tools;%PATH%"

call "%PROJECT_ROOT%\.android-tools\gradle\gradle-8.10.2\bin\gradle.bat" -p "%~dp0." :app:assembleRelease
if errorlevel 1 exit /b %errorlevel%

if not exist "%~dp0.signing" mkdir "%~dp0.signing"
if not exist "%PROJECT_ROOT%\dist" mkdir "%PROJECT_ROOT%\dist"

set "KEYSTORE=%~dp0.signing\hanhan-demo.jks"
if not exist "%KEYSTORE%" (
  "%JAVA_HOME%\bin\keytool.exe" -genkeypair -keystore "%KEYSTORE%" -storepass android -keypass android -alias hanhan-demo -keyalg RSA -keysize 2048 -validity 10000 -dname "CN=Ha Nhan Demo,O=Local Development,C=VN"
  if errorlevel 1 exit /b %errorlevel%
)

set "UNSIGNED_APK=%~dp0app\build\outputs\apk\release\app-release-unsigned.apk"
set "SIGNED_APK=%PROJECT_ROOT%\dist\ha-nhan-demo-arm64.apk"
call "%ANDROID_HOME%\build-tools\35.0.0\apksigner.bat" sign --ks "%KEYSTORE%" --ks-key-alias hanhan-demo --ks-pass pass:android --key-pass pass:android --out "%SIGNED_APK%" "%UNSIGNED_APK%"
if errorlevel 1 exit /b %errorlevel%

call "%ANDROID_HOME%\build-tools\35.0.0\apksigner.bat" verify --verbose --print-certs "%SIGNED_APK%"
if errorlevel 1 exit /b %errorlevel%

echo APK: %SIGNED_APK%
