//
// $Id$
//
/*!
  \file
  \ingroup utilities
  \brief   

  \author Charalampos Tsoumpas
  \author Kris Thielemans
  
  $Date$
  $Revision$

  \par Usage:
   \code
   find_fwhm_in_image filename [num_maxima][level][dimension][nema]
   \endcode
   \param num_maxima defaults to 1
     
*/
/*
    Copyright (C) 2004- $Date$, Hammersmith Imanet
    See STIR/LICENSE.txt for details
*/
#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>  
#include <string>
#include "stir/shared_ptr.h" // nedded?
#include "stir/DiscretisedDensity.h" // nedded?
#include "stir/DiscretisedDensityOnCartesianGrid.h" // nedded?
#include "stir/ProjDataInfoCylindricalNoArcCorr.h" 
#include "local/stir/scatter.h"
#include "stir/ProjDataInfo.h"
#include "stir/ProjDataInterfile.h"
#include "stir/ProjData.h"
#ifndef STIR_NO_NAMESPACES
using std::endl;
using std::cout;
using std::cerr;
using std::setw;
#endif

/***********************************************************/     
int main(int argc, char *argv[])                                  
{         
  USING_NAMESPACE_STIR
  using namespace std;          //   output_proj_data_filename                                                       
  if (argc< 2 || argc>6)
  {
    cerr << "Usage:" << argv[0] << " input_image transmission_image"
		 <<	"proj_data_filename [attenuation_threshold]"
		 <<	"[maximum_scatter_points][maximum_LoRs]\n"
	     << "\tattenuation_threshold defaults to 1000\n" 
         << "\tmaximum_scatter_points defaults to 100\n"  
         << "\tmaximum_LoRs defaults to 1000\n"  
         << "returns a sinogram file with the single scatter contribution\n\n";		
    return EXIT_FAILURE;            
  } 
  const float attenuation_threshold = argc>=5 ? atoi(argv[4]) : 1000 ;  
  int max_scat_points = argc>=6 ? atoi(argv[5]) : 1000 , 
      maximum_LoRs = argc>=7 ? atoi(argv[6]) : 1000 ;  
  
  shared_ptr< DiscretisedDensity<3,float> >  
	  activity_image_sptr= 
  DiscretisedDensity<3,float>::read_from_file(argv[1]), 
      density_image_sptr= 
  DiscretisedDensity<3,float>::read_from_file(argv[2]);
  
  shared_ptr<ProjData> template_proj_data_sptr = ProjData::read_from_file(argv[3]);  
  const ProjDataInfoCylindricalNoArcCorr* proj_data_info_ptr =
	  dynamic_cast<ProjDataInfoCylindricalNoArcCorr const *>(
	    template_proj_data_sptr->get_proj_data_info_ptr());

  if (proj_data_info_ptr==0 || density_image_sptr==0 || activity_image_sptr==0)
	  error("Check the input files\n");
  
  
  const DiscretisedDensityOnCartesianGrid<3,float>& activity_image = 
    dynamic_cast<const DiscretisedDensityOnCartesianGrid<3,float>&  > 
	(*activity_image_sptr.get());

  const DiscretisedDensityOnCartesianGrid<3,float>& density_image = 
    dynamic_cast<const DiscretisedDensityOnCartesianGrid<3,float>&  > 
	(*density_image_sptr.get());
 
    string output_proj_data_filename;
    string input_string(argv[1]);
    

/*    string:: iterator string_iter;
    for(string_iter=input_string.begin(); 
        string_iter!=input_string.end() && *string_iter!='.' ;
        ++string_iter)  
    output_proj_data_filename.push_back(*string_iter); 
	if (argc>=6)
	{ */
		string max_scat_points_string(argv[5]);             
		output_proj_data_filename =  input_string + '_' +  max_scat_points_string ;	
/*	}
    else
    output_proj_data_filename +=  '_' + "100" ;  
*/
  ProjDataInterfile output_proj_data(proj_data_info_ptr->clone(),
		                             output_proj_data_filename);
  scatter_viewgram(output_proj_data,
	  activity_image, density_image,
	  max_scat_points,attenuation_threshold);        
   
  return EXIT_SUCCESS;
}                 
