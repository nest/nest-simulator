/*
 *  music_event_handler.cpp
 *
 *  This file is part of NEST.
 *
 *  Copyright (C) 2004 The NEST Initiative
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
#include "config.h"

#ifdef HAVE_MUSIC

#include "music_event_handler.h"
#include "nest.h"
#include "event.h"
#include "communicator.h"
#include "network.h" // needed for event_impl.h to be included

namespace nest
{
  MusicEventHandler::MusicEventHandler()
          : music_port_(0),
            music_perm_ind_(0),
            published_(false),
            portname_(""),
            acceptable_latency_(0.0)
  {}

  MusicEventHandler::MusicEventHandler(std::string portname, double acceptable_latency, Network* net)
          : music_port_(0),
            music_perm_ind_(0),
            published_(false),
            portname_(portname),
            acceptable_latency_(acceptable_latency),
            net_(net)
  {}

  MusicEventHandler::~MusicEventHandler()
  {
    if (published_)
    {
      if (music_perm_ind_ != 0)
        delete music_perm_ind_;
      
      if (music_port_ != 0)
        delete music_port_;
    }
  }

  void MusicEventHandler::register_channel(int channel, nest::Node *mp)
  {
    if (channel >= channelmap_.size())
    {
      channelmap_.resize(channel + 1, 0); // all entries not explicitly set will be 0
      eventqueue_.resize(channel + 1);
    }
    
    if (channelmap_[channel] != 0)
      throw MUSICChannelAlreadyMapped("MusicEventHandler", portname_, channel);

    channelmap_[channel] = mp;
    indexmap_.push_back(channel);
  }

  void MusicEventHandler::publish_port()
  {
    if (!published_)
    {
      music_port_ = Communicator::get_music_setup()->publishEventInput(portname_);

      // MUSIC wants seconds, NEST has miliseconds
      double_t acceptable_latency = acceptable_latency_/1000.0;

      if (!music_port_->isConnected())
        throw MUSICPortUnconnected("MusicEventHandler", portname_);

      if (!music_port_->hasWidth())
	throw MUSICPortHasNoWidth("MusicEventHandler", portname_);

      unsigned int music_port_width = music_port_->width();

      // check, if all mappings are within the valid range of port width
      // the maximum channel mapped - 1 == size of channelmap
      if (channelmap_.size() > music_port_width)
        throw MUSICChannelUnknown("MusicEventHandler", portname_, channelmap_.size()-1);
    
      // create the permutation index mapping
      music_perm_ind_ = new MUSIC::PermutationIndex(&indexmap_.front(), indexmap_.size());
      // map the port
      music_port_->map(music_perm_ind_, this, acceptable_latency);

      std::string msg = String::compose("Mapping MUSIC input port '%1' with width=%2 and acceptable latency=%3 ms.",
                                        portname_, music_port_width, acceptable_latency_);
      net_->message(SLIInterpreter::M_INFO, "MusicEventHandler::publish_port()", msg.c_str());
    }
  }

  void MusicEventHandler::operator()(double t, MUSIC::GlobalIndex channel)
  {
    assert(channelmap_[channel] != 0);
    eventqueue_[channel].push(t*1e3); // MUSIC uses seconds as time unit
  }

  void MusicEventHandler::update(Time const & origin, const long_t from, const long_t to)
  {
    for (size_t channel = 0; channel < channelmap_.size(); ++channel)
      if (channelmap_[channel] != 0)
        while(! eventqueue_[channel].empty())
        {
          Time T = Time::ms(eventqueue_[channel].top());

          if (T > origin + Time::step(from) - Time::ms(acceptable_latency_) && T <= origin + Time::step(from + to))
          {
            nest::SpikeEvent se;
            se.set_offset(Time(Time::step(T.get_steps())).get_ms() - T.get_ms());
            se.set_stamp(T);
        
            channelmap_[channel]->handle(se); // deliver to the proxy for this channel
            eventqueue_[channel].pop();       // remove the sent event from the queue
          }
          else
            break;          
        }
  }

} // namespace nest

#endif // #ifdef HAVE_MUSIC
