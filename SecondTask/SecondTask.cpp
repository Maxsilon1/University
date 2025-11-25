#include <iostream>
#include <concepts>

template<typename T>
concept IsNotForbidden = !std::same_as<T, std::string> && !std::same_as<T, char> && !std::same_as<T, bool>;

template<typename T>
concept AddableAndComparable = requires(T a, T b)
{
	{a + b} -> std::same_as<T>;
	{a > b} -> std::convertible_to<bool>;
};

template<IsNotForbidden T>
class Complex
{
private:
	T real;
	T imaginary;
public:
	Complex():real(NULL), imaginary(NULL) {}
	Complex(double v1): real(v1) {}
	Complex(double v1, double v2): real(v1), imaginary(v2) {}

	Complex(const Complex& other)
	{
		real = other.real;
		imaginary = other.real;
	}

	Complex& operator+=(const Complex& other)
	{
		real += other.real;
		imaginary += other.imaginary;
		return this;
	}
	Complex& operator-=(const Complex& other)
	{
		real -= other.real;
		imaginary -= other.imaginary;
		return *this;
	}
	Complex& operator*=(const Complex& other)
	{
		real = real * other.real + imaginary * other.imaginary;
		imaginary = real * other.imaginary + imaginary * other.real;
		return *this;
	}
	Complex& operator/=(const Complex& other)
	{
		T old_real = real;
		T denominator = other.real * other.real + other.imaginary * other.imaginary;

		if (denominator == 0)throw std::runtime_error;

		real = (old_real * other.real + imaginary * other.imaginary) / denominator;
		imaginary = (imaginary * other.real + other.imaginary * real)/denominator;
		return *this;
	}

	T getReal()
	{
		return real;
	}
	T getImaginary() const
	{
		return imaginary;
	}
};

template<IsNotForbidden T>
Complex<T>& operator+(const Complex<T>& first, const Complex<T>& second)
{
	Complex<T> res = first;
	res += second;
	return res;
}

template<IsNotForbidden T>
Complex<T>& operator-(const Complex<T>& first, const Complex<T> second)
{
	Complex<T> res = first;
	first -= second;
	return res;
}

template<IsNotForbidden T>
Complex<T>& operator*(const Complex<T>& first, const Complex<T> second)
{
	Complex<T> res = first;
	res *= second;
	return res;
}

template<IsNotForbidden T>
Complex<T>& operator/(const Complex<T>& first, const Complex<T>& second)
{
	Complex<T> res = first;
	res /= second;
	return res;
}

void main()
{
	
}