/*
*  SrmPeceveskiAlpha.h
*
*  This file is an extension of NEST.
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
*  File based on poisson_dbl_exp_neuron.h in NEST::SPORE extension.
*
*/

#include "srm_pecevski_alpha.h"

#include "dict.h"
#include "integerdatum.h"
#include "doubledatum.h"
#include "dictutils.h"
#include "numerics.h"
#include "universal_data_logger_impl.h"
#include "compose.hpp"

#include "param_utils.h"
#include "../nestkernel/logging_manager.h"


namespace nest
{

	/*
	* Recordables map of SrmPecevskiAlpha.
	*/
	template < >
	void nest::RecordablesMap<SrmPecevskiAlpha>::create()
	{
		// use standard names whereever you can for consistency!
		insert_(names::V_m, &SrmPecevskiAlpha::get_V_m_);
		insert_(names::E_sfa, &SrmPecevskiAlpha::get_E_sfa_);
	}

	/*
	* Recordables map instance.
	*/
	nest::RecordablesMap<SrmPecevskiAlpha> SrmPecevskiAlpha::recordablesMap_;

	//
	// SrmPecevskiAlpha::Parameters_ implementation.
	//
	SrmPecevskiAlpha::Parameters_::Parameters_(): 
	epsilon_0_exc_(2.8),			// mv
	epsilon_0_inh_(2.8),			// mv
	tau_alpha_exc_(8.5),			// ms
	tau_alpha_inh_(8.5),			// ms
	input_conductance_(1),		
	dead_time_(1.0),				// ms
	dead_time_random_(false),		// ms
	dead_time_shape_(1l),
	with_reset_(true),
	c_1_(0),
	c_2_(1.238),
	c_3_(0.25),
	I_e_(0),						// pA
	t_ref_remaining_(0),			// ms
	target_rate_(10.0),				// Hz
	target_adaptation_speed_(0.0)
	{
		
	}

	void SrmPecevskiAlpha::Parameters_::get(DictionaryDatum& d) const
	{
		def< double >(d, names::dead_time, dead_time_);
		def< double >(d, names::dead_time_random, dead_time_random_);
		def< long >(d, names::dead_time_shape, dead_time_shape_);
		def< double >(d, names::e_0_exc, epsilon_0_exc_);
		def< double >(d, names::e_0_inh, epsilon_0_inh_);
		def< double >(d, names::tau_exc, tau_alpha_exc_);
		def< double >(d, names::tau_inh, tau_alpha_inh_);
		def< bool >(d, names::with_reset, with_reset_);
		def< double >(d, names::c_1, c_1_);
		def< double >(d, names::c_2, c_2_);
		def< double >(d, names::c_3, c_3_);
		def< double >(d, names::I_e, I_e_);
		def< double >(d, names::t_ref_remaining, t_ref_remaining_);
		def< double >(d, names::input_conductance, input_conductance_);
		def< double >(d, names::target_rate, target_rate_);
		def< double >(d, names::target_adaptation_speed, target_adaptation_speed_);
	}

	void SrmPecevskiAlpha::Parameters_::set(const DictionaryDatum& d)
	{
		updateValue< double >(d, names::dead_time, dead_time_);
		updateValue< double >(d, names::dead_time_random, dead_time_random_);
		updateValue< long >(d, names::dead_time_shape, dead_time_shape_);
		updateValue< double >(d, names::e_0_exc, epsilon_0_exc_);
		updateValue< double >(d, names::e_0_inh, epsilon_0_inh_);
		updateValue< double >(d, names::tau_exc, tau_alpha_exc_);
		updateValue< double >(d, names::tau_inh, tau_alpha_inh_);
		updateValue< bool >(d, names::with_reset, with_reset_);
		updateValue< double >(d, names::c_1, c_1_);
		updateValue< double >(d, names::c_2, c_2_);
		updateValue< double >(d, names::c_3, c_3_);
		updateValue< double >(d, names::I_e, I_e_);
		updateValue< double >(d, names::t_ref_remaining, t_ref_remaining_);
		updateValue< double >(d, names::input_conductance, input_conductance_);
		updateValue< double >(d, names::target_rate, target_rate_);
		updateValue< double >(d, names::target_adaptation_speed, target_adaptation_speed_);

		if (dead_time_ < 0.0)
		{
			throw BadProperty("Dead time must be >= 0.");
		}

		if (dead_time_shape_ < 1)
		{
			throw BadProperty("Dead time shape must be >= 1.");
		}

		if (tau_alpha_exc_ <= 0.0 || tau_alpha_inh_ <= 0.0)
		{
			throw BadProperty("All decay constants must be greater than 0.");
		}

		if (epsilon_0_exc_ <= 0.0 || epsilon_0_inh_ <= 0.0)
		{
			throw BadProperty("All PSP absolute amplitudes bust be greater than 0.");
		}

		if (c_3_< 0.0)
		{
			throw BadProperty("c_3 must be >= 0.");
		}

		if (t_ref_remaining_ < 0.0)
		{
			throw BadProperty("t_ref_remaining must be >= 0.");
		}

		if (target_rate_ < 0.0)
		{
			throw BadProperty("target_rate must be >= 0.");
		}

		if (target_adaptation_speed_ < 0.0)
		{
			throw BadProperty("target_adaptation_speed must be >= 0.");
		}
	}

	//
	// SrmPecevskiAlpha::State_ implementation.
	//


	/**
	* Default constructor.
	*/
	SrmPecevskiAlpha::State_::State_():
		u_membrane_(0.0),
		input_current_(0.0),
		adaptive_threshold_(0.0),
		r_(0)
	{
	}

	/**
	* State getter function.
	*/
	void SrmPecevskiAlpha::State_::get(DictionaryDatum& d, const Parameters_&) const
	{
		def<double>(d, nest::names::V_m, u_membrane_); // Membrane potential
		def<double>(d, names::adaptive_threshold, adaptive_threshold_);
	}

	/**
	* Sate setter function.
	*/
	void SrmPecevskiAlpha::State_::set(const DictionaryDatum& d, const Parameters_&)
	{
		updateValue<double>(d, nest::names::V_m, u_membrane_);
		updateValue<double>(d, names::adaptive_threshold, adaptive_threshold_);
	}

	//
	// SrmPecevskiAlpha::Buffers_ implementation.
	//

	/**
	* Constructor.
	*/
	SrmPecevskiAlpha::Buffers_::Buffers_(SrmPecevskiAlpha& n)
		: logger_(n)
	{
	}

	/**
	* Constructor.
	*/
	SrmPecevskiAlpha::Buffers_::Buffers_(const Buffers_&, SrmPecevskiAlpha& n)
		: logger_(n)
	{
	}

	//
	// SrmPecevskiAlpha implementation.
	//

	/**
	* Default Constructor.
	*/
	SrmPecevskiAlpha::SrmPecevskiAlpha()
		: TracingNode(),
		P_(),
		S_(),
		B_(*this)
	{
		recordablesMap_.create();
	}

	/**
	* Copy Constructor.
	*/
	SrmPecevskiAlpha::SrmPecevskiAlpha(const SrmPecevskiAlpha& n)
		: Archiving_Node(n),
		P_(n.P_),
		S_(n.S_),
		B_(n.B_, *this)
	{
	}

	/**
	* Node state initialization.
	*/
	void SrmPecevskiAlpha::init_state_(const nest::Node& proto)
	{
		const SrmPecevskiAlpha& pr = downcast<SrmPecevskiAlpha>(proto);
		S_ = pr.S_;
		S_.r_ = nest::Time(nest::Time::ms(P_.t_ref_remaining_)).get_steps();
	}

	/**
	* Initialize the node's spike and current buffers.
	*/
	void SrmPecevskiAlpha::init_buffers_()
	{
		B_.exc_queue_.Clear();
		B_.inh_queue_.Clear();
		B_.currents_.clear(); //!< includes resize
		B_.logger_.reset(); //!< includes resize

		init_traces(1);
	}

	/**
	* Calibrate the node.
	*/
	void SrmPecevskiAlpha::calibrate()
	{
		B_.logger_.init();

		V_.h_ = nest::Time::get_resolution().get_ms();
		V_.rng_ = nest::kernel().rng_manager.get_rng(get_thread());

		V_.t_1 = 0.23196095298653444;

		if (P_.dead_time_ != 0 && P_.dead_time_ < V_.h_)
			P_.dead_time_ = V_.h_;

		// TauR specifies the length of the absolute refractory period as
		// a double in ms. The grid based iaf_psp_delta can only handle refractory
		// periods that are integer multiples of the computation step size (h).
		// To ensure consistency with the overall simulation scheme such conversion
		// should be carried out via objects of class nest::Time. The conversion
		// requires 2 steps:
		//
		//     1. A time object r is constructed defining the representation of
		//        TauR in tics. This representation is then converted to computation time
		//        steps again by a strategy defined by class nest::Time.
		//     2. The refractory time in units of steps is read out by get_steps(), a member
		//        function of class nest::Time.
		//
		// The definition of the refractory period of the SrmPecevskiAlpha is consistent
		// with the one of iaf_neuron_ps.
		//
		// Choosing a TauR that is not an integer multiple of the computation time
		// step h will lead to accurate (up to the resolution h) and self-consistent
		// results. However, a neuron model capable of operating with real valued spike
		// time may exhibit a different effective refractory time.
		if (P_.dead_time_random_)
		{
			// Choose dead time rate parameter such that mean equals dead_time
			V_.dt_rate_ = P_.dead_time_shape_ / P_.dead_time_;
			V_.gamma_dev_.set_order(P_.dead_time_shape_);
		}
		else
		{
			V_.DeadTimeCounts_ = nest::Time(nest::Time::ms(P_.dead_time_)).get_steps();
			assert(V_.DeadTimeCounts_ >= 0); // Since t_ref_ >= 0, this can only fail in error
		}
	}

	/*
	* Spike response kernel.
	*/
	double SrmPecevskiAlpha::kernel(const double time_since_spike, const bool use_exc_kernel = true) const
	{
		const double& t = time_since_spike;
		if (use_exc_kernel)
		{
			return P_.epsilon_0_exc_ * ((t / P_.tau_alpha_exc_ + V_.t_1) * (std::exp(1 - (t / P_.tau_alpha_exc_ + V_.t_1))) - 0.5);
		}
		else
		{
			return P_.epsilon_0_inh_ * ((t / P_.tau_alpha_inh_ + V_.t_1) * (std::exp(1 - (t / P_.tau_alpha_inh_ + V_.t_1))) - 0.5);
		}
	}

	/* 
	 * Sums up the PSPs from excitatory or inhibitory spikes.
	 */
	double SrmPecevskiAlpha::get_psp_sum(nest::Time const& now, const bool use_exc_psp)
	{
		double psp = 0;

		if (use_exc_psp)
		{
			for (SpikeQueue::IteratorType it = B_.exc_queue_.Begin(); it != B_.exc_queue_.End(); ++it)
			{
				// Get spike time and value.
				long spike_time = it->first;
				double amplitude = it->second;

				double this_psp = amplitude * kernel((now - spike_time).get_ms(), true);
				if (this_psp <= 0)
				{
					this_psp = 0;
					
					// Erase the spike, because we won't need it anymore.
					it = B_.exc_queue_.EraseItemAt(it);
				}

				psp += this_psp;
			}
		}
		else
		{
			for (SpikeQueue::IteratorType it = B_.inh_queue_.Begin(); it != B_.inh_queue_.End(); ++it)
			{
				// Get spike time and value.
				long spike_time = it->first;
				double amplitude = it->second;

				double this_psp = amplitude * kernel((now - spike_time).get_ms(), false);
				if (this_psp <= 0)
				{
					this_psp = 0;

					// Erase the spike, because we won't need it anymore.
					it = B_.exc_queue_.EraseItemAt(it);
				}

				psp -= this_psp;
			}
		}

		return psp;
	}


	/**
	* Update the node to the given time point.
	*/
	void SrmPecevskiAlpha::update(nest::Time const& origin, const long from, const long to)
	{
		assert(from < to);

		for (long lag = from; lag < to; ++lag)
		{
			nest::Time now = nest::Time::step(origin.get_steps() + lag);

			double psp_exc = get_psp_sum(now, true);
			double psp_inh = get_psp_sum(now, false);

			S_.u_membrane_ = psp_exc + psp_inh + P_.input_conductance_ * (S_.input_current_ + P_.I_e_);

			S_.adaptive_threshold_ -= 1e-3 * V_.h_ * P_.target_rate_ * P_.target_adaptation_speed_;

			if (S_.r_ == 0)
			{
				// Neuron not refractory

				// Calculate instantaneous rate from transfer function:
				//     rate = c1 * u' + c2 * exp(c3 * u')

				double V_eff = S_.u_membrane_ - S_.adaptive_threshold_;

				double rate = (P_.c_1_ * V_eff + P_.c_2_ * std::exp(P_.c_3_ * V_eff));
				double spike_probability = -numerics::expm1(-rate * V_.h_ * 1e-3);
				long n_spikes = 0;

				if (rate > 0.0)
				{
					if (P_.dead_time_ > 0.0)
					{
						// Draw random number and compare to probability to have a spike
						if (V_.rng_->drand() <= spike_probability)
							n_spikes = 1;
					}
					else
					{
						// Draw Poisson random number of spikes
						V_.poisson_dev_.set_lambda(rate);
						n_spikes = V_.poisson_dev_.ldev(V_.rng_);
					}

					if (n_spikes > 0) // Is there a spike? Then set the new dead time.
					{
						// Set dead time interval according to parameters
						if (P_.dead_time_random_)
						{
							S_.r_ = nest::Time(nest::Time::ms(V_.gamma_dev_(V_.rng_) / V_.dt_rate_)).get_steps();
						}
						else
							S_.r_ = V_.DeadTimeCounts_;

						// And send the spike event
						nest::SpikeEvent se;
						se.set_multiplicity(n_spikes);
						nest::kernel().event_delivery_manager.send(*this, se, lag);

						// Reset the potential if applicable
						if (P_.with_reset_)
						{
							B_.exc_queue_.Clear();
							B_.inh_queue_.Clear();

							S_.u_membrane_ = 0.0;
						}

						S_.adaptive_threshold_ += P_.target_adaptation_speed_;
					} // S_.u_membrane_ = P_.V_reset_;
				} // if (rate > 0.0)

				set_trace(time.get_steps(), double(n_spikes) - spike_probability);
			}
			else // Neuron is within dead time
			{
				set_trace(time.get_steps(), 0.0);
				--S_.r_;
			}

			// Set new input current
			S_.input_current_ = B_.currents_.get_value(lag);

			// Voltage logging
			B_.logger_.record_data(origin.get_steps() + lag);
		}
	}

	/**
	* SpikeEvent handling.
	* @param e the event.
	*/
	void SrmPecevskiAlpha::handle(nest::SpikeEvent& e)
	{
		assert(e.get_delay() > 0);

		// We must compute the arrival time of the incoming spike
		// explicitly, since it depends on delay and offset within
		// the update cycle.  The way it is done here works, but
		// is clumsy and should be improved.
		if (e.get_rport() == 0)
		{
			// Add spike to the queue.
			// Note: we need to compute the absolute number of steps since the beginning of simulation time.
			B_.exc_queue_.AddSpike(e.get_rel_delivery_steps(0), e.get_weight() * e.get_multiplicity());
		}
		else if (e.get_rport() == 1)
		{
			// Add spike to the queue.
			// Note: we need to compute the absolute number of steps since the beginning of simulation time.
			B_.inh_queue_.AddSpike(e.get_rel_delivery_steps(0), e.get_weight() * e.get_multiplicity());
		}
		else
		{
			std::ostringstream msg;
			msg << "Unexpected rport id: " << e.get_rport();
			throw nest::BadProperty(msg.str());
		}
	}

	/**
	* CurrentEvent handling.
	* @param e the event.
	*/
	void SrmPecevskiAlpha::handle(nest::CurrentEvent& e)
	{
		assert(e.get_delay() > 0);

		const double c = e.get_current();
		const double w = e.get_weight();

		B_.currents_.add_value(e.get_rel_delivery_steps(nest::kernel().simulation_manager.get_slice_origin()), w * c);
	}

	/**
	* DataLoggingRequest handling.
	* @param e the event.
	*/
	void SrmPecevskiAlpha::handle(nest::DataLoggingRequest& e)
	{
		B_.logger_.handle(e);
	}

}
