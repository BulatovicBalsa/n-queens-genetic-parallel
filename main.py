import draw

if __name__ == "__main__":
    arr = input()
    arr = arr.split()
    brs = []
    for br in arr:
        brs.append(int(br))
    draw.draw(brs, len(arr))
