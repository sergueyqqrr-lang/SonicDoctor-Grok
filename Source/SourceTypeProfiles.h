#pragma once
#include "SoundProfile.h"

// ============================================================================
// SourceTypeProfiles
//
// referenceDb = forma espectral tipica expresada como dB relativos al promedio
// de las bandas ACTIVAS del propio sonido (media aritmetica en dB de bandas
// dentro de 40 dB del maximo). Cada fila tiene media ~0 para que, cuando la
// forma coincida, deviationDb ~ 0.
//
// Estas curvas son aproximaciones practicas de mezcla (no un modelo ML
// entrenado). Estan calibradas para:
//   Kick/Bajo  -> energia concentrada en Sub/Graves
//   Voz        -> centro en Medios/Upper-Mid, poco sub
//   Snare      -> Low-Mid + presencia/brillo
//   Mezcla     -> balance relativamente plano con leve rolloff de agudos
// ============================================================================

struct SourceTypeConfig
{
    const char* label;
    float referenceDb[SoundProfile::numBands]; // Sub, Graves, Low-Mid, Medios, Upper-Mid, Presencia, Brillo

    float rumbleThresholdDb;   // si rumbleRelativeDb > esto -> aviso (Kick/Bajo mas permisivos)
    bool  sibilanceRelevant;
    float sibilanceRelativeDb; // nivel relativo esperado de 4-10kHz vs promedio activo
    float crestFactorWarnDb;
    bool  loudnessCheckRelevant;

    // Bandas que "no deben avisarse por falta" si estan silenciosas y la
    // referencia ya esperaba que estuvieran bajas (evita "falta de Brillo -40 dB"
    // en un kick limpio). indices 0..6
    bool ignoreQuietDeficit[SoundProfile::numBands];
};

inline const SourceTypeConfig& getSourceTypeConfig (SourceType type)
{
    //                                     Sub    Graves LowMid Medios UppMid Presen Brillo   rumble sib   sibRel crest loud
    // ignoreQuietDeficit:                   Sub    Graves LowMid Medios UppMid Presen Brillo
    static const SourceTypeConfig table[] = {
        /* Generico */ {
            "Generico",
            { -3.0f,  4.0f,  3.0f,  4.0f,  2.0f, -2.0f, -8.0f },
            -12.0f, true, -8.0f, 6.0f, true,
            { false, false, false, false, false, false, true }
        },
        /* Voz */ {
            "Voz",
            {-18.0f, -8.0f,  3.0f,  9.0f,  8.0f,  4.0f,  2.0f },
            -16.0f, true, -6.0f, 6.0f, true,
            { true,  true,  false, false, false, false, false }
        },
        /* Kick */ {
            "Kick",
            { 13.4f, 11.4f,  3.4f, -2.6f, -6.6f, -8.6f,-10.6f },
            -3.0f, false, 0.0f, 3.5f, false,
            { false, false, false, true,  true,  true,  true }
        },
        /* Snare */ {
            "Snare",
            {-14.0f, -2.0f,  6.0f,  3.0f,  2.0f,  4.0f,  1.0f },
            -10.0f, false, 0.0f, 4.0f, false,
            { true,  false, false, false, false, false, false }
        },
        /* Bajo */ {
            "Bajo",
            { 15.4f, 11.4f,  5.4f, -0.6f, -6.6f,-10.6f,-14.6f },
            -2.0f, false, 0.0f, 3.5f, false,
            { false, false, false, true,  true,  true,  true }
        },
        /* Guitarra */ {
            "Guitarra",
            {-14.0f, -2.0f,  5.0f,  8.0f,  5.0f,  1.0f, -3.0f },
            -14.0f, true, -10.0f, 6.0f, true,
            { true,  false, false, false, false, false, false }
        },
        /* Piano */ {
            "Piano/Teclado",
            {-10.0f,  1.0f,  4.0f,  6.0f,  4.0f,  0.0f, -5.0f },
            -14.0f, true, -8.0f, 6.0f, true,
            { true,  false, false, false, false, false, false }
        },
        /* Cuerdas/Metales */ {
            "Cuerdas/Metales",
            {-12.0f, -1.0f,  3.0f,  7.0f,  5.0f,  1.0f, -3.0f },
            -14.0f, true, -8.0f, 6.0f, true,
            { true,  false, false, false, false, false, false }
        },
        /* Synth/Pad */ {
            "Synth/Pad",
            { -2.0f,  3.0f,  2.0f,  4.0f,  2.0f, -2.0f, -7.0f },
            -10.0f, true, -8.0f, 8.0f, true,
            { false, false, false, false, false, false, true }
        },
        /* Bateria completa */ {
            "Bateria completa",
            { -4.0f,  2.0f,  0.0f,  2.0f,  3.0f,  1.0f, -4.0f },
            -8.0f, false, 0.0f, 4.0f, true,
            { false, false, false, false, false, false, false }
        },
        /* Mezcla completa */ {
            "Mezcla completa",
            { -2.0f,  3.0f,  2.0f,  4.0f,  2.0f, -2.0f, -7.0f },
            -12.0f, true, -6.0f, 6.0f, true,
            { false, false, false, false, false, false, false }
        },
    };

    auto index = juce::jlimit (0, (int) (sizeof (table) / sizeof (table[0])) - 1, (int) type);
    return table[index];
}

inline juce::StringArray getSourceTypeChoices()
{
    juce::StringArray choices;
    for (int i = 0; i <= (int) SourceType::MezclaCompleta; ++i)
        choices.add (getSourceTypeConfig ((SourceType) i).label);
    return choices;
}
