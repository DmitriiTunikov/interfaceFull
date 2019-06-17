#include "Solver.h"
#include "error.h"
#include "ILog.h"
#include "Compact.h"
#include "Vector.h"
#include <new>          // std::nothrow
#include <QScopedPointer>
#include <QStringList>

#define REPORT(MSG) \
    QString qmsg("[SOLVER_DT20]:  "); \
    qmsg += QString(MSG); \
    qmsg += "\n\t\tFile: "; \
    qmsg += __FILE__; \
    qmsg += "\n\t\tLine: "; \
    qmsg += QString::number(__LINE__); \
    qmsg += "\n\t\tFunction: "; \
    qmsg += __FUNCTION__; \
    ILog::report(qmsg.toStdString().c_str())

bool Solver::canCastTo(Type type) const
{
    return type == IBrocker::SOLVER;
}

void * Solver::getInterfaceImpl(Type type) const
{
    if (type == IBrocker::PROBLEM)
    {
        return NULL;
    }

    return (ISolver *)(const_cast<Solver *>(this));
}

int Solver::release()
{
    m_start.reset();
    m_end.reset();
    m_solution.reset();
    m_params.reset();

    return ERR_OK;
}

Solver::~Solver()
{
}

Solver::Solver() : ISolver(){
    m_solved = false;
    m_solverParamsCount = 2;
}

int Solver::getId() const {
    return ISolver::INTERFACE_0;
}

int Solver::setParams(IVector const* params) {
    m_params.reset(params);
    if (m_params.isNull())
    {
        REPORT("wrong argument");
        return ERR_WRONG_ARG;
    }
    return ERR_OK;
}

int Solver::getVecFromStr(const QString& str, QScopedPointer<IVector> &res)
{
    QStringList coordsList = str.split(',');

    if ((int)m_argsDim != coordsList.size())
    {
        REPORT("wrong input args count");
        return ERR_DIMENSIONS_MISMATCH;
    }
    if (res == NULL)
    {
        REPORT("Solver::getVecFromStr: wrong input args - res == NULL");
        return ERR_WRONG_ARG;
    }

    for (int i = 0; i < coordsList.size(); i++)
    {
        bool ok;
        double d = coordsList.at(i).toDouble(&ok);
        if (ok == false)
        {
            REPORT("can't parse double args");
            return ERR_WRONG_ARG;
        }
        res->setCoord(i, d);
    }
    return ERR_OK;
}

int Solver::setParams(QString& str) {
    QStringList paramsList = str.split(';');

    if (paramsList.size() != (int)m_solverParamsCount)
    {
        REPORT("Solver::setParams - wrong params count");
        return ERR_WRONG_ARG;
    }

    //read [a;b]
    int ec = getVecFromStr(paramsList.at(0), m_start);
    if (ec != ERR_OK)
        return ec;
    ec = getVecFromStr(paramsList.at(1), m_end);
    if (ec != ERR_OK)
        return ec;

    return ERR_OK;
}

int Solver::setProblem(IProblem *ptr) {
    if (ptr == NULL)
    {
        REPORT("in Solvel::setProblem - ptr == NULL");
        return ERR_WRONG_ARG;
    }
    m_solved = false;
    m_problem.reset(ptr);
    m_problem->getArgsDim(m_argsDim);
    m_problem->getParamsDim(m_paramsDim);

    QScopedPointer<double, QScopedPointerArrayDeleter<double> > a (new (std::nothrow) double[m_argsDim]);
    QScopedPointer<double, QScopedPointerArrayDeleter<double> > b (new (std::nothrow) double[m_argsDim]);
    QScopedPointer<double, QScopedPointerArrayDeleter<double> > solution(new (std::nothrow) double[m_argsDim]);

    if (a.isNull() || b.isNull() || solution.isNull())
    {
        REPORT("Solver::setProblem - can't allocate memory for [a;b]");
        return ERR_MEMORY_ALLOCATION;
    }

    m_start.reset(Vector::createVector(m_argsDim, a.data()));
    m_end.reset(Vector::createVector(m_argsDim, b.data()));
    m_solution.reset(Vector::createVector(m_argsDim, solution.data()));
    if (m_start.isNull() || m_end.isNull() || m_solution.isNull())
    {
        REPORT("Solver::setProblem - can't allocate memory for [a;b]");
        return ERR_MEMORY_ALLOCATION;
    }

    return ERR_OK;
}

int Solver::initCompactAndIt(QScopedPointer<ICompact> &compact,
                             QScopedPointer<ICompact::IIterator> &it,
                             QScopedPointer<ICompact::IIterator> &itEnd)
{
    compact.reset(Compact::createCompact(m_start.data(), m_end.data()));
    if (compact.isNull())
    {
        REPORT("Solver::initCompactAndIt - can't create compact");
        return ERR_MEMORY_ALLOCATION;
    }
    it.reset(compact->begin());
    itEnd.reset(compact->end());

    if (it.isNull() || itEnd.isNull())
    {
        REPORT("Solver::initCompactAndIt - can't get begin for compact iterator");
        return ERR_MEMORY_ALLOCATION;
    }

    return ERR_OK;
}

int Solver::solve()
{
    QScopedPointer<ICompact> compact;
    QScopedPointer<ICompact::IIterator> it;
    QScopedPointer<ICompact::IIterator> itEnd;
    int ec = initCompactAndIt(compact, it, itEnd);
    if (ec != ERR_OK)
        return ec;

    QScopedPointer<double, QScopedPointerArrayDeleter<double> > tmpDouble(new double[m_argsDim]);
    if (tmpDouble.isNull())
    {
        REPORT("Solver::solve - can't allocate memory for calculate minimum");
        return ERR_MEMORY_ALLOCATION;
    }

    QScopedPointer<IVector> tmp(Vector::createVector(m_argsDim, tmpDouble.data()));
    if (tmp.isNull())
    {
        REPORT("Solver::solve - can't allocate memory for calculate minimum");
        return ERR_MEMORY_ALLOCATION;
    }

    double min = 0;

    IVector* b = m_solution.data();
    ec = compact->getByIterator(it.data(), b);
    if (ec != ERR_OK)
        return ec;
    ec = m_problem->goalFunction(m_solution.data(), m_params.data(), min);
    if (ec != ERR_OK)
        return ec;

    while (it->doStep() != ERR_OUT_OF_RANGE)
    {
        IVector *a = tmp.data();
        ec = compact->getByIterator(it.data(), a);
        if (ec != ERR_OK)
            return ec;

        double newMin = 0;
        ec = m_problem->goalFunction(tmp.data(), m_params.data(), newMin);
        if (ec != ERR_OK)
            return ec;
        if (newMin < min)
        {
            min = newMin;
            m_solution.reset(tmp.data()->clone());
        }
    }
    return ERR_OK;
}

int Solver::getSolution(IVector* &vec)const {
    if (!m_solved)
    {
        REPORT("problem wasn't solved yet");
        return ERR_ANY_OTHER;
    }
    vec = m_solution->clone();
    return ERR_OK;
}

int Solver::getQml(QUrl& qml) const {
    qml = QUrl("main.qml");
    return ERR_OK;
}
