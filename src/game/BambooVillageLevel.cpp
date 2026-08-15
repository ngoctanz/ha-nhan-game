#include "game/BambooVillageLevel.hpp"
#include "game/AssetLocator.hpp"
#include "game/Config.hpp"
#include "game/Ui.hpp"

#include <algorithm>
#include <array>
#include <cmath>

namespace game
{
namespace
{
constexpr float ElderInteractionDistance = 145.0F;
// Move the painted village upward so GroundY lands near the middle of the
// foreground road instead of directly beside the house wall.
constexpr float BackgroundLift = 88.0F;

void DrawRoadExtension()
{
    const int top = static_cast<int>(ScreenHeight - BackgroundLift);
    const int height = static_cast<int>(BackgroundLift);
    DrawRectangleGradientV(0, top, static_cast<int>(WorldWidth), height,
                           Color{174, 147, 102, 255}, Color{188, 159, 111, 255});

    const Color roadMark = Fade(Color{73, 65, 48, 255}, 0.38F);
    for (int x = 45; x < static_cast<int>(WorldWidth); x += 185)
    {
        const float y = static_cast<float>(top + 11 + ((x / 185) % 3) * 9);
        DrawLineEx({static_cast<float>(x), y},
                   {static_cast<float>(x + 58), y - 1.0F}, 1.4F, roadMark);
        DrawCircle(x + 91, static_cast<int>(y + 7.0F), 1.8F, roadMark);
    }
}

const char *ElderFaceName(std::string_view emotion)
{
    if (emotion == "laughing") return "verified_beaming_closed_eyes";
    if (emotion == "angry") return "verified_angry_side_eye";
    if (emotion == "shocked") return "verified_stunned_disbelief";
    if (emotion == "sad") return "verified_tearful_restraint";
    if (emotion == "scheming") return "verified_knowing_suspicion";
    if (emotion == "embarrassed") return "verified_embarrassed_blush";
    if (emotion == "proud") return "verified_cool_confidence";
    return "verified_wise_goatee";
}

void DrawShard(Vector2 position, float time)
{
    const float pulse = 1.0F + std::sin(time * 4.0F) * 0.12F;
    const float y = position.y + std::sin(time * 3.0F) * 9.0F;
    DrawCircleGradient(static_cast<int>(position.x), static_cast<int>(y), 45.0F * pulse,
                       Fade(SKYBLUE, 0.52F), Fade(BLUE, 0.0F));
    std::array<Vector2, 4> crystal = {
        Vector2{position.x, y - 27.0F * pulse}, Vector2{position.x + 17.0F * pulse, y},
        Vector2{position.x, y + 30.0F * pulse}, Vector2{position.x - 17.0F * pulse, y}};
    DrawTriangle(crystal[0], crystal[1], crystal[2], Color{83, 229, 255, 255});
    DrawTriangle(crystal[0], crystal[2], crystal[3], Color{39, 151, 255, 255});
    DrawLineStrip(crystal.data(), static_cast<int>(crystal.size()), RAYWHITE);
}

void DrawQuestMarker(Vector2 position, Font font, const char *mark, Color color, float time)
{
    position.y += std::sin(time * 4.0F) * 5.0F;
    DrawCircleV(position, 20, color);
    const Vector2 size = MeasureTextEx(font, mark, 26, 0.5F);
    DrawTextEx(font, mark, {position.x - size.x / 2.0F, position.y - size.y / 2.0F - 1.0F},
               26, 0.5F, WHITE);
}
} // namespace

BambooVillageLevel::~BambooVillageLevel()
{
    if (background_.id != 0) UnloadTexture(background_);
}

bool BambooVillageLevel::Load()
{
    background_ = LoadTexture(
        ResolveAssetPath("assets/environment/bamboo_village_meme.png").c_str());
    if (background_.id != 0) SetTextureFilter(background_, TEXTURE_FILTER_BILINEAR);
    const bool charactersLoaded = player_.Load() && elder_.Load() && faces_.Load() &&
                                  systemDog_.Load() && chicken_.Load();
    soundEffects_.Load();

    camera_.offset = {ScreenWidth / 2.0F, ScreenHeight / 2.0F};
    camera_.target = {ScreenWidth / 2.0F, ScreenHeight / 2.0F};
    camera_.zoom = 1.0F;
    chicken_.SetPosition({1460.0F, GroundY});

    dialogue_.Start({
        {"HỆ THỐNG", "Ting! Ký chủ đã xuyên không tới Thôn Trúc. Gói tân thủ hiện đang... nợ phí vận chuyển.", "system:idle"},
        {"HÀ NHÂN", "Hệ thống nhà người ta phát thần khí! Còn ngươi phát cho ta cái nón rách này à?!", "player:verified_angry_surprise", "point", "cao_ni_ma"},
        {"HỆ THỐNG", "Nhiệm vụ tân thủ cấp S: hỏi Trưởng thôn xem hôm nay có cơm không.", "system:quest"}});
    quest_.Start("bamboo_chicken", "Một bữa cơm xuyên không", "Nói chuyện với Trưởng thôn");
    return background_.id != 0 && charactersLoaded;
}

void BambooVillageLevel::Update(float deltaTime, const GameInput &input)
{
    input_ = input;
    worldTime_ += deltaTime;
    toastTimer_ = std::max(0.0F, toastTimer_ - deltaTime);
    levelTitleTimer_ = std::max(0.0F, levelTitleTimer_ - deltaTime);
    if (!dialogue_.IsActive())
        controlsHintTimer_ = std::max(0.0F, controlsHintTimer_ - deltaTime);

    const ChickenEvent chickenEvent = chicken_.Update(deltaTime);
    if (chickenEvent == ChickenEvent::Vanished && !dialogue_.IsActive())
        BeginChickenResultDialogue();

    if (dialogue_.IsActive())
        ApplyDialogueAction(dialogue_.Update(input.advanceDialoguePressed));
    else
        HandleInteraction(input);

    SyncDialoguePresentation();
    player_.Update(deltaTime, !dialogue_.IsActive(), input);
    systemDog_.Update(deltaTime, player_.Position(), player_.FacingRight());
    const float desiredX = std::clamp(player_.Position().x,
                                      ScreenWidth / 2.0F, WorldWidth - ScreenWidth / 2.0F);
    camera_.target.x += (desiredX - camera_.target.x) * std::min(1.0F, deltaTime * 5.5F);
    camera_.target.y = ScreenHeight / 2.0F;
}

void BambooVillageLevel::SyncDialoguePresentation()
{
    const std::uint64_t revision = dialogue_.Revision();
    if (revision == observedDialogueRevision_) return;
    observedDialogueRevision_ = revision;

    CharacterAction action = CharacterAction::None;
    if (dialogue_.IsActive())
    {
        action = ParseCharacterAction(dialogue_.Current().action);
        soundEffects_.Play(ParseMemeSound(dialogue_.Current().sound));
    }
    player_.TriggerAction(action);
}

void BambooVillageLevel::BeginChickenResultDialogue()
{
    dialogue_.Start({
        {"GÀ LINH KHÍ", "Cục tác! Ba lần mới chạm được ta, nhân loại yếu đuối.", ""},
        {"HÀ NHÂN", "Khoan, gà ở thế giới này biết nói à?!", "player:verified_shocked_gasp", "", "doit"},
        {"HỆ THỐNG", "Mục tiêu đã dùng truyền tống phù. Nói cách khác: con gà chạy mất dép.", "system:quest"}},
        DialogueAction::SpawnShard);
}

void BambooVillageLevel::HandleInteraction(const GameInput &input)
{
    const bool actorTap = input.pointerPressed && !input.pointerConsumedByControls;
    const Vector2 pointerWorld = GetScreenToWorld2D(input.pointerScreen, camera_);
    const bool elderTapped = actorTap && CheckCollisionPointRec(
        pointerWorld, {elderPosition_.x - 112.0F, elderPosition_.y - 315.0F, 224.0F, 315.0F});
    const bool chickenTapped = actorTap && CheckCollisionPointRec(
        pointerWorld, {chicken_.Position().x - 78.0F, chicken_.Position().y - 138.0F,
                       156.0F, 148.0F});
    const bool shardTapped = actorTap && shardVisible_ &&
        CheckCollisionPointCircle(pointerWorld, shardPosition_, 66.0F);

    if (!input.interactPressed && !elderTapped && !chickenTapped && !shardTapped) return;

    if (elderTapped ||
        (input.interactPressed && player_.IsNear(elderPosition_.x, ElderInteractionDistance)))
    {
        // Stationary NPCs only choose a direction when an interaction starts.
        // The direction is then kept until the next conversation.
        elder_.FaceToward(elderPosition_.x, player_.Position().x);
        if (questState_ == QuestState::MeetElder)
        {
            dialogue_.Start({
                {"TRƯỞNG THÔN", "Thiếu hiệp đến đúng lúc! Gà Linh Khí vừa trộm mất mảnh ngọc lạ.", "elder:shocked"},
                {"HÀ NHÂN", "Bắt gà thôi mà cũng gọi là nhiệm vụ chính tuyến sao?", "player:exhausted_blank", "present_open", "huh"},
                {"HỆ THỐNG", "Cảnh báo: con gà có tu vi cao hơn ký chủ hai cảnh giới. Đề nghị giữ lại chút thể diện.", "system:quest"},
                {"TRƯỞNG THÔN", "Bắt được nó, ta thưởng một cơm nắm. Không bắt được thì tối nay khỏi ăn.", "elder:scheming"}},
                DialogueAction::StartChickenQuest);
        }
        else if (questState_ == QuestState::ReturnToElder)
        {
            dialogue_.Start({
                {"HÀ NHÂN", "Đây là mảnh ngọc. Con gà kia chạy nhanh hơn cả cao thủ Kim Đan!", "player:verified_angry_surprise"},
                {"TRƯỞNG THÔN", "Ngọc gì? Đó là mảnh bảng hiệu chuồng gà nhà ta.", "elder:laughing"},
                {"HỆ THỐNG", "Phát hiện vật phẩm cấp Thần: Mảnh Hệ Thống giả. Hàng thật đang bảo trì.", "system:laugh"},
                {"HÀ NHÂN", "...Ta xuyên không tới đây để bị lừa đúng không?", "player:verified_baffled_closed_eyes", "", "huh"}},
                DialogueAction::CompleteQuest);
        }
        else if (questState_ == QuestState::Complete)
        {
            dialogue_.Start({
                {"TRƯỞNG THÔN", "Ngày mai nhớ quay lại. Ta còn một con vịt biết khinh công.", "elder:proud"},
                {"HÀ NHÂN", "Hệ thống! Cho ta xuyên về ngay, ta không bắt thêm con gì nữa đâu!", "player:verified_angry_surprise", "point"},
                {"HỆ THỐNG", "Tính năng chưa mở khóa. Vui lòng tu luyện thêm... hoặc nộp cơm nắm.", "system:laugh"}});
        }
        else
        {
            toast_ = "Trưởng thôn: Lo bắt gà đi, cơm nguội bây giờ!";
            toastTimer_ = 2.8F;
        }
        return;
    }

    if (questState_ == QuestState::CatchChicken && chicken_.CanInteract() &&
        (input.interactPressed || chickenTapped) &&
        player_.IsNear(chicken_.Position().x, 135.0F))
    {
        ++catchAttempts_;
        quest_.SetProgress(catchAttempts_);
        const Vector2 chickenPosition = chicken_.Position();
        if (catchAttempts_ < 3)
        {
            const Vector2 target = {
                std::min(chickenPosition.x + 310.0F, WorldWidth - 190.0F), GroundY};
            chicken_.StartEscape(target, false);
            toast_ = catchAttempts_ == 1 ? "Gà Linh Khí dùng: Lăng Ba Vi Bộ!"
                                         : "Nó quay lại nhìn bro bằng ánh mắt khinh thường...";
            toastTimer_ = 3.0F;
        }
        else
        {
            const Vector2 target = {
                std::min(chickenPosition.x + 230.0F, WorldWidth - 80.0F), GroundY};
            chicken_.StartEscape(target, true);
            toast_ = "Gà Linh Khí hoảng hốt rồi co giò bỏ chạy!";
            toastTimer_ = 2.0F;
        }
        return;
    }

    if (questState_ == QuestState::CollectShard && shardVisible_ &&
        (input.interactPressed || shardTapped) && player_.IsNear(shardPosition_.x, 120.0F))
    {
        dialogue_.Start({
            {"HỆ THỐNG", "Đã quét vật phẩm: không rõ nguồn gốc, có mùi... chuồng gà. Khuyên không nên liếm.", "system:idle"},
            {"HÀ NHÂN", "Thôi mang về trước, biết đâu bán được hai đồng.", "player:verified_awkward_conflict", "", "huh"}},
            DialogueAction::ReturnToElder);
    }
}

void BambooVillageLevel::ApplyDialogueAction(DialogueAction action)
{
    switch (action)
    {
        case DialogueAction::StartChickenQuest:
            questState_ = QuestState::CatchChicken;
            chicken_.SetAlert(true);
            quest_.SetObjective("Bắt Gà Linh Khí", 3);
            toast_ = "Nhiệm vụ mới: Bắt Gà Linh Khí";
            toastTimer_ = 3.5F;
            break;
        case DialogueAction::SpawnShard:
            questState_ = QuestState::CollectShard;
            quest_.SetObjective("Nhặt Mảnh Hệ Thống");
            shardVisible_ = true;
            toast_ = "Gà biến mất, để lại một vật phát sáng!";
            toastTimer_ = 3.5F;
            break;
        case DialogueAction::ReturnToElder:
            questState_ = QuestState::ReturnToElder;
            quest_.SetObjective("Mang mảnh vỡ về cho Trưởng thôn");
            shardVisible_ = false;
            toast_ = "Đã nhận: Mảnh Hệ Thống x1";
            toastTimer_ = 3.5F;
            break;
        case DialogueAction::CompleteQuest:
            questState_ = QuestState::Complete;
            quest_.Complete("Đã hoàn thành: Gà chạy mất dép");
            toast_ = "Nhiệm vụ hoàn thành! +3 Uy Tín, +1 Cơm Nắm";
            toastTimer_ = 6.0F;
            break;
        case DialogueAction::None: break;
    }
}

const std::string &BambooVillageLevel::Objective() const
{
    return quest_.DisplayText();
}

std::string BambooVillageLevel::InteractionPrompt() const
{
    if (dialogue_.IsActive()) return {};
    if (player_.IsNear(elderPosition_.x, ElderInteractionDistance) &&
        (questState_ == QuestState::MeetElder || questState_ == QuestState::ReturnToElder || questState_ == QuestState::Complete))
        return MobileControlsEnabled() ? "Chạm Trưởng thôn để nói chuyện" : "[E] Nói chuyện";
    if (questState_ == QuestState::CatchChicken && chicken_.CanInteract() &&
        player_.IsNear(chicken_.Position().x, 135.0F))
        return MobileControlsEnabled() ? "Chạm con gà để chộp" : "[E] Chộp lấy con gà";
    if (questState_ == QuestState::CollectShard && player_.IsNear(shardPosition_.x, 120.0F))
        return MobileControlsEnabled() ? "Chạm vật phẩm để nhặt" : "[E] Nhặt vật phẩm";
    return {};
}

const Texture2D *BambooVillageLevel::CurrentPortrait() const
{
    if (!dialogue_.IsActive()) return nullptr;
    const std::string &key = dialogue_.Current().portrait;
    constexpr const char *playerPrefix = "player:";
    if (key == "player" || key.rfind(playerPrefix, 0) == 0)
        return player_.PortraitTexture();
    if (key == "system:idle") return systemDog_.Pose(SystemDogPose::Idle);
    if (key == "system:laugh") return systemDog_.Pose(SystemDogPose::Laugh);
    if (key == "system:quest") return systemDog_.Pose(SystemDogPose::Quest);
    constexpr const char *prefix = "elder:";
    if (key.rfind(prefix, 0) == 0)
        return elder_.Portrait(std::string_view(key).substr(6));
    const std::string &speaker = dialogue_.Current().speaker;
    if (speaker.find("THỐNG") != std::string::npos ||
        speaker.find("THá»NG") != std::string::npos)
        return systemDog_.Pose(SystemDogPose::Quest);
    return nullptr;
}

const Texture2D *BambooVillageLevel::CurrentPlayerFace() const
{
    constexpr const char *playerPrefix = "player:";
    if (dialogue_.IsActive())
    {
        const std::string &key = dialogue_.Current().portrait;
        if (key.rfind(playerPrefix, 0) == 0) return faces_.Get(key.substr(7));
        if (key == "player") return faces_.Get("verified_skeptical_tease");
        return faces_.Get("verified_skeptical_tease");
    }
    return faces_.Get(player_.SuggestedFaceExpression());
}

const Texture2D *BambooVillageLevel::CurrentPortraitFace() const
{
    if (!dialogue_.IsActive()) return nullptr;
    const std::string &key = dialogue_.Current().portrait;
    if (key == "player" || key.rfind("player:", 0) == 0) return CurrentPlayerFace();
    constexpr const char *elderPrefix = "elder:";
    if (key.rfind(elderPrefix, 0) == 0)
        return faces_.Get(ElderFaceName(std::string_view(key).substr(6)));
    return nullptr;
}

void BambooVillageLevel::Draw(const Ui &ui) const
{
    BeginMode2D(camera_);
    // Lift the village without stretching it so GroundY sits inside the road,
    // then continue the small exposed strip with the sampled road-edge color.
    DrawRectangle(0, 0, static_cast<int>(WorldWidth), ScreenHeight,
                  Color{178, 150, 104, 255});
    DrawTexturePro(background_,
                   {0, 0, static_cast<float>(background_.width), static_cast<float>(background_.height)},
                   {0, -BackgroundLift, WorldWidth, static_cast<float>(ScreenHeight)},
                   {0, 0}, 0.0F, WHITE);
    DrawRoadExtension();
    DrawRectangle(0, static_cast<int>(GroundY + 3), static_cast<int>(WorldWidth), 100,
                  Fade(Color{46, 55, 35, 255}, 0.13F));

    const Texture2D *elderFace = faces_.Get("verified_wise_goatee");
    if (dialogue_.IsActive())
    {
        const std::string &portrait = dialogue_.Current().portrait;
        if (portrait.rfind("elder:", 0) == 0)
            elderFace = faces_.Get(ElderFaceName(std::string_view(portrait).substr(6)));
    }
    elder_.Draw(elderPosition_, worldTime_, elderFace);
    chicken_.Draw();
    if (shardVisible_) DrawShard(shardPosition_, worldTime_);

    const Font font = ui.GetFont();
    if (questState_ == QuestState::MeetElder || questState_ == QuestState::ReturnToElder || questState_ == QuestState::Complete)
        DrawQuestMarker({elderPosition_.x, elderPosition_.y - 340.0F}, font,
                        questState_ == QuestState::Complete ? "✓" : "!",
                        questState_ == QuestState::Complete ? Color{73, 181, 104, 255} : Color{229, 164, 42, 255},
                        worldTime_);
    if (questState_ == QuestState::CatchChicken && !chicken_.IsHidden())
        DrawQuestMarker({chicken_.Position().x, chicken_.Position().y - 145.0F}, font,
                        "!", Color{220, 80, 70, 255}, worldTime_);
    if (questState_ == QuestState::CollectShard)
        DrawQuestMarker({shardPosition_.x, shardPosition_.y - 75.0F}, font, "!", Color{52, 175, 225, 255}, worldTime_);

    SystemDogPose worldDogPose = SystemDogPose::Idle;
    if (questState_ == QuestState::Complete) worldDogPose = SystemDogPose::Laugh;
    if (dialogue_.IsActive())
    {
        const std::string &portrait = dialogue_.Current().portrait;
        if (portrait == "system:quest") worldDogPose = SystemDogPose::Quest;
        else if (portrait == "system:laugh") worldDogPose = SystemDogPose::Laugh;
    }
    systemDog_.DrawWorld(worldDogPose, worldTime_);
    player_.Draw(CurrentPlayerFace());
    EndMode2D();

    ui.DrawHud(Objective(), controlsHintTimer_);
    if (!dialogue_.IsActive()) ui.DrawMobileControls(input_);
    const SystemDogPose noticePose = questState_ == QuestState::Complete
                                         ? SystemDogPose::Laugh
                                         : SystemDogPose::Quest;
    if (dialogue_.IsActive())
        ui.DrawDialogue(dialogue_.Current(), CurrentPortrait(), CurrentPortraitFace());
    else
    {
        ui.DrawSystemNotice(toast_, toastTimer_, systemDog_.Pose(noticePose));
        ui.DrawPrompt(InteractionPrompt());
    }
    ui.DrawLevelTitle(levelTitleTimer_);
}
} // namespace game
