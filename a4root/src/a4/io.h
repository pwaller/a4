#ifndef _A4_ROOT_H_
#define _A4_ROOT_H_

#if 0
//def CERN_ROOT_SYSTEM

#include <TMessage.h>
#include <TObject.h>

#include <string>

#include <a4/storable.h>

#include <a4/root/CernRootSystem.pb.h>

namespace a4{ namespace root{

template<class CernRootSystemClass>
class RootClass : public a4::process::StorableAs<RootClass<CernRootSystemClass>, pb::CernRootSystemClass>
{
    public:
        RootClass() : _root_object(NULL) {
            
        }
        RootClass(const RootClass &);
        ~RootClass();

        virtual void to_pb(bool blank_pb) {
            assert(_root_object);
            if (!blank_pb) pb.reset(new pb::CernRootSystemClass());
            TMessage x;
            //x << _root_object;
            x.WriteObject(_root_object);
            pb->set_serialized_content(x.CompBuffer(), x.CompLength());
        }
        virtual void from_pb() {
        
            std::string serialized_content = pb->serialized_content();
            TMessage x;
            x.ReadBuf(serialized_content.c_str(), serialized_content.size());
            x
            
        }
        virtual RootClass& operator+=(const RootClass& other);


        void print(std::ostream &) const;

    private:
        // Prevent copying by assignment
        RootClass &operator =(const RootClass &);
        
        TObject* _root_object;

};

std::ostream &operator<<(std::ostream&, const RootClass&);

#include <THnSparse.h>

class RootSparseHistogram : public RootClass<THnSparseF>
{
public:
        void constructor(const char * _title) {
            _initializations_remaining++;
            title = _title;
        }
        void constructor(const uint32_t &bins, const double &min, const double &max, const char* label="");
        void constructor(const std::vector<double>& bins, const char* label="");
        void constructor(const std::initializer_list<double>& bins, const char* label="") {
            constructor(std::vector<double>(bins), label);
        }

        void fill(const double& x, const double& weight=1) {
            int bin = _axis->find_bin(x);
            *(_data.get() + bin) += weight;
            ++_entries;

            if (_weights_squared) {
                *(_weights_squared.get() + bin) += weight*weight;
            } else if (weight != 1.0) {
                const uint32_t total_bins = _axis->bins() + 2;
                _weights_squared.reset(new double[total_bins]);
                for(uint32_t i = 0; i < total_bins; i++)
                    _weights_squared[i] = _data[i];
                *(_weights_squared.get() + bin) += weight*weight;
            }
        }
        
        RootClass & __add__(const RootClass &);
        RootClass & __mul__(const double &);

        uint64_t entries() const { return _entries; }
        uint64_t bins() const { return _axis->bins(); }
        
        /// Sum of weights in _non overflow_ bins
        double integral() const;
        
        const Axis & x() const { return *_axis; }
        const Axis& axis(uint32_t i) const {
            switch (i) {
                case 0:
                    return x();
            }
            throw a4::Fatal("Requested invalid axis ", i, " of RootClass");
        }    
};

};};

#endif // CERN_ROOT_SYSTEM

#endif

