#pragma once
#include<string>

#ifndef BIGINT
#define BIGINT
using u64 = unsigned long long;

class bigint
{
public:
	unsigned size; /*number of allocated digits */
	unsigned hd; /* number of highest digit */
	u64* data; /* 64bit digits*/

	explicit bigint(unsigned n = 0);
	bigint(const bigint &b);
	bigint(bigint &&b);
	explicit bigint(const std::string &str);
	~bigint();
	bigint& operator=(const bigint &b);
	bigint& operator=(bigint &&b);

	int length() const;
	void resize(unsigned n);
	std::string hex() const;

	bool isUnity() const;
	bool isZero() const;

	void conlog() const;
};

long long cmp(const bigint &a, const bigint &b);
void lshift(const bigint &a, unsigned n, bigint &b);
void rshift(const bigint &a, unsigned n, bigint &b);
void mul(const bigint &a, const u64 b, bigint &c);
void mul(const bigint &a, const bigint &b, bigint &c);
void mulMod(const bigint &a, const bigint &b, const bigint &myu, const bigint &m, bigint &c);
void powMod(const bigint &a, const bigint &b, const bigint &myu, const bigint &m, bigint &c);
void reduction(const bigint &a, const bigint &m, const bigint &myu, bigint &b);
void div(const bigint &a, const bigint &b, bigint &q, bigint &r);
bigint myuInit(const bigint &m);
void sub(const bigint &a, const bigint &b, bigint &c);
u64 add(const bigint &a, const bigint &b, bigint &c);
u64 inc(bigint &a);
void generateRandom(bigint &a, unsigned length);
bigint gcd(const bigint &a, const bigint &b);
bool MillerRabin(const bigint &w, unsigned testnum);
void inverse(const bigint &a, const bigint &myu, const bigint &m, bigint &inv);
bigint generatePrime(unsigned length, unsigned testnum = 40);
bigint generatePrime1(unsigned length, unsigned testnum = 40);

#endif //BIGINT