from collections import defaultdict


def vertical_separation_line_exists(points : list[tuple[int, int]]) -> bool:
    if not points:
        return False

    xs = [x * 2 for x, _ in points]
    mid = (max(xs) + min(xs)) / 2

    cnt = defaultdict(int)
    for x in xs:
        d = x - mid
        cnt[abs(d)] += 1 if d > 0 else -1 if d < 0 else 0
    return sum(cnt.values()) == 0

if __name__ == "__main__":
    print(vertical_separation_line_exists([]))  # False
    print(vertical_separation_line_exists([(1, 1)]))  # True
    print(vertical_separation_line_exists([(1, 1), (2, 2)]))  # True
    print(vertical_separation_line_exists([(1, 1), (2, 2), (3, 3)]))  # True
    print(vertical_separation_line_exists([(1, 1), (2, 2), (4, 3)]))  # False

