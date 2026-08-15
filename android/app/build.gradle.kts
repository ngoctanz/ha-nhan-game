plugins {
    id("com.android.application")
}

android {
    namespace = "com.tanz.hanhan"
    compileSdk = 35
    ndkVersion = "27.2.12479018"

    defaultConfig {
        applicationId = "com.tanz.hanhan"
        minSdk = 24
        targetSdk = 35
        versionCode = 1
        versionName = "0.1.0"

        ndk {
            // The first mobile demo targets modern Android phones. Add
            // armeabi-v7a later only if an actual 32-bit device needs it.
            abiFilters += listOf("arm64-v8a")
        }

        externalNativeBuild {
            cmake {
                arguments += listOf(
                    "-DPLATFORM=Android",
                    "-DCMAKE_BUILD_TYPE=Release"
                )
                cppFlags += listOf("-std=c++20", "-O2", "-fvisibility=hidden")
            }
        }
    }

    externalNativeBuild {
        cmake {
            path = file("../../CMakeLists.txt")
            version = "3.31.6"
        }
    }

    buildTypes {
        debug {
            isJniDebuggable = true
        }
        release {
            isMinifyEnabled = false
            ndk.debugSymbolLevel = "SYMBOL_TABLE"
        }
    }

    sourceSets["main"].assets.srcDir(layout.buildDirectory.dir("generated/gameAssets"))

    packaging {
        jniLibs.useLegacyPackaging = true
        resources.excludes += setOf("META-INF/**")
    }
}

val syncGameAssets by tasks.registering(Sync::class) {
    val gameAssets = file("../../assets")
    from(gameAssets) {
        include("environment/bamboo_village_runtime.png")
        include("environment/home1.png")
        include("environment/myroom1.png")
        include("environment/neighbor_house_back_night.png")
        include("environment/neighbor_house_front_night.png")
        include("environment/village_gate.png")
        include("environment/foreground/village_gate_hiding_rock.png")
        include("effects/heavenly_meteor_burst.png")
        include("music/demo_village.mp3")
        include("music/map3.mp3")
        include("music/troll.mp3")
        include("sounds/*.mp3")
        include("characters/player/emotions/angry/actions/idle/idle_neutral.png")
        include("characters/player/emotions/angry/actions/walk/*.png")
        include("characters/player/emotions/angry/actions/gesture/*.png")
        include("characters/player/emotions/angry/actions/sit/sit_02.png")
        include("characters/player/actions/flying_kick/flying_kick_*.png")
        include("characters/player/actions/sneak/sneak_*.png")
        include("characters/player/actions/goofy_walk/goofy_*.png")
        include("characters/player/actions/run/run_*.png")
        include("characters/player/actions/throw/throw_*.png")
        include("characters/bandits/*/idle.png")
        include("characters/neighbor_woman/idle.png")
        include("characters/npc/elder/expressions/fullbody_meme.png")
        include("characters/system_dog/idle.png")
        include("characters/system_dog/quest.png")
        include("characters/system_dog/laugh.png")
        include("characters/creatures/chicken/idle.png")
        include("characters/creatures/chicken/suspicious.png")
        include("characters/creatures/chicken/run.png")
        include("characters/creatures/chicken/startled.png")
        include("faces/boy_faces/**/frame_*.png")
        include("faces/girl_faces/**/frame_*.png")
        include("ui/mobile/sneak.png")
        include("ui/items/*.png")
        include("fonts/NotoSans-Regular.ttf")
        include("fonts/LICENSE-Noto.txt")
        include("images/logo.png")
    }
    into(layout.buildDirectory.dir("generated/gameAssets/assets"))
}

tasks.named("preBuild").configure {
    dependsOn(syncGameAssets)
}
