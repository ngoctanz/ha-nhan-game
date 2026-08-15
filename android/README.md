# Android build

The Android target uses the same C++ gameplay code as desktop. Raylib keeps the
logical game surface at 1280x720 and scales it to the physical landscape display.

Pinned toolchain:

- Android Gradle Plugin 8.8.2
- Gradle 8.10.2
- Android SDK 35
- Android NDK 27.2.12479018
- CMake 3.31.6
- JDK 17
- ABI: arm64-v8a

Only curated runtime assets are copied into the APK by `syncGameAssets`; source
atlases, contact sheets, FLA files, and unused exports are excluded.

Build APK demo đã ký, có thể cài trực tiếp, từ thư mục gốc:

```bat
.\android\build-demo-apk.bat
```

Hoặc từ thư mục này:

```powershell
.\gradlew.bat :app:assembleRelease
```

Output:

```text
../dist/ha-nhan-demo-arm64.apk
```

Khóa trong `.signing` chỉ là khóa phát triển local. Khi phát hành Play Store phải
dùng upload key riêng và không commit khóa đó vào repository.
