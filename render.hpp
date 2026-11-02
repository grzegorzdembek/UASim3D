#pragma once
#include "hmain.hpp"

inline int printOversizedTriangles(shared_ptr<Wave> wave, const Cuboid_dimensions& Cube, float maxArea)
{
    // te same sta�e co w refine
    const float cuboidHalfWidth = Cube.width * 0.5f;
    const float cuboidHalfHeight = Cube.height * 0.5f;
    const float cuboidHalfDepth = Cube.depth * 0.5f;
    const float safetyMargin = 0.2f;

    // por�wnujemy |cross|^2 > (2*maxArea)^2
    const float thr2 = 4.0f * maxArea * maxArea;

    int count = 0;
    for (const auto &t : wave->triangles)
    {
        int a = t.indices[0], b = t.indices[1], c = t.indices[2];
        const vec3f &pa = wave->nodes[a].position;
        const vec3f &pb = wave->nodes[b].position;
        const vec3f &pc = wave->nodes[c].position;

        // pomijamy tr�jk�ty blisko �ciany (jak w refine)
        const bool nearWall =
            (std::fabs(pa.x) > cuboidHalfWidth - safetyMargin) ||
            (std::fabs(pa.y) > cuboidHalfHeight - safetyMargin) ||
            (std::fabs(pa.z) > cuboidHalfDepth - safetyMargin) ||
            (std::fabs(pb.x) > cuboidHalfWidth - safetyMargin) ||
            (std::fabs(pb.y) > cuboidHalfHeight - safetyMargin) ||
            (std::fabs(pb.z) > cuboidHalfDepth - safetyMargin) ||
            (std::fabs(pc.x) > cuboidHalfWidth - safetyMargin) ||
            (std::fabs(pc.y) > cuboidHalfHeight - safetyMargin) ||
            (std::fabs(pc.z) > cuboidHalfDepth - safetyMargin);

        if (nearWall)
            continue;

        // pole bez sqrt: |(pb-pa) x (pc-pa)|^2
        const vec3f cr = vec3f::cross(pb - pa, pc - pa);
        const float cr2 = vec3f::dot(cr, cr);

        if (cr2 > thr2)
            ++count;
    }
    float procent = (float)count / (float)wave->triangles.size() * 100.0f;
    std::cout << "Oversized (splittable) triangles: "
              << wave->triangles.size() << "\n";
    return count;
}

inline void buildBuffersFor(const std::vector<ray> &verts,
                     const std::vector<Triangle> &tris,
                     MeshGL &m,
                     bool dynamic)
{
    // if (!GLAD_GL_VERSION_1_5) { std::cerr << "VBO niedost�pne (GL < 1.5)\n"; return; }
    m.dynamic = dynamic;

    if (!m.vbo)
        glGenBuffers(1, &m.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);

    const GLsizeiptr vbSize = (GLsizeiptr)(verts.size() * sizeof(ray));
    const GLenum usage = dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
    glBufferData(GL_ARRAY_BUFFER, vbSize, verts.empty() ? nullptr : (const void *)verts.data(), usage);

    if (!m.ibo)
        glGenBuffers(1, &m.ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ibo);

    const GLsizeiptr ibSize = (GLsizeiptr)(tris.size() * 3 * sizeof(unsigned int));
    const void *idxSrc = tris.empty() ? nullptr : (const void *)&tris[0].indices[0];
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ibSize, idxSrc, GL_STATIC_DRAW);

    m.indexCount = (GLsizei)(tris.size() * 3);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
} // bufory

inline void updatePositionsFor(const std::vector<ray> &verts, MeshGL &m)
{
    if (!m.vbo)
        return;
    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);

    const GLsizeiptr vbSize = (GLsizeiptr)(verts.size() * sizeof(ray));
    glBufferData(GL_ARRAY_BUFFER, vbSize, nullptr, GL_DYNAMIC_DRAW);
    if (!verts.empty())
        glBufferSubData(GL_ARRAY_BUFFER, 0, vbSize, (const void *)verts.data());

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

inline void drawCuboidTransparentSorted(glm::vec3 cameraPos, const Cuboid_dimensions& temp_Cube)
{
    float halfWidth = temp_Cube.width / 2.0f;
    float halfHeight = temp_Cube.height / 2.0f;
    float halfDepth = temp_Cube.depth / 2.0f;

    glm::vec3 vertices[] = {
        {-halfWidth + temp_Cube.x_offset, -halfHeight + temp_Cube.y_offset, halfDepth + temp_Cube.z_offset},
        {halfWidth + temp_Cube.x_offset, -halfHeight + temp_Cube.y_offset, halfDepth + temp_Cube.z_offset},
        {halfWidth + temp_Cube.x_offset, halfHeight + temp_Cube.y_offset, halfDepth + temp_Cube.z_offset},
        {-halfWidth + temp_Cube.x_offset, halfHeight + temp_Cube.y_offset, halfDepth + temp_Cube.z_offset},
        {-halfWidth + temp_Cube.x_offset, -halfHeight + temp_Cube.y_offset, -halfDepth + temp_Cube.z_offset},
        {halfWidth + temp_Cube.x_offset, -halfHeight + temp_Cube.y_offset, -halfDepth + temp_Cube.z_offset},
        {halfWidth + temp_Cube.x_offset, halfHeight + temp_Cube.y_offset, -halfDepth + temp_Cube.z_offset},
        {-halfWidth + temp_Cube.x_offset, halfHeight + temp_Cube.y_offset, -halfDepth + temp_Cube.z_offset}};

    struct Face
    {
        int indices[4];
        float distance;
    };

    Face faces[6] = {
        {{0, 1, 2, 3}, 0.0f}, // prz�d
        {{4, 5, 6, 7}, 0.0f}, // ty�
        {{0, 3, 7, 4}, 0.0f}, // lewo
        {{1, 2, 6, 5}, 0.0f}, // prawo
        {{0, 1, 5, 4}, 0.0f}, // d�
        {{2, 3, 7, 6}, 0.0f}  // g�ra
    };

    // Oblicz odleg�o�ci �cian od kamery
    for (int i = 0; i < 6; ++i)
    {
        glm::vec3 center(0.0f);
        for (int j = 0; j < 4; ++j)
        {
            center += vertices[faces[i].indices[j]];
        }
        center /= 4.0f;
        faces[i].distance = glm::length(center - cameraPos);
    }

    // Sortuj �ciany od najdalszej do najbli�szej
    for (int i = 0; i < 5; ++i)
    {
        for (int j = i + 1; j < 6; ++j)
        {
            if (faces[i].distance < faces[j].distance)
            {
                std::swap(faces[i], faces[j]);
            }
        }
    }

    // --- WŁĄCZAMY PRZEZROCZYSTOŚĆ DLA ŚCIAN BASENU ---
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Wyłączamy zapis do bufora głębokości na czas rysowania przezroczystych ścian.
    glDepthMask(GL_FALSE);

    glBegin(GL_QUADS);
    for (int i = 0; i < 6; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            const glm::vec3& v = vertices[faces[i].indices[j]];
            glVertex3f(v.x, v.y, v.z);
        }
    }
    glEnd();

    // Przywracamy zapis do bufora głębokości i wyłączamy blendowanie
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    // KRAWĘDZIE
    glDisable(GL_DEPTH_TEST);
    glLineWidth(2.0f);

    static const int edges[12][2] = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0}, // prz�d
        {4, 5},
        {5, 6},
        {6, 7},
        {7, 4}, // ty�
        {0, 4},
        {1, 5},
        {2, 6},
        {3, 7} // ��cz�ce
    };
    glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
    glBegin(GL_LINES);
    for (int e = 0; e < 12; ++e)
    {
        const glm::vec3 &v0 = vertices[edges[e][0]];
        const glm::vec3 &v1 = vertices[edges[e][1]];
        glVertex3f(v0.x, v0.y, v0.z);
        glVertex3f(v1.x, v1.y, v1.z);
    }
    glEnd();
}

inline void drawMesh(const MeshGL &m, const glm::vec3 &offset = glm::vec3(0), 
    const glm::vec4 &fill = glm::vec4(1.0f, 0.5f, 0.0f, 1.0f), 
    const glm::vec4 &wire = glm::vec4(0.0f, 0.0f, 0.0f, 0.6f), 
    float wireWidth = 0.2f)
{
    if (!m.vbo || !m.ibo || m.indexCount == 0)
        return;

    glPushMatrix();
    if (offset.x != 0 || offset.y != 0 || offset.z != 0)
        glTranslatef(offset.x, offset.y, offset.z);

    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glEnableClientState(GL_VERTEX_ARRAY); // compat profile
    glVertexPointer(3, GL_FLOAT, (GLsizei)sizeof(ray), (const void *)offsetof(ray, position));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ibo);

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0f, 1.0f);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glColor4f(fill.r, fill.g, fill.b, fill.a);
    glDrawElements(GL_TRIANGLES, m.indexCount, GL_UNSIGNED_INT, (void *)0);

    glDisable(GL_POLYGON_OFFSET_FILL);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(wireWidth);
    glColor4f(wire.r, wire.g, wire.b, wire.a);
    glDrawElements(GL_TRIANGLES, m.indexCount, GL_UNSIGNED_INT, (void *)0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glPopMatrix();
}

inline void RenderSource(SoundSource& source, MeshGL& gSrcGL)
{
    glEnable(GL_DEPTH_TEST);
    glm::vec3 srcOffset = glm::vec3(source.src_x, source.src_y, source.src_z);
    drawMesh(gSrcGL, srcOffset, /*fill*/ {1.0f, 1.0f, 1.0f, 0.5f});
}

inline void RenderMicrophone(Microphone& Mic, MeshGL& gMicGL)
{
    glEnable(GL_DEPTH_TEST);
    glm::vec3 micOffset = glm::vec3(Mic.mic_x, Mic.mic_y, Mic.mic_z);
    drawMesh(gMicGL, micOffset, /*fill*/ {1.0f, 0.4f, 0.8f, 0.5f});
}
inline void RenderPool(glm::vec3 cameraPos, const Cuboid_dimensions& Cube )
{
    //glColor4f(0.2f, 0.5f, 0.2f, 0.7f);
    //drawCuboidTransparentSorted(cameraPos, Obstacle);
     glEnable(GL_DEPTH_TEST);
    //  Basen
    // glDisable(GL_DEPTH_TEST);
    glColor4f(0.1f, 0.4f, 0.9f, 0.08f);
    drawCuboidTransparentSorted(cameraPos, Cube);
}

inline void RenderTankBottom(const WaterTank& tank)
{
    // 1. Jezeli w GUI NIE wybrano pofalowanego dna lub mapa nie istnieje - nic nie rysujemy
    if (tank.bottomMode != WaterTank::BOTTOM_TEST || tank.bottomHeightmap == nullptr)
        return;

    const auto& ch = tank.bottomHeightmap;
    glEnable(GL_DEPTH_TEST);

    // Wyliczamy zakres wysokosci, zeby poprawnie nakladac kolory
    const float hRange = std::max(0.001f, ch->maxH - ch->minH);
    auto setSandColor = [&](float h) {
        float t = (h - ch->minH) / hRange; // ulamek w przedziale [0.0, 1.0]
        glColor3f(0.45f + 0.35f * t, 0.38f + 0.32f * t, 0.22f + 0.25f * t);
        };

    // -------------------------------------------------------------------------
    // ETAP 1: Rysowanie gornej powierzchni dna (powloki)
    // -------------------------------------------------------------------------
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0f, 1.0f);

    glBegin(GL_QUADS);
    for (int z = 0; z < ch->numZ - 1; ++z) {
        for (int x = 0; x < ch->numX - 1; ++x) {
            // TUTAJ POPRAWKA: Uzywamy Twoich nowych zmiennych startX i startZ
            float x0 = ch->startX + x * ch->cellSize;
            float x1 = ch->startX + (x + 1) * ch->cellSize;
            float z0 = ch->startZ + z * ch->cellSize;
            float z1 = ch->startZ + (z + 1) * ch->cellSize;

            float h00 = ch->heights[z * ch->numX + x];
            float h10 = ch->heights[z * ch->numX + (x + 1)];
            float h11 = ch->heights[(z + 1) * ch->numX + (x + 1)];
            float h01 = ch->heights[(z + 1) * ch->numX + x];

            setSandColor(h00); glVertex3f(x0, h00, z0);
            setSandColor(h10); glVertex3f(x1, h10, z0);
            setSandColor(h11); glVertex3f(x1, h11, z1);
            setSandColor(h01); glVertex3f(x0, h01, z1);
        }
    }
    glEnd();

    // -------------------------------------------------------------------------
    // ETAP 2: Rysowanie objetosci - wypelnienie miedzy krawedziami fali a dnem (bottomY)
    // -------------------------------------------------------------------------
    // Ciemniejszy brazowy kolor dla bocznych krawedzi wykopu
    glColor3f(0.35f, 0.28f, 0.15f);
    glBegin(GL_QUADS);

    // Tylna sciana (Z_min) - w osi Z=0
    for (int x = 0; x < ch->numX - 1; ++x) {
        float x0 = ch->startX + x * ch->cellSize;
        float x1 = ch->startX + (x + 1) * ch->cellSize;
        float h0 = ch->heights[0 * ch->numX + x];
        float h1 = ch->heights[0 * ch->numX + (x + 1)];
        // Tworzymy prostokat schodzacy z fali (h0, h1) pionowo w dol do dna basenu (ch->bottomY)
        glVertex3f(x0, ch->bottomY, ch->startZ);
        glVertex3f(x1, ch->bottomY, ch->startZ);
        glVertex3f(x1, h1, ch->startZ);
        glVertex3f(x0, h0, ch->startZ);
    }
    // Przednia sciana (Z_max)
    for (int x = 0; x < ch->numX - 1; ++x) {
        float x0 = ch->startX + x * ch->cellSize;
        float x1 = ch->startX + (x + 1) * ch->cellSize;
        float zMax = ch->startZ + (ch->numZ - 1) * ch->cellSize;
        float h0 = ch->heights[(ch->numZ - 1) * ch->numX + x];
        float h1 = ch->heights[(ch->numZ - 1) * ch->numX + (x + 1)];
        glVertex3f(x1, ch->bottomY, zMax);
        glVertex3f(x0, ch->bottomY, zMax);
        glVertex3f(x0, h0, zMax);
        glVertex3f(x1, h1, zMax);
    }
    // Lewa sciana (X_min)
    for (int z = 0; z < ch->numZ - 1; ++z) {
        float z0 = ch->startZ + z * ch->cellSize;
        float z1 = ch->startZ + (z + 1) * ch->cellSize;
        float h0 = ch->heights[z * ch->numX + 0];
        float h1 = ch->heights[(z + 1) * ch->numX + 0];
        glVertex3f(ch->startX, ch->bottomY, z1);
        glVertex3f(ch->startX, ch->bottomY, z0);
        glVertex3f(ch->startX, h0, z0);
        glVertex3f(ch->startX, h1, z1);
    }
    // Prawa sciana (X_max)
    for (int z = 0; z < ch->numZ - 1; ++z) {
        float z0 = ch->startZ + z * ch->cellSize;
        float z1 = ch->startZ + (z + 1) * ch->cellSize;
        float xMax = ch->startX + (ch->numX - 1) * ch->cellSize;
        float h0 = ch->heights[z * ch->numX + (ch->numX - 1)];
        float h1 = ch->heights[(z + 1) * ch->numX + (ch->numX - 1)];
        glVertex3f(xMax, ch->bottomY, z0);
        glVertex3f(xMax, ch->bottomY, z1);
        glVertex3f(xMax, h1, z1);
        glVertex3f(xMax, h0, z0);
    }
    glEnd();
    glDisable(GL_POLYGON_OFFSET_FILL);

    // -------------------------------------------------------------------------
    // ETAP 3: Czarna siatka krawedziowa na powierzchni
    // -------------------------------------------------------------------------
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(1.5f);
    glColor3f(0.1f, 0.1f, 0.1f);

    glBegin(GL_QUADS);
    for (int z = 0; z < ch->numZ - 1; ++z) {
        for (int x = 0; x < ch->numX - 1; ++x) {
            float x0 = ch->startX + x * ch->cellSize;
            float x1 = ch->startX + (x + 1) * ch->cellSize;
            float z0 = ch->startZ + z * ch->cellSize;
            float z1 = ch->startZ + (z + 1) * ch->cellSize;

            float h00 = ch->heights[z * ch->numX + x];
            float h10 = ch->heights[z * ch->numX + (x + 1)];
            float h11 = ch->heights[(z + 1) * ch->numX + (x + 1)];
            float h01 = ch->heights[(z + 1) * ch->numX + x];

            glVertex3f(x0, h00, z0);
            glVertex3f(x1, h10, z0);
            glVertex3f(x1, h11, z1);
            glVertex3f(x0, h01, z1);
        }
    }
    glEnd();

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void RenderCollisionObject(shared_ptr<Wave> wave, shared_ptr<CollisionObject> osptr)
{
    if(auto cb = dynamic_cast<CollisionBox*>(osptr.get()))
    {
        // Save matrix
        glPushMatrix();

        // Translate to position
        glTranslatef(cb->position.x, cb->position.y, cb->position.z);

        // Rotate: stored as radians (X = pitch, Y = yaw, Z = roll).
        const float RAD_TO_DEG = 180.0f / 3.14159265358979323846f;
        glRotatef(cb->rotation.z * RAD_TO_DEG, 0.0f, 0.0f, 1.0f); // roll (Z)
        glRotatef(cb->rotation.y * RAD_TO_DEG, 0.0f, 1.0f, 0.0f); // yaw  (Y)
        glRotatef(cb->rotation.x * RAD_TO_DEG, 1.0f, 0.0f, 0.0f); // pitch(X)

        // Local corners
        vec3f bmin = cb->bmin_local;
        vec3f bmax = cb->bmax_local;

        vec3f v[8] = {
            {bmin.x, bmin.y, bmin.z}, // 0
            {bmax.x, bmin.y, bmin.z}, // 1
            {bmax.x, bmax.y, bmin.z}, // 2
            {bmin.x, bmax.y, bmin.z}, // 3
            {bmin.x, bmin.y, bmax.z}, // 4
            {bmax.x, bmin.y, bmax.z}, // 5
            {bmax.x, bmax.y, bmax.z}, // 6
            {bmin.x, bmax.y, bmax.z}  // 7
        };

        // Draw wireframe edges
        glColor3f(1.0f, 0.0f, 0.0f); // red for example
        glBegin(GL_LINES);
        // bottom rectangle (0-1-2-3)
        glVertex3f(v[0].x, v[0].y, v[0].z); glVertex3f(v[1].x, v[1].y, v[1].z);
        glVertex3f(v[1].x, v[1].y, v[1].z); glVertex3f(v[2].x, v[2].y, v[2].z);
        glVertex3f(v[2].x, v[2].y, v[2].z); glVertex3f(v[3].x, v[3].y, v[3].z);
        glVertex3f(v[3].x, v[3].y, v[3].z); glVertex3f(v[0].x, v[0].y, v[0].z);

        // top rectangle (4-5-6-7)
        glVertex3f(v[4].x, v[4].y, v[4].z); glVertex3f(v[5].x, v[5].y, v[5].z);
        glVertex3f(v[5].x, v[5].y, v[5].z); glVertex3f(v[6].x, v[6].y, v[6].z);
        glVertex3f(v[6].x, v[6].y, v[6].z); glVertex3f(v[7].x, v[7].y, v[7].z);
        glVertex3f(v[7].x, v[7].y, v[7].z); glVertex3f(v[4].x, v[4].y, v[4].z);

        // vertical edges
        for(int i = 0; i < 4; ++i)
        {
            glVertex3f(v[i].x, v[i].y, v[i].z);
            glVertex3f(v[i+4].x, v[i+4].y, v[i+4].z);
        }
        glEnd();

        // Optionally draw filled faces (semi-transparent)
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.0f, 0.7f, 0.7f, 0.2f);

        glBegin(GL_QUADS);
        // +Z top (4,5,6,7)
        glVertex3f(v[4].x, v[4].y, v[4].z); glVertex3f(v[5].x, v[5].y, v[5].z);
        glVertex3f(v[6].x, v[6].y, v[6].z); glVertex3f(v[7].x, v[7].y, v[7].z);
        // -Z bottom (0,1,2,3)
        glVertex3f(v[0].x, v[0].y, v[0].z); glVertex3f(v[1].x, v[1].y, v[1].z);
        glVertex3f(v[2].x, v[2].y, v[2].z); glVertex3f(v[3].x, v[3].y, v[3].z);
        // +Y (3,2,6,7)
        glVertex3f(v[3].x, v[3].y, v[3].z); glVertex3f(v[2].x, v[2].y, v[2].z);
        glVertex3f(v[6].x, v[6].y, v[6].z); glVertex3f(v[7].x, v[7].y, v[7].z);
        // -Y (0,1,5,4)
        glVertex3f(v[0].x, v[0].y, v[0].z); glVertex3f(v[1].x, v[1].y, v[1].z);
        glVertex3f(v[5].x, v[5].y, v[5].z); glVertex3f(v[4].x, v[4].y, v[4].z);
        // +X (1,2,6,5)
        glVertex3f(v[1].x, v[1].y, v[1].z); glVertex3f(v[2].x, v[2].y, v[2].z);
        glVertex3f(v[6].x, v[6].y, v[6].z); glVertex3f(v[5].x, v[5].y, v[5].z);
        // -X (0,3,7,4)
        glVertex3f(v[0].x, v[0].y, v[0].z); glVertex3f(v[3].x, v[3].y, v[3].z);
        glVertex3f(v[7].x, v[7].y, v[7].z); glVertex3f(v[4].x, v[4].y, v[4].z);
        glEnd();

        glDisable(GL_BLEND);

        glPopMatrix();
    }
    else
    {
        Red::Failure("Unknown collision object type");
    }
}

inline void RenderWave(shared_ptr<Wave> wave, glm::vec3 cameraPos, 
    MeshGL& gWaveGL, const Cuboid_dimensions& Cube)
{
    static int frameCount = 0;
    if ((frameCount % 8) == 0)
    {
        size_t budget = std::min<size_t>(4000, std::max<size_t>(1, wave->triangles.size() / 10));
        budget = wave->triangles.size(); // TO DO: MOZNA ZMIENIC NA WIEKSZE (W SENSIE ZMIENIC np. 4 -> 2)
        int threads = std::max(1u, std::thread::hardware_concurrency());

        wave->refineIcosahedron_chunked_mt(0.05f, budget, Cube, threads);
    }
    frameCount++;

    // odbuduj bufory tylko gdy trzeba
    if (wave->mesh_dirty)
    {
        // buildSphereBuffers(/*dynamic=*/true);
        buildBuffersFor(wave->nodes, wave->triangles, gWaveGL, /*dynamic=*/true);
        wave->mesh_dirty = false;
    }
    else
    {
        // To nie powinno zawsze być wzywane?
        updatePositionsFor(wave->nodes, gWaveGL);
    }

    // kulka
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    drawMesh(gWaveGL);
}