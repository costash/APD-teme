/*
 * Complex.cpp
 *
 *  Created on: Dec 11, 2012
 *      Author: Constantin Șerban-Rădoi
 */

#include "Complex.h"
#include <cmath>

Complex::Complex() :
	re(0), im(0)
{
}

Complex::Complex(double re, double im) :
	re(re), im(im)
{
}

Complex::~Complex()
{
}

Complex& Complex::operator= (const Complex &nr2)
{
	if (this != &nr2)
	{
		re = nr2.re;
		im = nr2.im;
	}
	return *this;
}

Complex& Complex::operator+= (const Complex &nr2)
{
	re += nr2.re;
	im += nr2.im;

	return *this;
}

Complex& Complex::operator-= (const Complex &nr2)
{
	re -= nr2.re;
	im -= nr2.im;

	return *this;
}

Complex& Complex::operator*= (const Complex &nr2)
{
	double temp = re;
	re = re * nr2.re - im * nr2.im;
	im = temp * nr2.im + im * nr2.re;

	return *this;
}

const Complex Complex::operator+ (const Complex &nr2) const
{
	return Complex(*this) += nr2;
}

const Complex Complex::operator- (const Complex &nr2) const
{
	return Complex(*this) -= nr2;
}

const Complex Complex::operator* (const Complex &nr2) const
{
	return Complex(*this) *= nr2;
}

bool Complex::operator== (const Complex &nr2) const
{
	return re == nr2.re && im == nr2.im;
}

bool Complex::operator!= (const Complex &nr2) const
{
	return re != nr2.re || im != nr2.im;
}

std::ostream& operator<< (std::ostream& os, const Complex &nr)
{
	os << "(" << nr.re << "," << nr.im << ")";
	return os;
}

double Complex::module ()
{
	return sqrt(squareModule());
}

double Complex::squareModule ()
{
	return (re * re) + (im * im);
}

std::istream& operator>> (std::istream& is, Complex &nr)
{
	is >> nr.re >> nr.im;
	return is;
}
