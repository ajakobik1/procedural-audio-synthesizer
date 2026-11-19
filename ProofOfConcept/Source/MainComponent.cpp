#include "MainComponent.h"

// Zmienne pomocnicze do wyswietlania danych na ekranie
static float g_monsterHP = -100.0f;
static int g_packetCount = 0;
static float g_monsterScale = 1.0f;
static float g_monsterSpeed = 0.0f;
static float g_monsterCombat = 0.0f;
static float g_monsterAttack = 0.0f;

//==============================================================================
MainComponent::MainComponent()
{
    setSize(800, 1080);

    if (!connect(9000)) {
        juce::Logger::outputDebugString("BLAD: Nie udalo sie otworzyc portu 9000!");
    }
    else {
        juce::Logger::outputDebugString("SUKCES: Port 9000 otwarty poprawnie!");
    }

    addListener(this, "/synth/hp");
    addListener(this, "/synth/scale");
    addListener(this, "/synth/speed");
    addListener(this, "/synth/combat");
    addListener(this, "/synth/attack");

    if (juce::RuntimePermissions::isRequired(juce::RuntimePermissions::recordAudio)
        && !juce::RuntimePermissions::isGranted(juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request(juce::RuntimePermissions::recordAudio,
            [&](bool granted) { setAudioChannels(granted ? 2 : 0, 2); });
    }
    else
    {
        setAudioChannels(2, 2);
    }
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    currentSampleRate = static_cast<float>(sampleRate);
}

void MainComponent::releaseResources()
{
}

//==============================================================================
// Okienko z testami diagnostycznymi
void MainComponent::paint(juce::Graphics& g)
{
    // Tlo
    g.fillAll(juce::Colours::lightpink);

    g.setFont(juce::Font(20.0f));
    g.setColour(juce::Colours::white);
    g.drawText("Odebrane pakiety OSC /synth/hp: " + juce::String(g_packetCount), 20, 70, 760, 30, juce::Justification::centredLeft);

    // Wyswietlanie stanu potwora
    if (g_packetCount == 0)
    {
        g.setColour(juce::Colours::lightcyan);
        g.setFont(juce::Font(24.0f, juce::Font::bold));
        g.drawText("Status: oczekiwanie na sygnal", 20, 130, 760, 40, juce::Justification::centredLeft);
    }
    else if (g_monsterHP < 0.0f)
    {
        g.setColour(juce::Colours::lightyellow);
        g.setFont(juce::Font(24.0f, juce::Font::bold));
        g.drawText("Status: polaczono, brak potwora w poblizu", 20, 130, 760, 40, juce::Justification::centredLeft);
    }
    else
    {
        g.setColour(juce::Colours::lightgreen);
        g.setFont(juce::Font(24.0f, juce::Font::bold));
        g.drawText("Status: namierzono potwora, odbieranie danych...", 20, 130, 760, 40, juce::Justification::centredLeft);

        // Wyswietlanie HP potwora
        float hpPercent = g_monsterHP * 100.0f;
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(36.0f, juce::Font::bold));
        g.drawText("HP potwora: " + juce::String(hpPercent, 1) + " %", 20, 200, 760, 50, juce::Justification::centredLeft);

        g.setFont(juce::Font(20.0f));
        g.drawText("Wartosc znormalizowana: " + juce::String(g_monsterHP, 4), 20, 250, 760, 30, juce::Justification::centredLeft);

        g.setColour(juce::Colours::black);
        g.fillRect(20, 300, 600, 50);

        float clampedHP = juce::jlimit(0.0f, 1.0f, g_monsterHP);
        juce::Colour barColour = juce::Colour::fromHSV(clampedHP * 0.33f, 0.8f, 0.9f, 1.0f);
        g.setColour(barColour);
        g.fillRect(20, 300, static_cast<int>(600.0f * clampedHP), 50);

        g.setColour(juce::Colours::white);
        g.drawRect(20, 300, 600, 50, 2);

        // Wyswietlanie rozmiaru potwora
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(36.0f, juce::Font::bold));
        g.drawText("Rozmiar potwora: " + juce::String(g_monsterScale, 2) + "x", 20, 380, 760, 50, juce::Justification::centredLeft);

        g.setFont(juce::Font(20.0f));
        g.drawText("Wartosc skali: " + juce::String(g_monsterScale, 4), 20, 430, 760, 30, juce::Justification::centredLeft);

        g.setColour(juce::Colours::black);
        g.fillRect(20, 480, 600, 50);

        float clampedScale = juce::jlimit(0.0f, 1.0f, g_monsterScale / 3.0f);
        g.setColour(juce::Colours::lightskyblue);
        g.fillRect(20, 480, static_cast<int>(600.0f * clampedScale), 50);

        g.setColour(juce::Colours::white);
        g.drawRect(20, 480, 600, 50, 2);

        // Wyswietlanie predkosci potwora
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(36.0f, juce::Font::bold));
        g.drawText("Predkosc potwora: " + juce::String(g_monsterSpeed, 1), 20, 560, 760, 50, juce::Justification::centredLeft);

        g.setFont(juce::Font(20.0f));
        g.drawText("Wartosc predkosci: " + juce::String(g_monsterSpeed, 4), 20, 610, 760, 30, juce::Justification::centredLeft);

        g.setColour(juce::Colours::black);
        g.fillRect(20, 660, 600, 50);

        float clampedSpeed = juce::jlimit(0.0f, 1.0f, g_monsterSpeed / 500.0f);
        g.setColour(juce::Colours::plum);
        g.fillRect(20, 660, static_cast<int>(600.0f * clampedSpeed), 50);

        g.setColour(juce::Colours::white);
        g.drawRect(20, 660, 600, 50, 2);

        // Wyswietlanie stanu walki
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(36.0f, juce::Font::bold));
        g.drawText("Stan walki: " + juce::String(g_monsterCombat >= 0.5f ? "TAK" : "NIE"), 20, 740, 760, 50, juce::Justification::centredLeft);

        g.setFont(juce::Font(20.0f));
        g.drawText("Wartosc stanu walki: " + juce::String(g_monsterCombat, 4), 20, 790, 760, 30, juce::Justification::centredLeft);

        g.setColour(juce::Colours::black);
        g.fillRect(20, 840, 600, 50);

        float clampedCombat = juce::jlimit(0.0f, 1.0f, g_monsterCombat);
        g.setColour(juce::Colours::coral);
        g.fillRect(20, 840, static_cast<int>(600.0f * clampedCombat), 50);

        g.setColour(juce::Colours::white);
        g.drawRect(20, 840, 600, 50, 2);

        // Wyswietlanie akcji ataku
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(36.0f, juce::Font::bold));
        g.drawText("Atak potwora: " + juce::String(g_monsterAttack >= 0.5f ? "ATAKUJE!" : "BRAK"), 20, 920, 760, 50, juce::Justification::centredLeft);

        g.setFont(juce::Font(20.0f));
        g.drawText("Wartosc ataku: " + juce::String(g_monsterAttack, 4), 20, 970, 760, 30, juce::Justification::centredLeft);

        g.setColour(juce::Colours::black);
        g.fillRect(20, 1020, 600, 50);

        float clampedAttack = juce::jlimit(0.0f, 1.0f, g_monsterAttack);
        g.setColour(juce::Colours::crimson);
        g.fillRect(20, 1020, static_cast<int>(600.0f * clampedAttack), 50);

        g.setColour(juce::Colours::white);
        g.drawRect(20, 1020, 600, 50, 2);
    }
}

void MainComponent::resized()
{
}

//==============================================================================
void MainComponent::oscMessageReceived(const juce::OSCMessage& message)
{
    if (message.size() == 1 && message[0].isFloat32())
    {
        float value = message[0].getFloat32();

        if (message.getAddressPattern() == "/synth/hp")
        {
            g_monsterHP = value;

            if (value >= 0.0f) {
                g_packetCount++;
                healthPercentage = juce::jlimit(0.0f, 1.0f, value);
                samplesWithoutMessage = 0;
            }
            repaint();
        }
        else if (message.getAddressPattern() == "/synth/scale")
        {
            g_monsterScale = value;
            actorScale = juce::jlimit(0.2f, 5.0f, value);
            repaint();
        }
        else if (message.getAddressPattern() == "/synth/speed")
        {
            g_monsterSpeed = value;
            movementSpeed = juce::jmax(0.0f, value);
            repaint();
        }
        else if (message.getAddressPattern() == "/synth/combat")
        {
            g_monsterCombat = value;
            isInCombat = (value >= 0.5f);
            repaint();
        }
        else if (message.getAddressPattern() == "/synth/attack")
        {
            g_monsterAttack = value;
            isAttacking = (value >= 0.5f);
            repaint();
        }
    }
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    auto* leftBuffer = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
    auto* rightBuffer = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);

    samplesWithoutMessage += bufferToFill.numSamples;
    bool isGameConnected = (samplesWithoutMessage < currentSampleRate * 0.5f) && (g_monsterHP >= 0.0f);
    float targetVolume = isGameConnected ? 0.55f : 0.0f;

    // Zmienne stanu traktu glosowego wg modelu Wilkinsona i Reissa (2016)
    static float smoothedSpeed = 0.0f;
    static float smoothedCombat = 0.0f;
    static float mod1Phase = 0.0f, mod2Phase = 0.0f, mod3Phase = 0.0f;
    static float subPhase = 0.0f;
    static float shimmerPhase = 0.0f, jitterPhase = 0.0f;
    static float bp1Low = 0.0f, bp1Band = 0.0f;
    static float bp2Low = 0.0f, bp2Band = 0.0f;
    static float bp3Low = 0.0f, bp3Band = 0.0f;
    static float wheezeLow = 0.0f, wheezeBand = 0.0f;
    static float lipReflectState = 0.0f;

    float safeScale = juce::jlimit(0.25f, 4.0f, actorScale);
    float hpClamped = juce::jlimit(0.0f, 1.0f, healthPercentage);
    float damageAmount = 1.0f - hpClamped;

    // 2. Rozmiar modelu -> Pelny zakres f0 (3.0x = ~48 Hz, 1.0x = 115 Hz, 0.3x = ~295 Hz) i formantow
    float basePitch = 115.0f / std::pow(safeScale, 0.78f);
    float formantScale = 1.0f / std::pow(safeScale, 0.85f);
    float subWeight = juce::jlimit(0.0f, 0.75f, (safeScale - 0.45f) * 0.5f);

    // 4. Zdrowie (HP) -> Filtr LPF (otwarty przy 100% HP -> przytlumiony 320 Hz przy niskim HP)
    float cutoffHz = 320.0f * std::pow(35.0f, hpClamped * hpClamped);
    float rc = 1.0f / (juce::MathConstants<float>::twoPi * cutoffHz);
    float dt = 1.0f / currentSampleRate;
    float filterAlpha = dt / (rc + dt);

    // Czestotliwosci modulatorow gardlowych (w biologicznym pasmie warczenia 20-75 Hz)
    float rattleScale = 1.0f / std::pow(safeScale, 0.32f);
    float fmFreq1 = 28.0f * rattleScale;
    float fmFreq2 = 41.0f * rattleScale;
    float fmFreq3 = 19.0f * rattleScale;

    for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
    {
        if (currentVolume < targetVolume) currentVolume += 0.0005f;
        else if (currentVolume > targetVolume) currentVolume = juce::jmax(0.0f, currentVolume - 0.0001f);

        // Bezwladnosc predkosci ruchu
        if (movementSpeed > smoothedSpeed)
            smoothedSpeed += 0.0006f * (movementSpeed - smoothedSpeed);
        else
            smoothedSpeed += 0.00004f * (movementSpeed - smoothedSpeed);

        float combatTarget = isInCombat ? 1.0f : 0.0f;
        smoothedCombat += 0.0004f * (combatTarget - smoothedCombat);

        // 1. Predkosc ruchu -> LFO oddechu (0.28 Hz spoczynek -> 0.85 Hz sprint)
        float scaleCompensatedSpeed = smoothedSpeed / juce::jmax(1.0f, safeScale * 0.85f);
        float normSpeed = juce::jlimit(0.0f, 1.0f, scaleCompensatedSpeed / 550.0f);
        float lfoFreq = 0.28f + (normSpeed * 0.57f);

        lfoPhase += lfoFreq / currentSampleRate;
        if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;

        // Podzial cyklu na wdech (32%) i wydech (68%)
        float inhaleSplit = 0.32f;
        float inhaleEnv = 0.0f;
        float exhaleEnv = 0.0f;
        float exhaleProgress = 0.0f;

        if (lfoPhase < inhaleSplit)
        {
            float p = lfoPhase / inhaleSplit;
            inhaleEnv = std::sin(p * juce::MathConstants<float>::pi) * (0.4f + 0.45f * normSpeed);
        }
        else
        {
            exhaleProgress = (lfoPhase - inhaleSplit) / (1.0f - inhaleSplit);
            float attack = std::sin(exhaleProgress * juce::MathConstants<float>::pi);
            float decay = std::exp(-1.65f * exhaleProgress);
            exhaleEnv = attack * decay * 1.35f;
        }

        // 4. Zdrowie (HP) -> Nieregularna chrypa i dlawienie (Shimmer & Jitter wg Sekcji 2.2)
        shimmerPhase += (6.5f / currentSampleRate) * juce::MathConstants<float>::twoPi;
        if (shimmerPhase >= juce::MathConstants<float>::twoPi) shimmerPhase -= juce::MathConstants<float>::twoPi;

        jitterPhase += (3.2f / currentSampleRate) * juce::MathConstants<float>::twoPi;
        if (jitterPhase >= juce::MathConstants<float>::twoPi) jitterPhase -= juce::MathConstants<float>::twoPi;

        float shimmerMod = 1.0f - (damageAmount * 0.50f * ((std::sin(shimmerPhase) + 1.0f) * 0.5f));
        float pitchWeakness = 1.0f + (damageAmount * 0.12f * std::sin(jitterPhase)) - (damageAmount * 0.25f * exhaleProgress);

        // 3. Stan walki -> Biologiczna kaskada nieliniowosci krtani (Sekcja 2.2)
        mod1Phase += (fmFreq1 / currentSampleRate) * juce::MathConstants<float>::twoPi;
        mod2Phase += (fmFreq2 / currentSampleRate) * juce::MathConstants<float>::twoPi;
        mod3Phase += (fmFreq3 / currentSampleRate) * juce::MathConstants<float>::twoPi;
        if (mod1Phase >= juce::MathConstants<float>::twoPi) mod1Phase -= juce::MathConstants<float>::twoPi;
        if (mod2Phase >= juce::MathConstants<float>::twoPi) mod2Phase -= juce::MathConstants<float>::twoPi;
        if (mod3Phase >= juce::MathConstants<float>::twoPi) mod3Phase -= juce::MathConstants<float>::twoPi;

        float chaosWave = std::cos(mod1Phase + 0.8f * std::cos(mod2Phase + 0.5f * std::cos(mod3Phase)));
        float fmGrowlOffset = chaosWave * (basePitch * 0.22f * smoothedCombat);

        // Kontur tonu z lekkim opadaniem pod koniec wydechu
        float pitchContour = 1.0f - (0.12f * exhaleProgress);
        float instantFreq = juce::jmax(18.0f, (basePitch * pitchContour * pitchWeakness) + fmGrowlOffset);

        // Impulsowy przeplyw krtaniowy g[n] + subharmoniczna (aktywna tylko dla wiekszych potworow)
        mainPhase += instantFreq / currentSampleRate;
        if (mainPhase >= 1.0f) mainPhase -= 1.0f;

        subPhase += (instantFreq * 0.5f) / currentSampleRate;
        if (subPhase >= 1.0f) subPhase -= 1.0f;

        float rawSin = std::sin(mainPhase * juce::MathConstants<float>::twoPi);
        float glottalPulse = rawSin * std::exp(1.5f * (rawSin - 1.0f));
        float subPulse = std::sin(subPhase * juce::MathConstants<float>::twoPi);

        float throatFlutter = 1.0f - (0.45f * smoothedCombat * ((chaosWave + 1.0f) * 0.5f));
        float glottalWave = (glottalPulse + (subWeight * smoothedCombat * subPulse)) * throatFlutter;

        // Szum turbulencyjny wyzwalany otwarciem krtani (Sekcja 2.1)
        float whiteNoise = randomGenerator.nextFloat() * 2.0f - 1.0f;
        float pulsedTurbulence = whiteNoise * (0.45f + 0.55f * std::abs(glottalWave));
        float breathNoise = (whiteNoise * inhaleEnv) + (pulsedTurbulence * exhaleEnv);

        // Miks zrodla: poza walka czysty oddech, w walce gardlowy warkot zintegrowany z szumem
        float voiceLevel = exhaleEnv * (0.04f + 0.44f * smoothedCombat) * shimmerMod;
        float noiseLevel = 0.85f - (0.15f * smoothedCombat);
        float excitation = (glottalWave * voiceLevel) + (breathNoise * noiseLevel);

        // Model traktu glosowego (Sekcja 1.1: F1 zmiennosc 23%, F2 zmiennosc 61%)
        float jawOpen = exhaleEnv * (0.35f + 0.55f * smoothedCombat) + (inhaleEnv * 0.3f);
        float f1Hz = juce::jlimit(60.0f, 3600.0f, 200.0f * (1.0f + 0.23f * jawOpen) * formantScale);
        float f2Hz = juce::jlimit(180.0f, 5500.0f, 520.0f * (1.0f + 0.61f * jawOpen) * formantScale);
        float f3Hz = juce::jlimit(400.0f, 7500.0f, 1250.0f * (1.0f + 0.30f * jawOpen) * formantScale);

        float f1 = 2.0f * std::sin(juce::MathConstants<float>::pi * f1Hz / currentSampleRate);
        float f2 = 2.0f * std::sin(juce::MathConstants<float>::pi * f2Hz / currentSampleRate);
        float f3 = 2.0f * std::sin(juce::MathConstants<float>::pi * f3Hz / currentSampleRate);

        bp1Low = juce::jlimit(-4.0f, 4.0f, bp1Low + f1 * bp1Band);
        bp1Band = juce::jlimit(-4.0f, 4.0f, bp1Band + f1 * (excitation - bp1Low - 0.30f * bp1Band));

        bp2Low = juce::jlimit(-4.0f, 4.0f, bp2Low + f2 * bp2Band);
        bp2Band = juce::jlimit(-4.0f, 4.0f, bp2Band + f2 * (excitation - bp2Low - 0.28f * bp2Band));

        bp3Low = juce::jlimit(-4.0f, 4.0f, bp3Low + f3 * bp3Band);
        bp3Band = juce::jlimit(-4.0f, 4.0f, bp3Band + f3 * (excitation - bp3Low - 0.35f * bp3Band));

        // Swist oddechowy przy niskim HP
        float fWheeze = 2.0f * std::sin(juce::MathConstants<float>::pi * juce::jmin(2800.0f, 1400.0f * formantScale) / currentSampleRate);
        wheezeLow = juce::jlimit(-4.0f, 4.0f, wheezeLow + fWheeze * wheezeBand);
        wheezeBand = juce::jlimit(-4.0f, 4.0f, wheezeBand + fWheeze * (whiteNoise * inhaleEnv - wheezeLow - 0.16f * wheezeBand));

        float tractOut = (bp1Band * 1.0f) + (bp2Band * 1.1f) + (bp3Band * 0.65f) + (wheezeBand * damageAmount * 0.9f);

        // Filtr odbicia wargowego (Lip Reflection HPF) - wycina niski bas u malych potworow
        float lipCutoff = juce::jlimit(0.01f, 0.12f, 0.025f / safeScale);
        lipReflectState += lipCutoff * (tractOut - lipReflectState);
        float lipOutput = tractOut - (0.70f * lipReflectState);

        // Glowny filtr LPF i saturacja WaveShaper sterowane przez HP
        lastFilterOut += filterAlpha * (lipOutput - lastFilterOut);
        float drive = 1.0f + (damageAmount * 3.0f);
        float finalSignal = std::tanh(lastFilterOut * drive) * shimmerMod;

        float outputSample = finalSignal * currentVolume;

        leftBuffer[sample] = outputSample;
        rightBuffer[sample] = outputSample;
    }
}

//==============================================================================
void MainComponent::timerCallback()
{
}