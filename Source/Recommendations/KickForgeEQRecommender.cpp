#include "KickForgeEQRecommender.h"
#include "../SourceTypeProfiles.h"

std::vector<Recommendation> KickForgeEQRecommender::generate (const SoundProfile& profile) const
{
    std::vector<Recommendation> out;
    const auto& cfg = getSourceTypeConfig (profile.sourceType);

    // Rumble real (energia medible <30Hz por encima del umbral del tipo)
    if (profile.rumbleLevelDb > -80.0f && profile.rumbleRelativeDb > cfg.rumbleThresholdDb)
    {
        out.push_back ({ "Sub Rumble (30Hz)", "Corte -3 a -6 dB (o HPF suave)",
            "Energia <30 Hz a " + juce::String (profile.rumbleRelativeDb, 1)
            + " dB rel. (umbral de " + juce::String (cfg.label) + ": "
            + juce::String (cfg.rumbleThresholdDb, 1) + " dB)." });
    }

    const auto& subBand    = profile.bands[0]; // Sub 20-60Hz
    const auto& gravesBand = profile.bands[1]; // Graves 60-250Hz
    const auto& lowMidBand = profile.bands[2]; // Low-Mid 250-500Hz

    // Solo sugerir cortes si hay EXCESO claro (no "falta" — eso no se arregla con KickForge EQ de la misma forma)
    if (subBand.deviationDb > 5.0f)
        out.push_back ({ "Sub Fundamental (~50Hz)", "Corte " + juce::String (-juce::jmin (subBand.deviationDb * 0.5f, 6.0f), 1) + " dB",
            "Exceso de sub: " + juce::String (subBand.deviationDb, 1) + " dB sobre lo tipico para " + juce::String (cfg.label) + "." });

    if (gravesBand.deviationDb > 4.0f)
        out.push_back ({ "Low Body (80-120Hz)", "Corte suave -2 a -4 dB",
            "Graves " + juce::String (gravesBand.deviationDb, 1) + " dB por encima de lo esperado." });

    if (lowMidBand.deviationDb > 4.0f)
        out.push_back ({ "Boxiness / Low Mid Mud (200-350Hz)", "Corte -3 a -5 dB, Q medio",
            "Zona 250-500 Hz elevada (" + juce::String (lowMidBand.deviationDb, 1) + " dB): posible sonido boxy/embarrado." });

    // Resonancias claras
    for (int i = 0; i < profile.numResonancesFound; ++i)
    {
        const auto& res = profile.resonances[(size_t) i];
        if (res.prominenceDb < 5.0f)
            continue;
        out.push_back ({
            "EQ cerca de " + juce::String (res.frequencyHz, 0) + " Hz",
            "Corte -" + juce::String (juce::jmin (res.prominenceDb * 0.7f, 8.0f), 1) + " dB, Q alto (2.5-4)",
            "Resonancia: sobresale " + juce::String (res.prominenceDb, 1) + " dB sobre el promedio local."
        });
    }

    return out;
}
