"""
猜数字游戏
知识点：random、while循环、条件判断、类型转换
玩法：程序随机生成一个1~100之间的整数，玩家输入猜测，提示大了或小了，直到猜中。
"""

import  random
def guess_number():
        secret = random.randint(1,100)
        attempts = 0
        print("猜数字开始，我已经想好一个1~100之间的整数。")

        while True:
            try:
                guess = int(input("请输入你的猜测："))
            except ValueError:
                print("请输入有效整数！")
                continue

            attempts += 1
            if guess < secret:
                print("太小了，再大一点")
            elif guess > secret:
                print("太大了，再小一点")
            else:
                print(f"恭喜你猜对了！正确答案是{secret}。")
                print(f"你一共猜了{attempts}次。")
                break

if __name__ == "__main__":
    guess_number()
