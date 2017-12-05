#ifndef VIENNACL_DEVICE_SPECIFIC_BUILTIN_DATABASE_DEVICES_GPU_AMD_NORTHERN_ISLANDS_DEVASTATOR_HPP_
#define VIENNACL_DEVICE_SPECIFIC_BUILTIN_DATABASE_DEVICES_GPU_AMD_NORTHERN_ISLANDS_DEVASTATOR_HPP_

/* =========================================================================
   Copyright (c) 2010-2016, Institute for Microelectronics,
                            Institute for Analysis and Scientific Computing,
                            TU Wien.
   Portions of this software are copyright by UChicago Argonne, LLC.

                            -----------------
                  ViennaCL - The Vienna Computing Library
                            -----------------

   Project Head:    Karl Rupp                   rupp@iue.tuwien.ac.at

   (A list of authors and contributors can be found in the manual)

   License:         MIT (X11), see file LICENSE in the base directory
============================================================================= */

#include "device_specific/templates/matrix_product_template.hpp"

#include "device_specific/forwards.h"
#include "device_specific/builtin_database/common.hpp"

namespace viennacl{
namespace device_specific{
namespace builtin_database{
namespace devices{
namespace gpu{
namespace amd{
namespace northern_islands{
namespace devastator{

inline void add_4B(database_type<matrix_product_template::parameters_type> & db, char_to_type<'T'>, char_to_type<'T'>)
{
  db.add_4B(amd_id, CL_DEVICE_TYPE_GPU, ocl::northern_islands, "Devastator", matrix_product_template::parameters_type(1,8,16,8,2,1,2,FETCH_FROM_LOCAL,FETCH_FROM_LOCAL,8,8));
}

inline void add_4B(database_type<matrix_product_template::parameters_type> & db, char_to_type<'T'>, char_to_type<'N'>)
{
  db.add_4B(amd_id, CL_DEVICE_TYPE_GPU, ocl::northern_islands, "Devastator", matrix_product_template::parameters_type(1,16,16,8,2,1,2,FETCH_FROM_LOCAL,FETCH_FROM_LOCAL,16,8));
}

inline void add_4B(database_type<matrix_product_template::parameters_type> & db, char_to_type<'N'>, char_to_type<'T'>)
{
  db.add_4B(amd_id, CL_DEVICE_TYPE_GPU, ocl::northern_islands, "Devastator", matrix_product_template::parameters_type(2,64,16,4,2,1,2,FETCH_FROM_GLOBAL_CONTIGUOUS,FETCH_FROM_GLOBAL_CONTIGUOUS,0,0));
}

inline void add_4B(database_type<matrix_product_template::parameters_type> & db, char_to_type<'N'>, char_to_type<'N'>)
{
  db.add_4B(amd_id, CL_DEVICE_TYPE_GPU, ocl::northern_islands, "Devastator", matrix_product_template::parameters_type(1,16,16,8,1,2,2,FETCH_FROM_LOCAL,FETCH_FROM_LOCAL,8,16));
}


}
}
}
}
}
}
}
}
#endif
