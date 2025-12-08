#!/usr/bin/env python3

def popcount(d):
	assert len(d) in (8, 10)
	return sum(d)

def df(x):
	b = bin(int(round(x)))[2:]
	b = [int(bit) for bit in b]
	while len(b) < 8:  b = [0] + b
	b = list(reversed(b))
	return b

def dfr(d):
	x = 0
	add = 1
	d = d[:]
	while d:
		x += add if d[0] else 0
		d = d[1:]
		add <<= 1

	return x

xmin, xmax = 16, 235  # "Limited range"

def encode(x, neighbors=False):
	d = df(x)
	q = [0] * 10

	q[9] = 0
	q[8] = 1
	q[0] = d[0]
	for i in range(1, 8):  q[i] = q[i - 1] ^ d[i]

	if neighbors and popcount(q) != 5:
		uu = []
		for d in range(-2, 3):
			if d:
				y = encode(x + d)
				ye = abs(5 - popcount(y))
				uu.append((ye, y))
		uu = sorted(uu)
		return uu[0][1]

	return q

def decode(d):
	assert len(d) == 10
	assert d[8]
	assert not d[9]
	q = [None] * 8
	d = d[:]
	q[0] = d[0]
	for i in range(1, 8):  q[i] = d[i] ^ d[i - 1]
	return dfr(q)

m = 1 << 5
d = (xmax - xmin) / (m - 1)
x = xmin

while True:
	xr = int(round(x))
	if xr > xmax: break
	enc = encode(xr, neighbors=True)
	#assert decode(enc) in (xr - 1, xr, xr + 1)
	c = ("0b" + "".join(reversed(list(str(bit) for bit in enc))))
	print("%-12s,  /* %3d (%d ones) */" % (c, int(x), popcount(enc)))
	x += d
