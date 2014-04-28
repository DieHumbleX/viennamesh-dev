#include "viennamesh/algorithm/make_statistic.hpp"

#include "viennamesh/statistic/statistic.hpp"

namespace viennamesh
{
  make_statistic::make_statistic() :
    input_mesh(*this, "mesh"),
    metric_type(*this, "metric_type"),
    histogram_bins(*this, "histogram_bins"),
    histogram_min(*this, "histogram_min"),
    histogram_max(*this, "histogram_max"),
    histogram_bin_count(*this, "histogram_bin_count"),
    output_statistic(*this, "statistic") {}

  string make_statistic::name() const { return "ViennaGrid Statistic"; }
  string make_statistic::id() const { return "make_statistic"; }

  template<typename MeshT, typename SegmentationT>
  bool make_statistic::generic_run()
  {
    typedef viennagrid::segmented_mesh<MeshT, SegmentationT> SegmentedMeshType;
    typename viennamesh::result_of::const_parameter_handle<SegmentedMeshType>::type imp = input_mesh.get<SegmentedMeshType>();
    if (imp)
    {
      typedef viennamesh::statistic<double> StatisticType;
      typedef typename viennagrid::result_of::cell<MeshT>::type CellType;
      output_parameter_proxy<StatisticType> osp(output_statistic);

      if (histogram_bins.valid())
      {
        std::list<string> border_strings = stringtools::split_string( histogram_bins(), "," );
        std::vector<double> borders;
        for (std::list<string>::const_iterator it = border_strings.begin(); it != border_strings.end(); ++it)
          borders.push_back( lexical_cast<double>(*it) );
        osp().set_histogram( StatisticType::histogram_type::make(borders.begin(), borders.end()) );
      }
      else if (histogram_min.valid() && histogram_max.valid() && histogram_bin_count.valid())
      {
        osp().set_histogram( StatisticType::histogram_type::make_uniform(histogram_min(), histogram_max(), histogram_bin_count()) );
      }
      else
      {
        error(1) << "No histogram configuration provided" << std::endl;
        return false;
      }

      try
      {
        if (metric_type() == "aspect_ratio")
          osp()( imp().mesh, viennamesh::aspect_ratio<CellType> );
        else if (metric_type() == "condition_number")
          osp()( imp().mesh, viennamesh::condition_number<CellType> );
        else if (metric_type() == "min_angle")
          osp()( imp().mesh, viennamesh::min_angle<CellType> );
        else if (metric_type() == "max_angle")
          osp()( imp().mesh, viennamesh::max_angle<CellType> );
        else if (metric_type() == "min_dihedral_angle")
          osp()( imp().mesh, viennamesh::min_dihedral_angle<CellType> );
        else if (metric_type() == "radius_edge_ratio")
          osp()( imp().mesh, viennamesh::radius_edge_ratio<CellType> );
        else
        {
          error(1) << "Metric type \"" << metric_type() << "\" is not supported" << std::endl;
          return false;
        }
      }
      catch ( metric_not_implemented_or_supported_exception const & ex )
      {
        error(1) << "Metric type \"" << metric_type() << "\" is not supported" << std::endl;
        error(1) << ex.what() << std::endl;
      }

      info(5) << osp() << std::endl;

      return true;
    }

    return false;
  }

  bool make_statistic::run_impl()
  {
    if (generic_run<viennagrid::triangular_2d_mesh, viennagrid::triangular_2d_segmentation>())
      return true;

    if (generic_run<viennagrid::tetrahedral_3d_mesh, viennagrid::tetrahedral_3d_segmentation>())
      return true;


    error(1) << "Input Parameter 'default' (type: mesh) is missing or of non-convertable type" << std::endl;
    return false;
  }

}
