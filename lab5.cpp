#include "lab_m1/Lab5/Lab5.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <queue>
#include <random>
#include <unordered_map>

using namespace m1;

namespace
{
    //  CONSTANTE + FUNCTII MICI
    // pi si constante de gameplay pentru bila
    constexpr float PI = 3.14159265358979323846f;
    constexpr float VITEZA_BILA = 295.0f;

    // unghiurile limita folosite cand bila ricoaseaza din paleta
    // nu vrem unghiuri prea "plate" (aproape orizontale), ca bila ar sta mult in joc
    constexpr float UNGHI_MIN = 0.4887f; // ~28 grade
    constexpr float UNGHI_MAX = 2.6529f; // ~152 grade

    // clamp simplu pentru valori float
    inline float Clamp(float v, float a, float b)
    {
        return std::max(a, std::min(b, v));
    }

    // alege un shader valid in ordine:
    // 1) shaderul a daca e setat
    // 2) shaderul b daca e setat
    // 3) cauta "VertexColor" in map
    // 4) cauta "Simple" in map
    // 5) fallback la primul shader din map, daca exista
    Shader* GasesteShader(const std::unordered_map<std::string, Shader*>& sh,
        Shader* a, Shader* b)
    {
        if (a != nullptr)
        {
            return a;
        }

        if (b != nullptr)
        {
            return b;
        }

        auto it = sh.find("VertexColor");
        if (it != sh.end())
        {
            return it->second;
        }

        it = sh.find("Simple");
        if (it != sh.end())
        {
            return it->second;
        }

        if (sh.empty())
        {
            return nullptr;
        }

        return sh.begin()->second;
    }

    // construieste o matrice 2d (mat3) pentru un quad unit (0..1) scalat si translatat
    // in framework, RenderMesh2D foloseste mat3 (transformari 2d)
    inline glm::mat3 Mat(float x, float y, float sx, float sy)
    {
        glm::mat3 M(1.f);

        // scale pe axe
        M[0][0] = sx;
        M[1][1] = sy;

        // translatie in ultima linie (conventia framework-ului)
        M[2][0] = x;
        M[2][1] = y;

        return M;
    }


    //  FABRICA MESH-URI (forme: cerc, inima)
    struct FabricaMesh
    {
        static Mesh* CreeazaCerc(const char* nume, float r, float g, float b, int seg = 48)
        {
            // cercul e facut ca un triangle fan:
            //   - un vertex in centru
            //   - seg+1 vertexi pe circumferinta (primul si ultimul coincid)
            //   - indicii formeaza triunghiuri (0, i, i+1)
            std::vector<VertexFormat> v;
            std::vector<unsigned int> idx;

            v.reserve(seg + 2);
            idx.reserve(seg * 3);

            // centru in (0.5, 0.5) ca sa fie usor de scalat pe un quad 0..1
            v.emplace_back(glm::vec3(0.5f, 0.5f, 0.f), glm::vec3(r, g, b));

            float pas = 2.f * PI / seg;
            for (int i = 0; i <= seg; ++i)
            {
                float a = i * pas;

                float px = 0.5f + 0.5f * std::cos(a);
                float py = 0.5f + 0.5f * std::sin(a);

                v.emplace_back(glm::vec3(px, py, 0.f), glm::vec3(r, g, b));
            }

            for (int i = 1; i <= seg; ++i)
            {
                idx.push_back(0);
                idx.push_back(i);
                idx.push_back(i + 1);
            }

            Mesh* m = new Mesh(nume);
            m->InitFromData(v, idx);
            m->SetDrawMode(GL_TRIANGLES);
            return m;
        }

        static Mesh* CreeazaInima(const char* nume, float r, float g, float b, int seg = 128)
        {
            // parametrizare clasica pentru o inima (curba in plan)
            // apoi normalizam punctele in [0..1] ca sa incapa intr-un quad
            auto f = [](float t) -> glm::vec2 {
                float x = 16.f * std::pow(std::sin(t), 3.f);
                float y = 13.f * std::cos(t) - 5.f * std::cos(2.f * t)
                    - 2.f * std::cos(3.f * t) - std::cos(4.f * t);
                return { x, y };
                };

            std::vector<glm::vec2> P;
            P.reserve(seg + 1);

            // esantionam curba
            for (int i = 0; i <= seg; ++i)
            {
                float t = 2.f * PI * (float)i / (float)seg;
                P.push_back(f(t));
            }

            // calculam bounding box pentru normalizare
            glm::vec2 mn(1e9f), mx(-1e9f);
            for (auto& p : P)
            {
                mn = glm::min(mn, p);
                mx = glm::max(mx, p);
            }

            glm::vec2 sz = mx - mn;

            // evitam impartirea la 0 in cazuri degenerate
            if (sz.x == 0.f) sz.x = 1.f;
            if (sz.y == 0.f) sz.y = 1.f;

            std::vector<VertexFormat> v;
            std::vector<unsigned int> idx;

            v.reserve(seg + 2);
            idx.reserve(seg * 3);

            // centru pentru fan
            v.emplace_back(glm::vec3(0.5f, 0.5f, 0.f), glm::vec3(r, g, b));

            // normalizam punctele curbei in [0..1]
            // si mai "ridicam" putin inima pe y cu 0.08..1.0 ca sa nu atinga marginea jos
            for (auto& p : P)
            {
                glm::vec2 q = (p - mn) / sz;
                q.y = 0.08f + 0.92f * q.y;
                v.emplace_back(glm::vec3(q.x, q.y, 0.f), glm::vec3(r, g, b));
            }

            for (int i = 1; i <= seg; ++i)
            {
                idx.push_back(0);
                idx.push_back(i);
                idx.push_back(i + 1);
            }

            Mesh* m = new Mesh(nume);
            m->InitFromData(v, idx);
            m->SetDrawMode(GL_TRIANGLES);
            return m;
        }
    };


    //  CACHE CULORI (quads colorate)
    struct CacheCulori
    {
        // tinem un map de la culoare impachetata (rgb 8-bit) la mesh-ul corespunzator
        // ca sa nu cream cate un mesh nou la fiecare desenare de rectangle colorate
        std::unordered_map<uint32_t, Mesh*> map;

        static uint32_t Impachetare(float r, float g, float b)
        {
            // convertim float [0..1] in int [0..255] si impachetam in 0xRRGGBB
            auto to8 = [](float x)->uint32_t
                {
                    float clamped = Clamp(x, 0.f, 1.f);
                    int v = (int)std::round(clamped * 255.f);
                    v = std::max(0, std::min(255, v));
                    return (uint32_t)v;
                };

            uint32_t R = to8(r);
            uint32_t G = to8(g);
            uint32_t B = to8(b);

            return (R << 16) | (G << 8) | B;
        }

        Mesh* IaSauCreeaza(Tema1* s, float r, float g, float b)
        {
            uint32_t key = Impachetare(r, g, b);

            auto it = map.find(key);
            if (it != map.end())
            {
                return it->second;
            }

            // daca nu exista, construim un quad nou de culoarea ceruta
            char nm[64];
            std::snprintf(nm, sizeof(nm), "quad_%u", key);

            Mesh* m = s->MakeColoredQuad(r, g, b, nm);
            map[key] = m;

            return m;
        }
    };

    //  MOTOR DE RANDARI (coada + executie)
    class MotorRandari2D
    {
    public:
        struct Comanda
        {
            Mesh* mesh = nullptr;
            glm::mat3 M{ 1.f };
            int strat = 0;   // strat = "z-order" logic
            float lw = 1.f;  // line width (doar pentru mesh-uri cu GL_LINES)
        };

        void Reset()
        {
            comenzi.clear();
        }

        void Rezerva(size_t n)
        {
            comenzi.reserve(n);
        }

        void Pune(Mesh* m, const glm::mat3& M, int strat, float lw = 1.f)
        {
            if (m == nullptr)
            {
                return;
            }

            Comanda c;
            c.mesh = m;
            c.M = M;
            c.strat = strat;
            c.lw = lw;
            comenzi.push_back(c);
        }

        void Ruleaza(Tema1* scena, Shader* shader)
        {
            if (scena == nullptr)
            {
                return;
            }

            if (shader == nullptr)
            {
                return;
            }

            // sortam dupa strat ca sa desenam de jos in sus (fundal -> hud -> drag)
            std::stable_sort(
                comenzi.begin(),
                comenzi.end(),
                [](const Comanda& a, const Comanda& b)
                {
                    return a.strat < b.strat;
                }
            );

            float lwCurent = 1.f;
            glLineWidth(lwCurent);

            for (auto& c : comenzi)
            {
                // schimbam line width doar daca e nevoie (micro-optimizare)
                if (std::abs(c.lw - lwCurent) > 1e-4f)
                {
                    lwCurent = c.lw;
                    glLineWidth(lwCurent);
                }

                scena->RenderMesh2D(c.mesh, shader, c.M);
            }

            // reset ca sa nu afecteze alte draw-uri din framework
            glLineWidth(1.f);
        }

    private:
        std::vector<Comanda> comenzi;
    };


    //  VALIDATOR NAVA (conectivitate + reguli)
    class VerificatorNava
    {
    public:
        VerificatorNava(const std::vector<std::vector<Tema1::PieceType>>& g,
            int rows, int cols, int count, int maxCount)
            : G(g), R(rows), C(cols), nr(count), maxNr(maxCount)
        {
        }

        bool OK() const
        {
            // nava trebuie sa aiba cel putin 1 piesa si maxim MAX_PIECES
            if (nr <= 0)
            {
                return false;
            }

            if (nr > maxNr)
            {
                return false;
            }

            // trebuie sa fie conectata 4-neigh (sus/jos/stanga/dreapta)
            if (!Conectat())
            {
                return false;
            }

            // si sa respecte regulile locale (shield, weapon, propulsion)
            return ReguliLocale();
        }

        bool Conectat() const
        {
            if (nr == 0)
            {
                return false;
            }

            // gasim un start: prima celula nenula
            Tema1::GridPos start(-1, -1);
            for (int r = 0; r < R && start.r == -1; ++r)
            {
                for (int c = 0; c < C; ++c)
                {
                    if (G[r][c] != Tema1::NONE)
                    {
                        start = { r, c };
                        break;
                    }
                }
            }

            if (start.r == -1)
            {
                return false;
            }

            // bfs pe grid, doar pe celulele ocupate
            std::vector<std::vector<uint8_t>> viz(R, std::vector<uint8_t>(C, 0));
            std::queue<Tema1::GridPos> q;

            q.push(start);
            viz[start.r][start.c] = 1;

            static const int dr[4] = { -1, 1, 0, 0 };
            static const int dc[4] = { 0, 0,-1, 1 };

            int cnt = 0;
            while (!q.empty())
            {
                auto cur = q.front();
                q.pop();

                ++cnt;

                for (int k = 0; k < 4; ++k)
                {
                    int rr = cur.r + dr[k];
                    int cc = cur.c + dc[k];

                    if (rr < 0 || rr >= R || cc < 0 || cc >= C)
                    {
                        continue;
                    }

                    if (viz[rr][cc] != 0)
                    {
                        continue;
                    }

                    if (G[rr][cc] == Tema1::NONE)
                    {
                        continue;
                    }

                    viz[rr][cc] = 1;
                    q.push({ rr, cc });
                }
            }

            // conectat daca am vizitat exact nr piese
            return cnt == nr;
        }

    private:
        bool ReguliLocale() const
        {
            static const int dr4[4] = { -1, 1, 0, 0 };
            static const int dc4[4] = { 0, 0,-1, 1 };

            for (int r = 0; r < R; ++r)
            {
                for (int c = 0; c < C; ++c)
                {
                    Tema1::PieceType t = G[r][c];
                    if (t == Tema1::NONE)
                    {
                        continue;
                    }

                    // regula weapon: "deasupra" trebuie liber (weapon iese in sus)
                    if (t == Tema1::WEAPON)
                    {
                        if (r > 0)
                        {
                            if (G[r - 1][c] != Tema1::NONE)
                            {
                                return false;
                            }
                        }
                    }

                    // regula propulsion: sub trebuie liber (flacara iese in jos)
                    if (t == Tema1::PROPULSION)
                    {
                        if (r < R - 1)
                        {
                            if (G[r + 1][c] != Tema1::NONE)
                            {
                                return false;
                            }
                        }
                    }

                    // regula shield: are mai multe restrictii
                    if (t == Tema1::SHIELD)
                    {
                        // rand deasupra (3 celule) trebuie liber ca sa se poata desena arcul scutului
                        if (r > 0)
                        {
                            for (int dcc = -1; dcc <= 1; ++dcc)
                            {
                                int cc = c + dcc;
                                if (cc < 0 || cc >= C)
                                {
                                    continue;
                                }

                                if (G[r - 1][cc] != Tema1::NONE)
                                {
                                    return false;
                                }
                            }
                        }

                        // fara arma stanga/dreapta, ca s-ar suprapune vizual si logic
                        if (c > 0)
                        {
                            if (G[r][c - 1] == Tema1::WEAPON)
                            {
                                return false;
                            }
                        }

                        if (c < C - 1)
                        {
                            if (G[r][c + 1] == Tema1::WEAPON)
                            {
                                return false;
                            }
                        }

                        // fara shield vecin pe 4-neigh (evitam lipirea scuturilor)
                        for (int k = 0; k < 4; ++k)
                        {
                            int rr = r + dr4[k];
                            int cc = c + dc4[k];

                            if (rr < 0 || rr >= R || cc < 0 || cc >= C)
                            {
                                continue;
                            }

                            if (G[rr][cc] == Tema1::SHIELD)
                            {
                                return false;
                            }
                        }
                    }
                }
            }

            return true;
        }

        const std::vector<std::vector<Tema1::PieceType>>& G;
        int R, C, nr, maxNr;
    };


    //  UI EDITOR (desen modular)
    class UIEditor
    {
    public:
        // stratificare pentru desen:
        // ideea e sa tinem "z-order" explicit, ca sa nu depindem de ordinea apelurilor
        enum Strat
        {
            FUNDAL = 0,
            CADRE = 10,
            PANOU = 20,
            GRILA = 30,
            PIESE = 40,
            EXT = 50,
            HUD = 60,
            DRAG = 70
        };

        static void Recalculeaza(Tema1* s)
        {
            // calculeaza pozitia si dimensiunea grilei astfel incat:
            // - sa fie la dreapta sidebar-ului
            // - sa incapa pe ecran
            // - celulele sa fie patrate si cu o dimensiune minima (>= 30)
            float startX = s->sbX + s->sbW + 50.f;

            // y pentru bara de piese e "sus" (in coordonate gl 2d: y creste in sus)
            s->bY = (float)s->screenH - 120.f;

            float rama = 35.f;
            float spatiuSub = 110.f;
            float margR = 26.f;
            float margB = 26.f;

            float w = (float)s->screenW - startX - margR - 2.f * rama;
            float h = s->bY - spatiuSub - margB - 2.f * rama;

            float cw = std::floor(w / Tema1::COLS);
            float ch = std::floor(h / Tema1::ROWS);

            // dimensiunea efectiva a celulei = min dintre cele doua, cu clamp la 30
            float candidate = std::min(cw, ch);
            s->gCell = std::max(30.f, candidate);

            // center alignment in zona disponibila
            s->gX = startX + rama + std::floor((w - Tema1::COLS * s->gCell) * 0.5f);
            s->gY = margB + rama + std::floor((h - Tema1::ROWS * s->gCell) * 0.5f);
        }

        static void Deseneaza(Tema1* s, MotorRandari2D& R)
        {
            DeseneazaCadru(s, R);
            DeseneazaPanou(s, R);
            DeseneazaGrila(s, R);
            DeseneazaNava(s, R);
            DeseneazaBaraPieseSiPlay(s, R);
            DeseneazaGhost(s, R);
        }

    private:
        static void DeseneazaCadru(Tema1* s, MotorRandari2D& R)
        {
            // desenam doua rame (una mai groasa si una inset) ca efect vizual
            float pad = 35.f;

            float fx = s->gX - pad;
            float fy = s->gY - pad;
            float fw = Tema1::COLS * s->gCell + 2.f * pad;
            float fh = Tema1::ROWS * s->gCell + 2.f * pad;

            R.Pune(s->mBorder1, Mat(fx, fy, fw, fh), CADRE);
            R.Pune(s->mBorder2, Mat(fx + 10.f, fy + 10.f, fw - 20.f, fh - 20.f), CADRE + 1);
        }

        static void DeseneazaPanou(Tema1* s, MotorRandari2D& R)
        {
            // panoul din stanga (sidebar) + contururi si iconite pentru cele 4 piese
            const float t = 2.f;
            R.Pune(s->mSidePanel, Mat(s->sbX, s->sbY, s->sbW, s->sbH), PANOU);

            // contur panou (linii desenate cu quad-uri subtiri)
            R.Pune(s->mErrQuad, Mat(s->sbX, s->sbY + s->sbH - t, s->sbW, t), PANOU + 1);
            R.Pune(s->mErrQuad, Mat(s->sbX, s->sbY, s->sbW, t), PANOU + 1);
            R.Pune(s->mErrQuad, Mat(s->sbX, s->sbY, t, s->sbH), PANOU + 1);
            R.Pune(s->mErrQuad, Mat(s->sbX + s->sbW - t, s->sbY, t, s->sbH), PANOU + 1);

            // 4 sloturi, fiecare cu border si iconita
            for (int i = 0; i < 4; ++i)
            {
                float x, y, w, h;
                s->GetSlotRect(i, x, y, w, h);

                R.Pune(s->mErrQuad, Mat(x, y + h - t, w, t), PANOU + 2);
                R.Pune(s->mErrQuad, Mat(x, y, w, t), PANOU + 2);
                R.Pune(s->mErrQuad, Mat(x, y, t, h), PANOU + 2);
                R.Pune(s->mErrQuad, Mat(x + w - t, y, t, h), PANOU + 2);

                float dim = std::min(w, h) * 0.38f;
                float ix = x + (w - dim) * 0.5f;
                float iy = y + (h - dim) * 0.5f;

                // cast explicit pentru index -> PieceType
                Iconita(s, R, (Tema1::PieceType)i, ix, iy, dim);
            }
        }

        static void DeseneazaGrila(Tema1* s, MotorRandari2D& R)
        {
            // fiecare celula are:
            // - un fill (albastru)
            // - un outline (linii)
            float inset = (float)Tema1::GAP;

            for (int r = 0; r < Tema1::ROWS; ++r)
            {
                for (int c = 0; c < Tema1::COLS; ++c)
                {
                    float x = s->gX + c * s->gCell;
                    float y = s->gY + r * s->gCell;
                    float tile = s->gCell - 2.f * inset;

                    R.Pune(s->mCellFill, Mat(x + inset, y + inset, tile, tile), GRILA);
                    R.Pune(s->mCellLine, Mat(x + inset, y + inset, tile, tile), GRILA + 1);
                }
            }
        }

        static void DeseneazaNava(Tema1* s, MotorRandari2D& R)
        {
            float inset = (float)Tema1::GAP;

            // baza pieselor: un tile metalic + outline, plus simboluri mici in functie de tip
            for (int r = 0; r < Tema1::ROWS; ++r)
            {
                for (int c = 0; c < Tema1::COLS; ++c)
                {
                    Tema1::PieceType tip = s->grid[r][c];
                    if (tip == Tema1::NONE)
                    {
                        continue;
                    }

                    float x = s->gX + c * s->gCell;
                    float y = s->gY + r * s->gCell;
                    float tile = s->gCell - 2.f * inset;

                    R.Pune(s->mMetalQuad, Mat(x + inset, y + inset, tile, tile), PIESE);
                    R.Pune(s->mCellLine, Mat(x + inset, y + inset, tile, tile), PIESE + 1);

                    if (tip == Tema1::CORE)
                    {
                        // un patrat interior ca sa iasa in evidenta core-ul
                        float inner = tile - 0.26f * s->gCell;
                        float ix = x + inset + 0.13f * s->gCell;
                        float iy = y + inset + 0.13f * s->gCell;

                        R.Pune(s->mMetalQuad, Mat(ix, iy, inner, inner), PIESE + 2);
                        R.Pune(s->mCellLine, Mat(ix, iy, inner, inner), PIESE + 3);
                    }
                    else if (tip == Tema1::WEAPON)
                    {
                        // dreptunghi mic sus, ca "barrel"
                        float bw = tile * 0.55f;
                        float bh = tile * 0.30f;

                        float bx = x + inset + (tile - bw) * 0.5f;
                        float by = y + inset + 4.f;

                        R.Pune(s->mMetalQuad, Mat(bx, by, bw, bh), PIESE + 2);
                        R.Pune(s->mCellLine, Mat(bx, by, bw, bh), PIESE + 3);
                    }
                    else if (tip == Tema1::PROPULSION)
                    {
                        // doua patratele interne, sugerand motoare
                        R.Pune(
                            s->mMetalQuad,
                            Mat(
                                x + inset + 0.13f * s->gCell,
                                y + inset + 0.13f * s->gCell,
                                0.18f * s->gCell,
                                0.24f * s->gCell
                            ),
                            PIESE + 2
                        );

                        R.Pune(
                            s->mMetalQuad,
                            Mat(
                                x + inset + tile - 0.31f * s->gCell,
                                y + inset + 0.13f * s->gCell,
                                0.18f * s->gCell,
                                0.24f * s->gCell
                            ),
                            PIESE + 2
                        );
                    }
                }
            }

            // extensii: weapon top, flame, shield arc
            for (int r = 0; r < Tema1::ROWS; ++r)
            {
                for (int c = 0; c < Tema1::COLS; ++c)
                {
                    Tema1::PieceType tip = s->grid[r][c];
                    if (tip == Tema1::NONE)
                    {
                        continue;
                    }

                    float x = s->gX + c * s->gCell;
                    float y = s->gY + r * s->gCell;

                    if (tip == Tema1::WEAPON)
                    {
                        float inset2 = (float)Tema1::GAP;
                        float tile = s->gCell - 2.f * inset2;

                        float bw = tile * 0.55f;
                        float bh = tile * 0.30f;

                        float bx = x + inset2 + (tile - bw) * 0.5f;
                        float by = y + inset2 + 4.f;

                        // semicercul de sus (muzzle) e centrat, iar mat-ul foloseste x ca centru
                        if (s->mWeaponTop != nullptr)
                        {
                            R.Pune(
                                s->mWeaponTop,
                                Mat(bx + bw * 0.5f, by + bh, bw * 0.5f, bh),
                                EXT
                            );
                        }

                        // tija verticala 2 celule in sus (la desen)
                        float pw = tile * 0.22f;
                        float ph = s->gCell * 2.f;

                        float px = x + inset2 + (tile - pw) * 0.5f;
                        float py = y + s->gCell;

                        R.Pune(s->mMetalQuad, Mat(px, py, pw, ph), EXT + 1);
                        R.Pune(s->mCellLine, Mat(px, py, pw, ph), EXT + 2);
                    }
                    else if (tip == Tema1::PROPULSION)
                    {
                        // 3 flacari diferite ca dimensiune (efect vizual)
                        R.Pune(
                            s->mFlame,
                            Mat(
                                x + s->gCell * 0.5f - 0.42f * s->gCell,
                                y + s->gCell,
                                0.30f * s->gCell,
                                0.42f * s->gCell
                            ),
                            EXT
                        );

                        R.Pune(
                            s->mFlame,
                            Mat(
                                x + s->gCell * 0.5f - 0.10f * s->gCell,
                                y + s->gCell,
                                0.30f * s->gCell,
                                0.58f * s->gCell
                            ),
                            EXT
                        );

                        R.Pune(
                            s->mFlame,
                            Mat(
                                x + s->gCell * 0.5f + 0.22f * s->gCell,
                                y + s->gCell,
                                0.30f * s->gCell,
                                0.42f * s->gCell
                            ),
                            EXT
                        );
                    }
                    else if (tip == Tema1::SHIELD)
                    {
                        // semicercul scutului e desenat deasupra piesei, lat de 3 celule
                        // aici folosim Mat cu "x = centru", de aceea trimitem cx
                        if (s->mShieldTop != nullptr)
                        {
                            float aw = s->gCell * 3.f;
                            float ah = s->gCell * 1.f;
                            float cx = x + s->gCell * 0.5f;

                            R.Pune(
                                s->mShieldTop,
                                Mat(cx, y + s->gCell, aw * 0.5f, ah),
                                EXT
                            );
                        }
                    }
                }
            }
        }

        static void DeseneazaBaraPieseSiPlay(Tema1* s, MotorRandari2D& R)
        {
            const int N = Tema1::MAX_PIECES;
            const float slot = 28.f;
            const float gapMin = 10.f;
            const float padR = 84.f;

            // bara de sus: incepe dupa sidebar si are N sloturi
            s->bStart = s->sbX + s->sbW + 30.f;
            s->bY = (float)s->screenH - 120.f;

            float avail = (float)s->screenW - padR - s->bStart;
            s->bGap = (avail - N * slot) / (N - 1);

            if (s->bGap < gapMin)
            {
                s->bGap = gapMin;
            }

            int ramase = Tema1::MAX_PIECES - s->pieceCount;

            for (int i = 0; i < N; ++i)
            {
                float x = s->bStart + i * (slot + s->bGap);

                // inainte aveai operatorul ternar:
                //   (i < ramase) ? s->mOkQuad : s->mCellLine
                // l-am inlocuit cu if explicit, cum ai cerut
                Mesh* m = nullptr;
                if (i < ramase)
                {
                    m = s->mOkQuad;
                }
                else
                {
                    m = s->mCellLine;
                }

                R.Pune(m, Mat(x, s->bY, slot, slot), HUD);
            }

            // pozitia butonului play: dupa ultimul slot
            float last = s->bStart + (N - 1) * (slot + s->bGap);
            float bx = last + slot + 16.f;

            s->playX = bx;
            s->playY = s->bY;
            s->playS = slot;

            // verificam daca nava e valida, ca sa coloram butonul (verde/rosu)
            VerificatorNava val(s->grid, Tema1::ROWS, Tema1::COLS, s->pieceCount, Tema1::MAX_PIECES);
            bool ok = val.OK();

            Mesh* btn = nullptr;
            if (ok)
            {
                btn = s->mOkQuad;
            }
            else
            {
                btn = s->mErrQuad;
            }

            R.Pune(btn, Mat(bx, s->bY, slot, slot), HUD + 3);

            float inset = 6.f;

            if (ok)
            {
                // triunghiul de play e un mesh din lista generala meshes
                // accesam direct meshes["play_tri"]
                Mesh* tri = s->meshes["play_tri"];
                if (tri != nullptr)
                {
                    R.Pune(
                        tri,
                        Mat(
                            bx + inset,
                            s->bY + inset,
                            slot - 2 * inset,
                            slot - 2 * inset
                        ),
                        HUD + 4
                    );
                }
            }
            else
            {
                // daca nu e valid, desenam un x (mesh cu linii)
                if (s->mXMark != nullptr)
                {
                    R.Pune(
                        s->mXMark,
                        Mat(
                            bx + inset,
                            s->bY + inset,
                            slot - 2 * inset,
                            slot - 2 * inset
                        ),
                        HUD + 4,
                        3.5f
                    );
                }
            }

            // contur buton play
            R.Pune(s->mCellLine, Mat(bx, s->bY, slot, slot), HUD + 5);
        }

        static void DeseneazaGhost(Tema1* s, MotorRandari2D& R)
        {
            // ghost-ul apare doar cand tragi o piesa din sidebar
            if (s->dragging == false)
            {
                return;
            }

            if (s->heldPiece == Tema1::NONE)
            {
                return;
            }

            auto mp = s->window->GetCursorPosition();

            float mx = (float)mp.x;
            float my = (float)s->screenH - mp.y;

            // pozitionam un tile sub cursor (centrat)
            float x = mx - s->gCell * 0.5f;
            float y = my - s->gCell * 0.5f;

            float inset = (float)Tema1::GAP;
            float tile = s->gCell - 2.f * inset;

            R.Pune(s->mMetalQuad, Mat(x + inset, y + inset, tile, tile), DRAG);
            R.Pune(s->mCellLine, Mat(x + inset, y + inset, tile, tile), DRAG + 1);

            // panel mic in interior ca highlight
            R.Pune(
                s->mSidePanel,
                Mat(
                    x + inset + 0.18f * s->gCell,
                    y + inset + 0.18f * s->gCell,
                    tile - 0.36f * s->gCell,
                    tile - 0.36f * s->gCell
                ),
                DRAG + 2
            );
        }

        static void Iconita(Tema1* s, MotorRandari2D& R, Tema1::PieceType tip, float x, float y, float dim)
        {
            // iconita = un patrat metalic + contur + "desen" diferit pe tip
            R.Pune(s->mMetalQuad, Mat(x, y, dim, dim), PANOU + 10);
            R.Pune(s->mCellLine, Mat(x, y, dim, dim), PANOU + 11);

            float inset = dim * 0.12f;
            float mid = x + dim * 0.5f;

            if (tip == Tema1::CORE)
            {
                float in = dim - 2.f * inset;
                R.Pune(s->mMetalQuad, Mat(x + inset, y + inset, in, in), PANOU + 12);
                R.Pune(s->mCellLine, Mat(x + inset, y + inset, in, in), PANOU + 13);
            }
            else if (tip == Tema1::WEAPON)
            {
                float bw = dim * 0.55f;
                float bh = dim * 0.30f;

                float bx = x + (dim - bw) * 0.5f;
                float by = y + inset * 0.5f;

                R.Pune(s->mMetalQuad, Mat(bx, by, bw, bh), PANOU + 12);
                R.Pune(s->mCellLine, Mat(bx, by, bw, bh), PANOU + 13);

                if (s->mWeaponTop != nullptr)
                {
                    R.Pune(s->mWeaponTop, Mat(bx + bw * 0.5f, by + bh, bw * 0.5f, bh), PANOU + 14);
                }

                float pw = dim * 0.20f;
                float ph = dim * 0.50f;

                R.Pune(s->mMetalQuad, Mat(mid - pw * 0.5f, by + bh, pw, ph), PANOU + 12);
                R.Pune(s->mCellLine, Mat(mid - pw * 0.5f, by + bh, pw, ph), PANOU + 13);
            }
            else if (tip == Tema1::PROPULSION)
            {
                // flacarile din iconita (optional)
                if (s->mFlame != nullptr)
                {
                    R.Pune(s->mFlame, Mat(x + dim * 0.18f, y + dim, dim * 0.22f, dim * 0.42f), PANOU + 14);
                    R.Pune(s->mFlame, Mat(x + dim * 0.39f, y + dim, dim * 0.22f, dim * 0.42f), PANOU + 14);
                    R.Pune(s->mFlame, Mat(x + dim * 0.60f, y + dim, dim * 0.22f, dim * 0.42f), PANOU + 14);
                }

                // doua patratele ca motoare
                R.Pune(
                    s->mMetalQuad,
                    Mat(x + inset + 0.08f * dim, y + inset + 0.10f * dim, dim * 0.18f, dim * 0.24f),
                    PANOU + 12
                );

                R.Pune(
                    s->mMetalQuad,
                    Mat(x + dim - inset - 0.08f * dim - dim * 0.18f, y + inset + 0.10f * dim, dim * 0.18f, dim * 0.24f),
                    PANOU + 12
                );
            }
            else if (tip == Tema1::SHIELD)
            {
                if (s->mShieldTop != nullptr)
                {
                    R.Pune(s->mShieldTop, Mat(mid, y + dim, dim * 0.75f, dim * 0.55f), PANOU + 14);
                }

                float p = dim * 0.32f;
                R.Pune(s->mMetalQuad, Mat(mid - p * 0.5f, y + inset, p, p), PANOU + 12);
                R.Pune(s->mCellLine, Mat(mid - p * 0.5f, y + inset, p, p), PANOU + 13);
            }
        }
    };

} // namespace anon


Tema1::Tema1()
{
    // initializam grid-ul cu NONE
    grid.assign(ROWS, std::vector<PieceType>(COLS, NONE));

    // paleta de culori initiala
    bgR = bgG = bgB = 0.0f;
    cellR = 0.08f; cellG = 0.22f; cellB = 0.80f;
    lineR = 0.03f; lineG = 0.04f; lineB = 0.16f;
    borderR1 = 0.02f; borderG1 = 0.02f; borderB1 = 0.05f;
    borderR2 = 0.05f; borderG2 = 0.07f; borderB2 = 0.18f;
    sideR = sideG = sideB = 0.12f;
    okR = 0.0f; okG = 0.9f; okB = 0.0f;
    fireR = 1.0f; fireG = 0.55f; fireB = 0.0f;
    metalR = metalG = metalB = 0.62f;
    armorR = 0.9f; armorG = 0.88f; armorB = 0.72f;
    errR = 0.85f; errG = 0.1f; errB = 0.1f;

    // pornim in modul editor
    currentMode = EDIT;
}

void Tema1::Init()
{
    // luam rezolutia ferestrei ca sa setam camera ortografica
    auto res = window->GetResolution();
    screenW = res.x;
    screenH = res.y;

    auto cam = GetSceneCamera();
    cam->SetOrthographic(0.f, (float)screenW, 0.f, (float)screenH, 0.01f, 100.f);
    cam->SetPosition(glm::vec3(0, 0, 50));
    cam->SetRotation(glm::vec3(0));
    cam->Update();

    // dezactivam input de camera, ca vrem input custom (mouse/tastatura)
    GetCameraInput()->SetActive(false);

    // joc 2d, fara depth test
    glDisable(GL_DEPTH_TEST);
    glClearColor(bgR, bgG, bgB, 1.f);

    // alegem shaderul principal
    shCol = nullptr;
    shFall = nullptr;

    if (shaders.count("VertexColor") != 0)
    {
        shCol = shaders["VertexColor"];
    }

    if (shCol == nullptr)
    {
        if (shaders.count("Simple") != 0)
        {
            shFall = shaders["Simple"];
        }
    }

    if (shCol == nullptr && shFall == nullptr)
    {
        if (!shaders.empty())
        {
            shCol = shaders.begin()->second;
        }
    }

    // helper: creeaza un dreptunghi unit (0..1) plin sau doar contur
    auto Drept = [&](const char* nume, float r, float g, float b, bool plin) -> Mesh*
        {
            std::vector<VertexFormat> v =
            {
                VertexFormat(glm::vec3(0,0,0), glm::vec3(r,g,b)),
                VertexFormat(glm::vec3(1,0,0), glm::vec3(r,g,b)),
                VertexFormat(glm::vec3(1,1,0), glm::vec3(r,g,b)),
                VertexFormat(glm::vec3(0,1,0), glm::vec3(r,g,b))
            };

            std::vector<unsigned int> idxF = { 0,1,2,0,2,3 };
            std::vector<unsigned int> idxL = { 0,1,1,2,2,3,3,0 };

            Mesh* m = new Mesh(nume);

            if (plin)
            {
                m->InitFromData(v, idxF);
                m->SetDrawMode(GL_TRIANGLES);
            }
            else
            {
                m->InitFromData(v, idxL);
                m->SetDrawMode(GL_LINES);
            }

            AddMeshToList(m);
            return m;
        };

    // helper: triunghi unit
    auto Tri = [&](const char* nume, float r, float g, float b) -> Mesh*
        {
            std::vector<VertexFormat> v =
            {
                VertexFormat(glm::vec3(0,0,0), glm::vec3(r,g,b)),
                VertexFormat(glm::vec3(1,0,0), glm::vec3(r,g,b)),
                VertexFormat(glm::vec3(0.5f,1,0), glm::vec3(r,g,b))
            };

            std::vector<unsigned int> idx = { 0,1,2 };

            Mesh* m = new Mesh(nume);
            m->InitFromData(v, idx);
            m->SetDrawMode(GL_TRIANGLES);

            AddMeshToList(m);
            return m;
        };

    // helper: semicerc (folosit la shield/weapon top)
    auto Semi = [&](const char* nume, float r, float g, float b) -> Mesh*
        {
            const int N = 48;

            std::vector<VertexFormat> v;
            std::vector<unsigned int> idx;

            // centrul fan-ului
            v.emplace_back(glm::vec3(0, 0, 0), glm::vec3(r, g, b));

            // generam puncte pe semicerc (0..pi)
            for (int i = 0; i <= N; ++i)
            {
                float a = PI * (float)i / (float)N;
                v.emplace_back(glm::vec3(std::cos(a), std::sin(a), 0.f), glm::vec3(r, g, b));
            }

            for (int i = 1; i <= N; ++i)
            {
                idx.push_back(0);
                idx.push_back(i);
                idx.push_back(i + 1);
            }

            Mesh* m = new Mesh(nume);
            m->InitFromData(v, idx);
            m->SetDrawMode(GL_TRIANGLES);

            AddMeshToList(m);
            return m;
        };

    // mesh-uri pentru ui
    mCellFill = Drept("celula_plin", cellR, cellG, cellB, true);
    mCellLine = Drept("celula_linie", lineR, lineG, lineB, false);
    mSidePanel = Drept("panou", sideR, sideG, sideB, true);
    mOkQuad = Drept("ok", okR, okG, okB, true);
    mMetalQuad = Drept("metal", metalR, metalG, metalB, true);
    mBorder1 = Drept("rama1", borderR1, borderG1, borderB1, true);
    mBorder2 = Drept("rama2", borderR2, borderG2, borderB2, true);
    mErrQuad = Drept("eroare", errR, errG, errB, true);

    mFlame = Tri("flacara", fireR, fireG, fireB);
    mShieldTop = Semi("scut", armorR, armorG, armorB);
    mWeaponTop = Semi("arma", metalR, metalG, metalB);


    //  MESH-URI HUD (play, x, bila, viata)

    // play triangle (nu folosim helper-ul Tri ca vrem orientare specifica)
    {
        std::vector<VertexFormat> v =
        {
            VertexFormat(glm::vec3(1,0,0),   glm::vec3(okR,okG,okB)),
            VertexFormat(glm::vec3(1,1,0),   glm::vec3(okR,okG,okB)),
            VertexFormat(glm::vec3(0,0.5f,0),glm::vec3(okR,okG,okB))
        };

        std::vector<unsigned int> idx = { 0,1,2 };

        Mesh* m = new Mesh("play_tri");
        m->InitFromData(v, idx);
        m->SetDrawMode(GL_TRIANGLES);
        AddMeshToList(m);
    }

    // x mark (linii) pentru buton invalid
    {
        float hs = 0.4f;

        std::vector<VertexFormat> v;
        std::vector<unsigned int> idx = { 0,1,2,3 };

        v.emplace_back(glm::vec3(0.5f - hs, 0.5f + hs, 0), glm::vec3(1, 1, 1));
        v.emplace_back(glm::vec3(0.5f + hs, 0.5f - hs, 0), glm::vec3(1, 1, 1));
        v.emplace_back(glm::vec3(0.5f - hs, 0.5f - hs, 0), glm::vec3(1, 1, 1));
        v.emplace_back(glm::vec3(0.5f + hs, 0.5f + hs, 0), glm::vec3(1, 1, 1));

        mXMark = new Mesh("x_mark");
        mXMark->InitFromData(v, idx);
        mXMark->SetDrawMode(GL_LINES);
        AddMeshToList(mXMark);
    }

    // initializam jocul cu dimensiunile ferestrei
    game.Setup((float)screenW, (float)screenH);

    // mesh-uri pentru bila si vieti
    ballMesh = FabricaMesh::CreeazaCerc("bila", 0.20f, 0.60f, 1.00f);
    lifeMesh = FabricaMesh::CreeazaInima("viata", 0.95f, 0.25f, 0.25f);

    AddMeshToList(ballMesh);
    AddMeshToList(lifeMesh);
}

void Tema1::FrameStart()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, screenW, screenH);
}

void Tema1::Update(float dt)
{
    // avem doua moduri:
    // - edit: desenam editorul si asteptam input
    // - play: rulam breakout
    if (currentMode == EDIT)
    {
        UIEditor::Recalculeaza(this);

        MotorRandari2D motor;
        motor.Rezerva(4096);

        UIEditor::Deseneaza(this, motor);

        Shader* sh = GasesteShader(shaders, shCol, shFall);
        motor.Ruleaza(this, sh);
    }
    else if (currentMode == PLAY)
    {
        RunGame(dt);
    }
}

void Tema1::FrameEnd() {}

void Tema1::OnInputUpdate(float dt, int)
{
    if (currentMode != PLAY)
    {
        return;
    }

    float dir = 0.f;

    if (window->KeyHold(GLFW_KEY_LEFT))
    {
        dir -= 1.f;
    }

    if (window->KeyHold(GLFW_KEY_RIGHT))
    {
        dir += 1.f;
    }

    game.paddle.x += dir * game.paddle.speed * dt;

    // daca bila e lipita de paleta, o mutam odata cu paleta
    if (game.ball.stuck)
    {
        game.StickBall();
    }
}

void Tema1::OnKeyPress(int key, int)
{
    if (currentMode == EDIT)
    {
        if (key == GLFW_KEY_R)
        {
            // reset editor: stergem grid-ul si contorul de piese
            for (int r = 0; r < ROWS; ++r)
            {
                for (int c = 0; c < COLS; ++c)
                {
                    grid[r][c] = NONE;
                }
            }

            pieceCount = 0;
            validShape = false;
        }

        return;
    }

    // in play, esc revine in editor
    if (key == GLFW_KEY_ESCAPE)
    {
        currentMode = EDIT;
        return;
    }

    // space lanseaza bila daca e lipita
    if (key == GLFW_KEY_SPACE)
    {
        if (game.ball.stuck)
        {
            // lansare initiala pe diagonala
            float dx = 1.f;
            float dy = 1.f;

            float len = std::sqrt(dx * dx + dy * dy);
            if (len == 0.f)
            {
                len = 1.f;
            }

            game.ball.vx = (dx / len) * VITEZA_BILA;
            game.ball.vy = (dy / len) * VITEZA_BILA;
            game.ball.stuck = false;
        }
    }

    // pauza
    if (key == GLFW_KEY_P)
    {
        game.stopped = !game.stopped;
    }
}

void Tema1::OnKeyRelease(int, int) {}
void Tema1::OnMouseMove(int, int, int, int) {}
void Tema1::OnMouseScroll(int, int, int, int) {}

void Tema1::OnMouseBtnPress(int mx, int my, int button, int)
{
    if (currentMode != EDIT)
    {
        return;
    }

    // conversie la coordonate gl: in window y creste in jos, in scena y creste in sus
    float glY = (float)screenH - my;

    if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        // pick piesa din panou
        for (int slot = 0; slot < 4; ++slot)
        {
            float sx, sy, sw, sh;
            GetSlotRect(slot, sx, sy, sw, sh);

            bool inside = (mx >= sx && mx <= sx + sw && glY >= sy && glY <= sy + sh);
            if (inside)
            {
                heldPiece = (PieceType)slot;
                dragging = true;
                return;
            }
        }

        // apasa play daca exista butonul (playX setat)
        if (playX >= 0.f)
        {
            bool insidePlay = (mx >= playX && mx <= playX + playS && glY >= playY && glY <= playY + playS);
            if (insidePlay)
            {
                SwitchToPlay();
                return;
            }
        }
    }
    else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
    {
        // delete piesa din grid (middle click)
        GridPos p = WindowToGrid(mx, my);

        if (p.r >= 0 && p.r < ROWS && p.c >= 0 && p.c < COLS)
        {
            DelPiece(p.r, p.c);
        }
    }
}

void Tema1::OnMouseBtnRelease(int mx, int my, int button, int)
{
    if (currentMode != EDIT)
    {
        return;
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        if (dragging)
        {
            // drop piesa in grid (daca e in interior)
            GridPos p = WindowToGrid(mx, my);

            if (p.r != -1 && p.c != -1)
            {
                if (heldPiece != NONE)
                {
                    AddPiece(p.r, p.c, heldPiece);
                }
            }

            dragging = false;
            heldPiece = NONE;
        }
    }
}

void Tema1::OnWindowResize(int w, int h)
{
    screenW = w;
    screenH = h;

    auto cam = GetSceneCamera();
    cam->SetOrthographic(0.f, (float)w, 0.f, (float)h, 0.01f, 100.f);
    cam->Update();

    // refacem si setup-ul jocului ca sa se adapteze la noua rezolutie
    game.Setup((float)w, (float)h);
}


void Tema1::BuildMatrix(float px, float py, float sx, float sy, float* out)
{
    // matrice 3x3 pentru 2d:
    // [ sx  0  0 ]
    // [  0 sy  0 ]
    // [ px py  1 ]
    for (int i = 0; i < 9; ++i)
    {
        out[i] = 0.f;
    }

    out[0] = sx;
    out[4] = sy;
    out[6] = px;
    out[7] = py;
    out[8] = 1.f;
}

void Tema1::RenderRect(Mesh* m, float x, float y, float w, float h)
{
    if (m == nullptr)
    {
        return;
    }

    Shader* sh = GasesteShader(shaders, shCol, shFall);
    if (sh == nullptr)
    {
        return;
    }

    float a[9];
    BuildMatrix(x, y, w, h, a);

    glm::mat3 M;
    for (int r = 0; r < 3; ++r)
    {
        for (int c = 0; c < 3; ++c)
        {
            M[r][c] = a[r * 3 + c];
        }
    }

    RenderMesh2D(m, sh, M);
}

void Tema1::RenderSquare(Mesh* m, float x, float y, float s)
{
    RenderRect(m, x, y, s, s);
}

void Tema1::RenderTri(Mesh* m, float x, float y, float w, float h)
{
    RenderRect(m, x, y, w, h);
}

void Tema1::RecalcLayout()
{
    UIEditor::Recalculeaza(this);
}


void Tema1::DrawSidebar() {}
void Tema1::DrawBorder() {}
void Tema1::DrawEditGrid() {}
void Tema1::DrawPieceBar() {}
void Tema1::DrawPlayBtn() {}
void Tema1::DrawPiece(PieceType, float, float, bool) {}
void Tema1::DrawIcon(PieceType, float, float, float) {}
void Tema1::DrawPieceBase(PieceType, float, float, bool) {}
void Tema1::DrawPieceExtensions(PieceType, float, float) {}
void Tema1::DrawFlames(float, float) {}

void Tema1::GetSlotRect(int idx, float& x, float& y, float& w, float& h)
{
    // imparte sidebar-ul in 4 sloturi egale pe verticala
    w = sbW;
    h = sbH / 4.f;

    x = sbX;

    // slot 0 e sus, slot 3 e jos
    y = sbY + sbH - (idx + 1) * h;
}

Tema1::GridPos Tema1::WindowToGrid(int mx, int my)
{
    // convertim mouse y (window) la gl y (0 jos)
    float glY = (float)screenH - my;

    // coordonate in grila (index int)
    int c = (int)((mx - gX) / gCell);
    int r = (int)((glY - gY) / gCell);

    if (r >= 0 && r < ROWS && c >= 0 && c < COLS)
    {
        return GridPos(r, c);
    }

    return GridPos(-1, -1);
}

void Tema1::AddPiece(int r, int c, PieceType t)
{
    if (r < 0 || r >= ROWS || c < 0 || c >= COLS)
    {
        return;
    }

    // adaugam doar daca celula e libera si nu am depasit max piese
    if (grid[r][c] == NONE)
    {
        if (pieceCount < MAX_PIECES)
        {
            grid[r][c] = t;
            ++pieceCount;
        }
    }
}

void Tema1::DelPiece(int r, int c)
{
    if (r < 0 || r >= ROWS || c < 0 || c >= COLS)
    {
        return;
    }

    if (grid[r][c] != NONE)
    {
        grid[r][c] = NONE;
        pieceCount = std::max(0, pieceCount - 1);
    }
}

bool Tema1::CheckConnected()
{
    VerificatorNava v(grid, ROWS, COLS, pieceCount, MAX_PIECES);
    return v.Conectat();
}

bool Tema1::ValidateLayout()
{
    VerificatorNava v(grid, ROWS, COLS, pieceCount, MAX_PIECES);
    return v.OK();
}

// CONVERSIE NAVA - PALETA

void Tema1::ConvertShipToPaddle()
{
    // calculam bounding box al pieselor din grid si il folosim ca dimensiune pentru paleta
    validShape = false;

    minR = ROWS;
    maxR = -1;
    minC = COLS;
    maxC = -1;

    for (int r = 0; r < ROWS; ++r)
    {
        for (int c = 0; c < COLS; ++c)
        {
            if (grid[r][c] != NONE)
            {
                minR = std::min(minR, r);
                maxR = std::max(maxR, r);
                minC = std::min(minC, c);
                maxC = std::max(maxC, c);
            }
        }
    }

    // daca nu exista piese, bounding box e invalid
    if (maxR < minR || maxC < minC)
    {
        return;
    }

    validShape = true;

    int rr = maxR - minR + 1;
    int cc = maxC - minC + 1;

    // dimensiune tile in modul play (separata de dimensiunea din editor)
    tileSize_x = 20.f;
    tileSize_y = 20.f;

    game.paddle.w = cc * tileSize_x;
    game.paddle.h = rr * tileSize_y;

    game.paddle.x = (game.maxX - game.paddle.w) * 0.5f;
    game.paddle.y = 64.f;

    // lipim bila de paleta la inceput
    game.StickBall();
}

void Tema1::DrawShipAsPaddle()
{
    if (!validShape)
    {
        return;
    }

    float bx = game.paddle.x;
    float by = game.paddle.y;

    for (int r = minR; r <= maxR; ++r)
    {
        for (int c = minC; c <= maxC; ++c)
        {
            if (grid[r][c] == NONE)
            {
                continue;
            }

            int lr = r - minR;
            int lc = c - minC;

            float x = bx + lc * tileSize_x;
            float y = by + lr * tileSize_y;

            // culoarea tile-ului depinde de tipul piesei
            float R = 0.8f;
            float G = 0.8f;
            float B = 0.8f;

            if (grid[r][c] == WEAPON)
            {
                R = 0.9f; G = 0.9f; B = 0.5f;
            }

            if (grid[r][c] == PROPULSION)
            {
                R = 0.7f; G = 0.5f; B = 0.2f;
            }

            if (grid[r][c] == SHIELD)
            {
                R = 0.6f; G = 0.8f; B = 0.9f;
            }

            RenderColored(x, y, tileSize_x, tileSize_y, R, G, B);
        }
    }
}

void Tema1::SwitchToPlay()
{
    // nu trecem in play daca nava nu e valida
    if (!ValidateLayout())
    {
        return;
    }

    game.Setup((float)screenW, (float)screenH);
    game.ResetBlocks();
    ConvertShipToPaddle();

    currentMode = PLAY;
}

void Tema1::RunGame(float dt)
{
    game.Tick(dt, this);

    // desenam caramizile
    for (auto& b : game.blocks)
    {
        if (b.health > 0)
        {
            RenderColored(b.x, b.y, b.w, b.h, b.r, b.g, b.b);
        }
        else
        {
            if (b.dying)
            {
                // animatie: micsoram block-ul pe masura ce deathAnim creste
                float cx = b.x + b.w * 0.5f;
                float cy = b.y + b.h * 0.5f;

                float s = std::max(0.f, 1.f - b.deathAnim);

                RenderColored(
                    cx - (b.w * s) * 0.5f,
                    cy - (b.h * s) * 0.5f,
                    b.w * s,
                    b.h * s,
                    0.8f, 0.8f, 0.8f
                );
            }
        }
    }

    // desenam paleta: daca avem forma valida, o desenam ca nava
    if (validShape)
    {
        DrawShipAsPaddle();
    }
    else
    {
        RenderColored(game.paddle.x, game.paddle.y, game.paddle.w, game.paddle.h, 0.95f, 0.95f, 0.95f);
    }

    // desenam bila ca un cerc scalat
    float bx = game.ball.x - game.ball.rad;
    float by = game.ball.y - game.ball.rad;
    float bs = game.ball.rad * 2.f;

    RenderWithMatrix(ballMesh, bx, by, bs, bs);

    RenderScore(game.points);
    RenderLives(game.hp);
}

// QUAD COLORAT

Mesh* Tema1::MakeColoredQuad(float r, float g, float b, const char* name)
{
    // mesh unit (0..1) cu culoare pe vertex
    std::vector<VertexFormat> v =
    {
        VertexFormat(glm::vec3(0,0,0), glm::vec3(r,g,b)),
        VertexFormat(glm::vec3(1,0,0), glm::vec3(r,g,b)),
        VertexFormat(glm::vec3(1,1,0), glm::vec3(r,g,b)),
        VertexFormat(glm::vec3(0,1,0), glm::vec3(r,g,b))
    };

    std::vector<unsigned int> idx = { 0,1,2,0,2,3 };

    Mesh* m = new Mesh(name);
    m->InitFromData(v, idx);
    m->SetDrawMode(GL_TRIANGLES);

    AddMeshToList(m);
    return m;
}

uint32_t Tema1::PackColor(float r, float g, float b)
{
    // identic cu cache-ul local: float [0..1] -> rgb 8-bit -> 0xRRGGBB
    auto to8 = [](float x)->uint32_t
        {
            float clamped = Clamp(x, 0.f, 1.f);
            int v = (int)std::round(clamped * 255.f);
            v = std::max(0, std::min(255, v));
            return (uint32_t)v;
        };

    uint32_t R = to8(r);
    uint32_t G = to8(g);
    uint32_t B = to8(b);

    return (R << 16) | (G << 8) | B;
}

void Tema1::RenderColored(float x, float y, float w, float h, float r, float g, float b)
{
    // cache la nivel de clasa (colorCache) pentru a nu recrea mesh-uri
    uint32_t key = PackColor(r, g, b);

    Mesh* m = nullptr;

    auto it = colorCache.find(key);
    if (it == colorCache.end())
    {
        char nm[64];
        std::snprintf(nm, sizeof(nm), "quad_%u", key);

        m = MakeColoredQuad(r, g, b, nm);
        colorCache[key] = m;
    }
    else
    {
        m = it->second;
    }

    RenderRect(m, x, y, w, h);
}

void Tema1::RenderWithMatrix(Mesh* m, float x, float y, float w, float h)
{
    RenderRect(m, x, y, w, h);
}

void Tema1::RenderLives(int n)
{
    // desenam inimioare in dreapta sus
    float x = (float)screenW - 180.f;
    float y = (float)screenH - 30.f;

    for (int i = 0; i < n; ++i)
    {
        RenderWithMatrix(lifeMesh, x + i * 26.f, y, 22.f, 20.f);
    }
}

void Tema1::RenderScore(int s)
{
    // print in consola doar cand se schimba score sau lives
    static int scorUltim = -1;
    static int vietiUltime = -1;

    if (s != scorUltim || game.hp != vietiUltime)
    {
        std::cout << "Score: " << s << "   Lives: " << game.hp << "\n";
        scorUltim = s;
        vietiUltime = game.hp;
    }

    // o bara mica ca placeholder de hud
    RenderColored(16.f, (float)screenH - 28.f, 120.f, 18.f, 0.15f, 0.15f, 0.15f);
}

// GAMESTATE

void Tema1::GameState::Setup(float w, float h)
{
    maxX = w;
    maxY = h;
    minX = 0.f;
    minY = 0.f;

    paddle.w = 110.f;
    paddle.h = 18.f;
    paddle.x = (maxX - paddle.w) * 0.5f;
    paddle.y = 64.f;
    paddle.speed = 520.f;

    ball.rad = 10.f;

    StickBall();
}

void Tema1::GameState::ResetBlocks()
{
    // facem un grid de 5x13 blocuri, culori random
    blocks.clear();
    blocks.reserve(5 * 13);

    std::mt19937 rng{ std::random_device{}() };
    std::uniform_real_distribution<float> U(0.25f, 0.95f);

    for (int r = 0; r < 5; ++r)
    {
        for (int c = 0; c < 13; ++c)
        {
            Block b{};

            b.w = 96.f;
            b.h = 36.f;

            b.x = 16.f + c * (b.w + 4.f);
            b.y = 480.f + (4 - r) * (b.h + 4.f);

            b.health = 2;
            b.dying = false;
            b.deathAnim = 0.f;

            b.r = U(rng);
            b.g = U(rng);
            b.b = U(rng);

            blocks.push_back(b);
        }
    }

    hp = 3;
    points = 0;
    stopped = false;

    StickBall();
}

void Tema1::GameState::StickBall()
{
    // bila sta "lipita" de paleta, fara viteza
    ball.stuck = true;
    ball.vx = 0.f;
    ball.vy = 0.f;

    ball.x = paddle.x + paddle.w * 0.5f;
    ball.y = paddle.y + paddle.h + ball.rad + 1.f;
}

bool Tema1::GameState::HitTest(float cx, float cy, float r, float x1, float y1, float x2, float y2)
{
    // coliziune cerc-rect:
    // 1) proiectam centrul cercului pe rect (clamp)
    // 2) masuram distanta de la centru la punctul proiectat
    float px = Clamp(cx, x1, x2);
    float py = Clamp(cy, y1, y2);

    float dx = cx - px;
    float dy = cy - py;

    return dx * dx + dy * dy <= r * r;
}

void Tema1::GameState::Tick(float dt, Tema1* scena)
{
    if (stopped)
    {
        return;
    }

    // clamp paleta in bounds
    if (paddle.x < minX)
    {
        paddle.x = minX;
    }

    if (paddle.x + paddle.w > maxX)
    {
        paddle.x = maxX - paddle.w;
    }

    // miscare bila
    if (!ball.stuck)
    {
        ball.x += ball.vx * dt;
        ball.y += ball.vy * dt;
    }
    else
    {
        StickBall();
    }

    // coliziuni cu peretii (stanga/dreapta/sus)
    if (!ball.stuck)
    {
        if (ball.x - ball.rad < minX)
        {
            ball.x = minX + ball.rad;
            ball.vx = -ball.vx;
        }

        if (ball.x + ball.rad > maxX)
        {
            ball.x = maxX - ball.rad;
            ball.vx = -ball.vx;
        }

        if (ball.y + ball.rad > maxY)
        {
            ball.y = maxY - ball.rad;
            ball.vy = -ball.vy;
        }
    }

    // daca iese jos, pierdem o viata si resetam
    if (!ball.stuck && ball.y + ball.rad < minY)
    {
        hp--;

        if (hp <= 0)
        {
            // iesim in editor cand nu mai avem vieti
            scena->currentMode = EDIT;
            return;
        }

        paddle.x = (maxX - paddle.w) * 0.5f;
        paddle.y = 64.f;

        StickBall();
        return;
    }

    // coliziune cu paleta
    if (!ball.stuck)
    {
        float x1 = paddle.x;
        float y1 = paddle.y;
        float x2 = paddle.x + paddle.w;
        float y2 = paddle.y + paddle.h;

        // conditionam si cu ball.vy < 0 ca sa nu ricoseze de paleta cand urca
        if (HitTest(ball.x, ball.y, ball.rad, x1, y1, x2, y2) && ball.vy < 0.f)
        {
            // calculam unde a lovit bila paleta:
            // t in [-1..1], stanga -> -1, dreapta -> 1
            float centru = (x1 + x2) * 0.5f;
            float t = (ball.x - centru) / (paddle.w * 0.5f);
            t = Clamp(t, -1.f, 1.f);

            // mapam t in unghi intre UNGHI_MIN si UNGHI_MAX
            float ang = UNGHI_MIN + (UNGHI_MAX - UNGHI_MIN) * (t + 1.f) * 0.5f;

            // vector de directie din unghi
            float dx = std::cos(ang);
            float dy = std::sin(ang);

            float len = std::sqrt(dx * dx + dy * dy);
            if (len == 0.f)
            {
                len = 1.f;
            }

            ball.vx = (dx / len) * VITEZA_BILA;
            ball.vy = (dy / len) * VITEZA_BILA;

            // mutam bila putin deasupra paletei ca sa evitam coliziuni multiple in acelasi frame
            ball.y = y2 + ball.rad + 0.1f;
        }
    }

    // coliziune cu caramida (prima pe care o gasim)
    int hit = -1;
    float nx = 0.f;
    float ny = 0.f;

    for (int i = 0; i < (int)blocks.size(); ++i)
    {
        Block& b = blocks[i];

        if (b.health <= 0)
        {
            // animatie de moarte (shrink) cat timp b.dying e true
            if (b.dying)
            {
                b.deathAnim += dt * 6.f;
                if (b.deathAnim >= 1.f)
                {
                    b.dying = false;
                }
            }

            continue;
        }

        float x1 = b.x;
        float y1 = b.y;
        float x2 = b.x + b.w;
        float y2 = b.y + b.h;

        if (!HitTest(ball.x, ball.y, ball.rad, x1, y1, x2, y2))
        {
            continue;
        }

        // determinam normala aproximativa de coliziune:
        // comparam penetratia pe x vs y, iar directia vine din semnul lui dx/dy
        float cx = x1 + b.w * 0.5f;
        float cy = y1 + b.h * 0.5f;

        float dx = ball.x - cx;
        float dy = ball.y - cy;

        float px = (b.w * 0.5f + ball.rad) - std::abs(dx);
        float py = (b.h * 0.5f + ball.rad) - std::abs(dy);

        // inainte aveai ternar:
        //   if (px < py) nx = (dx > 0) ? 1.f : -1.f;
        // l-am inlocuit cu if-uri explicite
        if (px < py)
        {
            if (dx > 0.f)
            {
                nx = 1.f;
            }
            else
            {
                nx = -1.f;
            }

            ny = 0.f;
        }
        else
        {
            if (dy > 0.f)
            {
                ny = 1.f;
            }
            else
            {
                ny = -1.f;
            }

            nx = 0.f;
        }

        hit = i;
        break;
    }

    if (hit >= 0)
    {
        Block& b = blocks[hit];

        // reflectam viteza bilei pe normala (nx, ny)
        // v' = v - 2 * (v·n) * n
        float dot = ball.vx * nx + ball.vy * ny;
        ball.vx = ball.vx - 2.f * dot * nx;
        ball.vy = ball.vy - 2.f * dot * ny;

        // offset mic ca sa nu ramana "intra" in bloc
        ball.x += nx * 2.f;
        ball.y += ny * 2.f;

        // scadem viata blocului
        b.health--;

        if (b.health == 1)
        {
            // cand ramane cu 1 hp, il facem alb ca feedback
            b.r = 1.f;
            b.g = 1.f;
            b.b = 1.f;
        }
        else if (b.health == 0)
        {
            // cand moare, pornim animatia si cresterm scorul
            b.dying = true;
            b.deathAnim = 0.f;
            points += 1;
        }

        // renormalizam viteza la VITEZA_BILA ca sa nu se acumuleze erori
        float sp = std::sqrt(ball.vx * ball.vx + ball.vy * ball.vy);
        if (sp == 0.f)
        {
            sp = 1.f;
        }

        ball.vx = (ball.vx / sp) * VITEZA_BILA;
        ball.vy = (ball.vy / sp) * VITEZA_BILA;
    }

    // daca nu mai exista blocuri "alive", regeneram
    bool exista = false;

    for (auto& b : blocks)
    {
        if (b.health > 0)
        {
            exista = true;
            break;
        }
    }

    if (!exista)
    {
        ResetBlocks();
    }
}
