//
// Created by ayssar on 21.03.22.
//

#ifndef STRUCTURAL_PLASTICITY_VECTOR_H
#define STRUCTURAL_PLASTICITY_VECTOR_H

#include "vectorized_node.h"
namespace nest
{
class StructuralPlasticityVector : public VectorizedNode
{
public:
  /**
   * \fn StructuralPlasticityVector()
   * Constructor.
   */
  StructuralPlasticityVector();

  /**
   * \fn StructuralPlasticityNode()
   * Copy Constructor.
   */
  StructuralPlasticityVector( const StructuralPlasticityVector& );

  double get_synaptic_elements( Name n, index local_id ) const;

  int get_synaptic_elements_vacant( Name n, index local_id ) const;

  int get_synaptic_elements_connected( Name n, index local_id ) const;

  void resize( index extended_space );



  std::map< Name, double > get_synaptic_elements( index local_id ) const;

  void update_synaptic_elements( double t, index local_id );

  void decay_synaptic_elements_vacant( index local_id );

  void connect_synaptic_element( Name name, int n, index local_id );

  void get_status( DictionaryDatum& d, index local_id ) const;

  void set_status( const DictionaryDatum& d, index local_id );

  double get_tau_Ca( index local_id ) const;

  double get_Ca_minus( index local_id ) const;

protected:
  void clear_history( index local_id );
  void set_spiketime( Time const& t_sp, index local_id, double offset = 0.0 );


private:
  /**
   * Time of the last update of the Calcium concentration in ms
   */
  std::vector< double > Ca_t_;

  /**
   * Value of the calcium concentration [Ca2+] at Ca_t_.
   *
   * Intracellular calcium concentration has a linear factor to mean
   * electrical activity of 10^2, this means, for example, that a [Ca2+] of
   * 0.2 is equivalent to a mean activity of 20 Hz.
   */
  std::vector< double > Ca_minus_;

  /**
   * Time constant for exponential decay of the intracellular calcium
   * concentration
   */
  std::vector< double > tau_Ca_;

  /**
   * Increase in calcium concentration [Ca2+] for each spike of the neuron
   */
  std::vector< double > beta_Ca_;

  /**
   * Map of the synaptic elements
   */
  std::vector< std::map< Name, SynapticElement > > synaptic_elements_map_;
};
inline double
StructuralPlasticityVector::get_tau_Ca( index local_id ) const
{
  return tau_Ca_.at( local_id );
}

inline double
StructuralPlasticityVector::get_Ca_minus( index local_id ) const
{
  return Ca_minus_.at( local_id );
}
}


#endif // STRUCTURAL_PLASTICITY_VECTOR_H
