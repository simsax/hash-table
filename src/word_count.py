from time import perf_counter

def main():
    content = ""
    with open("../shakespeare.txt", "r") as f:
        content = f.read()

    start_ts = perf_counter()
    tokens = content.split()
    counter = dict()

    for token in tokens:
        if token in counter:
            counter[token] += 1
        else:
            counter[token] = 1

    end_ts = perf_counter()

    print(f"{len(counter.keys())} unique words")
    max = 100
    # print(f"Top {max}:")
    # dict_items = list(counter.items())
    # tops = sorted(dict_items, key=lambda x: x[1], reverse=True)[:max]
    # for top in tops:
    #     print(top)

    print(f"Elapsed: {end_ts - start_ts}s")


if __name__ == "__main__":
    main()
