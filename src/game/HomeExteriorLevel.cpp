#include "game/HomeExteriorLevel.hpp"
#include "game/ConversationFacing.hpp"
#include "game/GameProgress.hpp"
#include "game/TextureAsset.hpp"
#include "game/Ui.hpp"

#include <string_view>

namespace game
{
HomeExteriorLevel::~HomeExteriorLevel()
{
    UnloadTextureAsset(background_);
}

bool HomeExteriorLevel::Load()
{
    background_ = LoadTextureAsset("assets/environment/home1.png");
    if (background_.id == 0 || !player_.Load() || !neighbor_.Load() ||
        !systemDog_.Load() || !faces_.Load()) return false;

    // Hà Nhân stands beside his house on the right; the neighbor keeps her
    // natural right-facing artwork on the left, so neither her body nor face
    // needs to be mirrored in this conversation.
    constexpr Vector2 neighborPosition = {470.0F, 622.0F};
    constexpr Vector2 playerPosition = {770.0F, 622.0F};
    player_.Place(playerPosition, neighborPosition.x);
    neighbor_.Place(neighborPosition, playerPosition.x);
    systemDog_.PlaceForConversation(
        {playerPosition.x + 145.0F, playerPosition.y - 152.0F}, playerPosition.x);
    playerFace_.SetExpression(faces_, FaceGender::Male, "blushing");
    neighborFace_.SetExpression(faces_, FaceGender::Female, "happy");

    const std::string &checkpoint = Progress().Checkpoint();
    if (checkpoint == "neighbor_left")
    {
        neighborVisible_ = false;
        BeginReflectionDialogue();
    }
    else
    {
        Progress().SetLocation("home_exterior", "meeting_neighbor");
        Progress().Save();
        BeginMeetingDialogue();
    }
    return true;
}

void HomeExteriorLevel::BeginMeetingDialogue()
{
    dialogue_.Start({
        {"NGƯỜI DẪN CHUYỆN", "Hà Nhân đẩy cửa bước ra. Người gọi hắn là một cô gái trẻ đang ôm hộp thức ăn đứng chờ ngoài sân.", ""},
        {"HÀ NHÂN", "Mỹ... mỹ nữ?! Xuyên không quả nhiên vẫn còn chính sách bồi thường!", "player:blushing", "", "", DialogueTarget::Neighbor},
        {"TẨU TẨU", "Hà đệ! Đệ làm gì trong nhà từ sáng tới giờ vậy? Ta gọi khản cả cổ rồi.", "neighbor:annoyed", "", "", DialogueTarget::Player},
        {"HÀ NHÂN", "Tẩu gọi đệ à? Hộp kia... cũng là cho đệ sao?", "player:bashful_smile", "", "", DialogueTarget::Neighbor},
        {"TẨU TẨU", "Không cho đệ thì ta mang sang mời con chó sau lưng đệ chắc? Bánh ngô còn nóng đấy.", "neighbor:happy", "", "", DialogueTarget::Player},
        {"HỆ THỐNG", "Ting. Ta không phản đối phương án đó.", "system:laugh", "", "", DialogueTarget::Player},
        {"HÀ NHÂN", "Im! Ngươi vừa tịch thu quà của ta, giờ còn định ăn chặn bánh ngô?!", "player:annoyed", "point", "", DialogueTarget::System},
        {"TẨU TẨU", "Đệ lại nói chuyện với chó à? Thôi, ăn nhanh rồi tới nhà Trưởng thôn. Ông ấy đang tìm đệ khắp nơi.", "neighbor:concerned", "", "", DialogueTarget::Player},
        {"HÀ NHÂN", "Trưởng thôn tìm đệ? Có nói là chuyện gì không tẩu?", "player:confused", "hands_folded", "", DialogueTarget::Neighbor},
        {"TẨU TẨU", "Không nói. Nhưng mặt ông ấy nghiêm lắm. Đệ đừng để người ta chờ thêm nữa.", "neighbor:concerned", "", "", DialogueTarget::Player},
        {"HÀ NHÂN", "Đệ biết rồi. Đa tạ tẩu... với cả bánh ngô nữa.", "player:bashful_smile", "", "", DialogueTarget::Neighbor},
        {"TẨU TẨU", "Biết cảm ơn là được. Ta về đây, chiều nhớ trả hộp cho ta.", "neighbor:eyes_closed_smile", "", "", DialogueTarget::Player}},
        DialogueAction::NeighborLeaves);
    SyncDialoguePresentation();
}

void HomeExteriorLevel::BeginReflectionDialogue()
{
    dialogue_.Start({
        {"HỆ THỐNG", "Ting! Bánh Ngô Còn Nóng đã được cất vào Không gian lưu trữ. Dùng biểu tượng vòng sáng để kiểm tra.", "system:quest", "", "", DialogueTarget::Player},
        {"NGƯỜI DẪN CHUYỆN", "Bóng nàng khuất sau hàng rào. Những mảnh ký ức không thuộc về Hà Nhân chậm rãi hiện lên.", ""},
        {"HÀ NHÂN", "Thì ra nguyên chủ cũng tên Hà Nhân... cha mẹ mất sớm, lớn lên nhờ cơm của cả làng.", "player:reminiscing", "hands_folded"},
        {"NGƯỜI DẪN CHUYỆN", "Đây là làng Hà Gia. Nhà này do dân làng góp gỗ dựng, bữa đói bữa no đều có người sang ngó hắn.", ""},
        {"HÀ NHÂN", "Không họ hàng mà vẫn nuôi một đứa trẻ tới lớn... món nợ này nặng hơn tiền phòng karaoke rồi.", "player:emotional_pain", ""},
        {"HỆ THỐNG", "Ghi nhận mục tiêu sơ bộ: sống tử tế và đừng ném đá ân nhân.", "system:quest", "", "", DialogueTarget::Player},
        {"HÀ NHÂN", "Vế sau ngươi tự thêm đúng không?", "player:skeptical", "point", "", DialogueTarget::System},
        {"NGƯỜI DẪN CHUYỆN", "Hà Nhân ôm hộp bánh ngô, hít sâu rồi bước về phía nhà Trưởng thôn.", ""}},
        DialogueAction::EnterBambooVillage);
    SyncDialoguePresentation();
}

void HomeExteriorLevel::Update(float deltaTime, const GameInput &input)
{
    if (dialogue_.IsActive())
    {
        const DialogueAction action = dialogue_.Update(input.advanceDialoguePressed);
        if (action == DialogueAction::NeighborLeaves)
        {
            neighborVisible_ = false;
            Progress().GrantItem("banh_ngo_nong", 1);
            Progress().SetFlag("met_kind_neighbor");
            Progress().SetLocation("home_exterior", "neighbor_left");
            Progress().Save();
            BeginReflectionDialogue();
        }
        else if (action == DialogueAction::EnterBambooVillage)
        {
            Progress().SetLocation("bamboo_village", "level_start");
            Progress().Save();
            transition_ = LevelTransition::BambooVillage;
        }
        SyncDialoguePresentation();
        if (dialogue_.IsActive())
        {
            const std::string &portrait = dialogue_.Current().portrait;
            if (portrait.rfind("player:", 0) == 0) playerFace_.Update(deltaTime);
            else if (portrait.rfind("neighbor:", 0) == 0) neighborFace_.Update(deltaTime);
        }
    }
    player_.Update(deltaTime, false, input);
    systemDog_.UpdateReaction(deltaTime);
}

void HomeExteriorLevel::SyncDialoguePresentation()
{
    if (!dialogue_.IsActive() || observedDialogueRevision_ == dialogue_.Revision()) return;
    observedDialogueRevision_ = dialogue_.Revision();
    const DialogueLine &line = dialogue_.Current();
    if (line.target == DialogueTarget::System ||
        (line.target == DialogueTarget::Player && line.portrait.rfind("system:", 0) == 0))
        FaceConversationPartners(player_, systemDog_);
    else if (line.target == DialogueTarget::Neighbor ||
             (line.target == DialogueTarget::Player && line.portrait.rfind("neighbor:", 0) == 0))
        FaceConversationPartners(player_, neighbor_);

    if (line.portrait.rfind("player:", 0) == 0)
    {
        playerFace_.SetExpression(faces_, FaceGender::Male,
            std::string_view(line.portrait).substr(7));
        player_.TriggerAction(ParseCharacterAction(line.action));
    }
    else
    {
        player_.TriggerAction(CharacterAction::None);
    }
    if (line.portrait.rfind("neighbor:", 0) == 0)
        neighborFace_.SetExpression(faces_, FaceGender::Female,
            std::string_view(line.portrait).substr(9));
}

const Texture2D *HomeExteriorLevel::CurrentPortrait() const
{
    if (!dialogue_.IsActive()) return nullptr;
    const std::string &portrait = dialogue_.Current().portrait;
    if (portrait.rfind("player:", 0) == 0) return player_.PortraitTexture();
    if (portrait.rfind("neighbor:", 0) == 0) return neighbor_.PortraitTexture();
    if (portrait.rfind("system:", 0) == 0) return systemDog_.Pose(SystemDogPose::Idle);
    return nullptr;
}

const Texture2D *HomeExteriorLevel::CurrentPortraitFace() const
{
    if (!dialogue_.IsActive()) return nullptr;
    const std::string &portrait = dialogue_.Current().portrait;
    if (portrait.rfind("player:", 0) == 0) return playerFace_.Current(faces_);
    if (portrait.rfind("neighbor:", 0) == 0) return neighborFace_.Current(faces_);
    return nullptr;
}

void HomeExteriorLevel::Draw(const Ui &ui) const
{
    if (background_.id != 0)
    {
        const float sourceX = (background_.width - 1280.0F) * 0.5F;
        DrawTexturePro(background_, {sourceX, 0.0F, 1280.0F, 720.0F},
                       {0.0F, 0.0F, 1280.0F, 720.0F}, {0.0F, 0.0F}, 0.0F, WHITE);
    }
    systemDog_.DrawWorld(SystemDogPose::Idle, 0.0F);
    player_.Draw(playerFace_.Current(faces_));
    if (neighborVisible_) neighbor_.Draw(neighborFace_.Current(faces_));
    if (dialogue_.IsActive())
        ui.DrawDialogue(dialogue_.Current(), CurrentPortrait(), CurrentPortraitFace());
}

LevelTransition HomeExteriorLevel::RequestedTransition() const
{
    return transition_;
}
} // namespace game
