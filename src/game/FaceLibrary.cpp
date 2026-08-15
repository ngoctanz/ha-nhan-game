#include "game/FaceLibrary.hpp"
#include "game/TextureAsset.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <string>
#include <utility>

namespace game
{
namespace
{
constexpr std::array MaleFolders = {
    "01_confused", "02_deadpan", "03_knowing_look", "04_smug", "05_laughing",
    "06_surprised", "07_annoyed", "08_skeptical", "09_resigned", "10_realization",
    "11_blushing", "12_confused_frown", "13_worried", "14_blank_stare", "15_puzzled",
    "16_innocent_smile", "17_disbelief", "18_stunned_disbelief", "19_realizing",
    "20_grumpy", "21_detached_stare", "22_guilty_startle", "23_scheming_daydream",
    "24_uneasy", "25_cool", "26_suspicious", "27_concerned", "28_innocent",
    "29_weary", "30_nosebleed", "31_dizzy", "32_relieved_smile", "33_crying",
    "34_suspicious_stare", "35_ugly_crying", "36_emotional_pain", "37_relaxed_smile",
    "38_angry", "39_bashful_smile", "40_devious", "41_dumbfounded", "42_terrified",
    "43_mentally_exhausted", "44_happy", "45_old_man", "46_knowing_smirk",
    "47_serious", "48_disgusted", "49_furious", "50_reminiscing"};

constexpr std::array FemaleFolders = {
    "1_nervous_smile", "2_annoyed", "3_sad", "4_terrified", "5_pouting",
    "07_angry", "08_blank", "09_happy", "10_dizzy", "11_excited", "12_worried",
    "13_flustered", "14_crying", "15_furious", "16_panic_crying", "17_shocked",
    "18_smug", "19_confident_smile", "20_displeased", "21_uneasy", "22_mischievous",
    "23_frustrated", "24_embarrassed", "25_concerned", "26_wink_smile",
    "27_cheerful", "28_playful_wink", "29_eyes_closed_smile", "30_shadowed"};

std::string Key(FaceGender gender, std::string_view expression)
{
    std::string key = gender == FaceGender::Male ? "male:" : "female:";
    key.append(expression);
    return key;
}

std::string_view ExpressionFromFolder(std::string_view folder)
{
    const std::size_t separator = folder.find('_');
    return separator == std::string_view::npos ? folder : folder.substr(separator + 1);
}
} // namespace

FaceLibrary::~FaceLibrary()
{
    for (Clip &clip : clips_)
        for (Texture2D &frame : clip.frames) UnloadTextureAsset(frame);
}

bool FaceLibrary::Load()
{
    if (!clips_.empty()) return true;
    clips_.reserve(MaleFolders.size() + FemaleFolders.size());
    ids_.reserve(MaleFolders.size() + FemaleFolders.size());
    bool allLoaded = true;

    const auto loadGender = [this, &allLoaded](FaceGender gender, const auto &folders,
                                               std::string_view root)
    {
        for (std::string_view folder : folders)
        {
            if (clips_.size() >= FaceId::InvalidValue)
            {
                allLoaded = false;
                return;
            }
            Clip clip;
            clip.frames.reserve(6);
            for (int frame = 1; frame <= 6; ++frame)
            {
                const std::string path = std::string(root) + std::string(folder) +
                    "/frame_0" + std::to_string(frame) + ".png";
                Texture2D texture = LoadTextureAsset(path);
                allLoaded = allLoaded && texture.id != 0;
                clip.frames.push_back(texture);
            }
            const FaceId id{static_cast<std::uint16_t>(clips_.size())};
            ids_.emplace(Key(gender, ExpressionFromFolder(folder)), id);
            clips_.push_back(std::move(clip));
        }
    };

    loadGender(FaceGender::Male, MaleFolders, "assets/faces/boy_faces/");
    loadGender(FaceGender::Female, FemaleFolders, "assets/faces/girl_faces/");
    maleFallback_ = Find(FaceGender::Male, "deadpan");
    femaleFallback_ = Find(FaceGender::Female, "blank");
    return allLoaded && maleFallback_.IsValid() && femaleFallback_.IsValid();
}

FaceId FaceLibrary::Find(FaceGender gender, std::string_view expression) const
{
    const auto found = ids_.find(Key(gender, expression));
    if (found != ids_.end()) return found->second;
    return gender == FaceGender::Male ? maleFallback_ : femaleFallback_;
}

const Texture2D *FaceLibrary::Frame(FaceId face, float elapsedSeconds) const
{
    if (!face.IsValid() || face.value >= clips_.size()) return nullptr;
    const Clip &clip = clips_[face.value];
    if (clip.frames.empty()) return nullptr;
    const float safeTime = std::max(0.0F, elapsedSeconds);
    const std::size_t index = static_cast<std::size_t>(safeTime * clip.framesPerSecond) %
                              clip.frames.size();
    const Texture2D &texture = clip.frames[index];
    return texture.id == 0 ? nullptr : &texture;
}

std::size_t FaceLibrary::ExpressionCount() const { return clips_.size(); }

std::size_t FaceLibrary::TextureCount() const
{
    std::size_t count = 0;
    for (const Clip &clip : clips_) count += clip.frames.size();
    return count;
}

bool FaceAnimator::SetExpression(const FaceLibrary &library, FaceGender gender,
                                 std::string_view expression)
{
    const bool sameName = gender_ == gender && expression_.size() == expression.size() &&
        std::equal(expression_.begin(), expression_.end(), expression.begin());
    if (sameName && face_.IsValid()) return true;

    const FaceId next = library.Find(gender, expression);
    if (!next.IsValid()) return false;
    gender_ = gender;
    expression_.assign(expression);
    if (next == face_) return true;
    face_ = next;
    elapsedSeconds_ = 0.0F;
    return true;
}

void FaceAnimator::Update(float deltaTime)
{
    if (deltaTime > 0.0F) elapsedSeconds_ += deltaTime;
    if (elapsedSeconds_ > 3600.0F) elapsedSeconds_ = std::fmod(elapsedSeconds_, 3600.0F);
}

const Texture2D *FaceAnimator::Current(const FaceLibrary &library) const
{
    return library.Frame(face_, elapsedSeconds_);
}

FaceId FaceAnimator::CurrentId() const { return face_; }
} // namespace game
