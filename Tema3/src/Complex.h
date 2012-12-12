/*
 * Complex.h
 *
 *  Created on: Dec 11, 2012
 *      Author: Constantin Șerban-Rădoi
 */
#pragma once
#include <iostream>

class Complex
{
public:
	Complex();
	Complex(double re, double im);
	~Complex();
	Complex& operator= (const Complex &nr2);
	Complex& operator+= (const Complex &nr2);
	Complex& operator-= (const Complex &nr2);
	Complex& operator*= (const Complex &nr2);

	const Complex operator+ (const Complex &nr2) const;
	const Complex operator- (const Complex &nr2) const;
	const Complex operator* (const Complex &nr2) const;
	bool operator== (const Complex &nr2) const;
	bool operator!= (const Complex &nr2) const;

	double module ();
	double squareModule ();

	friend std::ostream& operator<< (std::ostream& os, const Complex &nr);
	friend std::istream& operator>> (std::istream& is, Complex &nr);

public:
	double re, im;
};
