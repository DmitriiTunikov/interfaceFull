#include <QtCore/QCoreApplication>

#include "Problem.h"
#include "Solver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    IProblem *problem = (IProblem *)(new Problem());
    ISolver *solver = (ISolver *)(new Solver());

    solver->setProblem(problem);
    QString params = QString("-1,-1; 1,1;");
    solver->setParams(params);

    solver->solve();

    return a.exec();
}
