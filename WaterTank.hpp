#pragma once

// WaterTank: reprezentacja srodowiska wodnego.
// Odpowiedzialnosc:
//  - Przechowywanie wymiarow geometrycznych zbiornika (Cuboid_dimensions).
//  - Zarzadzanie trybem dna (plaskie / pofalowane testowe / wlasne dane) oraz scian bocznych.
//  - Obsluga odbic promieni i ich kolizji z granicami zbiornika.
//  - Uutrzymywanie mikrofonu i zrodla wewnatrz basenu.

#include <vector>
#include <memory>
#include <iostream>
#include <algorithm>

#include "RedCppLib/RedCppLib.hpp"
using Red::vec2f;
using Red::vec3f;
using Red::vec3i;

#include "datatypes.hpp"
#include "CollisionMaterials/CollisionMaterial.hpp"
#include "CollisionMaterials/CollisionNullMaterial.hpp"
#include "CollisionObjects/CollisionObject.hpp"
#include "CollisionObjects/CollisionBox.hpp"
#include "CollisionObjects/CollisionHeightmap.hpp"
#include "UASimConstants.hpp"
#include "geometry.hpp"
#include "wave.hpp"
#include "microphone.hpp"

class WaterTank
{
public:
    // Dostepne tryby pracy dna zbiornika (dostepne beda do wyboru w GUI)
    enum BottomMode : int
    {
        BOTTOM_FLAT = 0,       // Plaskie dno na glebokosci -height/2 (bazowe zachowanie symulatora)
        BOTTOM_TEST = 1,  // Testowe pofalowane dno (funkcja sin/cos)
        BOTTOM_CUSTOM = 2      // Wlasne dane batymetryczne (miejsce na rozbudowe)
    };

    // Wymiary prostopadloscianu basenu
    Cuboid_dimensions dims;

    // Aktualnie wybrany tryb dna 
    int bottomMode = BOTTOM_TEST;

    // Wskaznik na obiekt pofalowanego dna (nullptr, gdy wybrane jest dno plaskie)
    std::shared_ptr<CollisionHeightmap> bottomHeightmap = nullptr;

    // Lista na obiekty graniczne 
    std::vector<std::shared_ptr<CollisionObject>> boundaryObjects;
    std::vector<Red::vec<float, 6>> boundaryAABBs;

    // Inicjalizacja wymiarow zbiornika i budowa jego granic wedlug wybranego trybu
    WaterTank(float width = 20.0f, float height = 6.0f, float depth = 15.0f, int initialBottomMode = BOTTOM_TEST)
        : bottomMode(initialBottomMode)
    {
        dims.width = width;
        dims.height = height;
        dims.depth = depth;
        dims.x_offset = 0.0f;
        dims.y_offset = 0.0f;
        dims.z_offset = 0.0f;

        RebuildBoundaries();
    }

    // Przebudowuje obiekty kolizyjne granic zbiornika (wywolywane przy starcie oraz po kazdej zmianie opcji w GUI).
    void RebuildBoundaries()
    {
        boundaryObjects.clear();
        boundaryAABBs.clear();
        bottomHeightmap = nullptr;

        if (bottomMode == BOTTOM_TEST)
        {
            // Siatka pofalowanego dna 41 x 31 o kroku 0.5m dopasowana do szerokosci i glebokosci basenu
            bottomHeightmap = std::make_shared<CollisionHeightmap>(41, 31, 0.5f, dims.width, dims.depth);
            bottomHeightmap->printStats();
            RegisterBoundaryObject(bottomHeightmap);
        }
        else if (bottomMode == BOTTOM_FLAT)
        {
            std::cout << "\n========================================\n";
            std::cout << " [WATER TANK] Aktywowano PLASKIE DNO (y = " << (-0.5f * dims.height + dims.y_offset) << " m)\n";
            std::cout << "========================================\n\n";
        }
        else if (bottomMode == BOTTOM_CUSTOM)
        {
            std::cout << "\n========================================\n";
            std::cout << " [WATER TANK] Wybrano WLASNE DANE BATYMETRYCZNE (brak implementacji)\n";
            std::cout << "========================================\n\n";
        }
    }

    // Funkcja odbicia jednowymiarowego dla plaskich granic prostopadloscianu
    static int Bounce1D(float& pos, float& vel, float minb, float maxb)
    {
        const float eps = 0.001f; 
        if (pos < minb)
        {
            pos = minb + eps;
            vel = -vel;
            return -1;
        }
        else if (pos > maxb)
        {
            pos = maxb - eps;
            vel = -vel;
            return +1;
        }
        return 0;
    }

    // Utrzymuje mikrofon oraz zrodlo dzwieku wewnatrz granic basenu
    void ConstrainRigidBodies(SoundSource& source, Microphone& Mic) const
    {
        const float halfW = 0.5f * dims.width;
        const float halfH = 0.5f * dims.height;
        const float halfD = 0.5f * dims.depth;
        const float micR = Mic.radius;

        // Odbicia mikrofonu od scian basenu
        Bounce1D(Mic.mic_x, Mic.mic_velocity.x, -halfW + dims.x_offset + micR, halfW + dims.x_offset - micR);
        Bounce1D(Mic.mic_y, Mic.mic_velocity.y, -halfH + dims.y_offset + micR, halfH + dims.y_offset - micR);
        Bounce1D(Mic.mic_z, Mic.mic_velocity.z, -halfD + dims.z_offset + micR, halfD + dims.z_offset - micR);

        // Odbicia zrodla dzwieku od scian basenu
        Bounce1D(source.src_x, source.velocity.x, -halfW + dims.x_offset, halfW + dims.x_offset);
        Bounce1D(source.src_y, source.velocity.y, -halfH + dims.y_offset, halfH + dims.y_offset);
        Bounce1D(source.src_z, source.velocity.z, -halfD + dims.z_offset, halfD + dims.z_offset);
    }

    // Sprawdza ciagla detekcje kolizji (CCD) dla wszystkich promieni ze wszystkimi pofalowanymi granicami zbiornika
    void ProcessRayCollisionsCCD(float dt, float time_passed, std::vector<ray>& nodes)
    {
        if (boundaryObjects.empty())
            return;

        // Aktualizacja AABB wszystkich granic
        for (size_t i = 0; i < boundaryObjects.size(); ++i)
        {
            boundaryObjects[i]->GetAABB(boundaryAABBs[i]);
        }

        // Test kolizji dla kazdego promienia fali
        for (auto& r : nodes)
        {
            ray_collision rc;
            for (size_t i = 0; i < boundaryAABBs.size(); ++i)
            {
                const auto& o = boundaryObjects[i];
                const auto& aabb = boundaryAABBs[i];

                bool inside_aabb = true;
                for (int ii = 0; ii < 3; ++ii)
                {
                    inside_aabb = (r.position[ii] >= aabb[ii] && r.position[ii] <= aabb[ii + 3]);
                    if (!inside_aabb)
                        break;
                }

                if (inside_aabb)
                {
                    o->RayCollision(dt, time_passed, r, rc);
                    if (rc.has_collided)
                    {
                        r.position = rc.exit_position;
                        r.velocity = rc.exit_vector;
                        r.bounces++;
                        break;
                    }
                }
            }
        }
    }

    // Obsluguje odbicia pojedynczego promienia od plaskich scian, lustra wody oraz plaskiego dna (jesli brak pofalowanego dna).
    void ProcessFlatBoundaryForRay(float time_passed, int nodeIdx, ray& r) const
    {
        const float halfW = 0.5f * dims.width;
        const float halfH = 0.5f * dims.height;
        const float halfD = 0.5f * dims.depth;

        auto& p = r.position;
        auto& v = r.velocity;
        auto& energy = r.energy;

        // 1. Odbicie od scian bocznych X (lewa / prawa): tlumienie 0.5
        int sideX = Bounce1D(p.x, v.x, -halfW + dims.x_offset, halfW + dims.x_offset);
        if (sideX != 0)
        {
            energy *= 0.5f;
        }

        // 2. Odbicie w osi Y (dno: tlumienie 0.5, lustro wody u gory: tlumienie 0.3)
        int sideY = Bounce1D(p.y, v.y, -halfH + dims.y_offset, halfH + dims.y_offset);
        if (sideY != 0)
        {
            if (sideY > 0)
            {
                // Gorna granica basenu (lustro wody y = +halfH)
                energy *= 0.3f;
            }
            else
            {
                // Dolna granica basenu (plaskie dno y = -halfH)
                energy *= 0.5f;

                static int licznik_dna = 0;
                if (licznik_dna < 5)
                {
                    printf("[DNO PLASKIE] t=%.6f s | node=%d | pos=(%.3f, %.3f, %.3f) | vel_po=(%.1f, %.1f, %.1f)\n",
                        time_passed, nodeIdx, p.x, p.y, p.z, v.x, v.y, v.z);
                    licznik_dna++;
                }
            }
        }

        // 3. Odbicie od scian bocznych Z (przednia / tylna): tlumienie 0.5
        int sideZ = Bounce1D(p.z, v.z, -halfD + dims.z_offset, halfD + dims.z_offset);
        if (sideZ != 0)
        {
            energy *= 0.5f;
        }
    }

private:
    void RegisterBoundaryObject(std::shared_ptr<CollisionObject> obj)
    {
        if (!obj)
            return;
        boundaryObjects.push_back(obj);
        Red::vec<float, 6> aabb;
        obj->GetAABB(aabb);
        boundaryAABBs.push_back(aabb);
    }
};