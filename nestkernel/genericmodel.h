/*
 *  genericmodel.h
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

#ifndef GENERICMODEL_H
#define GENERICMODEL_H

// Includes from nestkernel:
#include "logging_manager.h"
#include "model.h"

namespace nest
{
/**
 * Generic Model template.
 *
 * The template GenericModel should be used
 * as base class for custom model classes. It already includes the
 * element factory functionality, as well as a pool based memory
 * manager, so that the user can concentrate on the "real" model
 * aspects.
 * @ingroup user_interface
 */
template < typename ElementT >
class GenericModel : public Model
{
public:
  GenericModel( const std::string&, const std::string& deprecation_info );

  /**
   * Create copy of model with new name.
   */
  GenericModel( const GenericModel&, const std::string& );

  /**
   * Return pointer to cloned model with same name.
   */
  Model* clone( const std::string& ) const override;

  bool has_proxies() override;
  bool one_node_per_process() override;
  bool is_off_grid() override;
  void calibrate_time( const TimeConverter& tc ) override;

  /**
   * Send a test event to a target node.
   *
   * This is a forwarding function that calls Node::send_test_event() from the prototype.
   * Since proxies know the model they represent, they can now answer a call to check
   * connection by referring back to the model.
   */
  size_t send_test_event( Node&, size_t, synindex, bool ) override;

  void sends_secondary_event( GapJunctionEvent& ge ) override;

  SignalType sends_signal() const override;

  void sends_secondary_event( InstantaneousRateConnectionEvent& re ) override;

  void sends_secondary_event( DiffusionConnectionEvent& de ) override;

  void sends_secondary_event( DelayedRateConnectionEvent& re ) override;

  void sends_secondary_event( LearningSignalConnectionEvent& re ) override;

  void sends_secondary_event( SICEvent& sic ) override;

  Node const& get_prototype() const override;

  void set_model_id( int ) override;

  int get_model_id() override;

  void deprecation_warning( const std::string& ) override;

private:
  void set_status_( DictionaryDatum ) override;
  DictionaryDatum get_status_() override;

  size_t get_element_size() const override;

  /**
   * Call placement new on the supplied memory position.
   */
  Node* create_() override;

  /**
   * Prototype node from which all instances are constructed.
   */
  ElementT proto_;

  /**
   * String containing deprecation information; empty if model not deprecated.
   */
  std::string deprecation_info_;

  //! False until deprecation warning has been issued once
  bool deprecation_warning_issued_;
};

} // namespace nest

#endif
