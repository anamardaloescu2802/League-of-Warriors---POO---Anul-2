#pragma once

#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <cstdint>

#include "components/simple_scene.h"

namespace m1
{
    class Tema1 : public gfxc::SimpleScene
    {
    public:
        using gfxc::SimpleScene::RenderMesh2D;
        using gfxc::SimpleScene::meshes;
        using gfxc::SimpleScene::shaders;
        using gfxc::SimpleScene::window;

        struct GridPos
        {
            int r, c;
            GridPos(int row = -1, int col = -1) : r(row), c(col) {}
        };

        enum PieceType : int
        {
            NONE = -1,
            CORE = 0,
            WEAPON = 1,
            PROPULSION = 2,
            SHIELD = 3
        };

        enum { ROWS = 8, COLS = 12, GAP = 5, MAX_PIECES = 10 };

        // Breakout
        struct Block
        {
            float x, y, w, h;
            int health;
            bool dying;
            float deathAnim;
            float r, g, b;
        };

        struct Paddle
        {
            float x, y, w, h, speed;
        };

        struct Ball
        {
            float x, y, vx, vy, rad;
            bool stuck;
        };

        struct GameState
        {
            std::vector<Block> blocks;
            Paddle paddle;
            Ball ball;

            int hp = 3;
            int points = 0;
            bool stopped = false;

            float minX = 0, maxX = 0, minY = 0, maxY = 0;

            void Setup(float width, float height);
            void ResetBlocks();
            void StickBall();
            void Tick(float dt, Tema1* scena);

            static bool HitTest(float cx, float cy, float r,
                float x1, float y1, float x2, float y2);
        };

        Tema1();
        ~Tema1() override = default;

        void Init() override;

        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;

        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;

        void OnWindowResize(int width, int height) override;
        
        void BuildMatrix(float px, float py, float sx, float sy, float* out);
        void RenderRect(Mesh* m, float x, float y, float w, float h);
        void RenderSquare(Mesh* m, float x, float y, float s);
        void RenderTri(Mesh* m, float x, float y, float w, float h);

        void RecalcLayout();
        void DrawSidebar();
        void DrawBorder();
        void DrawEditGrid();
        void DrawPieceBar();
        void DrawPlayBtn();
        void DrawPiece(PieceType t, float x, float y, bool ghost);
        void DrawIcon(PieceType t, float x, float y, float sz);

        bool ValidateLayout();
        bool CheckConnected();

        GridPos WindowToGrid(int mx, int my);
        void AddPiece(int r, int c, PieceType t);
        void DelPiece(int r, int c);

        void GetSlotRect(int idx, float& x, float& y, float& w, float& h);

        enum Mode { EDIT, PLAY };

        void SwitchToPlay();
        void RunGame(float dt);

        void RenderColored(float x, float y, float w, float h, float r, float g, float b);
        void RenderLives(int n);
        void RenderScore(int s);

        Mesh* MakeColoredQuad(float r, float g, float b, const char* name);
        uint32_t PackColor(float r, float g, float b);
        void RenderWithMatrix(Mesh* m, float x, float y, float w, float h);

        void ConvertShipToPaddle();
        void DrawShipAsPaddle();

        void DrawPieceBase(PieceType t, float x, float y, bool ghost);
        void DrawPieceExtensions(PieceType t, float x, float y);
        void DrawFlames(float x, float y);

        std::vector<std::vector<PieceType>> grid;

        PieceType heldPiece = NONE;
        bool dragging = false;
        int pieceCount = 0;

        bool validShape = false;
        int minR = 0, maxR = -1, minC = 0, maxC = -1;
        float tileSize_x = 20.0f, tileSize_y = 20.0f;

        // Culori
        float bgR = 0.f, bgG = 0.f, bgB = 0.f;
        float cellR = 0.08f, cellG = 0.22f, cellB = 0.80f;
        float lineR = 0.03f, lineG = 0.04f, lineB = 0.16f;
        float borderR1 = 0.02f, borderG1 = 0.02f, borderB1 = 0.05f;
        float borderR2 = 0.05f, borderG2 = 0.07f, borderB2 = 0.18f;
        float sideR = 0.12f, sideG = 0.12f, sideB = 0.12f;
        float okR = 0.f, okG = 0.9f, okB = 0.f;
        float fireR = 1.f, fireG = 0.55f, fireB = 0.f;
        float metalR = 0.62f, metalG = 0.62f, metalB = 0.62f;
        float armorR = 0.9f, armorG = 0.88f, armorB = 0.72f;
        float errR = 0.85f, errG = 0.1f, errB = 0.1f;

        // Sidebar / UI
        float sbX = 24.f, sbY = 40.f, sbW = 270.f, sbH = 680.f;
        float gX = 350.f, gY = 100.f, gCell = 50.f;
        float bStart = 0.f, bY = 0.f, bGap = 10.f;

        // Mesh-uri
        Mesh* mCellFill = nullptr;
        Mesh* mCellLine = nullptr;
        Mesh* mSidePanel = nullptr;
        Mesh* mOkQuad = nullptr;
        Mesh* mMetalQuad = nullptr;
        Mesh* mBorder1 = nullptr;
        Mesh* mBorder2 = nullptr;
        Mesh* mErrQuad = nullptr;

        Mesh* mFlame = nullptr;
        Mesh* mShieldTop = nullptr;
        Mesh* mWeaponTop = nullptr;

        Mesh* mXMark = nullptr;

        Mesh* ballMesh = nullptr;
        Mesh* lifeMesh = nullptr;

        // Shader-e (in caz ca vrei acces direct)
        Shader* shCol = nullptr;
        Shader* shFall = nullptr;

        // Mod curent (GameState::Tick modifica currentMode)
        Mode currentMode = EDIT;

        // Rezolutie
        int screenW = 1280;
        int screenH = 720;

        // Cache culori + stare joc
        std::map<uint32_t, Mesh*> colorCache;
        GameState game;

        // Buton Play
        float playX = -1.f, playY = -1.f, playS = 28.f;
    };
} // namespace m1