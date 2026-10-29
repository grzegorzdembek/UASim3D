#pragma once
#include "CollisionObject.hpp"
#include "CollisionMaterials/CollisionNullMaterial.hpp"
#include <vector>
#include <cmath>
#include <iostream>
#include <algorithm>

class CollisionHeightmap : public CollisionObject
{
public:
    int numX = 0, numZ = 0;
    float cellSize = 1.0f;
    float offsetX = 0.0f, offsetZ = 0.0f;
    float minH = 0.0f, maxH = 0.0f;

    std::vector<float> heights;
    std::vector<vec3f> normals;

    CollisionHeightmap(int nx, int nz, float step, float width, float depth)
        : numX(nx), numZ(nz), cellSize(step)
    {
        offsetX = width / 2.0f;
        offsetZ = depth / 2.0f;

        heights.resize(numX * numZ, -2.5f);
        normals.resize(numX * numZ, vec3f{ 0.0f, 1.0f, 0.0f });

        auto s = std::make_shared<CollisionObject::Surface>();
        s->index = 0;
        s->material = std::make_shared<CollisionNullMaterial>();
        this->surfaces.push_back(s);

        minH = 1e9f;
        maxH = -1e9f;

        // Generowanie pofalowanego dna
        for (int z = 0; z < numZ; ++z) {
            for (int x = 0; x < numX; ++x) {
                float worldX = x * cellSize - offsetX;
                float worldZ = z * cellSize - offsetZ;
                float h = -2.5f + 0.3f * std::sin(worldX) * std::cos(worldZ);

                heights[z * numX + x] = h;
                minH = std::min(minH, h);
                maxH = std::max(maxH, h);
            }
        }

        // Prekalkulacja wektorów normalnych z ilorazów różnicowych
        for (int z = 0; z < numZ; ++z) {
            for (int x = 0; x < numX; ++x) {
                float lewy = (x > 0) ? heights[z * numX + (x - 1)] : heights[z * numX + x];
                float prawy = (x < numX - 1) ? heights[z * numX + (x + 1)] : heights[z * numX + x];
                float gora = (z > 0) ? heights[(z - 1) * numX + x] : heights[z * numX + x];
                float dol = (z < numZ - 1) ? heights[(z + 1) * numX + x] : heights[z * numX + x];

                float df_dx = (prawy - lewy) / (2.0f * cellSize);
                float df_dz = (dol - gora) / (2.0f * cellSize);

                vec3f n{ -df_dx, 1.0f, -df_dz };
                normals[z * numX + x] = vec3f::normalized(n);
            }
        }
    }

    virtual void GetAABB(Red::vec<float, 6>& aabb) override
    {
        aabb[0] = -offsetX;
        aabb[1] = minH - 1.0f;
        aabb[2] = -offsetZ;
        aabb[3] = offsetX;
        aabb[4] = maxH + 1.0f;
        aabb[5] = offsetZ;
    }

    virtual void RayCollisionInternal(float delta, float time, ray& r, ray_collision& rc) override
    {
        rc.has_collided = false;
    }

    float getInterpolatedHeight(float x, float z) const
    {
        float gridX = (x + offsetX) / cellSize;
        float gridZ = (z + offsetZ) / cellSize;

        int i = (int)gridX;
        int j = (int)gridZ;

        if (i < 0) i = 0; if (i >= numX - 1) i = numX - 2;
        if (j < 0) j = 0; if (j >= numZ - 1) j = numZ - 2;

        float u = gridX - i;
        float w = gridZ - j;

        float h00 = heights[j * numX + i];
        float h10 = heights[j * numX + (i + 1)];
        float h01 = heights[(j + 1) * numX + i];
        float h11 = heights[(j + 1) * numX + (i + 1)];

        return (1.0f - u) * (1.0f - w) * h00 + u * (1.0f - w) * h10 + (1.0f - u) * w * h01 + u * w * h11;
    }

    vec3f getInterpolatedNormal(float x, float z) const
    {
        float gridX = (x + offsetX) / cellSize;
        float gridZ = (z + offsetZ) / cellSize;

        int i = (int)gridX;
        int j = (int)gridZ;

        if (i < 0) i = 0; if (i >= numX - 1) i = numX - 2;
        if (j < 0) j = 0; if (j >= numZ - 1) j = numZ - 2;

        float u = gridX - i;
        float w = gridZ - j;

        vec3f n00 = normals[j * numX + i];
        vec3f n10 = normals[j * numX + (i + 1)];
        vec3f n01 = normals[(j + 1) * numX + i];
        vec3f n11 = normals[(j + 1) * numX + (i + 1)];

        vec3f interp = n00 * ((1.0f - u) * (1.0f - w)) +
            n10 * (u * (1.0f - w)) +
            n01 * ((1.0f - u) * w) +
            n11 * (u * w);
        return vec3f::normalized(interp);
    }

    void printStats() const
    {
        std::cout << "\n========================================\n";
        std::cout << " [HEIGHTMAP TEST] Utworzono mape dna:\n";
        std::cout << "  - Rozmiar siatki: " << numX << " x " << numZ << " (" << heights.size() << " punktow)\n";
        std::cout << "  - Rozstaw wezlow (cellSize): " << cellSize << " m\n";
        std::cout << "  - Wymiary obszaru: " << (offsetX * 2.0f) << " x " << (offsetZ * 2.0f) << " m\n";
        std::cout << "  - Zakres wysokosci: od " << minH << " m do " << maxH << " m\n";
        std::cout << "  - Srodek dna (x=0, z=0): h = " << heights[(numZ / 2) * numX + (numX / 2)] << " m\n";
        vec3f n_mid = normals[(numZ / 2) * numX + (numX / 2)];
        std::cout << "  - Normalna w srodku: (" << n_mid.x << ", " << n_mid.y << ", " << n_mid.z << ")\n";
        std::cout << "========================================\n\n";
    }
};