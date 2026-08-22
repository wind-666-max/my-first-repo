"""
消失井字棋 - Python 控制台版
规则：每方最多3枚棋子在场，下第4颗时最早的那颗消失
胜利优先，满50步则平局
使用 deque 实现队列
"""

import random
from collections import deque

board = [
    ["1", "2", "3"],
    ["4", "5", "6"],
    ["7", "8", "9"]
]
current_player = "X"
game_mode = ""
x_history = deque(maxlen=4)
o_history = deque(maxlen=4)
move_count = 0
MAX_STEPS = 50

def get_num(row, col):
    return row * 3 + col + 1

def print_board():
    print("\n")
    for i in range(3):
        row_cells = []
        for j in range(3):
            cell = board[i][j]
            if cell in ["X", "O"]:
                history = x_history if cell == "X" else o_history
                if len(history) == 3 and history[0] == (i, j):
                    row_cells.append(cell.lower())  # 显示小写
                else:
                    row_cells.append(cell)
            else:
                row_cells.append(cell)
        print(" " + " | ".join(row_cells))
        if i < 2:
            print("---+---+---")

    for player, history in [('X', x_history), ('O', o_history)]:
        if len(history) == 3:
            row, col = history[0]
            print(f"玩家 {player} 的棋子 ({row+1},{col+1}) 将在下次落子时消失（显示为 {player.lower()}）")
        else:
            print(f"玩家 {player} 暂无即将消失的棋子")

def check_winner():
    for row in board:
        if row[0] == row[1] == row[2]:
            return row[0]
    for col in range(3):
        if board[0][col] == board[1][col] == board[2][col]:
            return board[0][col]
    if board[0][0] == board[1][1] == board[2][2]:
        return board[0][0]
    if board[0][2] == board[1][1] == board[2][0]:
        return board[0][2]
    return None

def make_move(player, row, col):
    global board, x_history, o_history, move_count

    board[row][col] = player
    move_count += 1

    history = x_history if player == "X" else o_history
    history.append((row, col))

    if len(history) > 3:
        old_row, old_col = history.popleft()
        board[old_row][old_col] = str(get_num(old_row, old_col))

    winner = check_winner()
    if winner:
        return True, winner

    return False, None

def player_move(player):
    while True:
        try:
            pos = int(input(f"玩家 {player}，请选择位置 (1-9): "))
            if pos < 1 or pos > 9:
                print("请输入 1~9 之间的数字")
                continue
            row = (pos - 1) // 3
            col = (pos - 1) % 3
            if board[row][col] in ["X", "O"]:
                print("该位置已被占用，请重新选择")
                continue
            win, winner = make_move(player, row, col)
            return win, winner
        except ValueError:
            print("请输入有效的数字")

def computer_move():
    empty = [(i, j) for i in range(3) for j in range(3) if board[i][j] not in ["X", "O"]]
    if not empty:
        return False, None
    row, col = random.choice(empty)
    print(f"电脑下了 ({row+1}, {col+1})")
    return make_move("O", row, col)

def play_game():
    global current_player, move_count
    while True:
        print_board()
        if current_player == "X":
            win, winner = player_move("X")
        else:
            if game_mode == "pvc":
                win, winner = computer_move()
            else:
                win, winner = player_move("O")
        if win:
            print_board()
            print(f"玩家 {winner} 赢了！")
            break
        if move_count >= MAX_STEPS:
            print_board()
            print("步数已满，平局！")
            break
        current_player = "O" if current_player == "X" else "X"

def choose_mode():
    global game_mode
    print("请选择游戏模式：")
    print("  1. 双人对战")
    print("  2. 人机对战 (你执 X，电脑执 O)")
    while True:
        choice = input("请输入 1 或 2: ").strip()
        if choice == "1":
            game_mode = "pvp"
            break
        elif choice == "2":
            game_mode = "pvc"
            break
        else:
            print("无效输入，请重新输入")

def reset_game():
    global board, current_player, x_history, o_history, move_count
    board = [["1", "2", "3"], ["4", "5", "6"], ["7", "8", "9"]]
    current_player = "X"
    x_history.clear()
    o_history.clear()
    move_count = 0

if __name__ == "__main__":
    print("欢迎来到【消失井字棋】！")
    print("规则：每方最多3颗棋子，下第4颗时最早的那颗消失。")
    print("形成三连即获胜，50步未分胜负则平局。")
    choose_mode()
    reset_game()
    play_game()