#include "Set.h"
#include "error.h"
#include <stdio.h>
#include <new>          // std::nothrow

#include "ILog.h"
#include <cstring>
#include <QScopedPointer>

ISet::IIterator::IIterator(ISet const* const set, int pos)
{
}

IVector* Set::myCreateVector(unsigned int size, double const* vals){
    return Vector::createVector(size, vals);
}

Set::Set(IVector ** data, unsigned int dim) :
    m_data(data), m_curIdx(0), m_dim(dim)
{
}

int Set::copyAndDeleteData(IVector ** dst, IVector ** src, unsigned int size)
{
    if (src == NULL || dst == NULL || size < 0)
    {
        ILog::report("can't allocate mem for tmp vector* arr");
        return ERR_MEMORY_ALLOCATION;
    }

    memcpy(dst, src, size);
    for (size_t i = 0; i < size; i++)
    {
        QScopedPointer<double, QScopedPointerArrayDeleter<double> > tmpDouble(new double[m_dim]);
        if (tmpDouble.isNull())
        {
            cleanMemory(dst, i);
            ILog::report("can't allocate memory");
            return ERR_MEMORY_ALLOCATION;
        }
        dst[i] = myCreateVector(m_dim, tmpDouble.data());
        memcpy(dst[i], src[i], m_dim);
    }
    cleanMemory(const_cast<IVector**>(src), size);
    return ERR_OK;
}

void Set::cleanMemory(IVector ** data, unsigned int size)
{
    for (size_t i = 0; i < size; i++)
        delete data[i];
    delete[] data;
}

int Set::put(IVector const * const item){
    if (item == NULL){
        ILog::report("error while put item to set - item == NULL");
        return ERR_WRONG_ARG;
    }

    IVector ** tmpData = new (std::nothrow) IVector *[m_curIdx];
    if (tmpData == NULL)
    {
        ILog::report("can't allocate memory");
        return ERR_MEMORY_ALLOCATION;
    }
    int ec = copyAndDeleteData(tmpData, m_data, m_curIdx);
    if (ec != ERR_OK)
    {
        cleanMemory(tmpData, m_curIdx);
        return ec;
    }

    m_data = new (std::nothrow) IVector*[m_curIdx + 1];
    if (m_data == NULL)
    {
        cleanMemory(tmpData, m_curIdx);
        ILog::report("can't allocate memory");
        return ERR_MEMORY_ALLOCATION;
    }
    ec = copyAndDeleteData(m_data, tmpData, m_curIdx);
    if (ec != ERR_OK)
    {
        cleanMemory(tmpData, m_curIdx);
        return ec;
    }

    const Vector * const tmp = dynamic_cast<const Vector *const >(item);
    if (tmp == NULL)
    {
        cleanMemory(tmpData, m_curIdx);
        ILog::report("error while making cast from IVector to Vector");
        return ERR_WRONG_ARG;
    }
    m_data[m_curIdx++] = tmp->clone();

    return ERR_OK;
}

int Set::get(unsigned int index, IVector*& pItem) const{
    if (index < 0 || index > m_curIdx - 1){
        ILog::report("error while getting set item by index - wrong index");
        return ERR_OUT_OF_RANGE;
    }
    pItem = m_data[index]->clone();

    return ERR_OK;
}

int Set::remove(unsigned int index)
{
    IVector ** tmpData = new (std::nothrow) IVector*[m_curIdx];
    if (tmpData == NULL)
    {
        ILog::report("can't allocate memory");
        return ERR_MEMORY_ALLOCATION;
    }
    int ec = copyAndDeleteData(tmpData, m_data, m_curIdx);
    if (ec != ERR_OK)
    {
        cleanMemory(tmpData, m_curIdx);
        return ec;
    }

    m_data = new (std::nothrow) IVector*[m_curIdx - 1];
    if (m_data == NULL)
    {
        cleanMemory(tmpData, m_curIdx);
        ILog::report("can't allocate memory");
        return ERR_MEMORY_ALLOCATION;
    }
    size_t k = 0;
    for (; k < index; k++)
    {
        memcpy(m_data[k], tmpData[k], m_dim);
        delete tmpData[k];
    }
    delete tmpData[k];
    k++;
    for (;k < m_curIdx; k++)
    {
        memcpy(m_data[k - 1], tmpData[k], m_dim);
        delete tmpData[k];
    }
    delete[] tmpData;
    return ERR_OK;
}

int Set::contains(IVector const * const pItem, bool & rc) const{
    if (pItem == NULL){
        ILog::report("error while contains set method - wrong item");
        return ERR_WRONG_ARG;
    }
    rc = true;
    for (unsigned int i = 0; i < m_curIdx; i++){
        int ec = m_data[i]->eq(pItem, IVector::NormType::NORM_1, rc, 0.001);
        if (ec != ERR_OK)
            return ec;
        if (rc == false)
            return ERR_OK;
    }

    return ERR_OK;
}

unsigned int Set::getSize() const{
    return m_curIdx;
}


ISet* Set::createSet(unsigned int R_dim){
    Set *res = new (std::nothrow) Set(NULL, R_dim);
    if (res == NULL)
    {
        ILog::report("Set::createSet - can't allocate memory for Set");
        return NULL;
    }

    return res;
}

int Set::clear(){
    for (unsigned int i = 0; i < m_curIdx; i++)
        delete m_data[i];
    delete[] m_data;
    m_curIdx = 0;

    return ERR_OK;
}

int Set::deleteIterator(IIterator * pIter){
    delete pIter;
    return ERR_OK;
}

Set::Iterator::Iterator(Set const* const set, int pos) : IIterator(set, pos), m_set(set){
    m_idx = pos;
}

int Set::getByIterator(const IIterator *pIter, IVector *&pItem) const{
    if (pIter == NULL)
        return ERR_WRONG_ARG;

    const Iterator * it = dynamic_cast<const Iterator *>(pIter);
    if (it == NULL)
        return ERR_WRONG_ARG;

    pItem = m_data[it->m_idx]->clone();
    return ERR_OK;
}

int Set::Iterator::next(){
    if (isEnd())
        return ERR_OUT_OF_RANGE;

    ++m_idx;
    return ERR_OK;
}

ISet::IIterator* Set::end(){
    return new Iterator(this, m_curIdx - 1);
}

ISet::IIterator* Set::begin(){
    return new Iterator(this, 0);
}

Set::~Set(){
    clear();
}

int Set::Iterator::prev(){
    if (isBegin())
        return ERR_OUT_OF_RANGE;

    --m_idx;
    return ERR_OK;
}

bool Set::Iterator::isEnd() const{
    return m_idx == m_set->m_curIdx - 1;
}

bool Set::Iterator::isBegin() const{
    return m_idx == 0;
}
