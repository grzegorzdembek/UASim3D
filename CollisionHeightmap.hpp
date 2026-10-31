#pragma once
#include "UASimLib.hpp"
#include "CollisionMaterials/CollisionNullMaterial.hpp"

#include <vector>     
#include <cmath>     
#include <iostream>   
#include <algorithm>  

class CollisionHeightmap : public CollisionObject
{
public:
    // Liczba wezlow (punktow pomiarowych) wzdluz osi X oraz Z
    int numX = 0;
    int numZ = 0;

    // Odleglosc miedzy sasiednimi wezlami siatki w metrach 
    float cellSize = 1.0f;

    // Przesuniecie ukladu wspolrzednych
    float offsetX = 0.0f;
    float offsetZ = 0.0f;

    // Zmienne do zapamietania najnizszego i najwyzszego punktu pofalowanego dna
    float minH = 0.0f;
    float maxH = 0.0f;

    // Tablica przechowujaca wysokosci dna Y 
    // Wzor na indeks komorki (x, z): index = z * numX + x.
    std::vector<float> heights;

    // Tablica wektorow normalnych dla kazdego wezla siatki
    std::vector<vec3f> normals;

    CollisionHeightmap(int nx, int nz, float step, float width, float depth)
        : numX(nx), numZ(nz), cellSize(step)
    {
        // 1. Obliczamy przesuniecie srodka 
        offsetX = width / 2.0f;
        offsetZ = depth / 2.0f;

        // 2. Alokujemy pamiec dla calej siatki w jednym ciaglym bloku
        heights.resize(numX * numZ, -2.5f);
        normals.resize(numX * numZ, vec3f{ 0.0f, 1.0f, 0.0f });

        // 3. Rejestracja powierzchni w klasie bazowej CollisionObject
        auto s = std::make_shared<CollisionObject::Surface>();
        s->index = 0;
        s->material = std::make_shared<CollisionNullMaterial>();
        this->surfaces.push_back(s);

        // 4. Inicjalizacja wartownikow do szukania minimum i maksimum wysokosci
        minH = 1e9f;
        maxH = -1e9f;

        // 5. Wypelnienie tablicy heights
        for (int z = 0; z < numZ; ++z)
        {
            for (int x = 0; x < numX; ++x)
            {
                // Przeliczamy dyskretne indeksy siatki (x, z) na metry w ukladzie basenu (worldX, worldZ)
                float worldX = x * cellSize - offsetX;
                float worldZ = z * cellSize - offsetZ;

                // Wzor na testowe pofalowane dno
                float h = -2.5f + 0.3f * std::sin(worldX) * std::cos(worldZ);

                // Zapisujemy wyliczona wysokosc w tablicy
                heights[z * numX + x] = h;

                // Aktualizujemy najnizszy i najwyzszy punkt terenu
                minH = std::min(minH, h);
                maxH = std::max(maxH, h);
            }
        }

        // 6. Prekalkulacja wektorow normalnych 
        for (int z = 0; z < numZ; ++z)
        {
            for (int x = 0; x < numX; ++x)
            {
                // Pobieramy wysokosci 4 sasiadow danego punktu: lewego, prawego, gornego i dolnego
                // Jesli jestesmy na brzegu siatki, jako sasiada bierzemy samego siebie
                float lewy = (x > 0) ? heights[z * numX + (x - 1)] : heights[z * numX + x];
                float prawy = (x < numX - 1) ? heights[z * numX + (x + 1)] : heights[z * numX + x];
                float gora = (z > 0) ? heights[(z - 1) * numX + x] : heights[z * numX + x];
                float dol = (z < numZ - 1) ? heights[(z + 1) * numX + x] : heights[z * numX + x];

                // Liczymy pochodna czastkowa (df/dx oraz df/dz)
                float df_dx = (prawy - lewy) / (2.0f * cellSize);
                float df_dz = (dol - gora) / (2.0f * cellSize);

                // Z definicji gradientu funkcji uwiklanej F(x,y,z) = y - h(x,z) = 0
                // Wektor prostopadly do powierzchni ma postac [-dh/dx, 1.0, -dh/dz]
                vec3f n{ -df_dx, 1.0f, -df_dz };

                // Normalizujemy wektor (dzielimy przez jego dlugosc), aby mial dlugosc rowna 1.0
                normals[z * numX + x] = vec3f::normalized(n);
            }
        }
    }

    virtual void GetAABB(Red::vec<float, 6>& aabb) override
    {
        aabb[0] = -offsetX;
        aabb[1] = -4.0f;    
        aabb[2] = -offsetZ;
        aabb[3] = offsetX;
        aabb[4] = 4.0f;      
        aabb[5] = offsetZ;
    }

    virtual void RayCollisionInternal(float delta, float time, ray& r, ray_collision& rc) override
    {
        // 1. Na starcie zakladamy, ze kolizji nie bylo 
        rc.has_collided = false;

        // 2. Pozycja poczatkowa (p0) i przewidywana pozycja koncowa (p1) promienia w tym kroku czasowym:
        vec3f p0 = r.position;
        vec3f p1 = p0 + r.velocity * delta;

        // 3. Odczytujemy dokladna wysokosc dna dokladnie pod punktem p0 oraz pod punktem p1
        // Uzywamy interpolacji dwuliniowej, bo promien prawie nigdy nie trafia dokladnie w wezel siatki
        float h0 = getInterpolatedHeight(p0.x, p0.z);
        float h1 = getInterpolatedHeight(p1.x, p1.z);

        // 4. Liczymy dystans pionowy ("luke") miedzy promieniem a dnem
        // gap > 0 oznacza, ze promien jest nad dnem
        // gap < 0 oznacza, ze promien znalazl sie pod dnem
        float gap0 = p0.y - h0;
        float gap1 = p1.y - h1;

        // 5. Warunek kolizji ciaglej:
        // Zderzenie zachodzi, gdy przewidywana pozycja p1 wpadla pod dno (gap1 < 0)
        // oraz promien leci w dol w strone podloza (r.velocity.y < 0 lub wczesniej byl nad dnem gap0 >= 0).
        if (gap1 < 0.0f && (gap0 >= 0.0f || r.velocity.y < 0.0f))
        {
            // 6. Podobienstwo trojkatow: wyznaczamy dokladny ulamek kroku czasowego,
            // w ktorym promien uderzyl w dno (moment, w ktorym gap = 0)
            float denom = gap0 - gap1;
            float fraction = (std::fabs(denom) > 1e-9f) ? (gap0 / denom) : 0.0f;
            fraction = std::clamp(fraction, 0.0f, 1.0f); // Bezpiecznik: fraction musi byc w [0.0, 1.0]

            // 7. Wspolrzedne punktu uderzenia
            vec3f hit_point = p0 + r.velocity * (delta * fraction);

            // 8. Pobieramy zinterpolowany wektor normalny w punkcie uderzenia
            vec3f n = getInterpolatedNormal(hit_point.x, hit_point.z);

            // 9. Iloczyn skalarny bada kat padania promienia wzgledem normalnej
            // Warunek dot_vn < 0 oznacza, ze promien rzeczywiscie zbliza sie do podloza.
            float dot_vn = vec3f::dot(r.velocity, n);
            if (dot_vn < 0.0f)
            {
                // 10. Wzor odbicia: v_odbite = v - 2*(v · n)*n
                vec3f v_refl = r.velocity - n * (2.0f * dot_vn);

                // 11. Zapisujemy wyniki kolizji do struktury rc, ktora zwrocimy do silnika
                rc.collision_point = hit_point; 
                rc.collision_normal = n;         
                rc.exit_vector = v_refl;         

                // Odsuwamy nowa pozycje promienia o 2 milimetry w gore wzdluz normalnej
                rc.exit_position = hit_point + n * 0.002f;

                rc.surface = 0;           // Indeks powierzchni zero
                rc.has_delay = false;     // Fala odbija sie natychmiast, bez opoznienia czasowego
                rc.has_collided = true;   // Informujemy silnik: zderzenie nastapilo

                // 12. Straty energii przy odbiciu od dna 
                r.energy *= 0.5f;

                // 13. Wypisanie pierwszych 5 odbic do konsoli do testu
                static int licznik_odbic = 0;
                if (licznik_odbic < 5)
                {
                    printf("[HEIGHTMAP ODBICIE] t=%.6f s | hit=(%.3f, %.3f, %.3f) | n=(%.3f, %.3f, %.3f) | vel_po=(%.1f, %.1f, %.1f)\n",
                        time, hit_point.x, hit_point.y, hit_point.z, n.x, n.y, n.z, v_refl.x, v_refl.y, v_refl.z);
                    licznik_odbic++;
                }
            }
        }
    }

    float getInterpolatedHeight(float x, float z) const
    {
        // Krok 1: Przeliczamy wspolrzedne w metrach na ulamkowe indeksy siatki
        float gridX = (x + offsetX) / cellSize;
        float gridZ = (z + offsetZ) / cellSize;

        // Krok 2: Zaokraglamy w dol, aby uzyskac indeks lewego-dolnego wezla komorki
        int i = (int)gridX;
        int j = (int)gridZ;

        // Krok 3: Zabezpieczenie przed wyjsciem poza zakres tablicy 
        if (i < 0) i = 0;
        if (i >= numX - 1) i = numX - 2;
        if (j < 0) j = 0;
        if (j >= numZ - 1) j = numZ - 2;

        // Krok 4: Wagi interpolacji u oraz w 
        float u = gridX - i;
        float w = gridZ - j;

        // Krok 5: Pobieramy wartosci wysokosci z 4 naroznikow komorki siatki:
        // h00 -> lewy dolny,   h10 -> prawy dolny
        // h01 -> lewy gorny,   h11 -> prawy gorny
        float h00 = heights[j * numX + i];
        float h10 = heights[j * numX + (i + 1)];
        float h01 = heights[(j + 1) * numX + i];
        float h11 = heights[(j + 1) * numX + (i + 1)];

        // Krok 6: Wzor interpolacji dwuliniowej (srednia wazona z 4 wierzcholkow):
        return (1.0f - u) * (1.0f - w) * h00 +
            u * (1.0f - w) * h10 +
            (1.0f - u) * w * h01 +
            u * w * h11;
    }

    vec3f getInterpolatedNormal(float x, float z) const
    {
        // Krok 1: Przeliczamy wspolrzedne w metrach na ulamkowe indeksy siatki
        float gridX = (x + offsetX) / cellSize;
        float gridZ = (z + offsetZ) / cellSize;

        // Krok 2: Zaokraglamy w dol, aby uzyskac indeks lewego-dolnego wezla komorki
        int i = (int)gridX;
        int j = (int)gridZ;

        // Krok 3: Zabezpieczenie przed wyjsciem poza zakres tablicy 
        if (i < 0) i = 0;
        if (i >= numX - 1) i = numX - 2;
        if (j < 0) j = 0;
        if (j >= numZ - 1) j = numZ - 2;

        // Krok 4: Wagi interpolacji u oraz w 
        float u = gridX - i;
        float w = gridZ - j;

        // Krok 5: Pobieramy wartosci wysokosci z 4 naroznikow komorki siatki:
        // h00 -> lewy dolny,   h10 -> prawy dolny
        // h01 -> lewy gorny,   h11 -> prawy gorny
        vec3f n00 = normals[j * numX + i];
        vec3f n10 = normals[j * numX + (i + 1)];
        vec3f n01 = normals[(j + 1) * numX + i];
        vec3f n11 = normals[(j + 1) * numX + (i + 1)];

        // Krok 6: Wzor interpolacji dwuliniowej (srednia wazona z 4 wierzcholkow):
        vec3f interp = n00 * ((1.0f - u) * (1.0f - w)) +
            n10 * (u * (1.0f - w)) +
            n01 * ((1.0f - u) * w) +
            n11 * (u * w);

        // Krok 7: Normalizujemy wynikowy wektor
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