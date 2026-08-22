"""
单词拼写检查器
知识点：文件读取、集合操作、字符串处理
玩法：输入单词，检查是否在预置单词库中。
"""

word_list = [
    "apple", "banana", "cat", "dog", "elephant",
    "fish", "grape", "hello", "ice", "jacket",
    "key", "lion", "monkey", "night", "orange",
    "python", "queen", "rabbit", "sun", "tree",
    "umbrella", "violin", "water", "xenon", "yellow", "zebra"
]

def spell_check():
    word_set = set(word_list)  # 集合，查找速度 O(1)
    print("单词拼写检查器")
    print("输入单词检查是否正确，输入 'q' 退出")

    while True:
        word = input("请输入一个单词: ").strip().lower()
        if word == "q":
            print("再见！")
            break
        if not word.isalpha():
            print("请输入纯字母单词")
            continue
        if word in word_set:
            print(f" '{word}' 拼写正确")
        else:
            print(f" '{word}' 拼写错误")

if __name__ == "__main__":
    spell_check()