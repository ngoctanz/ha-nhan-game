#include "game/BambooVillageLevel.hpp"
#include "game/Config.hpp"
#include "game/ConversationFacing.hpp"
#include "game/GameProgress.hpp"
#include "game/TextureAsset.hpp"
#include "game/Ui.hpp"

#include <algorithm>
#include <array>
#include <cmath>

namespace game
{
namespace
{
constexpr float ElderInteractionDistance = 145.0F;

const char *ElderFaceName(std::string_view emotion)
{
    if (emotion == "laughing") return "laughing";
    if (emotion == "angry") return "angry";
    if (emotion == "shocked") return "stunned_disbelief";
    if (emotion == "sad") return "crying";
    if (emotion == "scheming") return "scheming_daydream";
    if (emotion == "embarrassed") return "bashful_smile";
    if (emotion == "proud") return "knowing_look";
    return "old_man";
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
    UnloadTextureAsset(background_);
}

bool BambooVillageLevel::Load()
{
    background_ = LoadTextureAsset("assets/environment/bamboo_village_runtime.png");
    const bool playerLoaded = player_.Load();
    const bool elderLoaded = elder_.Load();
    const bool facesLoaded = faces_.Load();
    const bool dogLoaded = systemDog_.Load();
    const bool chickenLoaded = chicken_.Load();
    const bool charactersLoaded = playerLoaded && elderLoaded && facesLoaded &&
                                  dogLoaded && chickenLoaded;
    playerFace_.SetExpression(faces_, FaceGender::Male, "smug");
    elderFace_.SetExpression(faces_, FaceGender::Male, "old_man");
    soundEffects_.Load();

    camera_.offset = {ScreenWidth / 2.0F, ScreenHeight / 2.0F};
    camera_.target = {ScreenWidth / 2.0F, ScreenHeight / 2.0F};
    camera_.zoom = 1.0F;
    chicken_.SetPosition({1460.0F, GroundY});

    quest_.Register({
        "bamboo_chicken", "Gà chạy mất dép", QuestCategory::Main,
        QuestDifficulty::Easy, 0, "", QuestReward{1, 0, 0, {}}});
    quest_.Register({
        "bamboo_duck", "Con vịt biết khinh công", QuestCategory::Main,
        QuestDifficulty::Normal, 1, "bamboo_chicken", QuestReward{0, 100, 0, {}}});

    if (quest_.IsComplete("bamboo_chicken"))
    {
        if (Progress().Checkpoint() == "village_exit_reached")
        {
            questState_ = QuestState::LeavingVillage;
            transition_ = LevelTransition::VillageGate;
        }
        else if (Progress().Flag("chicken_lore_discussed"))
        {
            questState_ = QuestState::LeaveVillage;
            dialogue_.Start({});
        }
        else
        {
            questState_ = QuestState::Complete;
            BeginChickenLoreDialogue();
        }
    }
    else
    {
        elder_.FaceToward(elderPosition_.x, player_.Position().x);
        player_.FaceToward(elderPosition_.x);
        dialogue_.Start({
        {"NGƯỜI DẪN CHUYỆN", "Theo lời tẩu tẩu, Hà Nhân ôm hộp bánh ngô tới chỗ Trưởng thôn làng Hà Gia.", ""},
        {"TRƯỞNG THÔN", "Hà Nhân! Cuối cùng ngươi cũng chịu ló mặt. Ta cho người tìm ngươi từ sáng tới giờ!", "elder:angry", "", "", DialogueTarget::Player},
        {"HÀ NHÂN", "Tẩu tẩu vừa bảo là con chạy sang ngay. Trưởng thôn tìm con có chuyện gì gấp vậy?", "player:concerned", "hands_folded", "", DialogueTarget::Elder},
        {"TRƯỞNG THÔN", "Gà Linh Khí nhà ta vừa trộm một mảnh bảng hiệu rồi chạy mất. Ngươi đi bắt nó về.", "elder:shocked", "", "", DialogueTarget::Player},
        {"HÀ NHÂN", "Khoan... vẻ mặt nghiêm trọng này chỉ vì một con gà với miếng bảng hiệu thôi á?!", "player:stunned_disbelief", "present_open", "huh", DialogueTarget::Elder},
        {"HỆ THỐNG", "Cảnh báo: con gà có tu vi cao hơn ký chủ hai cảnh giới. Đề nghị giữ lại chút thể diện.", "system:quest", "", "", DialogueTarget::Player},
        {"HÀ NHÂN", "Ta còn chưa có cảnh giới mà nó đã hơn hai cảnh?! Cái làng này nuôi gà hay nuôi tổ tông vậy?!", "player:furious", "point", "cao_ni_ma", DialogueTarget::System},
        {"TRƯỞNG THÔN", "Bắt được nó, ta thưởng thêm một cơm nắm. Bánh ngô trên tay ngươi không được tính là tiền công.", "elder:scheming", "", "", DialogueTarget::Player},
        {"HÀ NHÂN", "Đây là tẩu tẩu cho con! Thôi được, vì cơm nắm... con bắt!", "player:resigned", "", "", DialogueTarget::Elder}},
        DialogueAction::StartChickenQuest);
        quest_.Start("bamboo_chicken", "Nghe Trưởng thôn giao việc");
    }
    return background_.id != 0 && charactersLoaded;
}

void BambooVillageLevel::Update(float deltaTime, const GameInput &input)
{
    input_ = input;
    worldTime_ += deltaTime;
    toastTimer_ = std::max(0.0F, toastTimer_ - deltaTime);
    questIssuedTimer_ = std::max(0.0F, questIssuedTimer_ - deltaTime);
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
    if (questState_ == QuestState::LeaveVillage && !dialogue_.IsActive() &&
        player_.Position().x >= WorldWidth - 112.0F)
    {
        questState_ = QuestState::LeavingVillage;
        Progress().SetLocation("bamboo_village", "village_exit_reached");
        Progress().Save();
        transition_ = LevelTransition::VillageGate;
    }
    if (!dialogue_.IsActive())
        playerFace_.SetExpression(faces_, FaceGender::Male,
                                  player_.SuggestedFaceExpression());
    else
    {
        // Six-frame face clips represent talking motion. Keep every world
        // character static outside dialogue, and animate only the current
        // human speaker instead of making idle actors silently mouth words.
        const std::string &portrait = dialogue_.Current().portrait;
        const DialogueTarget target = dialogue_.Current().target;
        if (target == DialogueTarget::System ||
            (target == DialogueTarget::Player && portrait.rfind("system:", 0) == 0))
            FaceConversationPartners(player_, systemDog_);
        else if (target == DialogueTarget::Elder ||
                 (target == DialogueTarget::Player && portrait.rfind("elder:", 0) == 0))
            FaceConversationPartners(player_, elder_, elderPosition_);

        if (portrait == "player" || portrait.rfind("player:", 0) == 0)
            playerFace_.Update(deltaTime);
        else if (portrait.rfind("elder:", 0) == 0)
            elderFace_.Update(deltaTime);
    }
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

        playerFace_.SetExpression(faces_, FaceGender::Male, "smug");
        elderFace_.SetExpression(faces_, FaceGender::Male, "old_man");
        const std::string &portrait = dialogue_.Current().portrait;
        constexpr std::string_view playerPrefix = "player:";
        constexpr std::string_view elderPrefix = "elder:";
        if (portrait.rfind(playerPrefix, 0) == 0)
            playerFace_.SetExpression(faces_, FaceGender::Male,
                                      std::string_view(portrait).substr(playerPrefix.size()));
        else if (portrait == "player")
            playerFace_.SetExpression(faces_, FaceGender::Male, "skeptical");
        else if (portrait.rfind(elderPrefix, 0) == 0)
            elderFace_.SetExpression(faces_, FaceGender::Male,
                                     ElderFaceName(std::string_view(portrait).substr(elderPrefix.size())));
    }
    else
    {
        playerFace_.SetExpression(faces_, FaceGender::Male,
                                  player_.SuggestedFaceExpression());
        elderFace_.SetExpression(faces_, FaceGender::Male, "old_man");
    }
    player_.TriggerAction(action);
}

void BambooVillageLevel::BeginChickenResultDialogue()
{
    dialogue_.Start({
        {"GÀ LINH KHÍ", "Cục tác! Ba lần mới chạm được ta, nhân loại yếu đuối.", ""},
        {"HÀ NHÂN", "Khoan, gà ở thế giới này biết nói à?!", "player:stunned_disbelief", "", "doit"},
        {"HỆ THỐNG", "Mục tiêu đã dùng truyền tống phù. Nói cách khác: con gà chạy mất dép.", "system:quest"}},
        DialogueAction::SpawnShard);
}

void BambooVillageLevel::BeginChickenLoreDialogue()
{
    dialogue_.Start({
        {"HÀ NHÂN", "Khoan đã... con gà vừa rồi rốt cuộc là giống gì? Gà nhà ai lại chạy bằng truyền tống phù?!", "player:confused_frown", "hands_folded", "", DialogueTarget::System},
        {"HỆ THỐNG", "Đang tra cứu... Gà Linh Khí biến dị, huyết mạch không rõ, sở trường bỏ chạy và nhìn người bằng nửa con mắt.", "system:quest", "", "", DialogueTarget::Player},
        {"HÀ NHÂN", "Hai cái sau ta tự nhìn ra được! Nói phần đáng tiền đi!", "player:annoyed", "point", "", DialogueTarget::System},
        {"HỆ THỐNG", "Nó cảm nhận được vật mang dao động của Hệ Thống. Giữa cả cái làng, nó lại mổ đúng mảnh gỗ nhiễm năng lượng rồi tha đi.", "system:idle", "", "", DialogueTarget::Player},
        {"HÀ NHÂN", "Ý ngươi là con gà đó có thể tìm ra những mảnh Hệ Thống khác?", "player:realization", "", "", DialogueTarget::System},
        {"HỆ THỐNG", "Chính xác. Có thể xem nó là la bàn tìm bảo vật... có chân, có mỏ, và cực kỳ thiếu đạo đức nghề nghiệp.", "system:laugh", "", "", DialogueTarget::Player},
        {"HÀ NHÂN", "Nếu bắt được nó, ta vừa tìm được bảo vật, vừa có trứng ăn mỗi sáng! Một vốn hai lời!", "player:scheming_daydream", "present_open", "", DialogueTarget::System},
        {"HỆ THỐNG", "Nhắc nhẹ: tu vi của gà vẫn cao hơn ký chủ. Trong quan hệ hiện tại, ký chủ mới là nguồn thịt dự phòng.", "system:quest", "", "", DialogueTarget::Player},
        {"HÀ NHÂN", "Im! Nó chạy về phía đầu làng đúng không? Đi ngay, trước khi cái la bàn biết gáy ấy bay mất!", "player:annoyed", "point", "", DialogueTarget::System}},
        DialogueAction::UnlockVillageExit);
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

    bool interactPressed = input.interactPressed;

    // Ưu tiên đối tượng nhiệm vụ hiện tại trước
    // Gà: Tự động kích hoạt khi người chơi tiến lại gần, hoặc nếu người chơi click trực tiếp
    if (questState_ == QuestState::CatchChicken && chicken_.CanInteract() &&
        (chickenTapped || player_.IsNear(chicken_.Position().x, 135.0F)))
    {
        if (!input.sneakHeld)
        {
            // Player charged in without sneaking — chicken notices and bolts.
            const Vector2 chickenPosition = chicken_.Position();
            float dir = (GetRandomValue(0, 1) == 0) ? -1.0F : 1.0F;
            float destX = chickenPosition.x + dir * 260.0F;
            if (destX < 190.0F || destX > WorldWidth - 190.0F) destX = chickenPosition.x - dir * 260.0F;
            const Vector2 target = { std::clamp(destX, 190.0F, WorldWidth - 190.0F), GroundY };
            chicken_.StartEscape(target, false);
            toast_ = catchAttempts_ == 0
                ? "Gà Linh Khí giật mình bỏ chạy! Hãy thử đi lén lút lại gần..."
                : "Nó còn nhanh hơn cả trước. Phải bật chế độ NHẸ từ xa!";
            toastTimer_ = 3.2F;
            return;
        }

        // Bắt được gà (do đã bật sneak)
        ++catchAttempts_;
        quest_.SetProgress(catchAttempts_);
        const Vector2 chickenPosition = chicken_.Position();
        if (catchAttempts_ < 3)
        {
            float dir = (GetRandomValue(0, 1) == 0) ? -1.0F : 1.0F;
            float destX = chickenPosition.x + dir * 350.0F;
            if (destX < 190.0F || destX > WorldWidth - 190.0F) destX = chickenPosition.x - dir * 350.0F;
            const Vector2 target = { std::clamp(destX, 190.0F, WorldWidth - 190.0F), GroundY };
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

    if (!interactPressed && !elderTapped && !shardTapped) return;

    if (questState_ == QuestState::CollectShard && shardVisible_ &&
        (shardTapped || (interactPressed && player_.IsNear(shardPosition_.x, 120.0F))))
    {
        dialogue_.Start({
            {"HỆ THỐNG", "Đã quét vật phẩm: không rõ nguồn gốc, có mùi... chuồng gà. Khuyên không nên liếm.", "system:idle"},
            {"HÀ NHÂN", "Thôi mang về trước, biết đâu bán được hai đồng.", "player:uneasy", "", "huh"}},
            DialogueAction::ReturnToElder);
        return;
    }

    if (elderTapped ||
        (interactPressed && player_.IsNear(elderPosition_.x, ElderInteractionDistance)))
    {
        // Stationary NPCs only choose a direction when an interaction starts.
        // The direction is then kept until the next conversation.
        elder_.FaceToward(elderPosition_.x, player_.Position().x);
        if (questState_ == QuestState::MeetElder)
        {
            dialogue_.Start({
                {"TRƯỞNG THÔN", "Thiếu hiệp đến đúng lúc! Gà Linh Khí vừa trộm mất mảnh ngọc lạ.", "elder:shocked"},
                {"HÀ NHÂN", "Bắt gà thôi mà cũng gọi là nhiệm vụ chính tuyến sao?", "player:mentally_exhausted", "present_open", "huh"},
                {"HỆ THỐNG", "Cảnh báo: con gà có tu vi cao hơn ký chủ hai cảnh giới. Đề nghị giữ lại chút thể diện.", "system:quest"},
                {"TRƯỞNG THÔN", "Bắt được nó, ta thưởng một cơm nắm. Không bắt được thì tối nay khỏi ăn.", "elder:scheming"}},
                DialogueAction::StartChickenQuest);
        }
        else if (questState_ == QuestState::ReturnToElder)
        {
            dialogue_.Start({
                {"HÀ NHÂN", "Đây là mảnh ngọc. Con gà kia chạy nhanh hơn cả cao thủ Kim Đan!", "player:angry"},
                {"TRƯỞNG THÔN", "Ngọc gì? Đó là mảnh bảng hiệu chuồng gà nhà ta.", "elder:laughing"},
                {"HỆ THỐNG", "Phát hiện vật phẩm cấp Thần: Mảnh Hệ Thống giả. Hàng thật đang bảo trì.", "system:laugh"},
                {"HÀ NHÂN", "Thế cơm nắm của con đâu?", "player:skeptical", "point", "", DialogueTarget::Elder},
                {"TRƯỞNG THÔN", "À... lúc chờ ngươi lâu quá, ta đói nên ăn mất rồi.", "elder:embarrassed", "", "", DialogueTarget::Player},
                {"HỆ THỐNG", "Đang quyết toán phần thưởng: ký chủ thăng 1 cấp. Vật phẩm nhận được: không có.", "system:quest", "", "doit", DialogueTarget::Player},
                {"HÀ NHÂN", "Bắt gà ba vòng làng, nhận về đúng một con số?! Ít nhất cho ta cái lá gói cơm chứ!", "player:furious", "present_open", "cao_ni_ma", DialogueTarget::System}},
                DialogueAction::CompleteQuest);
        }
        else if (questState_ == QuestState::Complete)
        {
            dialogue_.Start({
                {"TRƯỞNG THÔN", "Ngày mai nhớ quay lại. Ta còn một con vịt biết khinh công.", "elder:proud"},
                {"HÀ NHÂN", "Hệ thống! Cho ta xuyên về ngay, ta không bắt thêm con gì nữa đâu!", "player:angry", "point", "", DialogueTarget::System},
                {"HỆ THỐNG", "Tính năng chưa mở khóa. Vui lòng tu luyện thêm... hoặc nộp cơm nắm.", "system:laugh"}});
        }
        else
        {
            toast_ = "Trưởng thôn: Lo bắt gà đi, cơm nguội bây giờ!";
            toastTimer_ = 2.8F;
        }
        return;
    }
}

void BambooVillageLevel::ApplyDialogueAction(DialogueAction action)
{
    switch (action)
    {
        case DialogueAction::FinishIntro:
        case DialogueAction::ThrowAtSystem:
        case DialogueAction::OfferSystemReward:
        case DialogueAction::LeaveBedroom:
        case DialogueAction::NeighborLeaves:
        case DialogueAction::EnterBambooVillage:
            break;
        case DialogueAction::StartChickenQuest:
            questState_ = QuestState::CatchChicken;
            chicken_.SetAlert(true);
            quest_.SetObjective("Bắt Gà Linh Khí", 3);
            Progress().SetLocation("bamboo_village", "quest_bamboo_chicken");
            Progress().Save();
            toast_ = "Nhiệm vụ mới: Bắt Gà Linh Khí";
            toastTimer_ = 3.5F;
            questIssuedTimer_ = 4.2F;
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
            if (Progress().ItemCount("manh_bang_hieu_chuong_ga") == 0)
                Progress().GrantItem("manh_bang_hieu_chuong_ga", 1);
            Progress().SetLocation("bamboo_village", "quest_bamboo_chicken_return");
            Progress().Save();
            toast_ = "Đã nhận: Mảnh Hệ Thống x1";
            toastTimer_ = 3.5F;
            break;
        case DialogueAction::CompleteQuest:
            questState_ = QuestState::Complete;
            Progress().RemoveItem("manh_bang_hieu_chuong_ga", 1);
            quest_.Complete();
            Progress().SetLocation("bamboo_village", "quest_bamboo_chicken_complete");
            Progress().Save();
            toast_ = "Nhiệm vụ hoàn thành! Cấp độ +1. Vật phẩm: không có.";
            toastTimer_ = 6.0F;
            BeginChickenLoreDialogue();
            break;
        case DialogueAction::UnlockVillageExit:
            questState_ = QuestState::LeaveVillage;
            Progress().SetFlag("chicken_lore_discussed");
            Progress().SetLocation("bamboo_village", "village_exit_unlocked");
            Progress().Save();
            toast_ = "Mục tiêu mới: Đi tới đầu làng";
            toastTimer_ = 4.0F;
            break;
        case DialogueAction::ReturnToNeighborHouse:
        case DialogueAction::ApproachNeighborFence:
        case DialogueAction::EnterNeighborBackyard:
        case DialogueAction::ShowProvocationChoice:
        case DialogueAction::BeginEnemyEncounter:
        case DialogueAction::PreviewCombatStorage:
        case DialogueAction::UnlockThrowSkill:
        case DialogueAction::FinishNeighborNight:
            break;
        case DialogueAction::None: break;
    }
}

const std::string &BambooVillageLevel::Objective() const
{
    if (questState_ == QuestState::LeaveVillage) return exitObjective_;
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
    {
        if (!input_.sneakHeld)
            return MobileControlsEnabled()
                ? "Bật NHẸ rồi đi lại gần để bắt"
                : "[Ctrl] bật đi lén lại gần để chộp gà";
        return ""; // Đã tự động kích hoạt, không cần prompt nữa
    }
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
    return playerFace_.Current(faces_);
}

const Texture2D *BambooVillageLevel::CurrentPortraitFace() const
{
    if (!dialogue_.IsActive()) return nullptr;
    const std::string &key = dialogue_.Current().portrait;
    if (key == "player" || key.rfind("player:", 0) == 0) return CurrentPlayerFace();
    if (key.rfind("elder:", 0) == 0) return elderFace_.Current(faces_);
    return nullptr;
}

void BambooVillageLevel::Draw(const Ui &ui) const
{
    BeginMode2D(camera_);
    // Runtime background is authored at the exact 2400x720 logical world size.
    // Draw it 1:1; GroundY already falls inside its continuous dirt road.
    DrawRectangle(0, 0, static_cast<int>(WorldWidth), ScreenHeight,
                  Color{178, 150, 104, 255});
    DrawTexturePro(background_,
                   {0, 0, static_cast<float>(background_.width), static_cast<float>(background_.height)},
                   {0, 0, WorldWidth, static_cast<float>(ScreenHeight)},
                   {0, 0}, 0.0F, WHITE);
    DrawRectangle(0, static_cast<int>(GroundY + 3), static_cast<int>(WorldWidth), 100,
                  Fade(Color{46, 55, 35, 255}, 0.13F));

    elder_.Draw(elderPosition_, worldTime_, elderFace_.Current(faces_));
    chicken_.Draw();
    if (shardVisible_) DrawShard(shardPosition_, worldTime_);

    const Font font = ui.GetFont();
    if (questState_ == QuestState::MeetElder || questState_ == QuestState::ReturnToElder)
        DrawQuestMarker({elderPosition_.x, elderPosition_.y - 340.0F}, font,
                        "!", Color{229, 164, 42, 255},
                        worldTime_);
    if (questState_ == QuestState::CatchChicken && !chicken_.IsHidden())
        DrawQuestMarker({chicken_.Position().x, chicken_.Position().y - 145.0F}, font,
                        "!", Color{220, 80, 70, 255}, worldTime_);
    if (questState_ == QuestState::CollectShard)
        DrawQuestMarker({shardPosition_.x, shardPosition_.y - 75.0F}, font, "!", Color{52, 175, 225, 255}, worldTime_);
    if (questState_ == QuestState::LeaveVillage)
        DrawQuestMarker({WorldWidth - 72.0F, GroundY - 145.0F}, font, ">",
                        Color{75, 218, 207, 255}, worldTime_);

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
        if (questIssuedTimer_ > 0.0F)
            ui.DrawQuestIssued("BẮT GÀ LINH KHÍ",
                               "Đuổi con gà phá làng và thu hồi thứ nó đánh rơi.",
                               "Thăng 1 cấp  ·  Vật phẩm: không có",
                               questIssuedTimer_, 4.2F,
                               systemDog_.Pose(SystemDogPose::Quest));
        else
            ui.DrawSystemNotice(toast_, toastTimer_, systemDog_.Pose(noticePose));
        ui.DrawPrompt(InteractionPrompt());
    }
    ui.DrawLevelTitle(levelTitleTimer_);
}
} // namespace game
