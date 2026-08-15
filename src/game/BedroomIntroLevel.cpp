#include "game/BedroomIntroLevel.hpp"
#include "game/ConversationFacing.hpp"
#include "game/GameInput.hpp"
#include "game/GameProgress.hpp"
#include "game/TextureAsset.hpp"
#include "game/Ui.hpp"

#include <string_view>

namespace game
{
BedroomIntroLevel::~BedroomIntroLevel()
{
    UnloadTextureAsset(background_);
}

bool BedroomIntroLevel::Load()
{
    background_ = LoadTextureAsset("assets/environment/myroom1.png");
    const bool playerLoaded = player_.Load();
    const bool facesLoaded = faces_.Load();
    const bool dogLoaded = systemDog_.Load();
    soundEffects_.Load();
    playerFace_.SetExpression(faces_, FaceGender::Male, "blank_stare");

    dialogue_.Start({
        {"NGƯỜI DẪN CHUYỆN", "Hà Nhân từ từ mở mắt. Trần gỗ. Giường tre. Một căn phòng cổ tới mức Wi-Fi cũng không dám vào.", ""},
        {"HÀ NHÂN", "...Ủa? Phòng karaoke đổi concept cổ trang hồi nào vậy?", "player:blank_stare", "", "huh"},
        {"NGƯỜI DẪN CHUYỆN", "Hắn nhìn quanh: bàn gỗ, đèn dầu, cửa giấy. Không TV. Không điều hòa. Không bảng giá.", ""},
        {"HÀ NHÂN", "Điện thoại đâu? Ví đâu? Dép tổ ong phiên bản giới hạn của ta đâu?!", "player:confused", "present_open"},
        {"NGƯỜI DẪN CHUYỆN", "Ngoài cửa là đường đất và mái ngói. Không một cột điện. Chỉ có tiếng gà gáy đầy sát khí.", ""},
        {"HÀ NHÂN", "Nhà cổ, đồ cổ, không sóng... Đừng nói với ta là...", "player:realization", "hands_folded"},
        {"HÀ NHÂN", "TA XUYÊN KHÔNG RỒI À?!", "player:stunned_disbelief", "point", "doit"},
        {"NGƯỜI DẪN CHUYỆN", "Ký ức cuối cùng ùa về: phòng karaoke, đèn tím, và em KTV đang nằm gọn trong vòng tay hắn.", ""},
        {"EM KTV (KÝ ỨC)", "Anh ơi... hôn thì tính thêm giờ nha anh.", ""},
        {"NGƯỜI DẪN CHUYỆN", "Môi nàng còn cách đúng 2,7 xăng-ti-mét thì đèn chớp. Vũ trụ lập tức cướp cò.", ""},
        {"HÀ NHÂN", "KHÔÔÔNG! Bảy tháng ăn mì gói! Bảy tháng không topping!", "player:crying", "point", "cao_ni_ma"},
        {"HÀ NHÂN", "Ta vừa dồn đủ tiền đi tay vịn một lần! Ôm được rồi mà còn chưa kịp hôn!", "player:ugly_crying", "present_open"},
        {"HÀ NHÂN", "Ông trời ơi, cho con quay lại ba giây thôi! Hai giây cũng được, con hôn nhanh lắm!", "player:emotional_pain", "hands_folded"},
        {"NGƯỜI DẪN CHUYỆN", "Hắn nhắm mắt, mở lại. Căn phòng vẫn cổ. Số tiền karaoke thì đã thành di sản.", ""},
        {"HÀ NHÂN", "Xuyên không nhà người ta có thần công, mỹ nữ. Ta có căn phòng nghèo và tiếng gà!", "player:furious", "present_open"},
        {"???", "Ting! Đã liên kết thành công: HỆ THỐNG SINH TỒN CỔ ĐẠI, bản 0.0.1 beta.", "system:quest", "", "doit"},
        {"HÀ NHÂN", "Cái gì vừa ting? Ai liên kết? Ta có bấm đồng ý đâu?!", "player:guilty_startle", "point"},
        {"HỆ THỐNG", "Ký chủ đã chớp mắt hai lần. Hệ thống ghi nhận: đồng ý điều khoản và tất cả cookie.", "system:laugh"},
        {"HÀ NHÂN", "Cookie cái đầu ngươi! Ngươi là chó à? Hệ thống nhà ai lại có bốn chân?!", "player:angry", "present_open"},
        {"HỆ THỐNG", "Ta là giao diện hỗ trợ sinh tồn thân thiện. Hình chó giúp ký chủ bớt tuyệt vọng khoảng 0,4%.", "system:idle"},
        {"HÀ NHÂN", "Ta mất nụ hôn 2,7 phân để đổi lấy 0,4% và một con chó biết ting?", "player:resigned", ""},
        {"HÀ NHÂN", "Thôi được. Rốt cuộc hệ thống ngươi có tác dụng gì?", "player:skeptical", "hands_folded"},
        {"HỆ THỐNG", "Biểu tượng hình người mở Hồ sơ Ký chủ: cấp độ, kinh nghiệm và trạng thái. Hiện tại ký chủ đang ở cấp 0.", "system:quest"},
        {"HÀ NHÂN", "Cấp 0 mà cũng làm hồ sơ long trọng thế? Ghi thẳng 'vô dụng nhưng còn thở' cho nhanh.", "player:resigned", "hands_folded"},
        {"HỆ THỐNG", "Biểu tượng vòng sáng mở Không gian lưu trữ. Bản cơ bản đã dùng được; cấp 10 mới mở rộng dung lượng.", "system:idle"},
        {"HÀ NHÂN", "Tên thì kêu như chứa được tam giới, hóa ra hiện giờ chỉ là cái tủ có đèn.", "player:annoyed", "point"},
        {"HÀ NHÂN", "Vậy mở cửa hàng thần khí!", "player:annoyed", "point"},
        {"HỆ THỐNG", "Chưa đủ cấp độ. Cửa hàng mở ở cấp 20 và yêu cầu lịch sử tín dụng sạch.", "system:quest"},
        {"HÀ NHÂN", "Bản đồ? Quét cao thủ? Chỉ đường tới quán cơm? Cái nào cũng được!", "player:worried", "present_open"},
        {"HỆ THỐNG", "Bản đồ mở ở cấp 5. Quét thuộc tính mở ở cấp 15. Chỉ đường đang bảo trì.", "system:idle"},
        {"HÀ NHÂN", "Thế dịch chuyển ta về phòng karaoke được không?!", "player:surprised", "point"},
        {"HỆ THỐNG", "Được. Vui lòng đạt cấp 999, cứu ba giới và mua gói Hồi Hương Premium.", "system:laugh"},
        {"HÀ NHÂN", "Vậy hiện tại ngươi làm được cái quái gì?!", "player:furious", "point", "cao_ni_ma"},
        {"HỆ THỐNG", "Ta có thể phát tiếng ting, báo ký chủ chưa đủ cấp và đồng hành về mặt tinh thần.", "system:quest", "", "doit"},
        {"HÀ NHÂN", "Ta không cần đồng hành tinh thần! Ta cần hoàn tiền karaoke!", "player:ugly_crying", "present_open"},
        {"HÀ NHÂN", "Hệ thống phế vật! Ít nhất mở cho ta chức năng chửi ngươi đi!", "player:crying", "point"},
        {"HỆ THỐNG", "Chức năng khiếu nại mở ở cấp 30. Chửi miễn phí đã được bật mặc định.", "system:laugh"},
        {"HÀ NHÂN", "Được. Nếu cái gì cũng khóa... chức năng ném đá chắc cấp 0 dùng được nhỉ?", "player:furious", ""}},
        DialogueAction::ThrowAtSystem);

    const bool loaded = background_.id != 0 && playerLoaded && facesLoaded && dogLoaded;
    if (!loaded) return false;

    const std::string &checkpoint = Progress().Checkpoint();
    if (checkpoint != "intro_start")
    {
        const Vector2 playerPosition = player_.Position();
        systemDog_.PlaceForConversation(
            {playerPosition.x + 170.0F, playerPosition.y - 152.0F}, playerPosition.x);
        systemVisible_ = true;
    }
    if (checkpoint == "throwing_system")
    {
        const Vector2 dogPosition = systemDog_.Position();
        dialogue_.Start({});
        player_.StartThrow({dogPosition.x, dogPosition.y - 58.0F});
    }
    else if (checkpoint == "after_throw") BeginAfterThrowDialogue();
    else if (checkpoint == "reward_prompt") BeginRewardChoice();
    else if (checkpoint == "after_system_penalty")
    {
        const ChoiceOption savedChoice = Progress().Choice("open_starter_reward_attempt") == "no"
                                             ? ChoiceOption::No
                                             : ChoiceOption::Yes;
        BeginPenaltyDialogue(savedChoice);
    }
    else if (checkpoint == "intro_complete")
    {
        dialogue_.Start({});
        transition_ = LevelTransition::HomeExterior;
    }
    else
    {
        Progress().SetLocation("bedroom_intro", "intro_start");
        Progress().Save();
    }
    return true;
}

void BedroomIntroLevel::Update(float deltaTime, const GameInput &input)
{
    if (systemChoice_.IsActive())
    {
        const ChoiceOption choice = systemChoice_.Update(input);
        if (choice != ChoiceOption::None) ResolveRewardChoice(choice);
    }
    else if (dialogue_.IsActive())
    {
        const DialogueAction action = dialogue_.Update(input.advanceDialoguePressed);
        if (action == DialogueAction::ThrowAtSystem)
        {
            const Vector2 dogPosition = systemDog_.Position();
            Progress().SetLocation("bedroom_intro", "throwing_system");
            Progress().Save();
            player_.StartThrow({dogPosition.x, dogPosition.y - 58.0F});
        }
        else if (action == DialogueAction::OfferSystemReward)
            BeginRewardChoice();
        else if (action == DialogueAction::LeaveBedroom)
        {
            Progress().SetLocation("home_exterior", "meeting_neighbor");
            Progress().Save();
            transition_ = LevelTransition::HomeExterior;
        }
        else if (action == DialogueAction::FinishIntro)
        {
            finished_ = true;
            Progress().SetLocation("bedroom_intro", "intro_complete");
            Progress().Save();
        }
        SyncDialoguePresentation();
        if (dialogue_.IsActive())
        {
            const std::string &portrait = dialogue_.Current().portrait;
            if (portrait == "player" || portrait.rfind("player:", 0) == 0)
                playerFace_.Update(deltaTime);
        }
    }
    if (player_.Update(deltaTime, false, input) == PlayerEvent::ThrowHit)
    {
        systemDog_.TriggerHit();
        systemPose_ = SystemDogPose::Idle;
        Progress().SetFlag("assaulted_system");
        BeginAfterThrowDialogue();
    }
    systemDog_.UpdateReaction(deltaTime);
}

void BedroomIntroLevel::BeginAfterThrowDialogue()
{
    Progress().SetLocation("bedroom_intro", "after_throw");
    Progress().Save();
    dialogue_.Start({
        {"NGƯỜI DẪN CHUYỆN", "Cục đá vẽ một đường cong đầy oán khí, đáp gọn lên trán Hệ Thống.", ""},
        {"HỆ THỐNG", "Ting! Đã mở khóa thành tựu: Bạo hành nhân viên chăm sóc khách hàng.", "system:quest", "", "doit"},
        {"HÀ NHÂN", "Ủa?! Cái thành tựu vô dụng này thì mở nhanh thế?!", "player:stunned_disbelief", "point"},
        {"HỆ THỐNG", "Tính năng né đòn mở ở cấp 40. Hiện tại ta chỉ có thể tiếp nhận bằng trán.", "system:idle"},
        {"HỆ THỐNG", "Bỏ qua hành vi thiếu văn minh: Gói Tân Thủ Bí Ẩn đã được chuyển vào Không gian lưu trữ. Dùng biểu tượng vòng sáng để kiểm tra.", "system:quest"}},
        DialogueAction::OfferSystemReward);
    SyncDialoguePresentation();
}

void BedroomIntroLevel::BeginRewardChoice()
{
    dialogue_.Start({});
    if (Progress().ItemCount("goi_tan_thu_bi_an") == 0)
        Progress().GrantItem("goi_tan_thu_bi_an");
    Progress().SetLocation("bedroom_intro", "reward_prompt");
    Progress().Save();
    systemChoice_.Start({
        "NHẬN ĐƯỢC VẬT PHẨM!",
        "Gói Tân Thủ Bí Ẩn đã được gửi vào Không gian lưu trữ. Ký chủ có muốn mở ngay không?",
        "Gói Tân Thủ Bí Ẩn  ×1"});
}

void BedroomIntroLevel::ResolveRewardChoice(ChoiceOption attemptedChoice)
{
    Progress().RecordChoice("open_starter_reward_attempt",
                            attemptedChoice == ChoiceOption::Yes ? "yes" : "no");
    Progress().RecordChoice("open_starter_reward_result", "forced_no");
    Progress().RemoveItem("goi_tan_thu_bi_an");
    Progress().SetFlag("starter_reward_confiscated");
    Progress().SetLocation("bedroom_intro", "after_system_penalty");
    Progress().Save();
    BeginPenaltyDialogue(attemptedChoice);
}

void BedroomIntroLevel::BeginPenaltyDialogue(ChoiceOption attemptedChoice)
{
    if (attemptedChoice == ChoiceOption::No)
    {
        dialogue_.Start({
            {"HỆ THỐNG", "Lựa chọn đã được ghi nhận: KHÔNG.", "system:idle", "", "doit"},
            {"HÀ NHÂN", "Ừ, ta chưa mở thôi. Cứ cất gói quà vào Không gian lưu trữ, lúc nào vui ta mở.", "player:skeptical", "hands_folded"},
            {"HỆ THỐNG", "Yêu cầu lưu trữ bị từ chối. Phần thưởng đã bị thu hồi do ký chủ hành hung Hệ Thống.", "system:quest"},
            {"HÀ NHÂN", "Ta chọn KHÔNG MỞ, chứ có chọn KHÔNG NHẬN đâu?! Ngươi đọc chữ bằng đuôi à?!", "player:furious", "point", "cao_ni_ma"},
            {"HỆ THỐNG", "Câu hỏi chỉ có tác dụng trang trí. Quyền giải thích cuối cùng thuộc về Hệ Thống.", "system:laugh"},
            {"HÀ NHÂN", "Trang trí cái đầu ngươi! Trả quà đây, đồ máy ATM biết sủa!", "player:furious", "point"},
            {"HỆ THỐNG", "Chức năng hoàn quà mở ở cấp 60. Chức năng chửi tiếp vẫn miễn phí.", "system:idle"},
            {"HÀ NHÂN", "Tốt! Vậy ta dùng tính năng duy nhất không khóa: CẨU HỆ THỐNG PHẾ VẬT!", "player:crying", "present_open"},
            {"HỆ THỐNG", "Liên kết đã hoàn tất. Khiếu nại sẽ được xử lý trong 999 ngày làm việc.", "system:quest"},
            {"GIỌNG NỮ NGOÀI CỬA", "Hà đệ! Đệ có ở trong đó không? Hà đệ!", ""},
            {"HÀ NHÂN", "Ai gọi ta vậy? Giọng nữ... chẳng lẽ phúc lợi xuyên không tới muộn?", "player:guilty_startle", ""},
            {"NGƯỜI DẪN CHUYỆN", "Hà Nhân phủi áo, mở cửa đi ra. Hệ Thống lặng lẽ lơ lửng theo sau.", ""}},
            DialogueAction::LeaveBedroom);
        SyncDialoguePresentation();
        return;
    }

    dialogue_.Start({
        {"HỆ THỐNG", "Lựa chọn đã được ghi nhận: KHÔNG.", "system:idle", "", "doit"},
        {"HÀ NHÂN", "Khoan! Ta bấm CÓ rõ ràng mà?! Hai cái nút nhà ngươi nối chung một dây à?!", "player:furious", "point", "cao_ni_ma"},
        {"HỆ THỐNG", "CÓ và KHÔNG đều đã quy đổi thành KHÔNG do ký chủ hành hung Hệ Thống.", "system:quest"},
        {"HỆ THỐNG", "Gói Tân Thủ Bí Ẩn đã bị thu hồi để khấu trừ phí tổn thương tinh thần.", "system:laugh"},
        {"HÀ NHÂN", "Ngươi có cái tinh thần chó nào để tổn thương?! Trả quà đây, đồ máy ATM biết sủa!", "player:furious", "point", "cao_ni_ma"},
        {"HỆ THỐNG", "Chức năng hoàn quà mở ở cấp 60. Chức năng chửi tiếp vẫn miễn phí.", "system:idle"},
        {"HÀ NHÂN", "Tốt! Vậy ta dùng tính năng duy nhất không khóa: CẨU HỆ THỐNG PHẾ VẬT!", "player:crying", "present_open"},
        {"HỆ THỐNG", "Liên kết đã hoàn tất. Khiếu nại sẽ được xử lý trong 999 ngày làm việc.", "system:quest"},
        {"GIỌNG NỮ NGOÀI CỬA", "Hà đệ! Đệ có ở trong đó không? Hà đệ!", ""},
        {"HÀ NHÂN", "Ai gọi ta vậy? Giọng nữ... chẳng lẽ phúc lợi xuyên không tới muộn?", "player:guilty_startle", ""},
        {"NGƯỜI DẪN CHUYỆN", "Hà Nhân phủi áo, mở cửa đi ra. Hệ Thống lặng lẽ lơ lửng theo sau.", ""}},
        DialogueAction::LeaveBedroom);
    SyncDialoguePresentation();
}

void BedroomIntroLevel::SyncDialoguePresentation()
{
    const std::uint64_t revision = dialogue_.Revision();
    if (revision == observedDialogueRevision_ || !dialogue_.IsActive()) return;
    observedDialogueRevision_ = revision;

    const DialogueLine &line = dialogue_.Current();
    if (!Progress().Flag("ui.host_profile_unlocked") &&
        line.text.rfind("Biểu tượng hình người", 0) == 0)
    {
        Progress().SetFlag("ui.host_profile_unlocked");
        Progress().Save();
    }
    if (!Progress().Flag("ui.storage_space_unlocked") &&
        line.text.rfind("Biểu tượng vòng sáng", 0) == 0)
    {
        Progress().SetFlag("ui.storage_space_unlocked");
        Progress().Save();
    }
    soundEffects_.Play(ParseMemeSound(line.sound));
    CharacterAction action = CharacterAction::None;
    constexpr std::string_view playerPrefix = "player:";
    if (line.portrait.rfind(playerPrefix, 0) == 0)
    {
        playerFace_.SetExpression(
            faces_, FaceGender::Male,
            std::string_view(line.portrait).substr(playerPrefix.size()));
        action = ParseCharacterAction(line.action);
    }
    player_.TriggerAction(action);

    if (line.portrait.rfind("system:", 0) == 0 && !systemVisible_)
    {
        const Vector2 playerPosition = player_.Position();
        systemDog_.PlaceForConversation(
            {playerPosition.x + 170.0F, playerPosition.y - 152.0F},
            playerPosition.x);
        systemVisible_ = true;
    }
    if (line.portrait.rfind("system:", 0) == 0)
        FaceConversationPartners(player_, systemDog_);

    if (line.portrait == "system:quest")
    {
        systemPose_ = SystemDogPose::Quest;
    }
    else if (line.portrait == "system:laugh")
    {
        systemPose_ = SystemDogPose::Laugh;
    }
    else if (line.portrait == "system:idle")
    {
        systemPose_ = SystemDogPose::Idle;
    }
}

const Texture2D *BedroomIntroLevel::CurrentPortrait() const
{
    if (!dialogue_.IsActive()) return nullptr;
    const std::string &portrait = dialogue_.Current().portrait;
    if (portrait == "player" || portrait.rfind("player:", 0) == 0)
        return player_.PortraitTexture();
    if (portrait.rfind("system:", 0) == 0) return systemDog_.Pose(systemPose_);
    return nullptr;
}

const Texture2D *BedroomIntroLevel::CurrentPortraitFace() const
{
    if (!dialogue_.IsActive()) return nullptr;
    const std::string &portrait = dialogue_.Current().portrait;
    if (portrait == "player" || portrait.rfind("player:", 0) == 0)
        return playerFace_.Current(faces_);
    return nullptr;
}

void BedroomIntroLevel::Draw(const Ui &ui) const
{
    if (background_.id != 0)
    {
        constexpr float targetAspect = 1280.0F / 720.0F;
        const float sourceWidth = background_.height * targetAspect;
        const float sourceX = (background_.width - sourceWidth) * 0.5F;
        DrawTexturePro(background_,
                       {sourceX, 0.0F, sourceWidth, static_cast<float>(background_.height)},
                       {0.0F, 0.0F, 1280.0F, 720.0F}, {0.0F, 0.0F}, 0.0F, WHITE);
        DrawRectangle(0, 0, 1280, 720, Fade(Color{35, 24, 18, 255}, 0.10F));
    }
    if (systemVisible_) systemDog_.DrawWorld(systemPose_, 0.0F);
    player_.Draw(playerFace_.Current(faces_));

    if (systemChoice_.IsActive())
    {
        ui.DrawSystemChoice(systemChoice_, systemDog_.Pose(SystemDogPose::Quest));
        return;
    }

    if (dialogue_.IsActive())
    {
        ui.DrawDialogue(dialogue_.Current(), CurrentPortrait(), CurrentPortraitFace());
        return;
    }
    if (!finished_) return;

    const Rectangle panel = {350.0F, 260.0F, 580.0F, 170.0F};
    DrawPanel(panel, Fade(Color{20, 24, 31, 255}, 0.92F), Fade(WHITE, 0.15F));
    const Font font = ui.GetFont();
    DrawTextEx(font, "LIÊN KẾT THÀNH CÔNG", {430.0F, 300.0F}, 30.0F, 0.5F,
               Color{241, 180, 55, 255});
    DrawTextEx(font, "Đang chờ tọa độ truyền tống...", {426.0F, 355.0F}, 24.0F,
               0.5F, Fade(WHITE, 0.82F));
}

LevelTransition BedroomIntroLevel::RequestedTransition() const
{
    return transition_;
}
} // namespace game
