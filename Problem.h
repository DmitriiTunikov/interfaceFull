#ifndef PROBLEM_H
#define PROBLEM_H

#include "IProblem.h"
#include "IBrocker.h"

class SHARED_EXPORT Problem : IProblem, IBrocker
{
public:

    int getId() const
    {
        return IProblem::INTERFACE_0;
    }

    int goalFunction(IVector const* args, IVector const* params, double& res) const;
    int goalFunctionByArgs(IVector const*  args, double& res) const;
    int goalFunctionByParams(IVector const*  params, double& res) const;
    int getArgsDim(size_t& dim) const;
    int getParamsDim(size_t& dim) const;

    int setParams(IVector const* params);
    int setArgs(IVector const* args);

    int derivativeGoalFunction(size_t order, size_t idx, DerivedType dr, double& value, IVector const* args, IVector const* params) const;
    int derivativeGoalFunctionByArgs(size_t order, size_t idx, DerivedType dr, double& value, IVector const* args) const;
    int derivativeGoalFunctionByParams(size_t order, size_t idx, DerivedType dr, double& value, IVector const* params) const;

    ~Problem();
    Problem();

    // Brocker
    bool canCastTo(Type type) const;
    void* getInterfaceImpl(Type type) const;
    int release();
private:
    ErrorEnum checkNullAndDimArgs(IVector const*  args) const;
    ErrorEnum checkNullAndDimParams(IVector const*  params) const;

    size_t m_paramsDim;
    size_t m_argsDim;
    IVector const* m_params;
    IVector const* m_args;
};

#endif // PROBLEM_H
