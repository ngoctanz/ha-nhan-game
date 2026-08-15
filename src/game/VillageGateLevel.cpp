#include "game/VillageGateLevel.hpp"

#include "game/Config.hpp"
#include "game/ConversationFacing.hpp"
#include "game/GameInput.hpp"
#include "game/GameProgress.hpp"
#include "game/TextureAsset.hpp"
#include "game/Ui.hpp"

#include <algorithm>
#include <cmath>
#include <string_view>

namespace game
{
namespace
{
constexpr float HidePositionX = 790.0F;
constexpr float ExitPositionX = WorldWidth - 100.0F;
constexpr float EscapePositionX = 105.0F;
constexpr Rectangle HidingRockDestination = {600.0F, GroundY - 95.0F, 380.0F, 101.0F};

void DrawFightCloud(Vector2 center, float time)
{
    const float pulse = 1.0F + std::sin(time * 18.0F) * 0.08F;
    DrawCircleV(center, 112.0F * pulse, Color{226, 215, 188, 245});
    DrawCircleV({center.x - 92, center.y - 25}, 55, Color{238, 228, 204, 245});
    DrawCircleV({center.x + 88, center.y - 20}, 60, Color{238, 228, 204, 245});
    DrawCircleV({center.x - 50, center.y - 92}, 52, Color{242, 232, 209, 245});
    DrawCircleV({center.x + 48, center.y - 88}, 58, Color{242, 232, 209, 245});
    for (int i = 0; i < 8; ++i)
    {
        const float angle = i * 0.785F + time * 5.0F;
        const Vector2 start = {center.x + std::cos(angle) * 118.0F,
                               center.y + std::sin(angle) * 92.0F};
        const Vector2 end = {center.x + std::cos(angle) * 154.0F,
                             center.y + std::sin(angle) * 120.0F};
        DrawLineEx(start, end, 5, Color{91, 67, 49, 230});
    }
    DrawPoly({center.x - 28, center.y - 18}, 8, 29, time * 180.0F,
             Color{242, 184, 48, 255});
}

float FightBlackout(float time)
{
    if (time < 0.85F) return 0.0F;
    if (time < 1.35F) return (time - 0.85F) / 0.50F;
    if (time < 2.15F) return 1.0F;
    return std::clamp(1.0F - (time - 2.15F) / 0.70F, 0.0F, 1.0F);
}
} // namespace

VillageGateLevel::~VillageGateLevel()
{
    UnloadTextureAsset(background_);
    UnloadTextureAsset(hidingRock_);
}

bool VillageGateLevel::Load()
{
    background_ = LoadTextureAsset("assets/environment/village_gate.png");
    hidingRock_ = LoadTextureAsset(
        "assets/environment/foreground/village_gate_hiding_rock.png");
    const bool playerLoaded = player_.Load();
    const bool facesLoaded = faces_.Load();
    const bool dogLoaded = systemDog_.Load();
    const bool ruffiansLoaded = facesLoaded && ruffians_.Load(faces_);
    soundEffects_.Load();

    camera_.offset = {ScreenWidth / 2.0F, ScreenHeight / 2.0F};
    camera_.target = {ScreenWidth / 2.0F, ScreenHeight / 2.0F};
    camera_.zoom = 1.0F;
    playerFace_.SetExpression(faces_, FaceGender::Male, "blank_stare");

    if (Progress().CurrentLevel() == "village_gate") RestoreCheckpoint();
    else StartFresh();
    return background_.id != 0 && hidingRock_.id != 0 && playerLoaded && facesLoaded &&
           dogLoaded && ruffiansLoaded;
}

void VillageGateLevel::StartFresh()
{
    phase_ = Phase::Entering;
    ruffiansVisible_ = false;
    objective_ = "Đi theo dấu vết Gà Linh Khí";
    player_.Place({150, GroundY}, ExitPositionX);
    player_.StartGoofyWalkTo(770.0F);
}

void VillageGateLevel::RestoreCheckpoint()
{
    const std::string &checkpoint = Progress().Checkpoint();
    if (checkpoint == "gate_choice")
    {
        ruffiansVisible_ = true;
        player_.Place({HidePositionX, GroundY}, 1450.0F);
        player_.SetSeated(true);
        playerBehindRock_ = true;
        BeginInterventionChoice();
    }
    else if (checkpoint == "gate_yes_confrontation")
    {
        ruffiansVisible_ = true;
        player_.Place({HidePositionX, GroundY}, 1450.0F);
        player_.SetSeated(false);
        BeginYesConfrontation();
    }
    else if (checkpoint == "gate_yes_after_fight")
    {
        player_.Place({1030, GroundY}, EscapePositionX);
        player_.SetDefeated(true);
        playerFace_.SetExpression(faces_, FaceGender::Male, "nosebleed");
        ruffiansVisible_ = true;
        ruffians_.FaceToward(player_.Position().x);
        ruffians_.SetExpression(faces_, 0, "laughing");
        ruffians_.SetExpression(faces_, 1, "smug");
        ruffians_.SetExpression(faces_, 2, "devious");
        ruffians_.SetExpression(faces_, 3, "laughing");
        BeginAfterFightDialogue();
    }
    else if (checkpoint == "gate_yes_escape")
    {
        ruffiansVisible_ = true;
        player_.Place({1030, GroundY}, EscapePositionX);
        playerFace_.SetExpression(faces_, FaceGender::Male, "nosebleed");
        player_.StartRunTo(EscapePositionX);
        phase_ = Phase::EscapeRun;
        objective_ = "Chạy ngược về phía làng";
    }
    else if (checkpoint == "gate_no_ruffians_leaving")
    {
        ruffiansVisible_ = true;
        player_.Place({HidePositionX, GroundY}, ExitPositionX);
        player_.SetSeated(true);
        playerBehindRock_ = true;
        ruffians_.StartRetreat();
        phase_ = Phase::RuffiansLeaving;
    }
    else if (checkpoint == "gate_no_pursuit")
    {
        ruffiansVisible_ = false;
        player_.Place({1080, GroundY}, ExitPositionX);
        player_.StartSneakTo(ExitPositionX);
        phase_ = Phase::SneakPursuit;
        objective_ = "Lén bám theo đám người khả nghi";
    }
    else if (checkpoint == "village_gate_complete")
    {
        phase_ = Phase::Finished;
        const char *nextCheckpoint =
            Progress().Choice("village_gate_intervention") == "yes"
                ? "return_village" : "front_arrival";
        Progress().SetLocation("neighbor_night", nextCheckpoint);
        Progress().Save();
        transition_ = LevelTransition::NeighborNight;
    }
    else
        StartFresh();
}

void VillageGateLevel::Update(float deltaTime, const GameInput &input)
{
    worldTime_ += deltaTime;
    toastTimer_ = std::max(0.0F, toastTimer_ - deltaTime);

    if (choice_.IsActive())
    {
        const ChoiceOption result = choice_.Update(input);
        if (result != ChoiceOption::None) ResolveChoice(result);
    }
    else if (dialogue_.IsActive())
        ApplyDialogueAction(dialogue_.Update(input.advanceDialoguePressed));

    const PlayerEvent playerEvent = player_.Update(deltaTime, false, input);
    if (playerEvent == PlayerEvent::ScriptedMoveFinished)
    {
        if (phase_ == Phase::Entering) BeginSpottingDialogue();
        else if (phase_ == Phase::Hiding)
        {
            player_.SetSeated(true);
            BeginEavesdropDialogue();
        }
        else if (phase_ == Phase::SneakPursuit) FinishScene("front_arrival");
        else if (phase_ == Phase::EscapeRun) FinishScene("return_village");
    }
    else if (playerEvent == PlayerEvent::FlyingKickHit && phase_ == Phase::FlyingKick)
    {
        phase_ = Phase::Fight;
        fightTimer_ = 0.0F;
        fightResultStaged_ = false;
    }

    const RuffianEvent ruffianEvent = ruffians_.Update(deltaTime, talkingRuffian_);
    if (ruffianEvent == RuffianEvent::Retreated && phase_ == Phase::RuffiansLeaving)
    {
        ruffiansVisible_ = false;
        playerBehindRock_ = false;
        BeginNoPursuitDialogue();
    }

    if (phase_ == Phase::Fight)
    {
        fightTimer_ += deltaTime;
        if (!fightResultStaged_ && fightTimer_ >= 1.35F)
        {
            fightResultStaged_ = true;
            ruffiansVisible_ = true;
            player_.Place({1030, GroundY}, EscapePositionX);
            player_.SetDefeated(true);
            playerFace_.SetExpression(faces_, FaceGender::Male, "nosebleed");
            ruffians_.FaceToward(player_.Position().x);
            ruffians_.SetExpression(faces_, 0, "laughing");
            ruffians_.SetExpression(faces_, 1, "smug");
            ruffians_.SetExpression(faces_, 2, "devious");
            ruffians_.SetExpression(faces_, 3, "laughing");
        }
        if (fightTimer_ >= 2.90F)
        {
            Progress().SetLocation("village_gate", "gate_yes_after_fight");
            Progress().Save();
            BeginAfterFightDialogue();
        }
    }

    SyncDialoguePresentation();
    if (dialogue_.IsActive() &&
        (dialogue_.Current().target == DialogueTarget::System ||
         (dialogue_.Current().target == DialogueTarget::Player &&
          dialogue_.Current().portrait.rfind("system:", 0) == 0)))
        FaceConversationPartners(player_, systemDog_);
    if (dialogue_.IsActive() && dialogue_.Current().portrait.rfind("player:", 0) == 0)
        playerFace_.Update(deltaTime);

    systemDog_.Update(deltaTime, player_.Position(), player_.FacingRight());
    float focusX = player_.Position().x;
    if (ruffiansVisible_ && (playerBehindRock_ || phase_ == Phase::Dialogue ||
                            phase_ == Phase::Choice || phase_ == Phase::FlyingKick ||
                            phase_ == Phase::Fight))
        focusX = (player_.Position().x + ruffians_.Position(1).x) * 0.5F;
    const float desiredX = std::clamp(focusX,
                                      ScreenWidth / 2.0F, WorldWidth - ScreenWidth / 2.0F);
    camera_.target.x += (desiredX - camera_.target.x) * std::min(1.0F, deltaTime * 5.5F);
    camera_.target.y = ScreenHeight / 2.0F;
    camera_.offset = {ScreenWidth / 2.0F, ScreenHeight / 2.0F};
    if (phase_ == Phase::Fight && fightTimer_ < 1.15F)
    {
        camera_.offset.x += static_cast<float>(GetRandomValue(-9, 9));
        camera_.offset.y += static_cast<float>(GetRandomValue(-7, 7));
    }
}

void VillageGateLevel::BeginSpottingDialogue()
{
    phase_ = Phase::Dialogue;
    ruffiansVisible_ = true;
    playerFace_.SetExpression(faces_, FaceGender::Male, "suspicious_stare");
    dialogue_.Start({
        {"HÀ NHÂN", "Ủa? Bốn cái đầu kia chụm lại như nồi lẩu không có thịt... đang bàn chuyện gì vậy?", "player:suspicious_stare", "hands_folded"},
        {"HỆ THỐNG", "Phát hiện một cụm người khả nghi. Độ khả nghi tăng mạnh vì tất cả đều đang cười bằng mũi.", "system:quest", "", "", DialogueTarget::Player},
        {"HÀ NHÂN", "Có biến để hóng! Ta núp sau tảng đá nghe thử.", "player:scheming_daydream", "point"}},
        DialogueAction::HideAtVillageRock);
}

void VillageGateLevel::BeginEavesdropDialogue()
{
    phase_ = Phase::Dialogue;
    objective_ = "Nghe lén cuộc bàn bạc";
    dialogue_.Start({
        {"THẰNG ÁO RÁCH", "Tối nay đợi đèn nhà Hà tẩu tắt, chúng ta vòng ra bờ rào sau nhà.", "ruffian0:devious"},
        {"THẰNG LỰC ĐIỀN", "Ta nghe nàng thường ra sau nhà tắm muộn. Chỉ cần im miệng là chẳng ai biết.", "ruffian1:knowing_smirk"},
        {"TÊN LƯNG KHOM", "Hà tẩu đẹp nhất vùng, lại chưa từng để đàn ông nào bén vào sân. Bỏ qua thì phí cả đời làm người xấu.", "ruffian2:devious"},
        {"THẰNG KHĂN ĐỎ", "Mà bánh ngô nàng làm cũng ngon. Rình xong tiện lấy luôn rổ bánh để cửa sổ.", "ruffian3:laughing"},
        {"THẰNG ÁO RÁCH", "Nhất trí. Ngắm người, lấy bánh, không được gây tiếng động. Ai bị bắt thì tự nhận là đi tìm gà.", "ruffian0:knowing_smirk"},
        {"HÀ NHÂN", "Lũ mất nết... dám tính kế tẩu tẩu, còn định lấy cả bánh của ta?!", "player:furious", "point"},
        {"HỆ THỐNG", "Cảnh báo: ký chủ cấp thấp, sức lực yếu. Đối phương có bốn người và ít nhất ba người biết đếm.", "system:quest", "", "", DialogueTarget::Player},
        {"HỆ THỐNG", "Ký chủ có muốn liều mạng bước ra không?", "system:idle", "", "", DialogueTarget::Player}},
        DialogueAction::ShowInterventionChoice);
}

void VillageGateLevel::BeginInterventionChoice()
{
    phase_ = Phase::Choice;
    Progress().SetLocation("village_gate", "gate_choice");
    Progress().Save();
    choice_.Start({"CẢNH BÁO CHÊNH LỆCH CẤP ĐỘ",
                   "Ký chủ cấp thấp, thể lực yếu. Có muốn lao ra đối đầu bốn tên khả nghi không?",
                   "Hậu quả: Hệ Thống từ chối bảo hành",
                   "LAO RA", "NHỊN"});
}

void VillageGateLevel::ResolveChoice(ChoiceOption choice)
{
    const bool intervene = choice == ChoiceOption::Yes;
    Progress().RecordChoice("village_gate_intervention", intervene ? "yes" : "no");
    Progress().SetLocation("village_gate", intervene ? "gate_yes_confrontation" : "gate_no_restraint");
    Progress().Save();
    playerBehindRock_ = !intervene;
    if (intervene) BeginYesConfrontation();
    else BeginNoRestraintDialogue();
}

void VillageGateLevel::BeginYesConfrontation()
{
    phase_ = Phase::Dialogue;
    playerBehindRock_ = false;
    player_.SetSeated(false);
    player_.FaceToward(ruffians_.Position(0).x);
    ruffians_.FaceToward(player_.Position().x);
    dialogue_.Start({
        {"HÀ NHÂN", "Bốn thằng đầu bếp địa ngục kia! Tẩu tẩu nhà ta mà các ngươi cũng dám đem ra bàn như đĩa rau à?!", "player:furious", "point", "cao_ni_ma"},
        {"THẰNG ÁO RÁCH", "Ồ, thằng cô nhi ăn cơm cả làng đây mà. Nghe lén xong còn bày đặt làm anh hùng?", "ruffian0:devious"},
        {"THẰNG LỰC ĐIỀN", "Ngươi gầy thế kia, ta thở mạnh một cái là bay xuống sông.", "ruffian1:smug"},
        {"HÀ NHÂN", "Ta gầy vì cơm làng ít, nhưng chân ta vẫn đủ để in dấu lên mặt các ngươi!", "player:furious", "present_open"},
        {"THẰNG KHĂN ĐỎ", "Bốn đánh một mà còn nói nhiều. Lại đây, bọn ta dạy ngươi thế nào là tình làng nghĩa xóm.", "ruffian3:laughing"},
        {"HÀ NHÂN", "Nhận lấy tuyệt kỹ vừa nghĩ ra: HÀ NHÂN PHI THIÊN ĐẠP CHÓ!", "player:furious", "point", "doit"}},
        DialogueAction::StartFlyingKick);
}

void VillageGateLevel::BeginNoRestraintDialogue()
{
    phase_ = Phase::Dialogue;
    dialogue_.Start({
        {"HÀ NHÂN", "Bình tĩnh... bốn thằng, một mình ta. Lao ra bây giờ khác gì tự gói mình làm quà.", "player:furious", "hands_folded"},
        {"HỆ THỐNG", "Lần đầu tiên ký chủ sử dụng não trước tay chân. Đã ghi nhận kỳ tích hiếm.", "system:quest", "", "", DialogueTarget::Player},
        {"HÀ NHÂN", "Ta không nhịn. Ta chỉ đang gửi cơn giận vào kho, lát nữa lấy ra dùng cả vốn lẫn lời.", "player:devious", "point"},
        {"THẰNG ÁO RÁCH", "Đi thôi. Tối gặp nhau ở bờ sông, rồi vòng về nhà Hà tẩu.", "ruffian0:knowing_smirk"}},
        DialogueAction::StartRuffianRetreat);
}

void VillageGateLevel::BeginNoPursuitDialogue()
{
    phase_ = Phase::Dialogue;
    playerFace_.SetExpression(faces_, FaceGender::Male, "serious");
    dialogue_.Start({
        {"HÀ NHÂN", "Tẩu tẩu tốt bụng, xinh đẹp, lại còn cho ta bánh ngô... món quà quý như vậy tuyệt đối không thể để lũ kia làm bẩn.", "player:serious", "hands_folded"},
        {"HỆ THỐNG", "Xác nhận: ký chủ đang bảo vệ ân nhân hay bảo vệ nguồn cung bánh ngô?", "system:idle", "", "", DialogueTarget::Player},
        {"HÀ NHÂN", "Cả hai đều là đại sự sinh tử! Ta sẽ lén theo chúng, xem hang ổ ở đâu rồi tính tiếp.", "player:suspicious", "point"}},
        DialogueAction::StartSneakPursuit);
}

void VillageGateLevel::BeginAfterFightDialogue()
{
    phase_ = Phase::Dialogue;
    objective_ = "Tìm cách thoát thân";
    dialogue_.Start({
        {"NGƯỜI DẪN CHUYỆN", "Một trận bụi bay đá chạy diễn ra. Trời tối đi một lúc... rồi sáng trở lại như chưa từng muốn chứng kiến.", ""},
        {"THẰNG ÁO RÁCH", "Một mình lao vào bốn người, còn hét tên chiêu trước khi đá. Ngu cũng phải có giới hạn chứ!", "ruffian0:laughing"},
        {"THẰNG LỰC ĐIỀN", "Ta còn tưởng cao thủ nào. Hóa ra bao cát biết bay.", "ruffian1:smug"},
        {"TÊN LƯNG KHOM", "Lần sau muốn làm anh hùng thì nhớ rủ thêm cái cáng tới đón.", "ruffian2:devious"},
        {"THẰNG KHĂN ĐỎ", "Thôi để hắn ngồi đếm răng. Chúng ta còn việc tối nay.", "ruffian3:laughing"},
        {"HÀ NHÂN", "Hệ Thống... kiểm tra giúp ta xem mặt còn đủ chỗ để bầm thêm không?", "player:nosebleed", ""},
        {"HỆ THỐNG", "Kết quả quét: ký chủ còn sống. Giải pháp chiến thắng hiện tại: không có. Giải pháp giữ mạng: cũng đang tải.", "system:quest", "", "", DialogueTarget::Player},
        {"HÀ NHÂN", "Ngươi bó tay thì nói đại đi! Chúng vẫn đứng đó cười kìa, giờ làm gì?!", "player:nosebleed", "point", "huh", DialogueTarget::System},
        {"HỆ THỐNG", "Khuyến nghị chiến thuật cổ đại tối cao: chạy.", "system:idle", "", "doit", DialogueTarget::Player},
        {"HÀ NHÂN", "Ý hay! Chân chưa gãy thì mặt mũi tính sau!", "player:mentally_exhausted", "present_open"}},
        DialogueAction::StartEscapeRun);
}

void VillageGateLevel::ApplyDialogueAction(DialogueAction action)
{
    switch (action)
    {
        case DialogueAction::HideAtVillageRock:
            phase_ = Phase::Hiding;
            playerBehindRock_ = true;
            player_.StartSneakTo(HidePositionX);
            break;
        case DialogueAction::ShowInterventionChoice:
            BeginInterventionChoice();
            break;
        case DialogueAction::StartFlyingKick:
            phase_ = Phase::FlyingKick;
            playerFace_.SetExpression(faces_, FaceGender::Male, "furious");
            player_.StartFlyingKick(1350.0F);
            break;
        case DialogueAction::StartRuffianRetreat:
            phase_ = Phase::RuffiansLeaving;
            Progress().SetLocation("village_gate", "gate_no_ruffians_leaving");
            Progress().Save();
            ruffians_.StartRetreat();
            break;
        case DialogueAction::StartSneakPursuit:
            phase_ = Phase::SneakPursuit;
            objective_ = "Lén bám theo đám người khả nghi";
            Progress().SetLocation("village_gate", "gate_no_pursuit");
            Progress().Save();
            player_.StartSneakTo(ExitPositionX);
            break;
        case DialogueAction::StartEscapeRun:
            phase_ = Phase::EscapeRun;
            objective_ = "Chạy ngược về phía làng";
            Progress().SetLocation("village_gate", "gate_yes_escape");
            Progress().Save();
            player_.SetDefeated(false);
            playerFace_.SetExpression(faces_, FaceGender::Male, "nosebleed");
            player_.StartRunTo(EscapePositionX);
            break;
        case DialogueAction::None:
        case DialogueAction::ThrowAtSystem:
        case DialogueAction::OfferSystemReward:
        case DialogueAction::LeaveBedroom:
        case DialogueAction::NeighborLeaves:
        case DialogueAction::EnterBambooVillage:
        case DialogueAction::FinishIntro:
        case DialogueAction::StartChickenQuest:
        case DialogueAction::SpawnShard:
        case DialogueAction::ReturnToElder:
        case DialogueAction::CompleteQuest:
        case DialogueAction::UnlockVillageExit:
        case DialogueAction::ReturnToNeighborHouse:
        case DialogueAction::ApproachNeighborFence:
        case DialogueAction::EnterNeighborBackyard:
        case DialogueAction::ShowProvocationChoice:
        case DialogueAction::BeginEnemyEncounter:
        case DialogueAction::PreviewCombatStorage:
        case DialogueAction::UnlockThrowSkill:
        case DialogueAction::FinishNeighborNight:
            break;
    }
}

void VillageGateLevel::SyncDialoguePresentation()
{
    if (!dialogue_.IsActive() || observedDialogueRevision_ == dialogue_.Revision()) return;
    observedDialogueRevision_ = dialogue_.Revision();
    const DialogueLine &line = dialogue_.Current();
    soundEffects_.Play(ParseMemeSound(line.sound));
    talkingRuffian_ = CurrentRuffianIndex();
    CharacterAction action = CharacterAction::None;
    if (line.portrait.rfind("player:", 0) == 0)
    {
        playerFace_.SetExpression(faces_, FaceGender::Male,
            std::string_view(line.portrait).substr(7));
        action = ParseCharacterAction(line.action);
    }
    else if (talkingRuffian_ >= 0)
    {
        const std::size_t separator = line.portrait.find(':');
        if (separator != std::string::npos)
            ruffians_.SetExpression(faces_, talkingRuffian_,
                                    std::string_view(line.portrait).substr(separator + 1));
    }
    player_.TriggerAction(action);
}

int VillageGateLevel::CurrentRuffianIndex() const
{
    if (!dialogue_.IsActive()) return -1;
    const std::string &key = dialogue_.Current().portrait;
    if (key.rfind("ruffian", 0) != 0 || key.size() < 8 || key[7] < '0' || key[7] > '3') return -1;
    return key[7] - '0';
}

const Texture2D *VillageGateLevel::CurrentPortrait() const
{
    if (!dialogue_.IsActive()) return nullptr;
    const std::string &key = dialogue_.Current().portrait;
    if (key.rfind("player:", 0) == 0) return player_.PortraitTexture();
    if (key == "system:idle") return systemDog_.Pose(SystemDogPose::Idle);
    if (key == "system:quest") return systemDog_.Pose(SystemDogPose::Quest);
    if (key == "system:laugh") return systemDog_.Pose(SystemDogPose::Laugh);
    const int index = CurrentRuffianIndex();
    return index >= 0 ? ruffians_.Portrait(index) : nullptr;
}

const Texture2D *VillageGateLevel::CurrentPortraitFace() const
{
    if (!dialogue_.IsActive()) return nullptr;
    if (dialogue_.Current().portrait.rfind("player:", 0) == 0)
        return playerFace_.Current(faces_);
    const int index = CurrentRuffianIndex();
    return index >= 0 ? ruffians_.PortraitFace(faces_, index) : nullptr;
}

const std::string &VillageGateLevel::Objective() const
{
    return objective_;
}

void VillageGateLevel::FinishScene(const char *checkpoint)
{
    phase_ = Phase::Finished;
    Progress().SetFlag("village_gate_scene_complete");
    Progress().SetLocation("neighbor_night", checkpoint);
    Progress().Save();
    transition_ = LevelTransition::NeighborNight;
}

void VillageGateLevel::Draw(const Ui &ui) const
{
    BeginMode2D(camera_);
    DrawTexturePro(background_, {0, 0, static_cast<float>(background_.width),
                                 static_cast<float>(background_.height)},
                   {0, 0, WorldWidth, static_cast<float>(ScreenHeight)},
                   {0, 0}, 0.0F, WHITE);

    systemDog_.DrawWorld(SystemDogPose::Idle, worldTime_);
    player_.Draw(playerFace_.Current(faces_));
    DrawTexturePro(hidingRock_,
                   {0, 0, static_cast<float>(hidingRock_.width),
                    static_cast<float>(hidingRock_.height)},
                   HidingRockDestination, {0, 0}, 0.0F, WHITE);
    if (ruffiansVisible_) ruffians_.Draw(faces_, worldTime_);
    if (phase_ == Phase::Fight && fightTimer_ < 1.35F)
        DrawFightCloud({1400, GroundY - 145.0F}, fightTimer_);
    EndMode2D();

    ui.DrawHud(Objective(), toastTimer_);
    if (choice_.IsActive())
        ui.DrawSystemChoice(choice_, systemDog_.Pose(SystemDogPose::Quest));
    else if (dialogue_.IsActive())
        ui.DrawDialogue(dialogue_.Current(), CurrentPortrait(), CurrentPortraitFace());

    if (phase_ == Phase::Fight)
        DrawRectangle(0, 0, ScreenWidth, ScreenHeight,
                      Fade(BLACK, FightBlackout(fightTimer_)));
}
} // namespace game
