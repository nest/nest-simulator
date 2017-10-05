/*
*  spike_queue.h
*
*  This file is part of SAM, an extension of NEST.
*
*  Copyright (C) 2017 D'Amato
*
*  NEST is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 2 of the License, or
*  (at your option) any later version.
*
*  NEST is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with NEST.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#ifndef SPIKE_QUEUE_H
#define SPIKE_QUEUE_H
#endif

#include <deque>

namespace sam
{
	/*
	 * A simple queue that takes time step - amplitude pairs
	 * and stores them for SRM kernel calculations.
	 */
	class SpikeQueue
	{
		typedef std::deque<std::pair<long, double>> BufferType;

	public:
		typedef BufferType::const_iterator IteratorType;

		// Methods.
		void AddSpike(const long timeStep, const double amplitude);
		void Clear();
		IteratorType Begin() const;
		IteratorType End() const;
		IteratorType EraseItemAt(IteratorType& it);

	private:
		BufferType buffer_;
	};

	inline void SpikeQueue::AddSpike(const long timeStep, const double amplitude)
	{
		buffer_.push_back(std::make_pair(timeStep, amplitude));
	}

	inline void SpikeQueue::Clear()
	{
		buffer_.clear();
	}

	inline std::deque<std::pair<long, double>>::const_iterator SpikeQueue::Begin() const
	{
		return buffer_.cbegin();
	}

	inline std::deque<std::pair<long, double>>::const_iterator SpikeQueue::End() const
	{
		return buffer_.cend();
	}

	inline SpikeQueue::IteratorType SpikeQueue::EraseItemAt(IteratorType& it)
	{
		return buffer_.erase(it);
	}
}