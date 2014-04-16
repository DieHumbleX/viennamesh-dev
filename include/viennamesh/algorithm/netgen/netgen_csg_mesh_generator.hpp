#ifndef VIENNAMESH_ALGORITHM_NETGEN_CSG_MESH_GENERATOR_HPP
#define VIENNAMESH_ALGORITHM_NETGEN_CSG_MESH_GENERATOR_HPP

#include "viennamesh/core/algorithm.hpp"

namespace viennamesh
{
  namespace netgen
  {
    class csg_mesh_generator : public base_algorithm
    {
    public:
      csg_mesh_generator();

      string name() const;
      string id() const;

      bool run_impl();

    private:
      required_input_parameter_interface<string>        input_csg_source;
      default_input_parameter_interface<double>         relative_find_identic_surface_eps;
      default_input_parameter_interface<double>         cell_size;
      default_input_parameter_interface<double>         grading;
      default_input_parameter_interface<int>            optimization_steps;
      default_input_parameter_interface<bool>           delaunay;
      optional_input_parameter_interface<string>        optimize_string;

      output_parameter_interface                        output_mesh;
    };
  }

}



#endif