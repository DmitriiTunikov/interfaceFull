#ifndef VECTOR_H
#define VECTOR_H

#include "IVector.h"

class SHARED_EXPORT Vector : IVector
{
public:
    int getId() const
    {
        return INTERFACE_0;
    }

    /*factories*/
    static IVector* createVector(unsigned int size, double const* vals);

    /*operations*/
    int add(IVector const* const right);
    int subtract(IVector const* const right);
    int multiplyByScalar(double scalar);
    int dotProduct(IVector const* const right, double& res) const;

    /*static operations*/
    static IVector* add(IVector const* const left, IVector const* const right);
    static IVector* subtract(IVector const* const left, IVector const* const right);
    static IVector* multiplyByScalar(IVector const* const left, double scalar);

    /*comparators*/
    int gt(IVector const* const right, NormType type, bool& result) const;
    int lt(IVector const* const right, NormType type, bool& result) const;
    int eq(IVector const* const right, NormType type, bool& result, double precision) const;

    /*utils*/
    unsigned int getDim() const;
    int norm(NormType type, double& res) const;
    int setCoord(unsigned int index, double elem);
    int getCoord(unsigned int index, double & elem) const;
    int setAllCoords(unsigned int dim, double* coords);
    int getCoordsPtr(unsigned int & dim, double const*& elem) const;
    IVector* clone() const;

private:
    Vector(unsigned int size, double* vals);

    // Data
    unsigned int m_size;
    double*  m_vals;
};

#endif // VECTOR_H
