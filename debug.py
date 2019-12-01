#functions for debugging purpose
from math import gcd
from random import randrange as rr


primes = [2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 
83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181,
191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293,
307, 311, 313, 317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421,
431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541, 547, 557, 
563, 569, 571, 577, 587, 593, 599, 601, 607, 613, 617, 619, 631, 641, 643, 647, 653, 659, 661, 673, 
677, 683, 691, 701, 709, 719, 727, 733, 739, 743, 751, 757, 761, 769, 773, 787, 797, 809, 811, 821, 
823, 827, 829, 839, 853, 857, 859, 863, 877, 881, 883, 887, 907, 911, 919, 929, 937, 941, 947, 953, 
967, 971, 977, 983, 991, 997]

def conlog(a):
	mask=(1<<64)-1
	while(a.bit_length()>0):
		print(hex(a&mask))
		a=a>>64

def pow(a,b,m):
	c=1
	blen=len(bin(b))-3
	for i in range(blen, 0,-1):
		if (b>>i) & 1:
			c=(c*a)%m
		c=(c**2)%m
	if b&1:
		c=(c*a)%m
	return c

def MillerRabin(w, iterations=40):
    if (w&1) == 0:
        return 0
    
    wMin1 = w-1
    a = 1
    m = wMin1>>1
    while (m&1) == 0:
        a += 1
        m >>= 1

    lim = 1 << w.bit_length()
    for i in range(iterations):
        b = wMin1
        while b >= wMin1:
            b = rr(lim)
        
        z = pow(b, m, w)
        if (z==1) or (z==wMin1):
            continue
        composite=True
        for j in range(1, a):
            z = (z**2) % w
            
            if z==wMin1:
                composite=False
                break
            if z==1:
                break
        if composite:
            return 0
    return 1

def generatePrime(length):
    uplim = 1<<length
    lowlim = 1<<(length-1)

    while(True):
        m = rr(lowlim, uplim)
        m |= 1
        while(m<uplim):
            notPrime = False
            for p in primes:
                if m%p==0:
                    notPrime = True
                    break

            if not notPrime and MillerRabin(m):
                return m
            m += 2  