#ifndef COMPACT_H
#define COMPACT_H

#include "ICompact.h"
#include "IVector.h"

class SHARED_EXPORT Compact : ICompact
{
public:
    int getId() const
    {
        return INTERFACE_0;
    }

    /*factories*/
    static ICompact* createCompact(IVector const* const begin, IVector const* const end, IVector const* const step = 0);

    int deleteIterator(ICompact::IIterator * pIter);
    int getByIterator(ICompact::IIterator const* pIter, IVector*& pItem) const;

    ICompact::IIterator* end(IVector const* const step = 0);
    ICompact::IIterator* begin(IVector const* const step = 0);

    int isContains(IVector const* const vec, bool& result) const;
    int getNearestNeighbor(IVector const* vec, IVector *& nn) const;

    ICompact* clone() const;

    ~Compact();

    class Iterator : ICompact::IIterator
    {
        friend class Compact;
    public:
        //adds step to current value in iterator
        int doStep();

        //change step
        int setStep(IVector const* const step);

        bool operator ==(IIterator const* const iterator) const;
        bool operator !=(IIterator const* const iterator) const;

    protected:
        Iterator(Compact const* const compact, unsigned int index,
                 IVector const* const step, unsigned int * integerCoords);
        /*dtor*/
        ~Iterator();
    private:
        void updateIntegerCoordsFromIndex();
        void updateIndexFromIntegerCoords();
        void updatePosFromIntegerCoords();

        static Iterator * createIterator(Compact *compact, unsigned int index,
                                         const IVector * const step);

        const IVector* m_step;
        IVector* m_pos;
        Compact const* const m_compact;
        unsigned int* m_integerCoords; // in non-snake coords
        unsigned int m_index;
    };

private:
    Compact(IVector const* const begin, IVector const* const end,
            IVector const* const step, unsigned int* step_counts);

    IVector const* m_begin;
    IVector const* m_end;
    IVector const* m_step;
    unsigned int m_dim;
    unsigned int* m_step_counts;
    unsigned int m_total_point_count;
};

#endif // COMPACT_H

