#pragma once
#ifndef ANIMATION_HPP
#define ANIMATION_HPP

#include <chrono>
#include <unordered_map>
#include <numbers>
#include <functional>

namespace rgui
{
	namespace math
	{
		namespace easing
		{
			namespace functions
			{
				inline float lerp(float t)
				{
					return t;
				}

				inline float in_sine(float t)
				{
					return 1.f - std::cosf((t * std::numbers::pi_v<float>) / 2.f);
				}

				inline float out_sine(float t)
				{
					return std::sinf((t * std::numbers::pi_v<float>) / 2.f);
				}

				inline float in_out_sine(float t)
				{
					return -(std::cosf(t * std::numbers::pi_v<float>) - 1.f) / 2.f;
				}
				
				inline float in_cubic(float t)
				{
                    return std::powf(t, 3.f);
				}

				inline float out_cubic(float t)
				{
					return 1.f - std::powf(1.f - t, 3.f);
				}

				inline float in_out_cubic(float t)
				{
					return t < 0.5f ? 4.f * std::powf(t, 3.f) : 1.f - std::powf(-2.f * t + 2.f, 3.f) / 2.f;
				}

				inline float in_quad(float t)
				{
					return t * t;
				}

				inline float out_quad(float t)
				{
					return t * (2.f - t);
				}

				inline float in_out_quad(float t)
				{
					return t < 0.5f ? 2.f * t * t : t * (4.f - 2.f * t) - 1.f;
				}

				inline float in_quart(float t) 
				{
					t *= t;
					return t * t;
				}

				inline float out_quart(float t)
				{
					t = (--t) * t;
					return 1.f - t * t;
				}

				inline float in_out_quart(float t)
				{
					if (t < 0.5f)
					{
						t *= t;
						return 8.f * t * t;
					}
					t = (--t) * t;
					return 1.f - 8.f * t * t;
				}

				inline float in_quint(float t)
				{
					float t2 = t * t;
					return t * t2 * t2;
				}

				inline float out_quint(float t)
				{
					float t2 = (--t) * t;
					return 1 + t * t2 * t2;
				}

				inline float in_out_quint(float t) 
				{
					float t2;
					if (t < 0.5f) {
						t2 = t * t;
						return 16.f * t * t2 * t2;
					}
					else {
						t2 = (--t) * t;
						return 1.f + 16.f * t * t2 * t2;
					}
				}

				inline float in_expo(float t) 
				{
					return (std::powf(2.f, 8.f * t) - 1.f) / 255.f;
				}

				inline float out_expo(float t) 
				{
					return 1.f - std::powf(2.f, -8.f * t);
				}

				inline float in_out_expo(float t)
				{
					if (t < 0.5f) {
						return (std::powf(2.f, 16.f * t) - 1.f) / 510.f;
					}
					else {
						return 1.f - 0.5f * std::powf(2.f, -16.f * (t - 0.5f));
					}
				}

				inline float in_circ(float t)
				{
					return 1.f - std::sqrtf(1.f - t);
				}

				inline float out_circ(float t) 
				{
					return std::sqrtf(t);
				}

				inline float in_out_circ(float t)
				{
					if (t < 0.5f) {
						return (1.f - std::sqrtf(1.f - 2.f * t)) * 0.5f;
					}
					else {
						return (1.f + std::sqrtf(2.f * t - 1.f)) * 0.5f;
					}
				}

				inline float in_back(float t) 
				{
					return t * t * (2.70158f * t - 1.70158f);
				}

				inline float out_back(float t) 
				{
					return 1.f + (--t) * t * (2.70158f * t + 1.70158f);
				}

				inline float in_out_back(float t) 
				{
					if (t < 0.5f) {
						return t * t * (7.f * t - 2.5f) * 2.f;
					}
					else {
						return 1.f + (--t) * t * 2.f * (7.f * t + 2.5f);
					}
				}

				inline float in_elastic(float t)
				{
					float t2 = t * t;
					return t2 * t2 * std::sinf(t * std::numbers::pi_v<float> * 4.5f);
				}

				inline float out_elastic(float t) 
				{
					float t2 = (t - 1.f) * (t - 1.f);
					return 1 - t2 * t2 * std::cosf(t * std::numbers::pi_v<float> * 4.5f);
				}

				inline float in_out_elastic(float t)
				{
					float t2;
					if (t < 0.45f) {
						t2 = t * t;
						return 8.f * t2 * t2 * std::sinf(t * std::numbers::pi_v<float> * 9.f);
					}
					else if (t < 0.55f) {
						return 0.5f + 0.75f * std::sinf(t * std::numbers::pi_v<float> * 4.f);
					}
					else {
						t2 = (t - 1.f) * (t - 1.f);
						return 1.f - 8.f * t2 * t2 * std::sinf(t * std::numbers::pi_v<float> * 9.f);
					}
				}

				inline float in_bounce(float t)
				{
					return std::powf(2.f, 6.f * (t - 1.f)) * std::fabsf(std::sinf(t * std::numbers::pi_v<float> * 3.5f));
				}

				inline float out_bounce(float t)
				{
					return 1.f - std::powf(2.f, -6.f * t) * std::fabsf(std::cosf(t * std::numbers::pi_v<float> * 3.5f));
				}

				inline float in_out_bounce(float t)
				{
					if (t < 0.5f) {
						return 8.f * std::powf(2, 8.f * (t - 1.f)) * std::fabsf(std::sinf(t * std::numbers::pi_v<float> * 7.f));
					}
					else {
						return 1.f - 8.f * std::powf(2.f, -8.f * t) * std::fabsf(std::sinf(t * std::numbers::pi_v<float> * 7.f));
					}
				}
			}

			enum class E_EASING
			{
				LERP = 0,
				IN_SINE,
				OUT_SINE,
				IN_OUT_SINE,
				IN_CUBIC,
				OUT_CUBIC,
				IN_OUT_CUBIC,
				IN_QUINT,
				OUT_QUIT,
				IN_OUT_QUINT,
				IN_CIRC,
				OUT_CIRC,
				IN_OUT_CIRC,
				IN_ELASTIC,
				OUT_ELASTIC,
				IN_OUT_ELASTIC,
				IN_QUAD,
				OUT_QUAD,
				IN_OUT_QUAD,
				IN_QUART,
				OUT_QUART,
				IN_OUT_QUART,
				IN_EXPO,
				OUT_EXPO,
				IN_OUT_EXPO,
				IN_BACK,
				OUT_BACK,
				IN_OUT_BACK,
				IN_BOUNCE,
				OUT_BOUNCE,
				IN_OUT_BOUNCE
			};

			inline float ease(const E_EASING method, const float time_fraction)
			{
				const static std::unordered_map<E_EASING, std::function<float(float)>> methods
				{
					{ E_EASING::LERP, functions::lerp },

					{ E_EASING::IN_SINE, functions::in_sine },
					{ E_EASING::OUT_SINE, functions::out_sine },
					{ E_EASING::IN_OUT_SINE, functions::in_out_sine },

					{ E_EASING::IN_CUBIC, functions::in_cubic },
					{ E_EASING::OUT_CUBIC, functions::out_cubic },
					{ E_EASING::IN_OUT_CUBIC, functions::in_out_cubic },

					{ E_EASING::IN_QUINT, functions::in_quint },
					{ E_EASING::OUT_QUIT, functions::out_quint },
					{ E_EASING::IN_OUT_QUINT, functions::in_out_quint },

					{ E_EASING::IN_CIRC, functions::in_circ },
					{ E_EASING::OUT_CIRC, functions::out_circ },
					{ E_EASING::IN_OUT_CIRC, functions::in_out_circ },

					{ E_EASING::IN_ELASTIC, functions::in_elastic },
					{ E_EASING::OUT_ELASTIC, functions::out_elastic },
					{ E_EASING::IN_OUT_ELASTIC, functions::in_out_elastic },

					{ E_EASING::IN_QUAD, functions::in_quad },
					{ E_EASING::OUT_QUAD, functions::out_quad },
					{ E_EASING::IN_OUT_QUAD, functions::in_out_quad },

					{ E_EASING::IN_QUART, functions::in_quart },
					{ E_EASING::OUT_QUART, functions::out_quart },
					{ E_EASING::IN_OUT_QUART, functions::in_out_quart },

					{ E_EASING::IN_EXPO, functions::in_expo },
					{ E_EASING::OUT_EXPO, functions::out_expo },
					{ E_EASING::IN_OUT_EXPO, functions::in_out_expo },

					{ E_EASING::IN_BACK, functions::in_back },
					{ E_EASING::OUT_BACK, functions::out_back },
					{ E_EASING::IN_OUT_BACK, functions::in_out_back },

					{ E_EASING::IN_BOUNCE, functions::in_bounce },
					{ E_EASING::OUT_BOUNCE, functions::out_bounce },
					{ E_EASING::IN_OUT_BOUNCE, functions::in_out_bounce }
				};

				return methods.at(method)(time_fraction);
			}

			inline float get_time_fraction(const std::chrono::system_clock::time_point timestamp, const int duration)
			{
				return std::clamp(static_cast<float>(duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - timestamp).count()) / static_cast<float>(duration), 0.f, 1.f);
			}

			struct animation_t
			{
				animation_t()
				{
					m_timestamp = std::chrono::system_clock::from_time_t(0);
				}

				inline void begin()
				{
					m_timestamp = std::chrono::system_clock::now();
				}

				inline float get_time_fraction(const int duration, const int passed = 0) const
				{
					return ::rgui::math::easing::get_time_fraction(m_timestamp, duration - passed);
				}

				inline bool get_has_ended(const int duration) const
				{
					return get_time_fraction(duration) >= 1.f;
				}

				inline float ease(const E_EASING method, const int duration, const bool reverse = false) const
				{
					const float blend = ::rgui::math::easing::ease(method, get_time_fraction(duration));
					if (reverse)
						return 1.f - blend;
					return blend;
				}

				inline float ease(const E_EASING method, const int duration, const int passed, const bool reverse = false) const
				{
					const float blend = ::rgui::math::easing::ease(method, get_time_fraction(duration, passed));
					if (reverse)
						return 1.f - blend;
					return blend;
				}


				std::chrono::system_clock::time_point m_timestamp;
			};
		}
	}
}

#endif