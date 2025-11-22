#pragma once

#include <cassert>
#include <concepts>
#include <queue>
#include <ranges>
#include <stdexcept>

namespace moving_average
{
	/// Concept to ensure that a queue type has necessary operations
	template <typename QueueT, typename ValueT, typename... Args>
	concept QueueConcept = requires(
		QueueT queue,
		const QueueT const_queue,
		ValueT value,
		Args... args) {
		typename decltype(queue)::value_type;
		{ queue.push(value) } -> std::same_as<void>;
		{ queue.pop() } -> std::same_as<void>;
		{ queue.front() } -> std::same_as<ValueT &>;
		{ queue.back() } -> std::same_as<ValueT &>;
		{ queue.emplace(args...) };

		{ const_queue.front() } -> std::same_as<const ValueT &>;
		{ const_queue.back() } -> std::same_as<const ValueT &>;
		{ const_queue.empty() } -> std::same_as<bool>;
	};

	/// Structure to represent an event with time and value
	// Unfortunately placing it inside the MovingAverage class is very tricky
	// due to default template parameters for the QueueT type.
	template <
		std::integral TimeT,
		std::floating_point ValueT>
	struct MovingAverageEvent
	{
		// Intentionally no member initializers for performance reasons
		TimeT time;
		ValueT value;
	};

	/**
		This class maintains addition of events in the following style:
		the addition of (0, 1), (3, 2) is equal to:
		time  0 1 2 3
		value 1 1 1 2

		So the moving average for window with:
		size = 1 is 2,
		size = 2 is 3 / 2
		size = 3 is 4 / 3
		size = 4 is 5 / 4
	*/
	template <
		std::unsigned_integral TimeT,
		std::floating_point ValueT,
		QueueConcept<MovingAverageEvent<TimeT, ValueT>> QueueT =
			std::queue<MovingAverageEvent<TimeT, ValueT>>>
	class MovingAverage
	{
	public:
		/// Constructor taking the window size
		explicit MovingAverage(TimeT window_size) : m_window_size(window_size)
		{
			if (window_size == TimeT(0))
			{
				throw std::invalid_argument(
					"The moving average window size must be greater than zero.");
			}
		}

		/// Adds a new event and returns the current moving average
		ValueT add_event(TimeT timestamp, ValueT value)
		{
			if (!m_events.empty())
			{
				// Assertion instead of exception for performance reasons
				assert(timestamp > m_events.back().time);

				const auto &last = m_events.back();
				// -1 here because we have already added this event
				// with the weight = 1
				// in the previous call
				m_sum += last.value * (timestamp - last.time - 1);
			}

			// Add the value of the new event with weight = 1
			m_events.emplace(timestamp, value);
			m_sum += value;

			// timestamp >= m_window_size to set to zero in case of underflow
			// in (timestamp - m_window_size) + 1 the parentheses are important to avoid underflow
			const TimeT window_start =
				((timestamp - m_window_size) + 1) *
				(timestamp >= m_window_size);

			while (m_events.front().time < window_start)
			{
				if (m_last_popped_event.time != UNSET_TIME)
				{
					m_sum -=
						m_last_popped_event.value *
						(m_events.front().time - m_last_popped_event.time);
				}
				m_last_popped_event = m_events.front();
				m_events.pop();
			}

			// If the last popped event is set and partially overlaps with the window,
			// adjust the sum and modify its time to the window start
			if (m_last_popped_event.time < window_start)
			{
				m_sum -=
					m_last_popped_event.value *
					(window_start - m_last_popped_event.time);
				m_last_popped_event.time = window_start;
			}

			// When the window is not full yet,
			// m_last_popped_event.time is UNSET_TIME = MAX,
			// and we use the front event time
			const TimeT actual_window_start = std::min(
				m_last_popped_event.time,
				m_events.front().time);

			return m_sum / (timestamp - actual_window_start + 1);
		}

	private:
		/// Marker for unset time
		static constexpr TimeT UNSET_TIME = std::numeric_limits<TimeT>::max();

		/// Window size for the moving average calculation
		const TimeT m_window_size;

		/// Queue to store events within the window
		QueueT m_events;

		/// The most recent event that was popped out of the queue.
		/// The max TimeT is used as an 'unset' marker.
		typename QueueT::value_type m_last_popped_event{UNSET_TIME, 0};

		/// Current sum of event values in the window
		ValueT m_sum{0};
	};

	// namespace moving_average
}
