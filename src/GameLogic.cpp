#include "GameLogic.h"
#include "AudioSystem.h"
#undef max
#undef min
void spawnExplosionFragments(float x, float y) {
    if (rainbowFragments.size() > MAX_RAINBOW_FRAGMENTS) return;

    int count = 60;

    for (int i = 0; i < count; ++i) {
        if (rainbowFragments.size() >= MAX_RAINBOW_FRAGMENTS) break;

        RainbowFragment rf;
        float angle = (float)(rand() % 628) / 100.0f;

        float speed = 3.0f + (float)(rand() % 600) / 100.0f;

        rf.x = x;
        rf.y = y;
        rf.vx = cos(angle) * speed;
        rf.vy = sin(angle) * speed;
        rf.t = 0.0f;
        rf.life = 0.8f + (rand() % 100) / 100.0f;

        rf.size = 15.0f + (rand() % 40);
        rf.alpha0 = 1.0f;
        rf.h = 0.0f;

        if (i % 4 == 0) {
            rf.type = 1;
        }
        else {
            rf.type = 4;
        }

        rainbowFragments.push_back(rf);
    }
}

void HSVtoRGB(float h, float s, float v, Uint8& r, Uint8& g, Uint8& b) {
    int i = int(h * 6);
    float f = h * 6 - i;
    float p = v * (1 - s);
    float q = v * (1 - f * s);
    float t = v * (1 - (1 - f) * s);
    switch (i % 6) {
    case 0: r = v * 255; g = t * 255; b = p * 255; break;
    case 1: r = q * 255; g = v * 255; b = p * 255; break;
    case 2: r = p * 255; g = v * 255; b = t * 255; break;
    case 3: r = p * 255; g = q * 255; b = v * 255; break;
    case 4: r = t * 255; g = p * 255; b = v * 255; break;
    case 5: r = v * 255; g = p * 255; b = q * 255; break;
    }
}

void spawnRainbowFragments(float x, float y, float t, float intensity) {
    const int count = static_cast<int>(80 * intensity);

    if (rainbowFragments.size() + count > MAX_RAINBOW_FRAGMENTS) {
        size_t remove = std::min(rainbowFragments.size(),
            rainbowFragments.size() + count - MAX_RAINBOW_FRAGMENTS);
        rainbowFragments.erase(rainbowFragments.begin(), rainbowFragments.begin() + remove);
    }

    for (int i = 0; i < count; ++i) {
        float angle = (rand() % 10000) * 0.000628318f;
        float r1 = (rand() % 1000) * 0.001f;
        float r2 = (rand() % 1000) * 0.001f;
        float speed = 1.5f + r1 * r1 * 2.0f;
        float breath = sinf(r2 * 6.28f + t);

        RainbowFragment rf;
        rf.x = x;
        rf.y = y;
        rf.vx = cosf(angle) * speed + breath * 0.3f;
        rf.vy = sinf(angle) * speed + breath * 0.3f;
        rf.t = 0.0f;
        rf.life = 1.0f + r1 * 1.0f;
        rf.size = 4.0f + r2 * 17.0f;
        rf.h = fmodf(r1 + r2 * 0.5f + t * 0.1f, 1.0f);
        rf.alpha0 = 0.6f + r2 * 0.4f;
        rf.type = 0;
        rf.angle = angle;
        rf.spiralSpeed = (r1 - 0.5f) * 0.6f;

        rainbowFragments.push_back(rf);
    }
}

void spawnBrushParticle(float x, float y, int brushEffectMode) {
    BrushParticle bp;
    bp.x = bp.baseX = x;
    bp.y = bp.baseY = y;

    bp.baseSize = 60.0f + rand() % 30;

    bp.t = 0.0f;
    bp.phase = (float)(rand() % 628) / 100.0f;
    bp.impact = 0.0f;
    bp.highImpactFrames = 0;
    bp.dissolveFrame = 0;
    bp.absorbed = false;
    bp.vx = bp.vy = 0.0f;

    if (brushEffectMode == 2) {
        bp.type = BRUSH_RAINBOW;
    } else if (brushEffectMode == 3) {
        bp.type = BRUSH_EXPLOSIVE;
        bp.baseSize *= 0.8f;
    } else {
        bp.type = BRUSH_BLUE;
    }
    brushParticles.push_back(bp);
}

void spawnMeteorDrop(float x, float y) {
    int count = 100;
    float randnum = rand() % 10 * 1.0f;
    for (int i = 0; i < count; ++i) {
        if (brushParticles.size() >= MAX_BRUSH_PARTICLES) break;
        BrushParticle bp;

        float r1 = (float)rand() / RAND_MAX;
        float r2 = (float)rand() / RAND_MAX;
        float radius_distribution = sqrt(-2.0f * log(r1));

        float spread = 60.0f;
        float offsetX = (radius_distribution * cos(6.28f * r2)) * spread;
        float offsetY = (radius_distribution * sin(6.28f * r2)) * spread;

        bp.x = bp.baseX = x + offsetX;
        bp.y = bp.baseY = y + offsetY;

        bp.vx = 0.0f;
        bp.vy = 2.0f+ randnum;
        bp.baseSize = 100.0f;
        bp.t = 0.0f;

        bp.phase = sqrt(offsetX * offsetX + offsetY * offsetY);

        bp.impact = atan2(offsetY, offsetX);

        bp.type = BRUSH_DROP;
        bp.absorbed = false;
        bp.dissolveFrame = 0;
        brushParticles.push_back(bp);
    }
}

void generateRainbowLUT() {
    for (int i = 0; i < RAINBOW_LUT_SIZE; ++i) {
        float h = (float)i / RAINBOW_LUT_SIZE;
        float s = 0.7f + 0.3f * sin(h * 2.0f * 3.14159f * 2.0f);
        float v = 0.8f + 0.2f * cos(h * 2.0f * 3.14159f * 3.0f);

        Uint8 r, g, b;
        HSVtoRGB(h, s, v, r, g, b);
        rainbowColorLUT[i] = { r, g, b, 255 };
    }
}

void update_brush_painting(bool painting, int brushEffectMode, float centerX, float centerY, float& lastBrushX, float& lastBrushY, float& lastVelX, float& lastVelY) {
    if (painting) {
        float dist = sqrt((centerX - lastBrushX) * (centerX - lastBrushX) + (centerY - lastBrushY) * (centerY - lastBrushY));
        if (dist > 1.0f) {
            float currentVelX = centerX - lastBrushX; float currentVelY = centerY - lastBrushY;
            float stepLen = (brushEffectMode == 3) ? 20.0f : 5.0f;
            int num_steps = static_cast<int>(dist / stepLen) + 1;

            for (int i = 1; i <= num_steps; ++i) {
                float t = (float)i / (float)num_steps;
                float t2 = t * t; float t3 = t2 * t;
                float h00 = 2 * t3 - 3 * t2 + 1; float h01 = -2 * t3 + 3 * t2;
                float h10 = t3 - 2 * t2 + t; float h11 = t3 - t2;
                float ix = h00 * lastBrushX + h01 * centerX + h10 * lastVelX + h11 * currentVelX;
                float iy = h00 * lastBrushY + h01 * centerY + h10 * lastVelY + h11 * currentVelY;
                spawnBrushParticle(ix, iy, brushEffectMode);
            }
            lastBrushX = centerX; lastBrushY = centerY;
            lastVelX = currentVelX; lastVelY = currentVelY;
        }
    } else {
        lastBrushX = centerX; lastBrushY = centerY;
        lastVelX = 0.0f; lastVelY = 0.0f;
    }
}
