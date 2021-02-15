#include <fmt/core.h>

#include "openglwindow.hpp"

#include <imgui.h>

void OpenGLWindow::initializeGL() {
  auto windowSettings{getWindowSettings()};
  fmt::print("Initial window size: {}x{}\n", windowSettings.width,
             windowSettings.height);
}

void OpenGLWindow::paintGL() {
  // Set the clear color
  glClearColor(m_clearColor[0], m_clearColor[1], m_clearColor[2],
               m_clearColor[3]);
  // Clear the color buffer
  glClear(GL_COLOR_BUFFER_BIT);
}

void fillBoard(char board[]) {
  for (int j = 0; j < 9; j++) {
    if (board[j] == ' ') board[j] = '-';
  }
}

bool resetBoard(char board[], char btns[]) {
  for (int j = 0; j < 9; j++) {
    board[j] = ' ';
    btns[j] = 0;
  }
  return false;
}

bool checkMove(char *player, char board[]) {

  if (((board[0] == *player && board[1] == *player) && board[2] == *player) ||
      ((board[0] == *player && board[4] == *player) && board[8] == *player) ||
      ((board[3] == *player && board[4] == *player) && board[5] == *player) ||
      ((board[6] == *player && board[7] == *player) && board[8] == *player) ||
      ((board[2] == *player && board[4] == *player) && board[6] == *player) ||
      ((board[0] == *player && board[3] == *player) && board[6] == *player) ||
      ((board[1] == *player && board[4] == *player) && board[7] == *player) ||
      ((board[2] == *player && board[6] == *player) && board[8] == *player)) {
    fillBoard(board);
    return true;
  }

  return false;
}

void OpenGLWindow::paintUI() {
  // Parent class will show fullscreen button and FPS meter
  abcg::OpenGLWindow::paintUI();

  // Our own ImGui widgets go below
  {
    // Window begin
    ImGui::SetNextWindowSize(ImVec2(350, 500));
    auto flags{ImGuiWindowFlags_MenuBar};
    ImGui::Begin("Tic-Tac-Toe", nullptr, flags);

    bool restart{};

    static char board[9] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
    static char btns[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    static int clicked = 0;
    static char xis = 'X';
    static char bolinha = 'O';
    static bool endGame = false;
    static char *pTurn = &xis;

    ImGui::AlignTextToFramePadding();
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Game")) {
        ImGui::MenuItem("Restart", nullptr, &restart);
        ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    if (endGame) {
      ImGui::Text("End game");
      if (*pTurn == 'O')
        ImGui::Text("X wins!");
      else
        ImGui::Text("O wins!");
    } else
      ImGui::Text("It's %c turn", *pTurn);

    if (restart) {
        endGame = resetBoard(board, btns);
        pTurn = &xis;
    }

    ImGui::Columns(3, NULL, true);

    for (int i = 0; i < 9; i++){

        ImGui::Button(std::string(1, board[i]).c_str(), ImVec2(100, 100));
        if (ImGui::IsItemClicked() && endGame != true) {
            clicked++;
            if (btns[i] == 0) {
                if (*pTurn == 'X') {
                    board[i] = 'X';
                    endGame = checkMove(pTurn, board);
                    pTurn = &bolinha;

                } else {
                    board[i] = 'O';
                    endGame = checkMove(pTurn, board);
                    pTurn = &xis;
                }
            }
            btns[i] = 1;
            clicked = 0;
        }
        ImGui::NextColumn();
        if (i == 3 or i == 6){
            ImGui::Separator();
        }
    }
    
    ImGui::Columns(1);
    ImGui::Spacing();
    ImGui::NewLine();
    if(ImGui::Button("Restart Game", ImVec2(-1, 40))){
        endGame = resetBoard(board, btns);
        pTurn = &xis;
    };

    // Window end
    ImGui::End();

  }
}