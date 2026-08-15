#include "game/NeighborNightLevel.hpp"

#include "game/Config.hpp"
#include "game/ConversationFacing.hpp"
#include "game/GameProgress.hpp"
#include "game/TextureAsset.hpp"
#include "game/Ui.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <string_view>
#include <utility>

namespace game
{
namespace
{
constexpr float ReturnStopX = 1500.0F;
constexpr float ReturnExitX = WorldWidth - 100.0F;
constexpr float FenceApproachX = 1420.0F;
constexpr float BackLandingX = 610.0F;
constexpr float BackEncounterX = 1010.0F;
constexpr const char *ThrowSkillId = "van_vat_phi_trich";
constexpr const char *CornCakeId = "banh_ngo_nong";
constexpr float SkillUnlockDuration = 4.2F;
constexpr float CombatImpactDuration = 2.35F;

void DrawLightning(Vector2 start, Vector2 end, float seed, float alpha)
{
    constexpr int Segments = 8;
    Vector2 previous = start;
    for (int i = 1; i <= Segments; ++i)
    {
        const float progress = static_cast<float>(i) / Segments;
        Vector2 point = {start.x + (end.x - start.x) * progress,
                         start.y + (end.y - start.y) * progress};
        if (i < Segments)
            point.x += std::sin(seed * 3.7F + i * 2.13F) * (18.0F - progress * 8.0F);
        DrawLineEx(previous, point, 8.0F, Fade(SKYBLUE, 0.15F * alpha));
        DrawLineEx(previous, point, 3.0F, Fade(RAYWHITE, alpha));
        previous = point;
    }
}

void DrawMeteorStrike(Vector2 center, float elapsed)
{
    const float descent = std::clamp((elapsed - 0.12F) / 0.62F, 0.0F, 1.0F);
    if (elapsed < 0.84F)
    {
        const Vector2 meteor = {center.x - 230.0F * (1.0F - descent),
                                center.y - 620.0F * (1.0F - descent) - 48.0F};
        const Vector2 direction = {-0.36F, -0.93F};
        for (int i = 7; i >= 1; --i)
        {
            const Vector2 trail = {meteor.x + direction.x * i * 24.0F,
                                   meteor.y + direction.y * i * 24.0F};
            DrawCircleV(trail, 8.0F + (8 - i) * 2.6F,
                        Fade(i > 4 ? RED : ORANGE, (8 - i) * 0.075F));
        }
        DrawCircleV(meteor, 48.0F, Fade(ORANGE, 0.18F));
        DrawCircleV(meteor, 31.0F, Color{242, 105, 38, 255});
        DrawCircleV({meteor.x - 8, meteor.y - 10}, 17.0F, Color{255, 218, 93, 255});
        DrawCircleV({meteor.x - 13, meteor.y - 14}, 7.0F, RAYWHITE);
    }

    static constexpr std::array<float, 3> StrikeOffsets = {-155.0F, 0.0F, 160.0F};
    static constexpr std::array<float, 3> StrikeDelays = {0.36F, 0.56F, 0.78F};
    for (int i = 0; i < 3; ++i)
    {
        const float local = elapsed - StrikeDelays[i];
        if (local < 0.0F || local > 0.34F) continue;
        const float boltAlpha = std::sin(local / 0.34F * PI);
        const Vector2 hit = {center.x + StrikeOffsets[i], center.y};
        DrawLightning({hit.x + (i - 1) * 34.0F, center.y - 650.0F}, hit,
                      1.7F + i * 2.4F, boltAlpha);
    }

    const float impact = std::clamp((elapsed - 0.68F) / 1.32F, 0.0F, 1.0F);
    if (impact <= 0.0F) return;
    const float fade = 1.0F - impact;
    DrawCircleV(center, 86.0F * fade, Fade(YELLOW, 0.42F * fade));
    DrawCircleLinesV(center, 55.0F + impact * 330.0F, Fade(GOLD, 0.95F * fade));
    DrawCircleLinesV(center, 24.0F + impact * 220.0F, Fade(RAYWHITE, 0.75F * fade));
    DrawEllipse(static_cast<int>(center.x), static_cast<int>(center.y + 8),
                250.0F * impact, 34.0F * fade, Fade(ORANGE, 0.38F * fade));
    for (int i = 0; i < 12; ++i)
    {
        const float angle = (-155.0F + i * 28.0F) * DEG2RAD;
        const float distance = 45.0F + impact * (110.0F + (i % 4) * 26.0F);
        const Vector2 debris = {center.x + std::cos(angle) * distance,
                                center.y + std::sin(angle) * distance * 0.58F};
        DrawCircleV(debris, 3.0F + (i % 3), Fade(i % 2 == 0 ? GOLD : ORANGE, fade));
    }
}
}

NeighborNightLevel::~NeighborNightLevel()
{
    UnloadTextureAsset(villageBackground_);
    UnloadTextureAsset(frontBackground_);
    UnloadTextureAsset(backBackground_);
    UnloadTextureAsset(heavenlyMeteorBurst_);
}

bool NeighborNightLevel::Load()
{
    villageBackground_ = LoadTextureAsset("assets/environment/bamboo_village_runtime.png");
    frontBackground_ = LoadTextureAsset("assets/environment/neighbor_house_front_night.png");
    backBackground_ = LoadTextureAsset("assets/environment/neighbor_house_back_night.png");
    heavenlyMeteorBurst_ = LoadTextureAsset("assets/effects/heavenly_meteor_burst.png");
    if (villageBackground_.id == 0 || frontBackground_.id == 0 ||
        backBackground_.id == 0 || heavenlyMeteorBurst_.id == 0 ||
        !player_.Load() || !systemDog_.Load() || !neighbor_.Load() ||
        !faces_.Load() || !ruffians_.Load(faces_))
        return false;

    camera_.offset = {ScreenWidth / 2.0F, ScreenHeight / 2.0F};
    camera_.target = {ScreenWidth / 2.0F, ScreenHeight / 2.0F};
    camera_.zoom = 1.0F;
    injured_ = Progress().Choice("village_gate_intervention") == "yes";
    RestoreCheckpoint();
    return true;
}

void NeighborNightLevel::RestoreCheckpoint()
{
    const std::string &checkpoint = Progress().Checkpoint();
    if (checkpoint == "return_village")
    {
        ConfigureReturnVillage();
        phase_ = Phase::ReturnEntering;
        player_.StartRunTo(ReturnStopX);
    }
    else if (checkpoint == "returning_to_neighbor")
    {
        ConfigureReturnVillage();
        player_.Place({ReturnStopX, GroundY}, ReturnExitX);
        playerFace_.SetExpression(faces_, FaceGender::Male, "nosebleed");
        phase_ = Phase::ReturnLeaving;
        player_.StartRunTo(ReturnExitX);
    }
    else if (checkpoint == "front_fence")
    {
        ConfigureFrontYard();
        player_.Place({760.0F, GroundY}, FenceApproachX);
        phase_ = Phase::FrontFenceWalk;
        player_.StartSneakTo(FenceApproachX);
    }
    else if (checkpoint == "backyard_transition")
    {
        ConfigureBackYard();
        phase_ = Phase::BackSneak;
        player_.StartSneakTo(BackLandingX);
    }
    else if (checkpoint == "combat_await_throw")
    {
        ConfigureBackYard();
        player_.Place({BackEncounterX, GroundY}, ruffians_.Position(0).x);
        ruffiansVisible_ = true;
        BeginCombatEncounter();
        Progress().UnlockSkill(ThrowSkillId);
        if (Progress().ItemCount(CornCakeId) == 0) Progress().GrantItem(CornCakeId, 1);
        phase_ = Phase::AwaitingThrow;
        objective_ = "Mở Không gian lưu trữ, chọn Bánh Ngô rồi bấm DÙNG";
    }
    else if (checkpoint == "combat_complete")
    {
        ConfigureBackYard();
        player_.Place({BackEncounterX, GroundY}, ruffians_.Position(0).x);
        ruffiansVisible_ = false;
        BeginNeighborAftermathDialogue();
    }
    else if (checkpoint == "awaiting_update")
    {
        ConfigureBackYard();
        phase_ = Phase::ChapterEnd;
        endingFadeAlpha_ = 1.0F;
        objective_.clear();
    }
    else if (checkpoint == "back_arrived" || checkpoint == "back_idle")
    {
        ConfigureBackYard();
        player_.Place({BackLandingX, GroundY}, BackEncounterX);
        BeginBackDialogue();
    }
    else
    {
        ConfigureFrontYard();
        BeginFrontDialogue();
    }
}

void NeighborNightLevel::ConfigureReturnVillage()
{
    scene_ = Scene::ReturnVillage;
    injured_ = true;
    ruffiansVisible_ = false;
    neighborVisible_ = false;
    player_.Place({WorldWidth - 105.0F, GroundY}, ReturnStopX);
    systemDog_.PlaceForConversation(
        {player_.Position().x + 145.0F, GroundY - 152.0F}, player_.Position().x);
    camera_.target.x = WorldWidth - ScreenWidth / 2.0F;
    playerFace_.SetExpression(faces_, FaceGender::Male, "nosebleed");
    objective_ = "Chạy về phía làng dưỡng thương";
}

void NeighborNightLevel::ConfigureFrontYard()
{
    scene_ = Scene::FrontYard;
    injured_ = false;
    ruffiansVisible_ = false;
    neighborVisible_ = false;
    player_.Place({270.0F, GroundY}, FenceApproachX);
    systemDog_.PlaceForConversation(
        {player_.Position().x - 145.0F, GroundY - 152.0F}, player_.Position().x);
    camera_.target.x = ScreenWidth / 2.0F;
    playerFace_.SetExpression(faces_, FaceGender::Male, "confused");
    objective_ = "Tìm dấu vết của đám người khả nghi";
}

void NeighborNightLevel::ConfigureBackYard()
{
    scene_ = Scene::BackYard;
    injured_ = false;
    ruffiansVisible_ = false;
    neighborVisible_ = false;
    player_.Place({260.0F, GroundY}, BackLandingX);
    systemDog_.PlaceForConversation(
        {player_.Position().x - 145.0F, GroundY - 152.0F}, player_.Position().x);
    camera_.target.x = ScreenWidth / 2.0F;
    playerFace_.SetExpression(faces_, FaceGender::Male, "knowing_smirk");
    objective_ = "Lẻn tới phía sau nhà";
}

void NeighborNightLevel::BeginReturnDialogue()
{
    phase_ = Phase::ReturnDialogue;
    objective_ = "Bình tâm lại sau trận đòn";
    dialogue_.Start({
        {"NGƯỜI DẪN CHUYỆN", "Hà Nhân chạy một mạch về con đường bắt gà ban sáng. Tới nơi, hai chân hắn mới nhớ ra mình vừa bị đánh.", ""},
        {"HÀ NHÂN", "Đau... đau từ lòng tự trọng lan thẳng xuống xương chậu. Bốn đánh một mà còn đánh rất có tổ chức!", "player:nosebleed", ""},
        {"HỆ THỐNG", "Chẩn đoán: ký chủ còn đủ răng để nghiến. Tình trạng tổng thể rất khả quan.", "system:quest", "", "", DialogueTarget::Player},
        {"HÀ NHÂN", "Khả quan cái đầu chó nhà ngươi! Nhưng ta cứ chạy thế này... tẩu tẩu phải làm sao?", "player:emotional_pain", "point", "", DialogueTarget::System},
        {"HÀ NHÂN", "Không cam tâm! Ta đánh không lại thì ta rình. Tẩu tẩu, chờ đệ!", "player:nosebleed", "present_open"}},
        DialogueAction::ReturnToNeighborHouse);
    SyncDialoguePresentation();
}

void NeighborNightLevel::BeginFrontDialogue()
{
    phase_ = Phase::FrontDialogue;
    dialogue_.Start({
        {"NGƯỜI DẪN CHUYỆN", "Trăng đã lên cao khi Hà Nhân tới trước nhà tẩu tẩu. Đèn vẫn sáng, đom đóm vẫn bay... chỉ có bốn cái mặt đáng ghét kia là mất hút.", ""},
        {"HÀ NHÂN", "Ủa? Người đâu? Ta bám theo sát thế mà...", "player:confused", "hands_folded"},
        {"HỆ THỐNG", "Phân tích chiến thuật: đối phương có chân. Ký chủ cũng có, nhưng dùng chưa hiệu quả.", "system:quest", "", "", DialogueTarget::Player},
        {"HÀ NHÂN", "Im. Cổng trước không có... vậy chúng vòng ra sau nhà?", "player:annoyed", "point", "", DialogueTarget::System},
        {"HÀ NHÂN", "Hàng rào này cản được gà, không cản được người đàn ông đang bảo vệ sự trong trắng của... dân làng!", "player:knowing_smirk", "present_open"}},
        DialogueAction::ApproachNeighborFence);
    SyncDialoguePresentation();
}

void NeighborNightLevel::BeginFenceDialogue()
{
    phase_ = Phase::FenceDialogue;
    dialogue_.Start({
        {"NGƯỜI DẪN CHUYỆN", "Hà Nhân ôm cọc tre, co một chân, đạp hai lần mới qua. Tư thế không giống cao thủ, giống con sâu đo có lòng tự trọng hơn.", ""},
        {"HỆ THỐNG", "Ting. Kỹ năng mới chưa mở khóa: Khinh Công. Ký chủ vừa dùng bản thử nghiệm bằng đầu gối.", "system:laugh", "", "", DialogueTarget::Player},
        {"HÀ NHÂN", "Nhỏ tiếng! Vòng ra sau nhà. Gặp chúng thì ưu tiên tìm chỗ núp trước, chính nghĩa tính sau.", "player:knowing_smirk", "point", "", DialogueTarget::System}},
        DialogueAction::EnterNeighborBackyard);
    SyncDialoguePresentation();
}

void NeighborNightLevel::BeginBackDialogue()
{
    phase_ = Phase::BackDialogue;
    objective_ = "Quan sát phía sau nhà";
    SaveCheckpoint("back_arrived");
    dialogue_.Start({
        {"NGƯỜI DẪN CHUYỆN", "Hà Nhân đáp xuống sau nhà bằng cả hai chân và gần nửa cái mông. Sân sau yên ắng đến đáng ngờ.", ""},
        {"HÀ NHÂN", "Không thấy bốn thằng kia... hay chúng đang núp rình cả ta luôn?", "player:confused", "hands_folded"},
        {"HỆ THỐNG", "Không phát hiện mục tiêu trong phạm vi gần. Khuyến nghị ký chủ thở nhỏ hơn tiếng trâu kéo cối.", "system:quest", "", "", DialogueTarget::Player},
        {"HÀ NHÂN", "Được. Từ giờ ta là bóng đêm... loại bóng đêm biết núp và rất quý mạng.", "player:knowing_smirk", ""}});
    SyncDialoguePresentation();
}

void NeighborNightLevel::BeginWhisperDialogue()
{
    phase_ = Phase::WhisperDialogue;
    ruffiansVisible_ = true;
    ruffians_.FaceToward(1750.0F);
    objective_ = "Nghe lén kế hoạch của đám nhóc hư";
    dialogue_.Start({
        {"NHÓC HƯ ÁO NÂU", "Nhỏ mồm thôi. Đợi đèn trong buồng tắt, hai đứa canh trước, hai đứa vòng sau.", "ruffian0:devious", ""},
        {"NHÓC HƯ ĐEO GÙI", "Ai hắt hơi thì tự úp mặt xuống bùn. Tối nay nhất định phải nhìn cho rõ... cái chum nước nhà nàng để đâu!", "ruffian1:knowing_smirk", ""},
        {"NHÓC HƯ TÓC DÀI", "Ừ, nhìn chum trước. Nếu tiện mắt thì nhìn thêm người tắm cạnh chum.", "ruffian2:smug", ""},
        {"HÀ NHÂN", "Bốn tên súc sinh này còn bày đội hình! Cẩu Hệ Thống, có phương án nào không?", "player:furious", "point", "", DialogueTarget::System},
        {"HỆ THỐNG", "Có. Ký chủ tự tin bước ra khiêu khích. Bổn Hệ Thống sẽ cung cấp tăng phúc chiến lực.", "system:quest", "", "", DialogueTarget::Player},
        {"HÀ NHÂN", "Nghe rất giống lần trước ngươi dụ ta lao ra ăn đòn.", "player:uneasy", "hands_folded"}},
        DialogueAction::ShowProvocationChoice);
    SyncDialoguePresentation();
}

void NeighborNightLevel::BeginProvocationChoice()
{
    phase_ = Phase::ProvocationChoice;
    choice_.Start({"PHƯƠNG ÁN TÁC CHIẾN",
                   "Tự tin bước ra khiêu khích? Hệ Thống sẽ buff năng lực cho ký chủ.",
                   "Độ đáng tin của lời hứa: chưa đo được.",
                   "RA CHỬI", "NÚP TIẾP"});
}

void NeighborNightLevel::ResolveProvocationChoice(ChoiceOption option)
{
    Progress().RecordChoice("neighbor_provocation", option == ChoiceOption::Yes ? "yes" : "no");
    Progress().Save();
    phase_ = Phase::ProvocationDialogue;
    if (option == ChoiceOption::Yes)
    {
        player_.FaceToward(ruffians_.Position(0).x);
        dialogue_.Start({
            {"HÀ NHÂN", "Bốn tên cẩu tặc kia! Đêm hôm không ngủ, tụ tập nghiên cứu chum nước nhà người ta à?", "player:furious", "point"},
            {"NHÓC HƯ ÁO NÂU", "Ơ, thằng ban chiều. Mặt hết bầm nhanh vậy?", "ruffian0:devious", ""},
            {"HÀ NHÂN", "Đó là tốc độ hồi phục của chính nghĩa!", "player:serious", "present_open"}},
            DialogueAction::BeginEnemyEncounter);
    }
    else
    {
        dialogue_.Start({
            {"HÀ NHÂN", "Không. Người quân tử phải biết nhịn... chờ hệ thống giải thích buff là buff cái gì đã.", "player:uneasy", "hands_folded"},
            {"NGƯỜI DẪN CHUYỆN", "Một cành khô dưới chân hắn phát ra tiếng 'rắc' rất có tinh thần tố giác.", ""},
            {"NHÓC HƯ ĐEO GÙI", "Ai ở đó?", "ruffian1:surprised", ""},
            {"HÀ NHÂN", "...Ta là tiếng dế có hình người.", "player:guilty_startle", ""}},
            DialogueAction::BeginEnemyEncounter);
    }
    SyncDialoguePresentation();
}

void NeighborNightLevel::BeginCombatEncounter()
{
    ruffiansVisible_ = true;
    player_.FaceToward(ruffians_.Position(0).x);
    ruffians_.FaceToward(player_.Position().x);
    combat_.BeginEncounter({"ruffian_group", "ĐÁM NHÓC HƯ HÀ GIA", 1, true});
}

void NeighborNightLevel::BeginEnemyEncounterDialogue()
{
    BeginCombatEncounter();
    phase_ = Phase::CaughtDialogue;
    dialogue_.Start({
        {"HÀ NHÂN", "Rồi, chúng thấy ta rồi. Tăng phúc đâu? Chiến lực đâu?", "player:terrified", "point", "", DialogueTarget::System},
        {"HỆ THỐNG", "Phương án tối ưu đã hoàn tất tính toán. Mời ký chủ mở Không gian lưu trữ.", "system:quest", "", "", DialogueTarget::Player},
        {"HÀ NHÂN", "Đang bị vây mà ngươi bảo ta kiểm kê tài sản?", "player:stunned_disbelief", ""}},
        DialogueAction::PreviewCombatStorage);
    SyncDialoguePresentation();
}

void NeighborNightLevel::BeginStorageExplanation()
{
    phase_ = Phase::StorageExplanation;
    dialogue_.Start({
        {"HÀ NHÂN", "Có quần què gì đâu! Trong đó chỉ có đúng một cái bánh ngô!", "player:furious", "point", "", DialogueTarget::System},
        {"HỆ THỐNG", "Chính xác. Đó là thần khí phù hợp nhất với tu vi hiện tại của ký chủ.", "system:quest", "", "", DialogueTarget::Player},
        {"HÀ NHÂN", "Thần khí nào thơm mùi bột với hành vậy?", "player:stunned_disbelief", "hands_folded"},
        {"HỆ THỐNG", "Ting. Phát hiện lòng quyết tử. Chuẩn bị mở khóa kỹ năng ném cơ bản.", "system:quest", "", "", DialogueTarget::Player},
        {"HÀ NHÂN", "Ta không quyết tử! Ta đang sợ chết đến muốn khóc đây này!", "player:ugly_crying", "present_open"}},
        DialogueAction::UnlockThrowSkill);
    SyncDialoguePresentation();
}

void NeighborNightLevel::UnlockThrowSkill()
{
    Progress().UnlockSkill(ThrowSkillId);
    if (Progress().ItemCount(CornCakeId) == 0) Progress().GrantItem(CornCakeId, 1);
    Progress().SetFlag("tutorial.throw_skill_unlocked", true);
    phase_ = Phase::AwaitingThrow;
    objective_ = "Mở Không gian lưu trữ, chọn Bánh Ngô rồi bấm DÙNG";
    notice_ = "VẠN VẬT PHI TRỊCH";
    skillUnlockTimer_ = SkillUnlockDuration;
    SaveCheckpoint("combat_await_throw");
}

void NeighborNightLevel::BeginVictoryDialogue()
{
    phase_ = Phase::VictoryDialogue;
    ruffiansVisible_ = false;
    skillUnlockTimer_ = 0.0F;
    if (!Progress().Flag("quest.night_ruffians.completed"))
    {
        Progress().SetFlag("quest.night_ruffians.completed", true);
        Progress().AddExperience(25);
        Progress().Save();
    }
    notice_ = "NHIỆM VỤ HOÀN THÀNH  ·  THƯỞNG 25 KINH NGHIỆM";
    rewardNoticeTimer_ = 5.0F;
    dialogue_.Start({
        {"NGƯỜI DẪN CHUYỆN", "Chiếc bánh ngô xé gió, xoay ba vòng đầy khí thế rồi đập trúng giữa đội hình. Bốn tên đồng loạt ngã vì... quá bất ngờ.", ""},
        {"NHÓC HƯ ÁO NÂU", "Nó dùng lương khô làm ám khí! Chạy! Thằng này nghèo đến mức không còn quy tắc!", "ruffian0:surprised", ""},
        {"HÀ NHÂN", "...Cái bánh vừa gọi thiên thạch thật à? Ta chỉ định ném cho đau thôi mà!", "player:surprised", ""},
        {"HÀ NHÂN", "Khoan... chiêu này là do ta mạnh, hay do cái bánh có thù với cuộc đời?", "player:stunned_disbelief", "hands_folded"},
        {"HỆ THỐNG", "Ting! Nhiệm vụ 'Đập tan âm mưu nhìn trộm' đã hoàn thành. Phần thưởng: 25 kinh nghiệm.", "system:quest", "", "", DialogueTarget::Player},
        {"HỆ THỐNG", "Xác nhận mục tiêu mất 1 điểm sinh lực. Vật phẩm Bánh Ngô đã tiêu hao. Bữa khuya của ký chủ cũng đã tiêu hao.", "system:laugh", "", "", DialogueTarget::Player},
        {"HÀ NHÂN", "...Khoan. Vậy lát nữa ta ăn gì?", "player:blank_stare", ""}});
    SyncDialoguePresentation();
}

void NeighborNightLevel::BeginNeighborEntrance()
{
    phase_ = Phase::NeighborEntering;
    neighborVisible_ = true;
    objective_ = "Có người đang đi ra từ phía nhà sau";
    neighbor_.Place({1600.0F, GroundY}, player_.Position().x);
    neighbor_.StartWalkTo(1250.0F);
    neighborFace_.SetExpression(faces_, FaceGender::Female, "nervous_smile");
    player_.FaceToward(neighbor_.Position().x);
    playerFace_.SetExpression(faces_, FaceGender::Male, "surprised");
}

void NeighborNightLevel::BeginNeighborAftermathDialogue()
{
    phase_ = Phase::NeighborAftermath;
    neighborVisible_ = true;
    neighbor_.Place({1250.0F, GroundY}, player_.Position().x);
    neighbor_.FaceToward(player_.Position().x);
    player_.FaceToward(neighbor_.Position().x);
    objective_ = "Nói chuyện với tẩu tẩu";
    dialogue_.Start({
        {"TẨU TẨU", "Hà đệ? Tỷ nghe ngoài này nổ như thiên lôi đánh bếp, lại thấy mấy người chạy mất dép. Đệ có bị thương không?", "neighbor:worried", "", "", DialogueTarget::Player},
        {"HÀ NHÂN", "Không sao! Đệ chỉ tiện tay dùng một cái bánh ngô giáo dục bốn thanh niên lầm đường.", "player:cool", "present_open", "", DialogueTarget::Neighbor},
        {"TẨU TẨU", "Bánh ngô... nổ thành cái hố này sao? Hà đệ, từ bao giờ đồ ăn nhà đệ nóng tính vậy?", "neighbor:shocked", "", "", DialogueTarget::Player},
        {"HÀ NHÂN", "Chuyện dài lắm. Tóm lại là bánh đã hy sinh, người vẫn còn nguyên, danh dự của tỷ cũng được bảo toàn.", "player:serious", "hands_folded", "", DialogueTarget::Neighbor},
        {"TẨU TẨU", "Đêm lạnh rồi. Nếu đệ không chê... vào nhà tỷ ngồi một lát đi. Tỷ pha trà, tiện xem vết thương cho đệ.", "neighbor:embarrassed", "", "", DialogueTarget::Player},
        {"HÀ NHÂN", "Vào... trong nhà? Đêm khuya? Chỉ có hai người? Vết thương này bỗng nhiên đau khắp những chỗ rất cần được chăm sóc!", "player:scheming_daydream", "present_open", "", DialogueTarget::Neighbor},
        {"TẨU TẨU", "Hà đệ, vẻ mặt đệ hiện giờ khiến tỷ muốn mời đệ đứng ngoài thêm một lúc.", "neighbor:annoyed", ""},
        {"HÀ NHÂN", "Khụ! Tỷ hiểu lầm rồi. Đây là vẻ mặt chính nhân quân tử đang cảm động trước tình làng nghĩa xóm!", "player:devious", "hands_folded"}},
        DialogueAction::FinishNeighborNight);
    SyncDialoguePresentation();
}

void NeighborNightLevel::BeginEndingFade()
{
    phase_ = Phase::EndingFade;
    objective_.clear();
    endingFadeAlpha_ = 0.0F;
}

void NeighborNightLevel::ApplyDialogueAction(DialogueAction action)
{
    switch (action)
    {
        case DialogueAction::ReturnToNeighborHouse:
            phase_ = Phase::ReturnLeaving;
            objective_ = "Quay lại bảo vệ tẩu tẩu";
            playerFace_.SetExpression(faces_, FaceGender::Male, "nosebleed");
            SaveCheckpoint("returning_to_neighbor");
            player_.StartRunTo(ReturnExitX);
            break;
        case DialogueAction::ApproachNeighborFence:
            phase_ = Phase::FrontFenceWalk;
            objective_ = "Lẻn tới hàng rào tre";
            playerFace_.SetExpression(faces_, FaceGender::Male, "knowing_smirk");
            SaveCheckpoint("front_fence");
            player_.StartSneakTo(FenceApproachX);
            break;
        case DialogueAction::EnterNeighborBackyard:
            SaveCheckpoint("backyard_transition");
            BeginSceneFade(Scene::BackYard, Phase::BackSneak);
            break;
        case DialogueAction::ShowProvocationChoice:
            phase_ = Phase::QuestIssued;
            questIssuedTimer_ = 4.2F;
            objective_ = "Bảo vệ tẩu tẩu khỏi đám nhóc hư";
            Progress().SetFlag("quest.protect_neighbor.started", true);
            Progress().Save();
            break;
        case DialogueAction::BeginEnemyEncounter:
            BeginEnemyEncounterDialogue();
            break;
        case DialogueAction::PreviewCombatStorage:
            phase_ = Phase::StoragePreview;
            pendingMenuCommand_ = SystemMenuCommand::PreviewStorage;
            break;
        case DialogueAction::UnlockThrowSkill:
            UnlockThrowSkill();
            break;
        case DialogueAction::FinishNeighborNight:
            BeginEndingFade();
            break;
        default: break;
    }
}

void NeighborNightLevel::Update(float deltaTime, const GameInput &input)
{
    worldTime_ += deltaTime;
    skillUnlockTimer_ = std::max(0.0F, skillUnlockTimer_ - deltaTime);
    rewardNoticeTimer_ = std::max(0.0F, rewardNoticeTimer_ - deltaTime);
    questIssuedTimer_ = std::max(0.0F, questIssuedTimer_ - deltaTime);

    if (phase_ == Phase::EndingFade)
    {
        endingFadeAlpha_ = std::min(1.0F, endingFadeAlpha_ + deltaTime / 1.15F);
        if (endingFadeAlpha_ >= 1.0F)
        {
            phase_ = Phase::ChapterEnd;
            SaveCheckpoint("awaiting_update");
        }
    }
    else if (phase_ == Phase::ChapterEnd)
    {
    }
    else if (phase_ == Phase::FadingOut)
    {
        fadeAlpha_ = std::min(1.0F, fadeAlpha_ + deltaTime / 0.55F);
        if (fadeAlpha_ >= 1.0F)
        {
            if (fadeDestination_ == Scene::FrontYard) ConfigureFrontYard();
            else ConfigureBackYard();
            phase_ = Phase::FadingIn;
        }
    }
    else if (phase_ == Phase::FadingIn)
    {
        fadeAlpha_ = std::max(0.0F, fadeAlpha_ - deltaTime / 0.55F);
        if (fadeAlpha_ <= 0.0F)
        {
            phase_ = phaseAfterFade_;
            if (phase_ == Phase::FrontDialogue)
            {
                SaveCheckpoint("front_arrival");
                BeginFrontDialogue();
            }
            else if (phase_ == Phase::BackSneak)
                player_.StartSneakTo(BackLandingX);
        }
    }
    else
    {
        if (choice_.IsActive())
        {
            const ChoiceOption selected = choice_.Update(input);
            if (selected != ChoiceOption::None) ResolveProvocationChoice(selected);
        }
        else if (dialogue_.IsActive())
        {
            const DialogueAction action = dialogue_.Update(input.advanceDialoguePressed);
            ApplyDialogueAction(action);
            SyncDialoguePresentation();
            if (!dialogue_.IsActive())
            {
                if (phase_ == Phase::BackDialogue)
                {
                    phase_ = Phase::BackApproach;
                    objective_ = "Rón rén tiến sâu vào sân sau";
                    player_.StartSneakTo(BackEncounterX);
                }
                else if (phase_ == Phase::VictoryDialogue)
                {
                    Progress().SetFlag("neighbor_ruffians_defeated", true);
                    SaveCheckpoint("combat_complete");
                    BeginNeighborEntrance();
                }
            }
        }
        else if (phase_ == Phase::QuestIssued && questIssuedTimer_ <= 0.0F)
        {
            BeginProvocationChoice();
        }
        else if (phase_ == Phase::StoragePreview)
        {
            BeginStorageExplanation();
        }

        if (phase_ == Phase::AwaitingThrow)
        {
            CombatCommand command = combat_.ConsumeCommand();
            if (command.function == ItemFunction::Throw)
            {
                activeCombatCommand_ = std::move(command);
                phase_ = Phase::Throwing;
                objective_ = "Vạn Vật Phi Trịch!";
                playerFace_.SetExpression(faces_, FaceGender::Male, "serious");
                player_.StartThrow({ruffians_.Position(0).x, GroundY - 150.0F},
                                   ThrowProjectileKind::CornCake);
            }
        }

        const bool inputEnabled = phase_ == Phase::BackIdle;
        const PlayerEvent event = player_.Update(deltaTime, inputEnabled, input);
        if (event == PlayerEvent::ScriptedMoveFinished)
        {
            if (phase_ == Phase::ReturnEntering) BeginReturnDialogue();
            else if (phase_ == Phase::ReturnLeaving)
                BeginSceneFade(Scene::FrontYard, Phase::FrontDialogue);
            else if (phase_ == Phase::FrontFenceWalk) BeginFenceDialogue();
            else if (phase_ == Phase::BackSneak) BeginBackDialogue();
            else if (phase_ == Phase::BackApproach) BeginWhisperDialogue();
        }
        else if (event == PlayerEvent::ThrowHit && phase_ == Phase::Throwing)
        {
            const bool defeated = combat_.ResolveHit(Progress(), activeCombatCommand_);
            Progress().Save();
            phase_ = Phase::CombatImpact;
            combatImpactTimer_ = defeated ? CombatImpactDuration : 1.65F;
        }

        if (phase_ == Phase::CombatImpact)
        {
            combatImpactTimer_ = std::max(0.0F, combatImpactTimer_ - deltaTime);
            if (combatImpactTimer_ <= 0.0F)
            {
                if (combat_.Health() <= 0)
                {
                    combat_.EndEncounter();
                    BeginVictoryDialogue();
                }
                else
                    phase_ = Phase::AwaitingThrow;
            }
        }

        const NeighborWomanEvent neighborEvent = neighbor_.Update(deltaTime);
        if (neighborEvent == NeighborWomanEvent::MoveFinished &&
            phase_ == Phase::NeighborEntering)
            BeginNeighborAftermathDialogue();
    }

    talkingRuffian_ = CurrentRuffianIndex();
    ruffians_.Update(deltaTime, talkingRuffian_);
    if (dialogue_.IsActive() && dialogue_.Current().portrait.rfind("player:", 0) == 0)
        playerFace_.Update(deltaTime);
    if (dialogue_.IsActive() && dialogue_.Current().portrait.rfind("neighbor:", 0) == 0)
        neighborFace_.Update(deltaTime);
    systemDog_.Update(deltaTime, player_.Position(), player_.FacingRight());

    float focusX = player_.Position().x;
    if (ruffiansVisible_ && scene_ == Scene::BackYard)
        focusX = (player_.Position().x + ruffians_.Position(1).x) * 0.5F;
    else if (neighborVisible_ && scene_ == Scene::BackYard)
        focusX = (player_.Position().x + neighbor_.Position().x) * 0.5F;
    const float desiredX = std::clamp(focusX,
        ScreenWidth / 2.0F, WorldWidth - ScreenWidth / 2.0F);
    camera_.target.x += (desiredX - camera_.target.x) * std::min(1.0F, deltaTime * 5.5F);
    camera_.target.y = ScreenHeight / 2.0F;
}

void NeighborNightLevel::SyncDialoguePresentation()
{
    if (!dialogue_.IsActive() || observedDialogueRevision_ == dialogue_.Revision()) return;
    observedDialogueRevision_ = dialogue_.Revision();
    const DialogueLine &line = dialogue_.Current();
    if (line.target == DialogueTarget::System ||
        (line.target == DialogueTarget::Player && line.portrait.rfind("system:", 0) == 0))
        FaceConversationPartners(player_, systemDog_);

    if (line.portrait.rfind("player:", 0) == 0)
    {
        playerFace_.SetExpression(faces_, FaceGender::Male,
            std::string_view(line.portrait).substr(7));
        player_.TriggerAction(ParseCharacterAction(line.action));
    }
    else
        player_.TriggerAction(CharacterAction::None);

    if (line.portrait.rfind("neighbor:", 0) == 0)
        neighborFace_.SetExpression(faces_, FaceGender::Female,
            std::string_view(line.portrait).substr(9));

    const int ruffian = CurrentRuffianIndex();
    if (ruffian >= 0)
    {
        ruffians_.SetExpression(faces_, ruffian,
            std::string_view(line.portrait).substr(9));
        if (phase_ != Phase::WhisperDialogue) ruffians_.FaceToward(player_.Position().x);
    }
}

SystemMenuCommand NeighborNightLevel::ConsumeSystemMenuCommand()
{
    const SystemMenuCommand command = pendingMenuCommand_;
    pendingMenuCommand_ = SystemMenuCommand::None;
    return command;
}

void NeighborNightLevel::BeginSceneFade(Scene destination, Phase afterFade)
{
    dialogue_.Start({});
    fadeDestination_ = destination;
    phaseAfterFade_ = afterFade;
    fadeAlpha_ = 0.0F;
    phase_ = Phase::FadingOut;
}

void NeighborNightLevel::SaveCheckpoint(const char *checkpoint)
{
    Progress().SetLocation("neighbor_night", checkpoint);
    Progress().Save();
}

const Texture2D &NeighborNightLevel::CurrentBackground() const
{
    if (scene_ == Scene::ReturnVillage) return villageBackground_;
    if (scene_ == Scene::BackYard) return backBackground_;
    return frontBackground_;
}

int NeighborNightLevel::CurrentRuffianIndex() const
{
    if (!dialogue_.IsActive()) return -1;
    const std::string &portrait = dialogue_.Current().portrait;
    if (portrait.size() < 9 || portrait.rfind("ruffian", 0) != 0) return -1;
    const int index = portrait[7] - '0';
    return index >= 0 && index < 4 && portrait[8] == ':' ? index : -1;
}

const Texture2D *NeighborNightLevel::CurrentPortrait() const
{
    if (!dialogue_.IsActive()) return nullptr;
    const std::string &portrait = dialogue_.Current().portrait;
    if (portrait.rfind("player:", 0) == 0) return player_.PortraitTexture();
    if (portrait.rfind("system:", 0) == 0) return systemDog_.Pose(SystemDogPose::Quest);
    if (portrait.rfind("neighbor:", 0) == 0) return neighbor_.PortraitTexture();
    const int ruffian = CurrentRuffianIndex();
    return ruffian >= 0 ? ruffians_.Portrait(ruffian) : nullptr;
}

const Texture2D *NeighborNightLevel::CurrentPortraitFace() const
{
    if (!dialogue_.IsActive()) return nullptr;
    if (dialogue_.Current().portrait.rfind("player:", 0) == 0)
        return playerFace_.Current(faces_);
    if (dialogue_.Current().portrait.rfind("neighbor:", 0) == 0)
        return neighborFace_.Current(faces_);
    const int ruffian = CurrentRuffianIndex();
    return ruffian >= 0 ? ruffians_.PortraitFace(faces_, ruffian) : nullptr;
}

void NeighborNightLevel::Draw(const Ui &ui) const
{
    const Texture2D &background = CurrentBackground();
    Camera2D effectCamera = camera_;
    float impactElapsed = 0.0F;
    if (phase_ == Phase::CombatImpact && combatImpactTimer_ > 0.0F)
    {
        const float duration = combat_.Health() <= 0 ? CombatImpactDuration : 1.65F;
        impactElapsed = duration - combatImpactTimer_;
        if (impactElapsed > 0.56F)
        {
            const float decay = 1.0F - std::clamp((impactElapsed - 0.56F) / 1.65F, 0.0F, 1.0F);
            effectCamera.target.x += std::sin(worldTime_ * 83.0F) * 16.0F * decay;
            effectCamera.target.y += std::cos(worldTime_ * 67.0F) * 11.0F * decay;
        }
    }

    BeginMode2D(effectCamera);
    DrawTexturePro(background,
        {0, 0, static_cast<float>(background.width), static_cast<float>(background.height)},
        {0, 0, WorldWidth, static_cast<float>(ScreenHeight)}, {0, 0}, 0.0F, WHITE);
    if (ruffiansVisible_) ruffians_.Draw(faces_, worldTime_);
    if (neighborVisible_) neighbor_.Draw(neighborFace_.Current(faces_));
    systemDog_.DrawWorld(SystemDogPose::Idle, worldTime_);
    player_.Draw(playerFace_.Current(faces_));
    if (phase_ == Phase::CombatImpact && combatImpactTimer_ > 0.0F)
    {
        const Vector2 impact = {(ruffians_.Position(1).x + ruffians_.Position(2).x) * 0.5F,
                                GroundY - 82.0F};
        DrawMeteorStrike(impact, impactElapsed);
        const float burstTime = impactElapsed - 0.48F;
        if (burstTime > 0.0F && heavenlyMeteorBurst_.id != 0)
        {
            const float reveal = std::clamp(burstTime / 0.24F, 0.0F, 1.0F);
            const float fade = 1.0F - std::clamp((burstTime - 1.05F) / 0.72F, 0.0F, 1.0F);
            const float overshoot = 1.0F + std::sin(reveal * PI) * 0.18F;
            const float size = 770.0F * reveal * overshoot;
            const Rectangle source = {0, 0,
                static_cast<float>(heavenlyMeteorBurst_.width),
                static_cast<float>(heavenlyMeteorBurst_.height)};
            const Rectangle destination = {impact.x - size * 0.5F,
                                           impact.y - size * 0.76F,
                                           size, size};
            BeginBlendMode(BLEND_ADDITIVE);
            DrawTexturePro(heavenlyMeteorBurst_, source, destination,
                           {0, 0}, std::sin(burstTime * 11.0F) * 1.2F,
                           Fade(WHITE, fade * 0.94F));
            if (burstTime < 0.92F)
            {
                const float echo = 1.0F - burstTime / 0.92F;
                const float echoSize = size * (1.08F + (1.0F - echo) * 0.12F);
                DrawTexturePro(heavenlyMeteorBurst_, source,
                               {impact.x - echoSize * 0.5F,
                                impact.y - echoSize * 0.76F,
                                echoSize, echoSize},
                               {0, 0}, -1.4F, Fade(SKYBLUE, echo * 0.22F));
            }
            EndBlendMode();
        }
    }
    EndMode2D();

    if (phase_ == Phase::CombatImpact && combatImpactTimer_ > 0.0F)
    {
        float flash = 0.0F;
        if (impactElapsed > 0.34F && impactElapsed < 0.48F)
            flash = std::sin((impactElapsed - 0.34F) / 0.14F * PI);
        if (impactElapsed > 0.66F && impactElapsed < 0.92F)
            flash = std::max(flash, std::sin((impactElapsed - 0.66F) / 0.26F * PI));
        if (flash > 0.0F)
            DrawRectangle(0, 0, ScreenWidth, ScreenHeight, Fade(RAYWHITE, flash * 0.48F));
    }

    ui.DrawHud(objective_, 0.0F);
    combat_.DrawHud(ui.GetFont());
    if (questIssuedTimer_ > 0.0F)
        ui.DrawQuestIssued("HỘ HOA TRỪ TẶC",
                           "Ngăn đám nhóc hư tiếp cận nhà tẩu tẩu trong đêm.",
                           "25 kinh nghiệm  ·  Giữ được mạng thì nhận",
                           questIssuedTimer_, 4.2F,
                           systemDog_.Pose(SystemDogPose::Quest));
    else if (skillUnlockTimer_ > 0.0F)
        ui.DrawSkillUnlock(notice_, skillUnlockTimer_, SkillUnlockDuration,
                           systemDog_.Pose(SystemDogPose::Quest));
    else if (rewardNoticeTimer_ > 0.0F)
        ui.DrawSystemNotice(notice_, rewardNoticeTimer_, systemDog_.Pose(SystemDogPose::Quest));
    if (dialogue_.IsActive())
        ui.DrawDialogue(dialogue_.Current(), CurrentPortrait(), CurrentPortraitFace());
    if (choice_.IsActive())
        ui.DrawSystemChoice(choice_, systemDog_.Pose(SystemDogPose::Quest));
    if (fadeAlpha_ > 0.0F)
        DrawRectangle(0, 0, ScreenWidth, ScreenHeight, Fade(BLACK, fadeAlpha_));
    if (endingFadeAlpha_ > 0.0F)
        DrawRectangle(0, 0, ScreenWidth, ScreenHeight, Fade(BLACK, endingFadeAlpha_));
    if (phase_ == Phase::ChapterEnd)
    {
        const Font font = ui.GetFont();
        const Rectangle panel = {330, 245, 620, 210};
        DrawRectangleRounded(panel, 0.08F, 10, Color{10, 24, 32, 245});
        DrawRectangleRoundedLinesEx(panel, 0.08F, 10, 2.0F,
                                    Color{75, 218, 207, 230});
        DrawTextLine(font, "TING  ·  THÔNG BÁO HỆ THỐNG",
                     {panel.x + 42, panel.y + 34}, 22, Color{75, 218, 207, 255});
        DrawTextLine(font, "NHIỆM VỤ TIẾP THEO ĐANG CẬP NHẬT",
                     {panel.x + 42, panel.y + 84}, 29, RAYWHITE);
        DrawTextLine(font, "Ký chủ vui lòng giữ nguyên tư thế chính nhân quân tử.",
                     {panel.x + 42, panel.y + 135}, 20, Fade(RAYWHITE, 0.72F));
    }
}
} // namespace game
