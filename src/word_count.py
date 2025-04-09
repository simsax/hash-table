def main():
    content = ""
    with open("../shakespeare_small.txt", "r") as f:
        content = f.read()

    tokens = content.split()
    counter = dict()

    for token in tokens:
        if token in counter:
            counter[token] += 1
        else:
            counter[token] = 1

    # print(counter)
    print(f"{len(counter.keys())} unique words")
    print("Top 20:")
    dict_items = list(counter.items())
    max = 20
    tops = sorted(dict_items, key=lambda x: x[1], reverse=True)[:max]
    for top in tops:
        print(top)


if __name__ == "__main__":
    main()
