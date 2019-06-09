#ifndef MOBILINKD__AGC_H_
#define MOBILINKD__AGC_H_

#include <cmath>
#include <array>
#include <numeric>
#include <algorithm>

namespace mobilinkd { namespace tnc {

template <typename T>
struct BaseAGC {

	typedef T float_type;
	float_type attack_;
	float_type decay_;
	float_type reference_;
	float_type max_gain_;
	bool has_max_gain_;
	
	float_type gain_;
	
	BaseAGC(float_type attack, float_type decay, float_type reference = 1.0)
	: attack_(attack), decay_(decay), reference_(reference)
	, max_gain_(0.0), has_max_gain_(false), gain_(1.0)
	{}

	BaseAGC(float_type decay, float_type attack, float_type reference, float_type max_gain)
	: attack_(attack), decay_(decay), reference_(reference)
	, max_gain_(max_gain), has_max_gain_(true), gain_(1.0)
	{}
	
	float_type operator()(float_type value) {
		
		float_type output = value * gain_;
		float_type tmp = fabs(output) - reference_;
		float_type rate = decay_;
		
		if (fabs(tmp) > gain_) {
			rate = attack_;
		}

		gain_ -= tmp * rate;

		if (gain_ < 0.0f) {
			gain_ = .000001f;
		}

		if (has_max_gain_ and (gain_ > max_gain_)) {
			gain_ = max_gain_;
		}

		return output;
	}
};

typedef BaseAGC<double> AGC;
typedef BaseAGC<float> FastAGC;

inline uint32_t log_2(const uint32_t x)
{
    if (x == 0) return 0;
    return (31 - __builtin_clz (x));
}

template <typename T, size_t Delay, size_t BlockSize, T Reference>
class FeedForwardAGC
{
    using buffer_type = std::array<T, Delay>;
    using block_type = std::array<T, BlockSize>;

    buffer_type buffer{0};
    block_type block{0};
    size_t index{0};
    bool full{false};
    T gain{0};

    T max_amplitude() const
    {
        T result = 0;
        for (T x : buffer)
        {
            T y = std::abs(x);
            result = std::max(y, result);
        }
        return result;
    }

public:

    T* operator()(T* input)
    {
        auto tmp_i = index;
        for (size_t i = 0; i != BlockSize; ++i) {
            block[i] = buffer[tmp_i] * gain;
            buffer[tmp_i++] = input[i];
            if (tmp_i == Delay) tmp_i = 0;
        }

        const auto amp = max_amplitude();
        gain = std::max(T(Reference >> log_2(amp)), T(1));

        return block.begin();
    }
};

}} // mobilinkd::tnc

#endif // MOBILINKD__AGC_H_


