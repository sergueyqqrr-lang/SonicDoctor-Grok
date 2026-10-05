# Sonic Doctor

Analizador de audio en tiempo real que diagnostica un sonido y da
recomendaciones concretas — tanto generales como específicas para tus
propios plugins (KickForge EQ, Surgical De-Esser, y los que hagas después).

## Correcciones de diagnosticos (v1.0.1)

- **Curvas de referencia normalizadas a media cero**: antes las referenceDb de cada
  tipo de fuente no promediaban 0, por lo que incluso un sonido con la forma
  espectral "perfecta" mostraba desviaciones sistematicas (excesos/faltas
  inventados). Ahora cada fila tiene media aritmetica ~0 y deviationDb = 0
  cuando la forma coincide con lo tipico.
- **Energia por banda / sibilancia / rumble en dominio de potencia** (media de
  magnitud^2) en lugar de media de magnitud lineal — refleja mejor la energia
  real y evita que picos estrechos se subestimen.
- **Resonancias solo en maximos locales estrictos** (mayor que vecinos
  inmediatos), para no reportar mesetas o ruido de banda ancha como picos.

## Qué mide (todo con DSP real, no adivina nada)

- **Loudness aproximado** (Short-term LUFS, ventana ~3s, K-weighting simplificado)
- **Pico, RMS y Crest Factor** (indicador de dinámica/compresión)
- **Correlación de fase** (problemas de fase entre canales L/R)
- **Balance espectral en 7 bandas**, comparado contra una curva de referencia
  típica según el tipo de fuente que elijas
- **Sibilancia** (energía 4-10kHz) y cuánto excede lo típico
- **Rumble** (ruido bajo 30Hz — por debajo de lo que produce cualquier
  instrumento musical, incluido un kick)
- **Detección de resonancias**: picos angostos que sobresalen del promedio local
- **Clipping, offset de DC, piso de ruido**

## Tipos de fuente disponibles

Genérico, Voz, Kick, Snare, Bajo, Guitarra, Piano/Teclado, Cuerdas/Metales,
Synth/Pad, Batería completa, Mezcla completa.

Cada tipo tiene su propia curva de referencia espectral Y sus propios
umbrales de diagnóstico — esto es importante porque varios chequeos NO
tienen sentido igual para todo:

- **Rumble**: un Kick o un Bajo tienen sub real por diseño, así que su
  umbral es mucho más permisivo que el de una Voz (donde casi no debería
  haber nada por debajo de 30Hz). Antes de esta corrección, el análisis
  marcaba el sub de un kick como "ruido" — ya no.
- **Sibilancia**: desactivada para Kick, Snare, Bajo y Batería completa —
  no tienen "eses", así que recomendar un de-esser no tenía sentido.
- **Crest factor ("sobre-comprimido")**: umbral más permisivo para Kick/Bajo/
  Snare, que suelen comprimirse fuerte a propósito por género.
- **Loudness (LUFS)**: desactivado para one-shots percusivos aislados
  (Kick, Snare), donde esa métrica no es muy informativa.

Si necesitas una categoría que no está en la lista, agrégala en
`Source/SourceTypeProfiles.h` — es una sola fila en la tabla.

## Qué NO hace (y por qué, para que no haya sorpresas)

- **No detecta qué efectos ya tiene aplicado un sonido.** Eso es un problema
  de investigación en Machine Learning (requiere modelos entrenados con
  miles de ejemplos), no algo que se resuelva con análisis de señal
  tradicional. Ningún plugin comercial serio promete esto de forma confiable.
- **No carga ni controla plugins de terceros automáticamente.** Hacer que un
  plugin "hostee" otros plugins dentro de sí mismo es un proyecto de
  ingeniería del tamaño de un mini-DAW, y muchos hosts (incluyendo
  probablemente Studio One) no lo soportan de forma estable. En vez de eso,
  Sonic Doctor te dice **exactamente qué valores poner** en tus propios
  plugins — tú los aplicas con un clic, pero la decisión de qué ajustar la
  toma el análisis, no tu oído a ciegas.

## Arquitectura (para que sea fácil agregar tu próximo plugin)

```
SoundProfile          <- todo lo medido, en un solo lugar
      |
      v
RecommendationEngine  <- reparte el SoundProfile a cada recomendador
      |
      +--> KickForgeEQRecommender
      +--> SurgicalDeEsserRecommender
      +--> (tu próximo plugin aquí)
```

### Cómo agregar soporte para un plugin nuevo tuyo

1. Copia `Source/Recommendations/SurgicalDeEsserRecommender.h` y `.cpp` como
   plantilla, y renómbralos con el nombre de tu plugin nuevo.
2. Dentro de `generate()`, lee lo que necesites de `SoundProfile` (ver
   `Source/SoundProfile.h` — ahí está TODO lo disponible: loudness, bandas,
   resonancias, sibilancia, fase, etc.) y arma tu lista de `Recommendation`.
3. En `Source/Recommendations/RecommendationEngine.cpp`, agrega una sola
   línea en el constructor:
   ```cpp
   recommenders.push_back (std::make_unique<TuNuevoPluginRecommender>());
   ```
4. Agrega tu `.cpp` nuevo a `CMakeLists.txt` (`target_sources`).
5. Listo — aparece automáticamente como una pestaña nueva en la interfaz,
   sin tocar el motor de análisis ni la interfaz para nada más.

## Compilar

```bash
git clone <tu-repo>
cd SonicDoctor
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```
El VST3 queda en `build/SonicDoctor_artefacts/Release/VST3/`.

También puedes usar el workflow de GitHub Actions incluido
(`.github/workflows/build.yml`, solo Windows por estabilidad).

## Instalar en Studio One

Copia el `.vst3` a `C:\Program Files\Common Files\VST3\` y reescanea plugins
en Studio One (Opciones > Ubicaciones > VST Plug-Ins > Reescanear).

## Nota sobre precisión

Los valores de LUFS, curvas de referencia y umbrales de "lo típico" son
aproximaciones prácticas para guiar decisiones de mezcla — no son mediciones
certificadas para broadcast (EBU R128 / ATSC A/85) ni sustituyen tu propio
criterio. Úsalo como punto de partida, no como verdad absoluta.

**Importante sobre el balance espectral, la sibilancia y el rumble**: estas
tres mediciones comparan la banda en cuestión contra el **promedio general
del propio sonido que estás analizando**, no contra un nivel absoluto de dB.
Esto es intencional — significa que el análisis funciona igual sin importar
si el archivo está grabado fuerte o flojo, normalizado o no. Si notas algo
que no cuadra (por ejemplo, una recomendación que no tiene sentido para el
sonido que estás analizando), lo más probable es que la curva de referencia
de ese tipo de fuente en `Source/SourceTypeProfiles.h` necesite un ajuste —
es solo una tabla de números, fácil de retocar sin tocar el resto del código.
