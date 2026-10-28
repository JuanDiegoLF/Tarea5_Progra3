#include <cmath>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

class Expression;
using ExprPtr = std::shared_ptr<Expression>;

class Expression {
public:
    virtual ~Expression() = default;
    virtual double evaluate(double x) const = 0;
    virtual ExprPtr derivative() const = 0;   // derivada respecto a x
    virtual std::string toString() const = 0;
};

class Constant : public Expression {
    double value_;
public:
    explicit Constant(double v) : value_(v) {}
    double evaluate(double) const override { return value_; }
    ExprPtr derivative() const override { return std::make_shared<Constant>(0); }
    std::string toString() const override {
        std::ostringstream os;
        os << value_;
        return os.str();
    }
};

class Variable : public Expression {
public:
    double evaluate(double x) const override { return x; }
    ExprPtr derivative() const override { return std::make_shared<Constant>(1); }
    std::string toString() const override { return "x"; }
};

class BinaryOperation : public Expression {
protected:
    ExprPtr left_, right_;
    virtual double apply(double a, double b) const = 0;
    virtual std::string symbol() const = 0;
public:
    BinaryOperation(ExprPtr l, ExprPtr r) : left_(std::move(l)), right_(std::move(r)) {}
    double evaluate(double x) const override {
        return apply(left_->evaluate(x), right_->evaluate(x));
    }
    std::string toString() const override {
        return "(" + left_->toString() + " " + symbol() + " " + right_->toString() + ")";
    }
};

class Add : public BinaryOperation {
    double apply(double a, double b) const override { return a + b; }
    std::string symbol() const override { return "+"; }
public:
    using BinaryOperation::BinaryOperation;
    // (u + v)' = u' + v'
    ExprPtr derivative() const override {
        return std::make_shared<Add>(left_->derivative(), right_->derivative());
    }
};

class Multiply : public BinaryOperation {
    double apply(double a, double b) const override { return a * b; }
    std::string symbol() const override { return "*"; }
public:
    using BinaryOperation::BinaryOperation;
    // (u * v)' = u' * v + u * v'
    ExprPtr derivative() const override {
        return std::make_shared<Add>(
            std::make_shared<Multiply>(left_->derivative(), right_),
            std::make_shared<Multiply>(left_, right_->derivative()));
    }
};

class Power : public BinaryOperation {
    double apply(double a, double b) const override { return std::pow(a, b); }
    std::string symbol() const override { return "^"; }
public:
    using BinaryOperation::BinaryOperation;
    // Regla de la potencia (exponente constante): (u^n)' = n * u^(n-1) * u'
    ExprPtr derivative() const override {
        auto nMinus1 = std::make_shared<Add>(right_, std::make_shared<Constant>(-1));
        return std::make_shared<Multiply>(
            std::make_shared<Multiply>(right_, std::make_shared<Power>(left_, nMinus1)),
            left_->derivative());
    }
};

class UnaryFunction : public Expression {
protected:
    ExprPtr arg_;
    virtual double apply(double u) const = 0;
    virtual std::string name() const = 0;
    virtual ExprPtr outerDerivative() const = 0;   // f'(u)
public:
    explicit UnaryFunction(ExprPtr a) : arg_(std::move(a)) {}
    double evaluate(double x) const override { return apply(arg_->evaluate(x)); }
    std::string toString() const override { return name() + "(" + arg_->toString() + ")"; }
    // Regla de la cadena: (f(u))' = f'(u) * u'
    ExprPtr derivative() const override {
        return std::make_shared<Multiply>(outerDerivative(), arg_->derivative());
    }
};

class Sin : public UnaryFunction {
    double apply(double u) const override { return std::sin(u); }
    std::string name() const override { return "sin"; }
    ExprPtr outerDerivative() const override;
public:
    using UnaryFunction::UnaryFunction;
};

class Cos : public UnaryFunction {
    double apply(double u) const override { return std::cos(u); }
    std::string name() const override { return "cos"; }
    ExprPtr outerDerivative() const override;
public:
    using UnaryFunction::UnaryFunction;
};

ExprPtr Sin::outerDerivative() const { return std::make_shared<Cos>(arg_); }
ExprPtr Cos::outerDerivative() const {
    return std::make_shared<Multiply>(std::make_shared<Constant>(-1),
                                      std::make_shared<Sin>(arg_));
}

int main() {
    auto x = std::make_shared<Variable>();

    auto expr =
        std::make_shared<Multiply>(
            std::make_shared<Add>(
                std::make_shared<Power>(
                    x,
                    std::make_shared<Constant>(2)
                ),
                std::make_shared<Multiply>(
                    std::make_shared<Constant>(3),
                    x
                )
            ),
            std::make_shared<Sin>(x)
        );

    auto deriv = expr->derivative();

    std::cout << "f(x) = " << expr->toString() << std::endl;
    std::cout << "f'(x) = " << deriv->toString() << std::endl;

    std::cout << "f(2) = " << expr->evaluate(2) << std::endl;
}
