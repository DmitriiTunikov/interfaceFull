#include "Problem.h"
#include "ILog.h"

#include <QString>
#include <math.h>
#include <string.h>

#define REPORT(MSG) \
    QString qmsg("[PROBLEM_IK6]:  "); \
    qmsg += QString(MSG); \
    qmsg += "\n\t\tFile: "; \
    qmsg += __FILE__; \
    qmsg += "\n\t\tLine: "; \
    qmsg += QString::number(__LINE__); \
    qmsg += "\n\t\tFunction: "; \
    qmsg += __FUNCTION__; \
    ILog::report(qmsg.toStdString().c_str())

ErrorEnum Problem::checkNullAndDimArgs(const IVector *args) const
{
    if (args == NULL)
    {
        REPORT("Empty arguments");
        return ERR_WRONG_ARG;
    }
    if (args->getDim() != m_argsDim)
    {
        REPORT("Dimensions mismatch");
        return ERR_DIMENSIONS_MISMATCH;
    }
    return ERR_OK;
}

ErrorEnum Problem::checkNullAndDimParams(const IVector *params) const
{
    if (params != NULL)
    {
        REPORT("Params must be null");
        return ERR_DIMENSIONS_MISMATCH;
    }
    return ERR_OK;
}

int Problem::goalFunction(IVector const* args, IVector const* params, double& res) const
{
    ErrorEnum err = checkNullAndDimArgs(args);
    if (err != ERR_OK)
    {
        return err;
    }
    err = checkNullAndDimParams(params);
    if (err != ERR_OK)
    {
        return err;
    }

    double x1, x2;
    args->getCoord(0, x1);
    args->getCoord(1, x2);

    res = pow(x1, 4) + 2 * pow(x2, 4) + x1 * x1 * x2 * x2 + 2 * x1 + x2;

    return ERR_OK;
}

int Problem::goalFunctionByArgs(IVector const*  args, double& res) const
{
    ErrorEnum err = checkNullAndDimArgs(args);
    if (err != ERR_OK)
    {
        return err;
    }

    double x1, x2;
    args->getCoord(0, x1);
    args->getCoord(1, x2);

    res = pow(x1, 4) + 2 * pow(x2, 4) + x1 * x1 * x2 * x2 + 2 * x1 + x2;

    return ERR_OK;
}

int Problem::goalFunctionByParams(IVector const*  params, double& res) const
{
    ErrorEnum err = checkNullAndDimParams(params);
    if (err != ERR_OK)
    {
        return err;
    }
    if (m_args == NULL)
    {
        REPORT("Must call setArgs()");
        return ERR_WRONG_ARG;
    }

    double x1, x2;
    m_args->getCoord(0, x1);
    m_args->getCoord(1, x2);

    res = pow(x1, 4) + 2 * pow(x2, 4) + x1 * x1 * x2 * x2 + 2 * x1 + x2;

    return ERR_OK;
}

int Problem::getArgsDim(size_t& dim) const
{
    dim = m_argsDim;
    return ERR_OK;
}

int Problem::getParamsDim(size_t& dim) const
{
    dim = m_paramsDim;
    return ERR_OK;
}

int Problem::setParams(IVector const* params)
{
    ErrorEnum err = checkNullAndDimParams(params);
    if (err != ERR_OK)
    {
        return err;
    }
    m_params = params->clone();
    return ERR_OK;
}

int Problem::setArgs(IVector const* args)
{
    ErrorEnum err = checkNullAndDimArgs(args);
    if (err != ERR_OK)
    {
        return err;
    }
    m_args = args->clone();
    return ERR_OK;
}

int Problem::derivativeGoalFunction(size_t order, size_t idx, DerivedType dr, double& value, IVector const* args, IVector const* params) const
{
    ErrorEnum err = checkNullAndDimArgs(args);
    if (err != ERR_OK)
    {
        return err;
    }
    err = checkNullAndDimParams(params);
    if (err != ERR_OK)
    {
        return err;
    }
    if (dr == BY_PARAMS)
    {
        REPORT("Can't derivative by params");
        return ERR_ANY_OTHER;
    }
    if (idx >= m_argsDim)
    {
        REPORT("Derivative idx is out of range");
        return ERR_OUT_OF_RANGE;
    }

    double x1, x2;
    args->getCoord(0, x1);
    args->getCoord(1, x2);

    switch (order)
    {
    case 0:
        value = pow(x1, 4) + 2 * pow(x2, 4) + x1 * x1 * x2 * x2 + 2 * x1 + x2;
        break;
    case 1:
        if (idx == 0)
        {
            value = 4 * pow(x1, 3) + 2 * x1 * x2 * x2 + 2;
        }
        else
        {
            value = 2 * x1 * x1 * x2 + 8 * pow(x2, 3) + 1;
        }
        break;
    case 2:
        if (idx == 0)
        {
            value = 12 * x1 * x1 + 2 * x2 * x2;
        }
        else
        {
            value = 2 * x1 * x1 + 24 * x2 * x2;
        }
        break;
    case 3:
        if (idx == 0)
        {
            value = 24 * x1;
        }
        else
        {
            value = 48 * x2;
        }
        break;
    case 4:
        if (idx == 0)
        {
            value = 24;
        }
        else
        {
            value = 48;
        }
        break;
    default:
        value = 0;
        break;
    }

    return ERR_OK;
}

int Problem::derivativeGoalFunctionByArgs(size_t order, size_t idx, DerivedType dr, double& value, IVector const* args) const
{
    ErrorEnum err = checkNullAndDimArgs(args);
    if (err != ERR_OK)
    {
        return err;
    }
    if (dr == BY_PARAMS)
    {
        REPORT("Can't derivative by params");
        return ERR_ANY_OTHER;
    }
    if (idx >= m_argsDim)
    {
        REPORT("Derivative idx is out of range");
        return ERR_OUT_OF_RANGE;
    }

    double x1, x2;
    args->getCoord(0, x1);
    args->getCoord(1, x2);

    switch (order)
    {
    case 0:
        value = pow(x1, 4) + 2 * pow(x2, 4) + x1 * x1 * x2 * x2 + 2 * x1 + x2;
        break;
    case 1:
        if (idx == 0)
        {
            value = 4 * pow(x1, 3) + 2 * x1 * x2 * x2 + 2;
        }
        else
        {
            value = 2 * x1 * x1 * x2 + 8 * pow(x2, 3) + 1;
        }
        break;
    case 2:
        if (idx == 0)
        {
            value = 12 * x1 * x1 + 2 * x2 * x2;
        }
        else
        {
            value = 2 * x1 * x1 + 24 * x2 * x2;
        }
        break;
    case 3:
        if (idx == 0)
        {
            value = 24 * x1;
        }
        else
        {
            value = 48 * x2;
        }
        break;
    case 4:
        if (idx == 0)
        {
            value = 24;
        }
        else
        {
            value = 48;
        }
        break;
    default:
        value = 0;
        break;
    }

    return ERR_OK;
}

int Problem::derivativeGoalFunctionByParams(size_t order, size_t idx, DerivedType dr, double& value, IVector const* params) const
{
    REPORT("Problem has no params");
    return ERR_ANY_OTHER;
}

Problem::~Problem()
{
    if (m_args != NULL)
    {
        delete m_args;
    }
    if (m_params != NULL)
    {
        delete m_params;
    }
    ILog::destroy();
}

Problem::Problem() :
    m_paramsDim(0), m_argsDim(2),
    m_params(NULL), m_args(NULL)
{
}

bool Problem::canCastTo(Type type) const
{
    return type == IBrocker::PROBLEM;
}

void* Problem::getInterfaceImpl(Type type) const
{
    if (type == IBrocker::SOLVER)
    {
        return NULL;
    }
    return (IProblem *)(const_cast<Problem *>(this));
}

int Problem::release()
{
    if (m_args != NULL)
    {
        delete m_args;
    }
    if (m_params != NULL)
    {
        delete m_params;
    }
    return ERR_OK;
}
