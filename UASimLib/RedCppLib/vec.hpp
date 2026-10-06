#pragma once

#include "RedCppLib.hpp"

namespace Red {

    namespace __priv
    {
        template<typename T, uint32_t LEN>
        struct _vecn
        {
            union
            {
                T data[LEN];
            };
        };
        template<typename T>
        struct _vecn<T, 1>
        {
            union
            {
                T data[1];

                struct
                {
                    T x;
                };
            };
        };
        template<typename T>
        struct _vecn<T, 2>
        {
            union
            {
                T data[2];

                struct
                {
                    T x;
                    T y;
                };
            };
        };
        template<typename T>
        struct _vecn<T, 3>
        {
            union
            {
                T data[3];

                struct
                {
                    T x;
                    T y;
                    T z;
                };
            };
        };
        template<typename T>
        struct _vecn<T, 4>
        {
            union
            {
                T data[4];

                struct
                {
                    T x;
                    T y;
                    T z;
                    T w;
                };
            };
        };
    }

    template<typename T, int32_t LEN>
    struct vec : public __priv::_vecn<T, LEN>
    {
        vec() = default;
        template<typename... Args>
        vec(Args... args)
        {
            constexpr int N = sizeof...(Args);
            if constexpr (N > 0) {
                T arr[N] = { static_cast<T>(args)... };
                for (int i = 0; i < LEN; i++) {
                    
                    if (i < N) this->data[i] = arr[i];
                    else this->data[i] = T(0);
                }
            }
        }

        static_assert(LEN > 0);
        static_assert(sizeof(__priv::_vecn<T, LEN>) == sizeof(T) * LEN);
        static int32_t size()
        {
            return LEN;
        }
        T* begin()
        {
            return this->data;
        }
        T* end()
        {
            return this->data + size();
        }
        T sum()
        {
            T sm = this->data[0];
            if constexpr (LEN > 1)
            {
                for (int32_t i = 1; i < LEN; i++)
                {
                    sm += this->data[i];
                }
            }
            return sm;
        }
        T area()
        {
            T sm = this->data[0];
            if constexpr (LEN > 1)
            {
                for (int32_t i = 1; i < LEN; i++)
                {
                    sm *= this->data[i];
                }
            }
            return sm;
        }
        T length() const
        {
            T sm = this->data[0] * this->data[0];
            if constexpr (LEN > 1)
            {
                for (int32_t i = 1; i < LEN; i++)
                {
                    sm += this->data[i] * this->data[i];
                }
            }
            return sqrt(sm);
        }
        static T length(const vec& v)
        {
            return v.length();
        }

        vec normalized() const
        {
            vec ret;
            T len = this->length();
            for (int i = 0; i < LEN; i++)
            {
                ret[i] = this->data[i] / len;
            }
            return ret;
        }

        static vec normalized(const vec& v)
        {
            return v.normalized();
        }

        vec min(T vmin) const
        {
            vec ret;
            for (int32_t i = 0; i < LEN; i++)
            {
                ret[i] = std::min<T>(this->data[i], vmin);
            }
            return ret;
        }
        vec max(T vmax) const
        {
            vec ret;
            for (int32_t i = 0; i < LEN; i++)
            {
                ret[i] = std::max<T>(this->data[i], vmax);
            }
            return ret;
        }
        vec constrain(T vmin, T vmax) const
        {
            vec ret;
            for (int32_t i = 0; i < LEN; i++)
            {
                ret[i] = std::min<T>(std::max<T>(vmin, this->data[i]), vmax);
            }
            return ret;
        }
        vec min(vec vmin) const
        {
            vec ret;
            for (int32_t i = 0; i < LEN; i++)
            {
                ret[i] = std::min<T>(this->data[i], vmin[i]);
            }
            return ret;
        }
        vec max(vec vmax) const
        {
            vec ret;
            for (int32_t i = 0; i < LEN; i++)
            {
                ret[i] = std::max<T>(this->data[i], vmax[i]);
            }
            return ret;
        }
        vec constrain(vec vmin, vec vmax) const
        {
            vec ret;
            for (int32_t i = 0; i < LEN; i++)
            {
                ret[i] = std::min<T>(std::max<T>(vmin[i], this->data[i]), vmax[i]);
            }
            return ret;
        }

        std::string str() const
        {
            std::string ret = "{";
            int32_t i = 0;
            while (true)
            {
                ret += std::to_string(this->data[i]);
                i++;
                if (i == LEN)
                    break;
                ret += ", ";
            }
            return ret + "}";
        }

        template<typename NT>
        vec<NT, LEN> convert() const
        {
            vec<NT, LEN> ret;
            for (int i = 0; i < LEN; i++)
            {
                ret[i] = (NT)this->data[i];
            }
            return ret;
        }

        T& operator[](uint32_t i)
        {
            return *(this->data + i);
        }
        const T& operator[](uint32_t i) const
        {
            return this->data[i];
        }

        constexpr const vec operator+(vec v) const
        {
            for (uint32_t i = 0; i < LEN; i++)
            {
                v.data[i] += this->data[i];
            }
            return v;
        }
        constexpr const vec operator-(vec v) const
        {
            for (int32_t i = 0; i < LEN; i++)
            {
                v.data[i] = this->data[i] - v.data[i];
            }
            return v;
        }
        constexpr const vec operator*(vec v) const
        {
            for (int32_t i = 0; i < LEN; i++)
            {
                v.data[i] *= this->data[i];
            }
            return v;
        }
        constexpr const vec operator/(vec v) const
        {
            for (int32_t i = 0; i < LEN; i++)
            {
                v.data[i] = this->data[i] / v.data[i];
            }
            return v;
        }
        constexpr const vec operator%(vec v) const
        {
            for (int32_t i = 0; i < LEN; i++)
            {
                v.data[i] = this->data[i] % v.data[i];
            }
            return v;
        }

        constexpr const vec operator+(T v) const
        {
            vec<T, LEN> ret;
            for (int32_t i = 0; i < LEN; i++)
            {
                ret.data[i] = this->data[i] + v;
            }
            return ret;
        }
        constexpr const vec operator-(T v) const
        {
            vec<T, LEN> ret;
            for (int32_t i = 0; i < LEN; i++)
            {
                ret.data[i] = this->data[i] - v;
            }
            return ret;
        }
        constexpr const vec operator*(T v) const
        {
            vec<T, LEN> ret;
            for (int32_t i = 0; i < LEN; i++)
            {
                ret.data[i] = this->data[i] * v;
            }
            return ret;
        }
        constexpr const vec operator/(T v) const
        {
            vec<T, LEN> ret;
            for (int32_t i = 0; i < LEN; i++)
            {
                ret.data[i] = this->data[i] / v;
            }
            return ret;
        }
        constexpr const vec operator%(T v) const
        {
            vec<T, LEN> ret;
            for (int32_t i = 0; i < LEN; i++)
            {
                ret.data[i] = this->data[i] % v;
            }
            return ret;
        }

        constexpr const vec operator-() const
        {
            vec<T, LEN> ret;
            for (int32_t i = 0; i < LEN; i++)
            {
                ret.data[i] = -this->data[i];
            }
            return ret;
        }


        constexpr const vec& operator+=(const vec& v)
        {
            return *this = *this + v;
        }
        constexpr const vec& operator-=(const vec& v)
        {
            return *this = *this - v;
        }
        constexpr const vec& operator*=(const vec& v)
        {
            return *this = *this * v;
        }
        constexpr const vec& operator/=(const vec& v)
        {
            return *this = *this / v;
        }
        constexpr const vec& operator%=(const vec& v)
        {
            return *this = *this % v;
        }

        constexpr const vec& operator+=(const T& v)
        {
            return *this = *this + v;
        }
        constexpr const vec& operator-=(const T& v)
        {
            return *this = *this - v;
        }
        constexpr const vec& operator*=(const T& v)
        {
            return *this = *this * v;
        }
        constexpr const vec& operator/=(const T& v)
        {
            return *this = *this / v;
        }
        constexpr const vec& operator%=(const T& v)
        {
            return *this = *this % v;
        }

        const vec operator<<(vec v) const
        {
            for (uint32_t i = 0; i < LEN; i++)
            {
                v.data[i] = this->data[i] << v.data[i];
            }
            return v;
        }
        const vec operator>>(vec v) const
        {
            for (int32_t i = 0; i < LEN; i++)
            {
                v.data[i] = this->data[i] >> v.data[i];
            }
            return v;
        }
        const vec operator&(vec v) const
        {
            for (int32_t i = 0; i < LEN; i++)
            {
                v.data[i] = this->data[i] & v.data[i];
            }
            return v;
        }
        const vec operator|(vec v) const
        {
            for (int32_t i = 0; i < LEN; i++)
            {
                v.data[i] = this->data[i] | v.data[i];
            }
            return v;
        }
        const vec operator^(vec v) const
        {
            for (int32_t i = 0; i < LEN; i++)
            {
                v.data[i] = this->data[i] ^ v.data[i];
            }
            return v;
        }

        const vec& operator<<=(const vec& v)
        {
            return *this = *this << v;
        }
        const vec& operator>>=(const vec& v)
        {
            return *this = *this >> v;
        }
        const vec& operator&=(const vec& v)
        {
            return *this = *this & v;
        }
        const vec& operator|=(const vec& v)
        {
            return *this = *this | v;
        }
        const vec& operator^=(const vec& v)
        {
            return *this = *this ^ v;
        }

        static T dot(const vec& left, const vec& right)
            requires (std::same_as<T, float> || std::same_as<T, double>)
        {
            T ret = T(0);
            for (auto i = 0; i < LEN; ++i)
                ret += left.data[i] * right.data[i];
            return ret;
        }

        T dot(const vec& other) const
            requires (std::same_as<T, float> || std::same_as<T, double>)
        {
            return vec::dot(*this, other);
        }
        static vec cross(const vec& left, const vec& right)
            requires ((std::same_as<T, float> || std::same_as<T, double>) && LEN == 3)
        {
            return vec(
                left.data[1] * right.data[2] - left.data[2] * right.data[1],
                left.data[2] * right.data[0] - left.data[0] * right.data[2],
                left.data[0] * right.data[1] - left.data[1] * right.data[0]
            );
        }
        vec cross(const vec& other) const
            requires ((std::same_as<T, float> || std::same_as<T, double>) && LEN == 3)
        {
            return vec::cross(*this, other);
        }

#include "vec_comb.hpp"

    };

    template<typename T>
    using vec2 = vec<T, 2>;
    template<typename T>
    using vec3 = vec<T, 3>;
    template<typename T>
    using vec4 = vec<T, 4>;

    using int2 = vec<int, 2>;
    using int3 = vec<int, 3>;
    using int4 = vec<int, 4>;

    using byte2 = vec<uint8_t, 2>;
    using byte3 = vec<uint8_t, 3>;
    using byte4 = vec<uint8_t, 4>;

    using float2 = vec<float, 2>;
    using float3 = vec<float, 3>;
    using float4 = vec<float, 4>;

    using double2 = vec<double, 2>;
    using double3 = vec<double, 3>;
    using double4 = vec<double, 4>;

    using vec2b = vec<uint8_t, 2>;
    using vec3b = vec<uint8_t, 3>;
    using vec4b = vec<uint8_t, 4>;

    using vec2i = vec<int, 2>;
    using vec3i = vec<int, 3>;
    using vec4i = vec<int, 4>;

    using vec2f = vec<float, 2>;
    using vec3f = vec<float, 3>;
    using vec4f = vec<float, 4>;

    using vec2d = vec<double, 2>;
    using vec3d = vec<double, 3>;
    using vec4d = vec<double, 4>;

    template<typename T, uint32_t LEN>
    std::string to_string(vec<T, LEN> v)
    {
        return v.str();
    }

} 