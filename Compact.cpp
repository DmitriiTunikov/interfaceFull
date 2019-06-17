#include <limits>
#include <new>          // std::nothrow
#include <cstring>
#include <algorithm>
#include <QString>
#include <string.h>

#include "Vector.h"
#include "ILog.h"
#include "Compact.h"

#define ABS(x) ((x) > 0 ? (x) : -(x))

#define REPORT(MSG) \
    QString qmsg("[COMPACT]:  "); \
    qmsg += QString(MSG); \
    qmsg += "\n\t\tFile: "; \
    qmsg += __FILE__; \
    qmsg += "\n\t\tLine: "; \
    qmsg += QString::number(__LINE__); \
    qmsg += "\n\t\tFunction: "; \
    qmsg += __FUNCTION__; \
    ILog::report(qmsg.toStdString().c_str())


ICompact::IIterator::IIterator(const ICompact *const compact, int pos, const IVector *const step)
{
}

Compact::Compact(IVector const* const begin, IVector const* const end,
                 IVector const* const step, unsigned int* step_counts) :
    m_begin(begin), m_end(end), m_step(step),
    m_dim(begin->getDim()), m_step_counts(step_counts)
{
    if (m_step_counts == NULL)
        return;
    m_total_point_count = 1;
    for (size_t i = 0; i < m_dim; i++)
        m_total_point_count *= (m_step_counts[i] + 1);
}

/*factories*/
ICompact* Compact::createCompact(IVector const* const begin, IVector const* const end, IVector const* const step)
{
    if (begin == NULL || end == NULL)
        return NULL;

    unsigned int dim = begin->getDim();

    if (dim != end->getDim())
        return NULL;

    IVector* begin_copy = begin->clone();
    if (begin_copy == NULL)
        return NULL;

    IVector* step_copy;

    if (step != NULL)
    {
        if (step->getDim() == dim)
            step_copy = step->clone();
    }
    else
    {
        double *coords = new (std::nothrow) double[dim];
        if (coords == NULL)
            return NULL;
        // set as minimum
        for (unsigned i = 0; i < dim; i++)
            coords[i] = std::numeric_limits<double>::min();
        step_copy = Vector::createVector(dim, coords);
        delete [] coords;
    }

    IVector* end_copy = end->clone();
    IVector* delta = end_copy->clone();

    unsigned int* step_counts = NULL;
    if (step_copy != NULL && delta != NULL)
    {
        delta->subtract(begin_copy);

        step_counts = new (std::nothrow) unsigned int[dim];
        if (step_counts != NULL)
        {
            for (unsigned int i = 0; i < dim; i++)
            {
                double step_i = 0;
                step_copy->getCoord(i, step_i);

                double delta_i = 0;
                delta->getCoord(i, delta_i);

                if (step_i <= 0 || delta_i == 0) // check for positive
                {
                    delete [] step_counts;
                    step_counts = NULL;
                    break;
                }
                else
                {
                    step_counts[i] = (unsigned int)(ABS(delta_i)) + !!(ABS(delta_i) - (unsigned int)(ABS(delta_i))); // round

                    step_copy->setCoord(i, delta_i > 0 ? ABS(step_i) : -(ABS(step_i)));
                }
            }
        }
    }


    delete delta;

    Compact* res = new (std::nothrow) Compact(begin_copy, end_copy,
                                              step_copy, step_counts);
    if (begin_copy == NULL || end_copy == NULL || step_copy == NULL ||
        res == NULL || step_counts == NULL || delta == NULL)
    {
        if (res != NULL)
        {
            delete res;
            res = NULL;
        }
        else
        {
            delete begin_copy;
            delete step_copy;
            delete end_copy;
        }
    }

    return res;
}

int Compact::deleteIterator(ICompact::IIterator * pIter)
{
    delete pIter;
    return ERR_OK;
}

int Compact::getByIterator(ICompact::IIterator const* pIter, IVector*& pItem) const
{
    Compact::Iterator const * pIter_c = dynamic_cast<Compact::Iterator const *> (pIter);
    if (pIter_c == NULL)
    {
        REPORT("Can't convert to iterator");
        return ERR_WRONG_ARG;
    }
    pItem = pIter_c->m_pos->clone();

    if (pItem == NULL)
    {
        REPORT("Not enough memory");
        return ERR_MEMORY_ALLOCATION;
    }
    return ERR_OK;
}

Compact::Iterator * Compact::Iterator::createIterator(Compact *compact,
                                                      unsigned int index,
                                                      const IVector * const step)
{
    if (compact == NULL)
        return NULL;

    unsigned int *integerCoords = new (std::nothrow) unsigned int[compact->m_dim];
    if (integerCoords == NULL)
        return NULL;

    Iterator * res = new (std::nothrow)Iterator(compact, index, step, integerCoords);
    if (res == NULL)
    {
        delete[] integerCoords;
        return NULL;
    }

    return res;
}

ICompact::IIterator* Compact::end(IVector const* const step)
{
    return Iterator::createIterator(this, m_total_point_count, step);
}

ICompact::IIterator* Compact::begin(IVector const* const step)
{
    return Iterator::createIterator(this, 0, step);
}

int Compact::isContains(IVector const* const vec, bool& result) const
{
    if (vec->getDim() != m_dim)
        return ERR_DIMENSIONS_MISMATCH;
    for (unsigned int i = 0; i < m_dim; i++)
    {
        double vec_val, begin_val, end_val;
        int res = vec->getCoord(i, vec_val);
        if (res != ERR_OK)
            return res;
        res = m_begin->getCoord(i, begin_val);
        if (res != ERR_OK)
            return res;
        res = m_end->getCoord(i, end_val);
        if (res != ERR_OK)
            return res;
        if (end_val < begin_val)
            std::swap(end_val, begin_val);
        if (!(begin_val <= vec_val && end_val >= vec_val))
        {
            result = false;
            return ERR_OK;
        }
    }
    result = true;
    return ERR_OK;
}

int Compact::getNearestNeighbor(IVector const* vec, IVector *& nn) const
{
    if (vec->getDim() != m_dim)
    {
        REPORT("Vectors dimensions mismatch");
        return ERR_DIMENSIONS_MISMATCH;
    }

    nn = m_begin->clone();
    if (nn == NULL)
    {
        REPORT("Not enough memory");
        return ERR_MEMORY_ALLOCATION;
    }

    for (unsigned int i = 0; i < m_dim; i++)
    {
        double vec_val, begin_val, step_i;
        int res = vec->getCoord(i, vec_val);
        if (res != ERR_OK)
        {
            delete nn;
            return res;
        }
        res = m_begin->getCoord(i, begin_val);
        if (res != ERR_OK)
        {
            delete nn;
            return res;
        }
        res = m_step->getCoord(i, step_i);
        if (res != ERR_OK)
        {
            delete nn;
            return res;
        }

        double p = ((vec_val - begin_val) / step_i);
        if (p < 0)
        {
            delete nn;
            REPORT("Result is out of range");
            return ERR_OUT_OF_RANGE;
        }
        unsigned int point_index_i = (unsigned int)(p + 0.5);
        if (m_step_counts[i] < point_index_i)
        {
            delete nn;
            REPORT("Result is out of range");
            return ERR_OUT_OF_RANGE;
        }

        nn->setCoord(i, begin_val + point_index_i * step_i);

    }
    return ERR_OK;
}

ICompact* Compact::clone() const
{
    return createCompact(m_begin, m_end, m_step);
}

Compact::~Compact()
{
    delete m_begin;
    delete m_end;
    delete m_step;
    delete m_step_counts;
}

int Compact::Iterator::doStep()
{
    if (m_index >= m_compact->m_total_point_count)
    {
        REPORT("Result aleady is out of range");
        return ERR_OUT_OF_RANGE;
    }

    if (m_step == NULL)
    {
        m_index++;
        if (m_index >= m_compact->m_total_point_count)
        {
            REPORT("Result is out of range");
            return ERR_OUT_OF_RANGE;
        }
        updateIntegerCoordsFromIndex();
    }
    else
    {
        // move by step
        m_pos->add(m_step);

        // find nearest
        IVector *nearPos;
        int error = m_compact->getNearestNeighbor(m_pos, nearPos);

        if (error != ERR_OK)
        {
            delete nearPos;
            if (error == ERR_OUT_OF_RANGE)
                m_index = m_compact->m_total_point_count;
            return error;
        }
        delete m_pos;
        m_pos = nearPos;

        // calculate integer coords
        for (size_t i = 0; i < m_compact->m_dim; i++)
        {
            double pos_i, begin_i, comp_step_i;
            int res = m_pos->getCoord(i, pos_i);
            if (res != ERR_OK)
                return res;
            res = m_compact->m_begin->getCoord(i, begin_i);
            if (res != ERR_OK)
                return res;
            res = m_step->getCoord(i, comp_step_i);
            if (res != ERR_OK)
                return res;

            double p = ((pos_i - begin_i) / comp_step_i);
            m_integerCoords[i] = (unsigned int)(p + 0.5);
        }

        // Update index
        updateIndexFromIntegerCoords();
    }

    return ERR_OK;
}

void Compact::Iterator::updateIntegerCoordsFromIndex()
{
    // Calculate snake index
    size_t prod = m_compact->m_total_point_count;
    for (size_t i = 0; i < m_compact->m_dim; i++)
    {
        prod /= m_compact->m_step_counts[i] + 1;
        m_integerCoords[i] = m_index / prod;
    }

    // Convert to non-snake index
    int sign = 1;
    for (size_t i = m_compact->m_dim - 1; i > 0; i--)
    {
        if (m_integerCoords[i] % 2 == 1)
            sign *= -1;
        // Invert if needed
        if (sign == -1)
            m_integerCoords[i - 1] = m_compact->m_step_counts[i - 1] - m_integerCoords[i - 1];
    }
}

void Compact::Iterator::updateIndexFromIntegerCoords()
{
    // Already have non-snake indices

    // Convert to snake indices and sum
    m_index = 0;
    int sign = 1;
    size_t prod = m_compact->m_total_point_count;

    prod /= m_compact->m_step_counts[m_compact->m_dim - 1];
    m_index += m_integerCoords[m_compact->m_dim - 1] * prod;
    for (size_t i = m_compact->m_dim - 1; i > 0; i--)
    {
        if (m_integerCoords[i] % 2 == 1)
            sign *= -1;
        // Invert if needed
        if (sign == -1)
            m_integerCoords[i - 1] = m_compact->m_step_counts[i - 1] - m_integerCoords[i - 1];

        prod /= m_compact->m_step_counts[i - 1];
        m_index += m_integerCoords[i - 1] * prod;
    }
}

void Compact::Iterator::updatePosFromIntegerCoords()
{
    delete m_pos;
    m_pos = m_compact->m_begin->clone();

    for (size_t i = 0; i < m_compact->m_dim; i++)
    {
        double b_i;
        double s_i;
        m_pos->getCoord(i, b_i);
        m_compact->m_step->getCoord(i, s_i);
        m_pos->setCoord(i, b_i + s_i * m_integerCoords[i]);
    }
}

int Compact::Iterator::setStep(IVector const* const step)
{
    delete m_step;

    m_step = step->clone();
    if (m_step == NULL)
    {
        REPORT("Not enough memory");
        return ERR_MEMORY_ALLOCATION;
    }
    return ERR_OK;
}

Compact::Iterator::~Iterator()
{
    delete[] m_integerCoords;
}

Compact::Iterator::Iterator(Compact const* const compact, unsigned int index,
                            IVector const* const step, unsigned int * integerCoords) :
    IIterator(compact, (int)index, step),
    m_step(step), m_pos(NULL), m_compact(compact), m_integerCoords(integerCoords), m_index(index)
{
    updateIntegerCoordsFromIndex();
    updatePosFromIntegerCoords();
}

bool Compact::Iterator::operator==(IIterator const* const iterator) const
{
    const Iterator * const otherIt = dynamic_cast<const Iterator * const>(iterator);
    if (otherIt == NULL || otherIt->m_compact != m_compact)
        return false;

    bool selfContains, otherContains;
    m_compact->isContains(this->m_pos, selfContains);
    m_compact->isContains(otherIt->m_pos, otherContains);

    if (!selfContains && !otherContains)
        return true;
    if (selfContains != otherContains)
        return false;

    return !memcmp(m_integerCoords, otherIt->m_integerCoords,
                   m_compact->m_dim * sizeof(unsigned int));
}

bool Compact::Iterator::operator!=(IIterator const* const iterator) const
{
    return !operator ==(iterator);
}
