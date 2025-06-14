#include "NPCDialog.hpp"
#include "Engine/Resources.hpp"
#include "Engine/LOG.hpp"
#include "Scene/PlayScene.hpp"
#include "Scene/BattleScene.hpp"
#include "Scene/SmallEatScene.hpp"
#include "Player/Player.hpp"
#include "Engine/GameEngine.hpp"

#include <fstream>
#include <cstdlib>
#include <ctime>

#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro.h>

NPCDialog::NPCDialog() 
    : isActive(false),
      currentMessageIndex(0),
      charDisplayTimer(0.0f),
      charDisplayDelay(0.05f),  // 每個字顯示的間隔時間(秒)
      isDisplayingFullMessage(false),
      font(nullptr),
      boxWidth(1600.0f),   // 加寬對話框
      boxHeight(200.0f),   // 對話框高度
      boxX(240.0f),        // 水平位置 (居中：1280/2 - 800/2 = 240)
      boxY(500.0f),        // 垂直位置 (靠近底部) 500
      padding(25.0f)       // 內邊距加大
{
    textColor = al_map_rgb(255, 255, 255);  // 白色文字
    boxColor = al_map_rgba(0, 0, 0, 200);   // 半透明黑色背景
    nameColor = al_map_rgb(255, 215, 0);    // 金色NPC名字
    
    // 計算其他位置
    avatarX = boxX + padding;
    avatarY = boxY + padding;
    textX = boxX + padding + 128 + padding;  // 頭像寬度假設為128
    textY = boxY + padding + 30;  // 稍微下移避免與名字重疊
    nameX = textX;
    nameY = boxY + padding;
}

NPCDialog::~NPCDialog() {
    // 字體由Resources管理，不需要在這裡釋放
}

bool NPCDialog::talking = false;

bool NPCDialog::Initialize() {
    // 使用 Resources 的 GetFont 方法，傳入字體名稱和大小
    font = Engine::Resources::GetInstance().GetFont("normal.ttf", 24).get();
    if (!font) {
        Engine::LOG(Engine::WARN) << "Failed to load font for NPCDialog";
        return false;
    }

    // if (Player::curPos.x / 64 < PlayScene::window_x / 2) boxX = (PlayScene::window_x / 2) * PlayScene::BlockSize;
    // else if (Player::curPos.x / 64 > PlayScene::MapWidth - PlayScene::window_x / 2) boxX = (PlayScene::MapWidth - PlayScene::window_x / 2) * PlayScene::BlockSize;
    // else boxX = Player::curPos.x;
    // boxX -= boxWidth / 4;
    // if (Player::curPos.y / 64 < PlayScene::window_y / 2) boxY = (PlayScene::window_y / 2) * PlayScene::BlockSize;
    // else if (Player::curPos.y / 64 > PlayScene::MapHeight - PlayScene::window_y / 2) boxY = (PlayScene::MapWidth - PlayScene::window_x / 2) * PlayScene::BlockSize;
    // else boxY = Player::curPos.y;
    // boxY += boxHeight / 4;
    //printf("%f %f<<<<<<<<<<<<<<<<<\n", boxX, boxY);
    // 設置對話框位置：視角中央下方
    // PlayScene* scene = dynamic_cast<PlayScene*>(Engine::GameEngine::GetInstance().GetCurrentScene());
    // if (scene) {
    //     Engine::Point clientSize = scene->GetClientSize(); // 384x192
    //     boxX = clientSize.x / 2 - boxWidth / 2; // 192 - 150 = 42
    //     boxY = clientSize.y / 2 + 30;           // 96 + 30 = 126
    //     avatarX = boxX + padding;
    //     avatarY = boxY + padding;
    //     textX = boxX + padding + 64 + padding;  // 頭像寬度 64
    //     textY = boxY + padding + 20;
    //     nameX = textX;
    //     nameY = boxY + padding;
    // }

    return true;
}

void NPCDialog::StartDialog(const std::string& npcName, 
                          std::shared_ptr<ALLEGRO_BITMAP> npcAvatar,
                          const std::vector<std::string>& messages) {
    if (messages.empty()) {
        Engine::LOG(Engine::WARN) << "Attempted to start dialog with no messages";
        return;
    }

    talking = true;

    this->npcName = npcName;
    this->npcAvatar = npcAvatar;
    this->messages = messages;
    
    currentMessageIndex = 0;
    currentDisplayText.clear();
    charDisplayTimer = 0.0f;
    isDisplayingFullMessage = false;
    isActive = true;

    Engine::Point clientSize;
    Engine::Point camera;
    if (!PlayScene::inPlay) {
        clientSize = SmallEatScene::GetClientSize();
        camera = SmallEatScene::getCamera();
    }
    else {
        clientSize = PlayScene::GetClientSize();
        camera = PlayScene::getCamera();
    }
    boxX = camera.x + clientSize.x / 2 - boxWidth / 2 - 300;
    boxY = camera.y + clientSize.y / 2 + 30;
    avatarX = boxX + padding;
    avatarY = boxY + padding;
    textX = boxX + padding + 128 + padding;  // 頭像寬度 64
    textY = boxY + padding + 30;
    nameX = textX;
    nameY = boxY + padding;
    //}
}

void NPCDialog::Update(float deltaTime) {
    if (!isActive || isDisplayingFullMessage) return;

    // 逐字顯示效果
    charDisplayTimer += deltaTime + 0.025f;
    if (charDisplayTimer >= charDisplayDelay) {
        charDisplayTimer = 0.0f;
        
        const std::string& fullMessage = messages[currentMessageIndex];
        if (currentDisplayText.length() < fullMessage.length()) {
            currentDisplayText = fullMessage.substr(0, currentDisplayText.length() + 1);
        } else {
            isDisplayingFullMessage = true;
        }
    }
}

void NPCDialog::Draw() const {
    if (!isActive) return;

    // 繪製對話框背景 (增加圓角效果)
    al_draw_filled_rounded_rectangle(boxX, boxY, 
                                    boxX + boxWidth, boxY + boxHeight, 
                                    15, 15, boxColor);
    al_draw_rounded_rectangle(boxX, boxY, 
                             boxX + boxWidth, boxY + boxHeight, 
                             15, 15, al_map_rgb(255,255,255), 2);

    // 繪製NPC頭像 (增加邊框效果)
    if (npcAvatar) {
        float avatarSize = 128; // 頭像大小
        // 頭像背景圓框
        al_draw_filled_circle(avatarX + avatarSize/2, avatarY + avatarSize/2, 
                             avatarSize/2 + 5, al_map_rgb(50,50,50));
        // 實際頭像 (圓形裁切)
        al_draw_scaled_bitmap(npcAvatar.get(), 
                             0, 0, 
                             al_get_bitmap_width(npcAvatar.get()),
                             al_get_bitmap_height(npcAvatar.get()),
                             avatarX, avatarY, avatarSize, avatarSize, 0);
    }
    
    // 繪製NPC名字
    if (font && !npcName.empty()) {
        al_draw_text(font, nameColor, nameX, nameY, ALLEGRO_ALIGN_LEFT, npcName.c_str());
    }

    // 繪製對話文字
    if (font && !currentDisplayText.empty()) {
        // 計算可用寬度 (減去頭像和內邊距)
        float availableWidth = boxWidth - (textX - boxX) - padding;
        float lineHeight = al_get_font_line_height(font);
        float currentY = textY;
        
        // 分割文本為單詞
        std::vector<std::string> words;
        std::string currentWord;
        for (char c : currentDisplayText) {
            if (c == ' ') {
                if (!currentWord.empty()) {
                    words.push_back(currentWord);
                    currentWord.clear();
                }
                words.push_back(" "); // 空格作為單獨元素
            } else {
                currentWord += c;
            }
        }
        if (!currentWord.empty()) {
            words.push_back(currentWord);
        }
        
        // 構建行
        std::string currentLine;
        for (const auto& word : words) {
            std::string testLine = currentLine + word;
            float testWidth = al_get_text_width(font, testLine.c_str());
            
            if (testWidth > availableWidth) {
                // 如果添加這個詞會超過寬度，先繪製當前行
                if (!currentLine.empty()) {
                    al_draw_text(font, textColor, textX, currentY, ALLEGRO_ALIGN_LEFT, currentLine.c_str());
                    currentY += lineHeight;
                }
                currentLine = word; // 新行從這個詞開始
            } else {
                currentLine = testLine;
            }
        }
        
        // 繪製最後一行
        // if (!currentLine.empty()) {
        //     al_draw_text(font, textColor, textX, currentY, ALLEGRO_ALIGN_LEFT, currentLine.c_str());
        // }
        
        if (!currentLine.empty()) {
            ALLEGRO_COLOR dynamicColor = textColor;
            if (currentDisplayText.find("【大吉】") != std::string::npos) {
                dynamicColor = al_map_rgb(255, 223, 0); // 金色
            } else if (currentDisplayText.find("【中吉】") != std::string::npos) {
                dynamicColor = al_map_rgb(144, 238, 144); // 淺綠
            } else if (currentDisplayText.find("【小吉】") != std::string::npos || currentDisplayText.find("【吉】") != std::string::npos) {
                dynamicColor = al_map_rgb(173, 216, 230); // 淡藍
            } else if (currentDisplayText.find("【凶】") != std::string::npos) {
                dynamicColor = al_map_rgb(255, 99, 71); // 番茄紅
            }

            al_draw_text(font, dynamicColor, textX, currentY, ALLEGRO_ALIGN_LEFT, currentLine.c_str());
        }
    }
}

bool NPCDialog::IsDialogActive() const {
    return isActive;
}

void NPCDialog::AdvanceDialog() {
    if (!isActive) return;

    // 第一次顯示該訊息就檢查是否是特殊指令
    if (isDisplayingFullMessage) {
        currentMessageIndex++;
        if (currentMessageIndex < messages.size()) {
            // 檢查是否是 #lottery 指令
            std::string& msg = messages[currentMessageIndex];
            if (msg == "#lottery") {
                msg = DrawRandomLot(); // 抽籤
            }
            currentDisplayText.clear();
            isDisplayingFullMessage = false;
        } else {
            EndDialog();
            talking = false;
        }
    } else {
        currentDisplayText = messages[currentMessageIndex];
        isDisplayingFullMessage = true;
    }
}

void NPCDialog::EndDialog() {
    isActive = false;
}

std::string NPCDialog::DrawRandomLot() {
    std::ifstream fin("Resource/lottery.txt");
    std::vector<std::string> lots;
    std::string line;

    while (std::getline(fin, line)) {
        if (!line.empty()) {
            lots.push_back(line);
        }
    }
    fin.close();

    if (lots.empty()) return "抽籤失敗，沒有籤詩。";

    std::srand(static_cast<unsigned>(std::time(nullptr))); // 設定隨機種子
    int idx = std::rand() % lots.size();
    return "【你抽到了】：\n" + lots[idx];
}