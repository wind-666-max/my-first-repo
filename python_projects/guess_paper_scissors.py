"""
石头剪刀布
知识点：列表、字典、随机选择、多分支条件
玩法：玩家输入石头/剪刀/布，电脑随机出拳，判断胜负。
"""

import random

def rock_paper_scissors():
    choices = ["石头", "剪刀", "布"]
    wins = {"石头": "剪刀", "剪刀": "布", "布": "石头"}

    print("石头剪刀布游戏")
    print("输入 石头/剪刀/布 开始，输入'退出'结束")

    while True:
        player = input("你的选择: ")

        if player == "退出":
            print(" 游戏结束！")
            break

        if player not in choices:
            print("无效输入，请重新输入")
            continue

        computer = random.choice(choices)
        print(f"电脑出了: {computer}")

        if player == computer:
            print("平局！")
        elif wins[player] == computer:
            print("你赢了！")
        else:
            print("你输了！")

if __name__ == "__main__":
    rock_paper_scissors()