#ifndef SOLVER_H
#define SOLVER_H

#include "ISolver.h"
#include "IBrocker.h"
#include "Vector.h"
#include "ICompact.h"
#include "IProblem.h"
#include <QScopedPointer>

class SHARED_EXPORT Solver : ISolver, IBrocker
{
public:
    Solver();
    ~Solver();

    int getId() const ;

    int setParams(IVector const* params) ;
    int setParams(QString& str) ;
    int setProblem(IProblem *ptr) ;
    int solve() ;
    int getSolution(IVector* &vec)const ;
    int getQml(QUrl& qml) const ;

    bool canCastTo(Type type) const ;
    void* getInterfaceImpl(Type type) const ;
    int release() ;

private:
    bool m_solved;
    QScopedPointer<IVector> m_start;
    QScopedPointer<IVector> m_end;
    size_t m_argsDim;
    size_t m_paramsDim;
    size_t m_solverParamsCount;
    QScopedPointer<IVector> m_solution;
    QScopedPointer<IProblem> m_problem;
    QScopedPointer<const IVector> m_params;

    /*non default copyable*/
    Solver(const ISolver& other) = delete;
    void operator=(const ISolver& other) = delete;
    int getVecFromStr(const QString& str, QScopedPointer<IVector> &res);
    int initCompactAndIt(QScopedPointer<ICompact> &compact,
                                 QScopedPointer<ICompact::IIterator> &it,
                                 QScopedPointer<ICompact::IIterator> &itEnd);
};

#endif // SOLVER_H
