import numpy as np

np.random.seed(808)

def dump(var, x_):
    print("vector<T> ", var+"_", " = {", ", ".join(list(map(str, np.reshape(x_, -1)))), "}; ")

d = 16
k = 4
w = 1.0
x = np.random.randn(d)
a = np.random.randn(k, d)
b = np.random.rand(k)

y = np.ceil((a.dot(x) + b) / w)

dump("x", x)
dump("a", a)
dump("b", b)
dump("y", y)