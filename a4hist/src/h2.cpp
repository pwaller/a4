#include <iomanip>
#include <iostream>
#include <string.h>

#include <a4/axis.h>
#include <a4/histogram.h>

#include <a4/hist/Histograms.pb.h>

namespace a4 {
namespace hist {
using namespace std;

H2::H2() :
    _entries(0) {
    _initializations_remaining = 2;
}

void H2::bin_init() {
    _entries = 0;
    const uint32_t total_bins = (_x_axis->bins() + 2) * (_y_axis->bins() + 2);
    _data.reset(new double[total_bins]());
}

void H2::add_axis(UNIQUE<Axis> axis) {
    if (_initializations_remaining == 1) {
        _x_axis = std::move(axis);
    } else {
        _y_axis = std::move(axis);
        bin_init();
    }
}

void H2::constructor(const uint32_t & bins,
                     const double   & min,
                     const double   & max,
                     const char     * label) {
    UNIQUE<Axis> axis(new SimpleAxis(bins, min, max));
    axis->label = label;
    add_axis(std::move(axis));
}

void H2::constructor(const std::vector<double> & bins, const char * label) {
    UNIQUE<Axis> axis(new VariableAxis(bins));
    axis->label = label;
    add_axis(std::move(axis));
}

H2::H2(const H2 & h) :
    title(h.title),
    _entries(h._entries) {
    _x_axis = Axis::from_proto(*h._x_axis->get_proto());
    _y_axis = Axis::from_proto(*h._y_axis->get_proto());
    const uint32_t total_bins = (_x_axis->bins() + 2) * (_y_axis->bins() + 2);
    _data.reset(new double[total_bins]);
    memcpy(_data.get(), h._data.get(), total_bins * sizeof(double));
    if (h._weights_squared) {
        _weights_squared.reset(new double[total_bins]);
        memcpy(_weights_squared.get(),
               h._weights_squared.get(), total_bins * sizeof(double));
    }
    _initializations_remaining = 0;
}

H2::~H2()
{}

H2 & H2::__mul__(const double & w) {
    const uint32_t total_bins = (_x_axis->bins() + 2) * (_y_axis->bins() + 2);

    ensure_weights();
    for (uint32_t bin = 0, bins = total_bins; bins > bin; ++bin) {
        *(_data.get() + bin) *= w;
    }
    for (uint32_t bin = 0, bins = total_bins; bins > bin; ++bin) {
        *(_weights_squared.get() + bin) *= w * w;
    }
    _entries *= w;
    return *this;
}

// Implements StorableAs
void H2::to_pb(bool blank_pb) {
    if (!blank_pb) { pb.reset(new pb::H2()); }
    pb->mutable_x()->CopyFrom(*_x_axis->get_proto());
    pb->mutable_y()->CopyFrom(*_y_axis->get_proto());
    const uint32_t total_bins = (_x_axis->bins() + 2) * (_y_axis->bins() + 2);
    for (uint32_t i = 0; i < total_bins; i++) {
        pb->add_data(_data[i]);
    }
    if (_weights_squared) {
        for (uint32_t i = 0; i < total_bins; i++) {
            pb->add_weights_squared(_weights_squared[i]);
        }
    }
    pb->set_entries(_entries);
    pb->set_title(title);
}

void H2::from_pb() {
    _x_axis  = Axis::from_proto(pb->x());
    _y_axis  = Axis::from_proto(pb->y());
    _entries = pb->entries();
    const uint32_t total_bins = (_x_axis->bins() + 2) * (_y_axis->bins() + 2);
    _data.reset(new double[total_bins]);
    for (uint32_t i = 0; i < total_bins; i++) {
        _data[i] = pb->data(i);
    }
    if (pb->weights_squared_size() > 0) {
        _weights_squared.reset(new double[total_bins]);
        for (uint32_t i = 0; i < total_bins; i++) {
            _weights_squared[i] = pb->weights_squared(i);
        }
    }
    title = pb->title();
}

H2 & H2::operator+=(const H2 & other) {
    return this->__add__(other);
}

double H2::integral() const {
    double    integral = 0;
    const int skip     = _x_axis->bins() + 2;

    for (uint32_t bin = 1, bins = bin + _x_axis->bins(); bins > bin; ++bin) {
        for (uint32_t ybin = 1, ybins = ybin + _y_axis->bins();
             ybins > ybin;
             ++ybin) {
            integral += *(_data.get() + ybin * skip + bin);
        }
    }

    return integral;
}

void H2::print(std::ostream & out) const {
    if (!_data) {
        cerr << "Histogram is not initialized" << endl;
        return;
    }

    if (!title.empty()) { out << "Title: " << title << endl; }
    out << "Entries: " << entries() << " Integral: " << integral() << endl;

    const int skip = _x_axis->bins() + 2;
    for (uint32_t bin = 1, bins = bin + _x_axis->bins(); bins > bin; ++bin) {
        for (uint32_t ybin = 1, ybins = ybin + _y_axis->bins();
             ybins > ybin;
             ++ybin) {
            out << "[" << setw(3) << bin << ", " << ybin << "]: " <<
                    setiosflags(ios::fixed) << setprecision(3) <<
                    *(_data.get() + ybin * skip + bin) << endl;
        }
    }
}

H2 & H2::__add__(const H2 & source) {
    const uint32_t total_bins = (_x_axis->bins() + 2) * (_y_axis->bins() + 2);

    for (uint32_t bin = 0, bins = total_bins; bins > bin; ++bin) {
        *(_data.get() + bin) += *(source._data.get() + bin);
    }
    if (source._weights_squared) {
        ensure_weights();
        for (uint32_t bin = 0, bins = total_bins; bins > bin; ++bin) {
            *(_weights_squared.get() +
              bin) += *(source._weights_squared.get() + bin);
        }
    } else if (_weights_squared) {
        for (uint32_t bin = 0, bins = total_bins; bins > bin; ++bin) {
            *(_weights_squared.get() + bin) += *(source._data.get() + bin);
        }
    }
    _entries += source._entries;
    return *this;
}

// Helpers
//
ostream & operator<<(ostream & out, const H2 & hist) {
    hist.print(out);

    return out;
}
};
}; // namespace a4::hist;
