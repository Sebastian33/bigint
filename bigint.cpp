#include"bigint.h"
#include<intrin.h>
#include<iostream>
#include<fstream>
#include<random>
#include<chrono>

int len(u64 a)
{
	int res(static_cast<int>((a >> (      32)) > 0) << 5);
	res +=  static_cast<int>((a >> (res + 16)) > 0) << 4;
	res +=  static_cast<int>((a >> (res +  8)) > 0) << 3;
	res +=  static_cast<int>((a >> (res +  4)) > 0) << 2;
	res +=  static_cast<int>((a >> (res +  2)) > 0) << 1;
	res +=  static_cast<int>((a >> (res +  1)) > 0);
	return res + 1;
}

int length(const u64* a, unsigned n)
{
	const u64* aa(a + n - 1);
	while ((*aa == 0) && (aa != a))
	{
		aa--;
	}
	return len(*aa) + ((aa - a) << 6);
}

void clog(const u64* a, unsigned n)
{
	for (int i = 0; i < n; i++)
	{
		std::cout << std::hex << a[i] << std::endl;
	}
	std::cout << std::endl;
}

u64 rawAdd(const u64* a, const u64* b, u64* c, unsigned n)
{
	u64 tmpa, tmpb;
	u64 carry(0);
	for (int i = 0; i < n; i++)
	{
		tmpa = a[i];
		tmpb = b[i];
		c[i] = tmpa + tmpb + carry;
		carry = (tmpa & tmpb) | ((tmpa | tmpb) & (~c[i]));
		carry >>= 63;
	}
	return carry;
}

u64 rawSub(const u64* a, const u64* b, u64* c, unsigned n)
{
	u64 tmpa, tmpb;
	u64 carry(0);
	for (int i = 0; i < n; i++)
	{
		tmpa = a[i];
		tmpb = b[i];
		c[i] = tmpa - tmpb - carry;
		carry = (tmpa < tmpb) | ((tmpa - tmpb) < carry);
	}
	return carry;
}

bigint::bigint(unsigned n):size(n), hd(0)
{
	data = new u64[size];
	memset(data, 0, size << 3);
}

bigint::bigint(const bigint &b):size(b.size), hd(b.hd)
{
	data = new u64[size];
	memcpy(data, b.data, size << 3);
}

bigint::bigint(bigint &&b):size(b.size), hd(b.hd), data(b.data)
{
	b.size = b.hd = 0;
}

bigint::bigint(const std::string &str)
{
	//TODO: add empty string check
	unsigned len(str.size()), curdgt(0), offset(0);

	u64 tmp(0);
	hd = (len >> 4) + static_cast<unsigned>((len & 0xf) != 0) - 1;
	size = hd + 1;
	data = new u64[size];
	memset(data, 0, size << 3);

	for (int i = len - 1; i >= 0; i--)
	{
		switch (str[i])
		{
		case 'a':
		case 'A':
		{
			tmp = 0xa;
			break;
		}
		case 'b':
		case 'B':
		{
			tmp = 0xb;
			break;
		}
		case 'c':
		case 'C':
		{
			tmp = 0xc;
			break;
		}
		case 'd':
		case 'D':
		{
			tmp = 0xd;
			break;
		}
		case 'e':
		case 'E':
		{
			tmp = 0xe;
			break;
		}
		case 'f':
		case 'F':
		{
			tmp = 0xf;
			break;
		}
		default:
			tmp = str[i] - '0';
		}
		data[curdgt] |= tmp << offset;
		offset += 4;
		if (offset == 64)
		{
			offset = 0;
			curdgt++;
		}
	}
}

bigint::~bigint()
{
	if (size != 0)
		delete[] data;
}

bool bigint::isUnity() const
{
	return (hd == 0) && (data[0] == 1);
}

bool bigint::isZero() const
{
	u64 res(0);
	for (int i = 0; i <= hd; i++)
	{
		res |= data[i];
	}
	return res == 0;
}

void bigint::resize(unsigned n)
{
	u64* tmp = new u64[n];
	memcpy(tmp, data, n << 3);
	if(size)
		delete[]data;
	size = n;
	data = tmp;
}

std::string bigint::hex() const
{
	if ((hd==0)&&(data[0]==0))
		return "0";

	const char hextable[] = { '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f' };
	std::string res = "";
	int offset(60), curdgt(hd), mask(0xf);

	while (((data[curdgt] >> offset) & mask) == 0)
	{
		offset -= 4;
	}

	while (curdgt >= 0)
	{
		res += hextable[(data[curdgt] >> offset) & mask];
		offset -= 4;
		if (offset < 0)
		{
			curdgt--;
			offset = 60;
		}
	}

	return res;
}

void bigint::conlog() const
{
	std::cout << std::dec << size << ' ' << hd << std::endl;
	clog(data, size);
}

bigint& bigint::operator=(const bigint &b)
{
	if (this != &b)
	{
		if ((b.hd + 1) > size)
		{
			delete[] data;
			size = b.hd + 1;
			data = new u64[size];
		}
		hd = b.hd;
		memcpy(data, b.data, (hd + 1) << 3);
	}
	return *this;
}

bigint& bigint::operator=(bigint &&b)
{
	if (this != &b)
	{
		if (size != 0)
			delete[] data;
		size = b.size;
		hd = b.hd;
		data = b.data;
		b.hd = b.size = 0;
	}
	return *this;
}

void sub(const bigint &a, const bigint &b, bigint &c)
{
	unsigned size((a.hd > b.hd ? a.hd : b.hd) + 1);
	if (size > c.size)
		c.resize(size);
	bigint subtr(size);
	subtr = b;
	c = a;
	rawSub(c.data, subtr.data, c.data, size);
	int i(size - 1);
	while ((c.data[i] == 0) && (i > 0))
	{
		i--;
	}
	c.hd = i;
}

/* a++ */
u64 inc(bigint &a)
{
	bool carry(1);
	u64 *adata(a.data), *fin(a.data + a.hd + 1);
	while (carry && (adata < fin))
	{
		(*adata)++;
		carry = *adata == 0;
		adata++;
	}
	if ((adata == fin) && carry && (fin < a.data + a.size))
	{
		*adata = 1;
		a.hd++;
		carry = 0;
	}
	return static_cast<u64>(carry);
}

u64 add(const bigint &a, const bigint &b, bigint &c)
{
	u64 *pl, *ps;
	unsigned lsize, ssize;
	if (a.hd < b.hd)
	{
		pl = b.data;
		ps = a.data;
		lsize = b.hd + 1;
		ssize = a.hd + 1;
	}
	else
	{
		pl = a.data;
		ps = b.data;
		lsize = a.hd + 1;
		ssize = b.hd + 1;
	}
	if (lsize > c.size)
		c.resize(lsize);
	memcpy(c.data, ps, ssize * sizeof(u64));
	return rawAdd(pl, c.data, c.data, lsize);
}

/* |c|=|a|+1 */
void mul(const bigint &a, const u64 b, bigint& c)
{
	u64 prodl(0), tmp;
	c.data[0] = 0;
	int i(0);
	for (; i <= a.hd; i++)
	{
		prodl = _umul128(a.data[i], b, c.data + i + 1);
		tmp = c.data[i];
		c.data[i] += prodl;
		c.data[i + 1] += ((prodl & tmp) | ((prodl | tmp) & (~c.data[i]))) >> 63;
	}
	c.hd = a.hd + static_cast<unsigned>(c.data[i] != 0);
}

void mul(const bigint &a, const bigint& b, bigint& c)
{
	bigint tmp(a.hd + 2);
	u64 *cd(c.data);
	const u64 *bd(b.data), *fin(b.data + b.hd + 1);
	memset(cd, 0, (c.hd + 1) << 3);
	for (; bd < fin; bd++)
	{
		mul(a, *bd, tmp);
		rawAdd(cd, tmp.data, cd, tmp.size);
		cd++;
	}
	c.hd = a.hd + b.hd + 1;
	c.hd -= static_cast<unsigned>(c.data[c.hd] == 0);
}

void mulMod(const bigint &a, const bigint &b, const bigint &myu, const bigint &m, bigint &c)
{
	bigint prod(a.hd + b.hd + 2);
	mul(a, b, prod);
	int cmp(cmp(prod, m));
	if (cmp > 0)
	{
		reduction(prod, m, myu, c);
	}
	else if (cmp < 0)
	{
		memcpy(c.data, prod.data, (prod.hd + 1) << 3);
		c.hd = prod.hd;
	}
	else 
	{
		c.data[0] = 0;
		c.hd = 0;
	}
}

/* c = a^b mod m */
void powMod(const bigint &a, const bigint &b, const bigint &myu, const bigint &m, bigint &c)
{
	int i(b.length() - 2);
	int curdgt(b.hd);
	int offset;
	bigint aa(a), tmp(m.hd + 1);
	tmp = aa;
	bigint *state(&tmp), *res(&c), *swap;
	
	for (; i >= 0; i--)
	{
		
		offset = i & 63;
		curdgt -= static_cast<int>(offset == 63);
	
		mulMod(*state, *state, myu, m, *res);
		swap = res;
		res = state;
		state = swap;
		
		if ((b.data[curdgt] >> offset) & 1)
		{
			mulMod(*state, aa, myu, m, *res);
			swap = res;
			res = state;
			state = swap;
		}
		
	}
	c = *state;
}

/*barret reduction
b = a mod m 
m < a < m^2 */
void reduction(const bigint &a, const bigint &m, const bigint &myu, bigint &b)
{
	unsigned msize(m.hd + 1), asize(a.hd + 1), mlen(m.length()); 
	bigint tmp1(asize), tmp2(asize+2);

	/*tmp1 = a / 2^(|m| - 1)  */
	rshift(a, mlen - 1, tmp1);

	/* tmp2 = tmp1 * myu / 2^( |m| + 1)	*/
	mul(tmp1, myu, tmp2);
	rshift(tmp2, mlen + 1, tmp1);

	/* c = a - tmp2*m	*/
	mul(tmp1, m, tmp2);
	//tmp2.conlog();
	sub(a, tmp2, tmp1);
	/*90% probability that this condition is false */
	if (cmp(tmp1, m) > 0)
	{
		sub(tmp1, m, tmp1);
		if (cmp(tmp1, m) > 0)
			sub(tmp1, m, tmp1);
	}

	b.hd = tmp1.hd;
	memcpy(b.data, tmp1.data, (b.hd + 1) << 3);
}

/* 
1  a > b
-1 a < b
0  a == b*/
long long cmp(const bigint &a, const bigint &b)
{
	long long res(static_cast<long long>(a.hd > b.hd) - static_cast<long long>(a.hd < b.hd));
	int i(a.hd);

	while ((res == 0) && (i >= 0))
	{
		res = static_cast<long long>(a.data[i] > b.data[i]) - static_cast<long long>(a.data[i] < b.data[i]);
		i--;
	}
	return res;
}

int bigint::length() const
{
	return len(data[hd]) + (hd << 6);
}

void lshift(const bigint &a, unsigned n, bigint &b)
{
	int offset(n >> 6);
	b.hd = 0;
	n = n & 0x3f; // mod 64
	int mn(64 - n);
	u64 mask = -static_cast<long long>(n != 0); // for cases when n = 64*k
	unsigned newsize(a.hd + 1 + offset + static_cast<unsigned>(n != 0));
	if (b.size < newsize)
		b.resize(newsize);

	b.data[newsize - 1] = 0;
	b.data[offset] = a.data[0] << n;
	for (int i = 1; i <= a.hd; i++)
	{
		b.data[offset + i] = (a.data[i] << n) | ((a.data[i - 1] >> mn) & mask);
	}
	b.data[newsize - 1] |= (a.data[a.hd] >> mn) & mask;
	b.hd = newsize - 1 - static_cast<unsigned>(b.data[newsize - 1] == 0);
}

void rshift(const bigint &a, unsigned n, bigint &b)
{
	int offset(n >> 6), size(a.hd + 1 - offset);
	if (b.size < size)
		b.resize(size);
	memset(b.data, 0, b.size << 3);
	if (size <= 0)
		return;

	n = n & 0x3f;
	int mn(64 - n);
	u64 mask = -static_cast<long long>(n != 0);

	b.data[size - 1] = a.data[a.hd] >> n;
	for (int i = size - 2; i >= 0; i--)
	{
		b.data[i] = (a.data[i + offset] >> n) | ((a.data[i + 1 + offset] << mn) & mask);
	}

	b.hd = size - 1; 
	b.hd -= static_cast<unsigned>((b.data[b.hd] == 0) && (b.hd != 0));
}

/* a = b*q + r */
void div(const bigint &a, const bigint &b, bigint &q, bigint &r)
{
	int blen(b.length());
	int qsize(a.hd - b.hd + 1);
	if (qsize <= 0)
	{
		q.hd = 0;
		q.data[0] = 0;
		r = a;
		return;
	}
	else if (q.size < qsize)
		q.resize(qsize);
	bigint min(a), sbt(a.hd + 1);// min == minuend, вычитаемое
	memset(q.data, 0, q.size << 3);
	int shift(0);
	while (cmp(min, b) >= 0)
	{
		shift = min.length() - blen;
		lshift(b, shift, sbt);
		if (cmp(min, sbt) < 0)
		{
			shift--;
			lshift(b, shift, sbt);
		}
		sub(min, sbt, min);
		q.data[shift >> 6] |= 1ULL << (shift & 0x3f);
	}
	q.hd = qsize - 1;
	q.hd -= static_cast<unsigned>((q.data[q.hd] == 0) && (q.hd != 0));
	r = min;
}

/*myu = 2^2|m| / m, |myu| = |m| + 1 */
bigint myuInit(const bigint &m)
{
	unsigned mlen(m.length());
	unsigned dmlen(mlen * 2);
	unsigned myuSize((mlen >> 6) + 1);
	bigint myu(myuSize);
	bigint tmp(2 * myuSize), r(m.size);
	/* tmp = 2^2|m|*/
	tmp.hd = dmlen >> 6;
	tmp.data[tmp.hd] = 1ULL << (dmlen & 0x3f); 
	div(tmp, m, myu, r);
	return myu;
}

/*greatest common divisor*/
bigint gcd(const bigint &a, const bigint &b)
{
	unsigned size((a.hd > b.hd ? a.hd : b.hd) + 1);
	bigint aa(size), bb(size), q(size), buf(size);
	aa = a;
	bb = b;
	bigint *ro(&aa), *r(&bb), *tmp(&buf), *swap;

	/*while r != 0 */
	while ((r->data[0] != 0) || (r->hd != 0))
	{
		/* r_old = q*r + tmp*/
		div(*ro, *r, q, *tmp);
		swap = ro;
		ro = r;
		r = tmp;
		tmp = swap;
	}
	return *ro;
}

/*length in bits*/
void generateRandom(bigint &a, unsigned length)
{
	unsigned size(length / 64);
	length &= 63;
	size += static_cast<unsigned>(length > 0);
	if (a.size < size)
		a.resize(size);

	auto time = std::chrono::high_resolution_clock::now();
	std::mt19937_64 generator(time.time_since_epoch().count()); 
	std::uniform_int_distribution<u64> dstr;
	for (int i = 0; i < size; i++)
		a.data[i] = dstr(generator);
	a.hd = size - 1;

	if (length > 0)
		a.data[a.hd] &= (1ULL << length) - 1;
}

/*0 - composite, 1 - prime with error probability 4^-n
According to FIPS 186-4*/
bool MillerRabin(const bigint &w, unsigned testnum)
{
	if (!(w.data[0] & 1))
		return false;
	
	bigint wMin1(w);
	wMin1.data[0] ^= 1;

	int i(0);
	while ((wMin1.data[i] == 0) && (i <= wMin1.hd))
	{
		i++;
	}
	int a(1);
	while (((wMin1.data[i] >> a) & 1) == 0)
	{
		a++;
	}
	a += i * 64;
	bigint m(w.hd + 1);
	/*m = (w - 1) / 2^a */
	rshift(w, a, m);
	int wlen(w.length());
	int wsize(w.hd + 1);

	bigint b(wsize);
	bigint z(wsize);
	bigint myu = myuInit(w);
	bool composite;
	for (i = 0; i < testnum; i++)
	{
		do
		{
			generateRandom(b, wlen);
		} while (cmp(b, wMin1) >= 0);

		if (!gcd(b, w).isUnity())
			return false;
		/* z=b^m mod w */
		powMod(b, m, myu, w, z);
		if (z.isUnity() || (cmp(z, wMin1) == 0))
			continue;

		composite = true;
		for (int j = 1; j < a; j++)
		{
			/*z=z^2 mod w*/
			mulMod(z, z, myu, w, z);
			if (cmp(z, wMin1) == 0)
			{
				composite = false;
				break;
			}
			if (z.isUnity())
				break;
		}
		if (composite)
			return false;
	}
	return true;
}

/* i = a^-1 mod m, m - prime
yeah, yeah, not the best way of computing inverse element*/
void inverse(const bigint &a, const bigint &myu, const bigint &m, bigint &inv)
{
	/* pw = m - 2 */
	bigint pw(m);
	bool carry = pw.data[0] == 1 ? 1 : 0;
	pw.data[0] -= 2;
	int i = 1;
	while (carry && (i <= pw.hd))
	{
		carry = pw.data[i] == 0;
		pw.data[i]--;
		i++;
	}
	pw.hd -= static_cast<u64>(pw.data[pw.hd] == 0);
	powMod(a, pw, myu, m, inv);
}

bigint generatePrime(unsigned length, unsigned testnum)
{
	unsigned size(length >> 6);
	size += (length & 0x3F) > 0 ? 1 : 0;
	bigint m(size);
	bigint two(size);
	two.data[0] = 2;
	two.hd = 0;
	while(true)
	{
		generateRandom(m, length);
		m.data[0] |= 1;
		m.data[size - 1] |= 1ULL << (length & 0x3F);
		do
		{
			if (MillerRabin(m, testnum))
			{
				return m;
			}
		} while (!rawAdd(m.data, two.data, m.data, size));
	}
}