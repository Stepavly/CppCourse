#include "big_integer.h"
#include <stdexcept>
#include <algorithm>
#include <functional>
#include <utility>
#include <iostream>

bool big_integer::positive() const {
	return sign;
}
big_integer abs(const big_integer &a) {
	return a < 0 ? -a : a;
}

std::pair<uint32_t, uint32_t> add(uint32_t a, uint32_t b) {
	std::pair<uint32_t, uint32_t> result(0u, a + b);

	if (0xFFFFFFFFu - a < b) {
		result.first = 1;
	}

	return result;
}
std::pair<uint32_t, uint32_t> mult(uint32_t a, uint32_t b) {
	std::pair<uint32_t, uint32_t> result(0u, 0u);

	if (b & 1u) {
		result.second = a;
	}

	for (uint32_t bit = 1, high_mask = 1u << 31u; bit < 32u; high_mask |= 1u << (31 - bit), bit++) {
		if (b >> bit & 1u) {
			uint32_t high = (a & high_mask) >> (32 - bit);
			uint32_t low = (a & (~high_mask)) << bit;

			std::pair<uint32_t, uint32_t> sum = add(result.second, low);
			result.first += high + sum.first;
			result.second = sum.second;
		}
	}

	return result;
}

big_integer::big_integer() : sign(true), dig(1, 0u) {}
big_integer::big_integer(const big_integer &other) : sign(other.sign), dig(other.dig) {}
big_integer::big_integer(int a) {
	sign = a >= 0;
	uint32_t b = (a == std::numeric_limits<int>::min() ?
								(uint32_t) std::numeric_limits<int>::max() + 1u :
								(uint32_t) (a < 0 ? -a : a));
	dig = std::vector<uint32_t>();
	dig.push_back(b);
}
big_integer::big_integer(uint32_t a) : sign(true) {
	dig = std::vector<uint32_t>();
	dig.push_back(a);
}
big_integer::big_integer(const std::string &str) {
	if (str.empty()) {
		throw std::length_error("can not create big_int from empty string");
	}
	sign = str[0] != '-';
	big_integer a;

	for (size_t i = isdigit(str[0]) ? 0 : 1; i < str.size(); i++) {
		if (!isdigit(str[i])) {
			throw std::logic_error("string must contain only digits");
		}

		a *= 10;
		a += str[i] - '0';
	}

	if (a == 0) {
		sign = 0;
	}

	*this = sign ? a : -a;
}
big_integer::big_integer(bool sign, std::vector<uint32_t> digits) : sign(sign), dig(std::move(digits)) {}
big_integer::~big_integer() {}

big_integer &big_integer::operator=(big_integer const &other) {
	if (this == &other) {
		return *this;
	}
	this->sign = other.sign;
	this->dig = std::vector<uint32_t>(other.dig);

	return *this;
}

int compare(const big_integer &a, const big_integer &b) {
	if (a.dig.size() > b.dig.size()) {
		return a.positive() ? +1 : -1;
	} else if (a.dig.size() < b.dig.size()) {
		return b.positive() ? -1 : +1;
	} else {
		if (a.positive() != b.positive()) {
			return a.positive() ? +1 : -1;
		}

		for (size_t i = a.dig.size() - 1;; i--) {
			if (a.dig[i] < b.dig[i]) {
				return a.positive() ? -1 : +1;
			} else if (a.dig[i] > b.dig[i]) {
				return a.positive() ? +1 : -1;
			}
			if (i == 0) {
				break;
			}
		}

		return 0;
	}
}

bool operator==(const big_integer &a, const big_integer &b) {
	return compare(a, b) == 0;
}
bool operator!=(const big_integer &a, const big_integer &b) {
	return compare(a, b) != 0;
}
bool operator<(const big_integer &a, const big_integer &b) {
	return compare(a, b) < 0;
}
bool operator>(const big_integer &a, const big_integer &b) {
	return compare(a, b) > 0;
}
bool operator<=(const big_integer &a, const big_integer &b) {
	return compare(a, b) <= 0;
}
bool operator>=(const big_integer &a, const big_integer &b) {
	return compare(a, b) >= 0;
}

void big_integer::normalize() {
	while (dig.size() > 1u && dig.back() == 0u) {
		dig.pop_back();
	}

	if (dig.size() == 1 && dig[0] == 0) {
		sign = true;
	}
}

big_integer big_integer::operator+() const {
	return *this;
}
big_integer big_integer::operator-() const {
	big_integer negative(*this);
	if (negative != 0) {
		negative.sign = !negative.sign;
	}
	return negative;
}
big_integer big_integer::operator~() const {
	return -(*this) - 1;
}

big_integer &big_integer::operator+=(big_integer const &rhs) {
	if (sign != rhs.sign) {
		return *this -= -rhs;
	}

	big_integer c;
	c.sign = sign;
	c.dig = std::vector<uint32_t>(std::max(dig.size(), rhs.dig.size()) + 2);
	uint32_t carry = 0;

	for (size_t i = 0; i < c.dig.size(); i++) {
		std::pair<uint32_t, uint32_t> curDig(0, carry);

		if (i < dig.size()) {
			curDig = add(carry, dig[i]);
		}

		if (i < rhs.dig.size()) {
			std::pair<uint32_t, uint32_t> temp = add(curDig.second, rhs.dig[i]);
			curDig = std::pair<uint32_t, uint32_t>(curDig.first + temp.first, temp.second);
		}

		carry = curDig.first;
		c.dig[i] = curDig.second;
	}

	c.normalize();

	return *this = c;
}
big_integer &big_integer::operator-=(big_integer const &rhs) {
	if (sign != rhs.sign) {
		return *this += -rhs;
	} else if ((positive() && *this < rhs) || (!positive() && *this > rhs)) {
		big_integer temp = rhs;
		return *this = -(temp -= *this);
	}

	big_integer c;
	c.sign = sign;
	c.dig = std::vector<uint32_t>(dig.size());

	uint32_t take = 0;

	for (size_t i = 0; i < dig.size(); i++) {
		bool high = false;
		uint32_t sub = 0;

		if (i < rhs.dig.size()) {
			sub = rhs.dig[i];
		}

		if (dig[i] < take || dig[i] - take < sub) {
			high = true;
		}

		c.dig[i] = dig[i] - sub - take;
		take = high;
	}

	c.normalize();

	return *this = c;
}
big_integer &big_integer::operator*=(big_integer const &rhs) {
	big_integer c;
	c.sign = sign == rhs.sign;
	c.dig = std::vector<uint32_t>(dig.size() + rhs.dig.size() + 1);

	for (size_t i = 0; i < dig.size(); i++) {
		uint32_t carry = 0;

		for (size_t j = 0; j < rhs.dig.size() || carry; j++) {
			auto aMulB = mult(dig[i], j < rhs.dig.size() ? rhs.dig[j] : 0); // a[i]*b[j]
			auto temp = add(aMulB.second, carry);
			aMulB = std::pair<uint32_t, uint32_t>(aMulB.first + temp.first, temp.second); // a[i]*b[j]+carry
			temp = add(aMulB.second, c.dig[i + j]);
			aMulB = std::pair<uint32_t, uint32_t>(aMulB.first + temp.first, temp.second); // a[i]*b[j]+carry+c[i+j]

			c.dig[i + j] = aMulB.second;
			carry = aMulB.first;
		}
	}

	c.normalize();

	return *this = c;
}
big_integer &big_integer::operator/=(big_integer const &rhs) {
	if (rhs == 0) {
		throw std::range_error("division by zero");
	} else if (abs(*this) < abs(rhs)) {
		return *this = big_integer(0);
	}

	big_integer c;
	c.sign = sign == rhs.sign;
	c.dig = std::vector<uint32_t>(dig.size());
	big_integer cur;
	cur.dig.clear();

	for (size_t i = dig.size() - 1;; i--) {
		cur.dig.insert(cur.dig.begin(), 0);
		cur += dig[i];

		uint32_t l = 0, r = 0xFFFFFFFFu;

		while (r - l > 1) {
			uint32_t m = l + (r - l) / 2;
			big_integer temp = abs(rhs * m);

//			for (auto x : cur.dig) {
//				std::cout << x << " ";
//			}
//
//			std::cout << std::endl;
//
//			for (auto x : temp.dig) {
//				std::cout << x << " ";
//			}
//
//			std::cout << std::endl;
//			std::cout << std::endl;

			if (cur >= abs(rhs * m)) {
				l = m;
			} else {
				r = m;
			}
		}

		if (cur >= abs(rhs * r)) {
			c.dig[i] = r;
		} else {
			c.dig[i] = l;
		}

		cur = cur - abs(rhs * c.dig[i]);

		if (i == 0) {
			break;
		}
	}

	c.normalize();

	return *this = c;
}
big_integer &big_integer::operator%=(big_integer const &rhs) {
	return *this = *this - *this / rhs * rhs;
}

big_integer operator+(big_integer a, const big_integer &b) {
	return a += b;
}
big_integer operator-(big_integer a, const big_integer &b) {
	return a -= b;
}
big_integer operator*(big_integer a, const big_integer &b) {
	return a *= b;
}
big_integer operator/(big_integer a, const big_integer &b) {
	return a /= b;
}
big_integer operator%(big_integer a, const big_integer &b) {
	return a %= b;
}

big_integer &big_integer::operator&=(big_integer const &rhs) {
	for (size_t i = 0; i < dig.size(); i++) {
		dig[i] = (sign ? +1 : -1) * dig[i] & (i < rhs.dig.size() ? (rhs.sign ? +1 : -1) * rhs.dig[i] : 0);
	}
	normalize();
	return *this;
}
big_integer &big_integer::operator|=(big_integer const &rhs) {
	if (dig.size() < rhs.dig.size()) {
		dig.resize(rhs.dig.size());
	}
	for (size_t i = 0; i < dig.size(); i++) {
		uint32_t a = (sign ? dig[i] : ~dig[i] + 1);
		uint32_t b = (i < rhs.dig.size() ? (rhs.sign ? rhs.dig[i] : ~rhs.dig[i] + 1) : 0);

		uint32_t c = a | b;

		if (sign != rhs.sign) {
			c = ~c + 1;
		}

		dig[i] = c;
	}
	sign = sign == rhs.sign;

	normalize();
	return *this;
}
big_integer &big_integer::operator^=(big_integer const &rhs) {
	if (dig.size() < rhs.dig.size()) {
		dig.resize(rhs.dig.size());
	}
	for (size_t i = 0; i < dig.size(); i++) {
		uint32_t a = (sign ? dig[i] : ~dig[i] + 1);
		uint32_t b = (i < rhs.dig.size() ? (rhs.sign ? rhs.dig[i] : ~rhs.dig[i] + 1) : 0);

		uint32_t c = a ^b;

		if (sign != rhs.sign) {
			c = ~c + 1;
		}

		dig[i] = c;
	}
	sign = sign == rhs.sign;

	normalize();
	return *this;
}

big_integer &big_integer::operator<<=(uint32_t rhs) {
	return *this = *this << rhs;
}
big_integer &big_integer::operator>>=(uint32_t rhs) {
	return *this = *this >> rhs;
}

big_integer operator&(big_integer a, const big_integer &b) {
	return a &= b;
}
big_integer operator|(big_integer a, const big_integer &b) {
	return a |= b;
}
big_integer operator^(big_integer a, const big_integer &b) {
	return a ^= b;
}

big_integer operator<<(big_integer a, uint32_t b) {
	while (b > 0) {
		a *= 2;
		b--;
	}

	return a;
}
big_integer operator>>(big_integer a, uint32_t b) {
	while (b > 0) {
		if (a.sign) {
			a /= 2;
		} else {
			a = (a - 1) / 2;
		}
		b--;
	}

	return a;
}

big_integer &big_integer::operator++() {
	return *this = *this + 1;
}
big_integer big_integer::operator++(int) {
	big_integer copy(*this);
	++*this;
	return copy;
}
big_integer &big_integer::operator--() {
	return *this = *this + 1;
}
big_integer big_integer::operator--(int) {
	big_integer copy(*this);
	++*this;
	return copy;
}

std::string to_string(big_integer a) {
	std::string result;
	std::string sign = a.positive() ? "" : "-";

	a = abs(a);
	size_t needZero = 0;

	do {
		std::string d = std::to_string((a % 1'000'000'000).dig[0]);
		std::string t = std::string(needZero, '0');
		needZero = 9u - d.size();
		d += t;
		std::reverse(d.begin(), d.end());
		result += d;
		a /= 1'000'000'000;
	} while (a != 0);

	result += sign;

	std::reverse(result.begin(), result.end());
	return result;
}

std::ostream &operator<<(std::ostream &s, const big_integer &a) {
	s << to_string(a);
	return s;
}