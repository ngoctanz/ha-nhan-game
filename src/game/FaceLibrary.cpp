#include "game/FaceLibrary.hpp"
#include "game/AssetLocator.hpp"

#include <sstream>

namespace game
{
FaceLibrary::~FaceLibrary()
{
    for (auto &[_, texture] : faces_) UnloadTexture(texture);
}

bool FaceLibrary::Load()
{
    const std::string catalogPath = ResolveAssetPath("assets/faces/catalog.csv");
    char *catalogText = LoadFileText(catalogPath.c_str());
    if (catalogText == nullptr) return false;
    std::istringstream catalog(catalogText);
    UnloadFileText(catalogText);

    std::string line;
    std::getline(catalog, line); // header
    int expectedCount = 0;
    const std::string root = "assets/faces/expressions/";
    while (std::getline(catalog, line))
    {
        if (line.empty()) continue;
        std::istringstream row(line);
        std::string sourceId;
        std::string name;
        std::getline(row, sourceId, ',');
        std::getline(row, name, ',');
        if (name.empty()) continue;
        ++expectedCount;
        Texture2D texture = LoadTexture(ResolveAssetPath(root + name + ".png").c_str());
        if (texture.id == 0) continue;
        SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR);
        faces_.emplace(name, texture);
    }
    return expectedCount > 0 && static_cast<int>(faces_.size()) == expectedCount;
}

const Texture2D *FaceLibrary::Get(const std::string &expression) const
{
    const auto found = faces_.find(expression);
    if (found != faces_.end()) return &found->second;

    // Generic aliases keep old dialogue/data files compatible with the
    // larger exported library.
    static const std::unordered_map<std::string, std::string> aliases = {
        {"neutral", "verified_skeptical_tease"},
        {"happy", "verified_beaming_closed_eyes"},
        {"laugh", "verified_beaming_closed_eyes"},
        {"smug", "verified_roundface_teasing_smirk"},
        {"angry", "verified_angry_side_eye"},
        {"furious", "verified_angry_surprise"},
        {"annoyed", "verified_indecisive_thought"},
        {"suspicious", "verified_knowing_suspicion"},
        {"shocked", "verified_shocked_gasp"},
        {"scared", "verified_disbelief_panic"},
        {"crying", "verified_full_cry"},
        {"pleading", "worried_talking"},
        {"embarrassed", "verified_embarrassed_blush"},
        {"confused", "verified_confused_side_eye"},
        {"disgusted", "verified_angry_side_eye"},
        {"deadpan", "exhausted_blank"},
        {"smug_side_smile", "verified_awkward_conflict"},
        {"furious_shout", "verified_angry_surprise"},
        {"skeptical_wink", "verified_focused_contemplation"},
        {"panicked_shout", "verified_baffled_shout"},
        {"content_smile", "verified_displeased_smile"},
        {"cheerful_surprise", "verified_excited_surprise"},
        {"angry_side_eye", "verified_helpless_contemplation"},
        {"worried_grimace", "verified_goofy_grin"},
        {"cool_sunglasses", "verified_cool_confidence"},
        {"shy_pout", "verified_childlike_pout"},
        {"mischievous_grin", "verified_beaming_closed_eyes"},
        {"embarrassed_nosebleed", "verified_embarrassed_blush"},
        {"smug_realistic", "verified_dazed_blank_stare"},
        {"sad_tears", "verified_tearful_restraint"},
        {"laughing_tears", "verified_full_cry"},
        {"crying_grimace", "verified_pained_flinch"},
        {"surprised_open_mouth", "verified_disbelief_open_mouth"},
        {"delighted_laugh", "verified_angry_yell"},
        {"terrified_scream", "verified_startled_panic"},
        {"subtle_smirk", "verified_roundface_teasing_smirk"},
        {"confused_side_glance", "verified_friendly_teasing_glance"},
        {"worried_goatee", "verified_wise_goatee"},
        {"disgusted_side_eye", "verified_angry_side_eye"},
        {"stern_angry", "verified_stunned_disbelief"},
        {"suspicious_concern", "verified_extreme_disbelief"},
        {"furious_disapproval", "verified_steely_determination"},
        {"neutral_realistic", "verified_skeptical_tease"}};
    const auto alias = aliases.find(expression);
    if (alias != aliases.end())
    {
        const auto resolved = faces_.find(alias->second);
        if (resolved != faces_.end()) return &resolved->second;
    }

    const auto neutral = faces_.find("verified_skeptical_tease");
    return neutral == faces_.end() ? nullptr : &neutral->second;
}
} // namespace game
