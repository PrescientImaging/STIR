//
//
/*!
  \file
  \ingroup IO
  \ingroup GE
  \brief Declaration of class stir::GE::RDF_HDF5::GEHDF5Wrapper

  \author Nikos Efthimiou
  \author Palak Wadhwa
  \author Ander Biguri
  \author Kris Thielemans

*/
/*
    Copyright (C) 2017-2019, University of Leeds
    Copyright (C) 2018 University of Hull
    Copyright (C) 2018-2021, 2024, University College London
    This file is part of STIR.

    SPDX-License-Identifier: Apache-2.0

    See STIR/LICENSE.txt for details
*/

#include "stir/IO/GEHDF5Wrapper.h"
#include "stir/IndexRange3D.h"
#include "stir/Radionuclide.h"
#include "stir/RadionuclideDB.h"
#include "stir/is_null_ptr.h"
#include "stir/info.h"
#include "stir/error.h"
#include <sstream>

START_NAMESPACE_STIR
namespace GE
{
namespace RDF_HDF5
{

std::uint32_t
GEHDF5Wrapper::read_dataset_uint32(const std::string& dataset_name)
{
  std::uint32_t tmp = 0U;
  H5::DataSet dataset = file.openDataSet(dataset_name);
  dataset.read(&tmp, H5::PredType::NATIVE_UINT32);
  return tmp;
}

std::int32_t
GEHDF5Wrapper::read_dataset_int32(const std::string& dataset_name)
{
  std::int32_t tmp = 0;
  H5::DataSet dataset = file.openDataSet(dataset_name);
  dataset.read(&tmp, H5::PredType::NATIVE_INT32);
  return tmp;
}

static float
read_float(const H5::H5File& file, const std::string& dataset)
{
  float tmp = 0.F;
  H5::DataSet ds = file.openDataSet(dataset.c_str());
  ds.read(&tmp, H5::PredType::NATIVE_FLOAT);
  return tmp;
}

static std::string
read_string(const H5::H5File& file, const std::string& dataset)
{
  H5::DataSet ds = file.openDataSet(dataset.c_str());
  H5::StrType datatype = ds.getStrType();

  std::string value;
  ds.read(value, datatype);
  return value;
}

bool
GEHDF5Wrapper::check_GE_signature(const std::string& filename)
{
  try
    {
      if (!H5::H5File::isHdf5(filename))
        return false;

      H5::H5File file;
      file.openFile(filename, H5F_ACC_RDONLY);

      return (check_GE_signature(file));
    }
  catch (...)
    {
      return false;
    }
}

bool
GEHDF5Wrapper::check_GE_signature(H5::H5File& file)
{
  if (file.getId() == -1)
    error("File is not open. Aborting");

  std::string read_str_manufacturer = read_string(file, "/HeaderData/ExamData/manufacturer");

  if (read_str_manufacturer == "GE MEDICAL SYSTEMS")
    {
      return true;
    }
  return false;
}

bool
GEHDF5Wrapper::is_list_file() const
{
  // have we already checked this? (note: initially set to false in check_file())
  if (is_list)
    return true;

  if (file.getId() == -1)
    error("File is not open. Aborting");

  // All RDF files shoudl have this DataSet
  H5::DataSet dataset = file.openDataSet("/HeaderData/RDFConfiguration/isListFile");
  std::uint32_t is_list_file;
  dataset.read(&is_list_file, H5::PredType::NATIVE_UINT32);
  return is_list_file;
}

// Checks if file is a sino file.
// AB todo: only valid for RDF9 (until they tell us otherwise)
bool
GEHDF5Wrapper::is_sino_file() const
{
  if (is_sino)
    return true;
  if (file.getId() == -1)
    error("File is not open. Aborting");

  // If this Group exists, its a sino file.
  // huh, no C++ way to do this without try catch. https://stackoverflow.com/questions/35668056/test-group-existence-in-hdf5-c
  return H5Lexists(file.getId(), "/SegmentData/Segment2", H5P_DEFAULT) > 0;
}
bool
GEHDF5Wrapper::is_geo_file() const
{
  // Apparently the norm file has the geo file inside. This means that the geo file is superfluous.
  // For now, lets just make this fucntion is_geo_or_norm_file(). Perhaps later versions will no be like this and this we need to
  // change this function.

  if (is_geo || is_norm)
    return true;
  if (file.getId() == -1)
    error("File is not open. Aborting");
  return H5Lexists(file.getId(), "/SegmentData/Segment4/3D_Norm_Correction/slice1", H5P_DEFAULT) > 0;
  // AB if you want to make sure geo is definetly geo, and not geo or norm, add to the avobe line: && !H5Lexists( file.getId(),
  // "/3DCrystalEfficiency/crystalEfficiency", H5P_DEFAULT );
}

bool
GEHDF5Wrapper::is_norm_file() const
{
  if (is_norm)
    return true;
  if (file.getId() == -1)
    error("File is not open. Aborting");

  return H5Lexists(file.getId(), "/3DCrystalEfficiency/crystalEfficiency", H5P_DEFAULT) > 0;
}
GEHDF5Wrapper::GEHDF5Wrapper()
{
  // Not much.
}

GEHDF5Wrapper::GEHDF5Wrapper(const std::string& filename)
{
  if (open(filename) == Succeeded::no)
    error("GEHDF5Wrapper: Error opening HDF5 file. Abort.");
}

unsigned int
GEHDF5Wrapper::check_geo_type()
{
  if (!is_geo)
    error("Not a geo file. Aborting");
  if (file.getId() == -1)
    error("File is not open. Aborting");

  unsigned int geo_type = 0;
  H5::DataSet str_geo_size = file.openDataSet("/HeaderData/Sorter/Segment4/dimension3Size");
  str_geo_size.read(&geo_type, H5::PredType::NATIVE_UINT32);
  if (geo_type > 1)
    geo_type = 3;
  else
    geo_type = 2;
  return geo_type;
}
// Function that error checks the input file and sets flags for the correct formats. GEHDF5Wrapper.file must be already open.
// AB todo: this file is only valid for RDF 9.
Succeeded
GEHDF5Wrapper::check_file()
{
  if (file.getId() == -1)
    error("File is not open. Aborting");
  if (!check_GE_signature(file))
    error("File is HDF5 but not GE data. Aborting");

  // AB: We are openign a new file. The same class should not be used twice, but lets make sure that if it happens, we reseted the
  // file identifiers.
  is_list = false;
  is_norm = false;
  is_geo = false;
  is_sino = false;

  // AB  Find out the RDF version of the file.
  H5::DataSet str_file_version = file.openDataSet("/HeaderData/RDFConfiguration/fileVersion/majorVersion");
  str_file_version.read(&rdf_ver, H5::PredType::NATIVE_UINT32);

  // AB Lets just error for now.
  if (rdf_ver != 9)
    error("Only RDF version 9 supported. Aborting");

  if (is_list_file())
    {
      is_list = true;
      // AB Now lets check all things that are required
      if (rdf_ver == 9) // AB todo: is this valid for 10?
        {
          // Check 1: Is the file compressed?
          unsigned int is_compressed;
          H5::DataSet str_file_version = file.openDataSet("/HeaderData/ListHeader/isListCompressed");
          str_file_version.read(&is_compressed, H5::PredType::NATIVE_UINT32);
          if (is_compressed)
            error("The RDF9 Listmode file is compressed, we won't be able to read it. Please uncompress it and retry. Aborting");
        }

      return Succeeded::yes;
    }
  if (is_sino_file())
    {
      is_sino = true;
      return Succeeded::yes;
    }
  if (is_norm_file())
    {
      is_norm = true;
      is_geo = true; // in RFD9, if its norm, it is also geo (it contains it)
      // Check the type of geo file it contains.
      geo_dims = check_geo_type();

      return Succeeded::yes;
    }
  if (is_geo_file())
    {
      is_geo = true;
      geo_dims = check_geo_type();

      return Succeeded::yes;
    }
  // should not get here.
  return Succeeded::no;
}

Succeeded
GEHDF5Wrapper::open(const std::string& filename)
{
  if (!file.isHdf5(filename))
    error("GEHDF5Wrapper: The input file is not HDF5! Abort.");

  file.openFile(filename, H5F_ACC_RDONLY);

  // AB: check if the input file is valid, not only as a HDF5, also a valid PET file.
  check_file();

  initialise_exam_info();
  initialise_proj_data_info_from_HDF5();

  // functions above will throw actually, so if we get here, it worked.
  return Succeeded::yes;
}

shared_ptr<Scanner>
GEHDF5Wrapper::get_scanner_from_HDF5()
{
  std::string read_str_scanner = read_string(this->file, "/HeaderData/ExamData/scannerDesc");
  float effective_ring_diameter;
  int num_transaxial_blocks_per_bucket = 0;
  int num_axial_blocks_per_bucket = 0;
  int axial_blocks_per_unit = 0;
  int radial_blocks_per_unit = 0;
  int axial_units_per_module = 0;
  int radial_units_per_module = 0;
  int axial_modules_per_system = 0;
  int radial_modules_per_system = 0;
  int max_num_non_arccorrected_bins = 0;
  int num_transaxial_crystals_per_block = 0;
  int num_axial_crystals_per_block = 0;
  float detector_axial_size = 0.0;
  float intrinsic_tilt = 0.0;
  int num_detector_layers = 1;

  H5::DataSet str_effective_ring_diameter = file.openDataSet("/HeaderData/SystemGeometry/effectiveRingDiameter");
  H5::DataSet str_axial_blocks_per_module = file.openDataSet("/HeaderData/SystemGeometry/axialBlocksPerModule");
  H5::DataSet str_radial_blocks_per_module = file.openDataSet("/HeaderData/SystemGeometry/radialBlocksPerModule");
  H5::DataSet str_axial_blocks_per_unit = file.openDataSet("/HeaderData/SystemGeometry/axialBlocksPerUnit");
  H5::DataSet str_radial_blocks_per_unit = file.openDataSet("/HeaderData/SystemGeometry/radialBlocksPerUnit");
  H5::DataSet str_axial_units_per_module = file.openDataSet("/HeaderData/SystemGeometry/axialUnitsPerModule");
  H5::DataSet str_radial_units_per_module = file.openDataSet("/HeaderData/SystemGeometry/radialUnitsPerModule");
  H5::DataSet str_axial_modules_per_system = file.openDataSet("/HeaderData/SystemGeometry/axialModulesPerSystem");
  H5::DataSet str_radial_modules_per_system = file.openDataSet("/HeaderData/SystemGeometry/radialModulesPerSystem");
  //! \todo P.W: Find the crystal gaps and other info missing.
  H5::DataSet str_detector_axial_size = file.openDataSet("/HeaderData/SystemGeometry/detectorAxialSize");
  H5::DataSet str_intrinsic_tilt = file.openDataSet("/HeaderData/SystemGeometry/transaxial_crystal_0_offset");

  H5::DataSet str_max_number_of_non_arc_corrected_bins;
  // TODO RDF10, what happens here?
  if (rdf_ver == 9)
    { // Bug in RDF9 makes this dimension2Size instead of the expected dimension1Size
      if (is_sino_file())
        str_max_number_of_non_arc_corrected_bins = file.openDataSet("/HeaderData/Sorter/dimension2Size");
      else
        str_max_number_of_non_arc_corrected_bins = file.openDataSet("/HeaderData/Sorter/dimension1Size");
    }
  H5::DataSet str_axial_crystals_per_block = file.openDataSet("/HeaderData/SystemGeometry/axialCrystalsPerBlock");
  H5::DataSet str_radial_crystals_per_block = file.openDataSet("/HeaderData/SystemGeometry/radialCrystalsPerBlock");
  // Convert to numbers.

  str_radial_blocks_per_module.read(&num_transaxial_blocks_per_bucket, H5::PredType::NATIVE_UINT32);
  str_axial_blocks_per_module.read(&num_axial_blocks_per_bucket, H5::PredType::NATIVE_UINT32);
  str_axial_blocks_per_unit.read(&axial_blocks_per_unit, H5::PredType::NATIVE_UINT32);
  str_radial_blocks_per_unit.read(&radial_blocks_per_unit, H5::PredType::NATIVE_UINT32);
  str_axial_units_per_module.read(&axial_units_per_module, H5::PredType::NATIVE_UINT32);
  str_radial_units_per_module.read(&radial_units_per_module, H5::PredType::NATIVE_UINT32);
  str_axial_modules_per_system.read(&axial_modules_per_system, H5::PredType::NATIVE_UINT32);
  str_radial_modules_per_system.read(&radial_modules_per_system, H5::PredType::NATIVE_UINT32);
  str_detector_axial_size.read(&detector_axial_size, H5::PredType::NATIVE_FLOAT);
  str_intrinsic_tilt.read(&intrinsic_tilt, H5::PredType::NATIVE_FLOAT);
  str_effective_ring_diameter.read(&effective_ring_diameter, H5::PredType::NATIVE_FLOAT);
  str_max_number_of_non_arc_corrected_bins.read(&max_num_non_arccorrected_bins, H5::PredType::NATIVE_UINT32);
  str_radial_crystals_per_block.read(&num_transaxial_crystals_per_block, H5::PredType::NATIVE_UINT32);
  str_axial_crystals_per_block.read(&num_axial_crystals_per_block, H5::PredType::NATIVE_UINT32);

  // TOF related
  const float timingResolutionInPico = read_float(file, "/HeaderData/SystemGeometry/timingResolutionInPico");
  const int posCoincidenceWindow = read_dataset_int32("/HeaderData/AcqParameters/EDCATParameters/posCoincidenceWindow");
  const int negCoincidenceWindow = read_dataset_int32("/HeaderData/AcqParameters/EDCATParameters/negCoincidenceWindow");
  const float coincTimingPrecisionInPico
      = read_float(file, "/HeaderData/AcqParameters/EDCATParameters/coincTimingPrecision") * 1000; // in nanoSecs in file
  const int num_tof_bins = posCoincidenceWindow + negCoincidenceWindow + 1;

  int num_rings = num_axial_blocks_per_bucket * num_axial_crystals_per_block * axial_modules_per_system;
  int num_detectors_per_ring = num_transaxial_blocks_per_bucket * num_transaxial_crystals_per_block * radial_modules_per_system;
  float ring_spacing = detector_axial_size / num_rings;
  // PW Bin Size, default num of arc corrected bins and inner ring radius not found in RDF header.
  //  They will be set from the default STIR values
  shared_ptr<Scanner> scanner_sptr(Scanner::get_scanner_from_name(read_str_scanner));
  if (is_null_ptr(scanner_sptr))
    error("Scanner read from RDF file is " + read_str_scanner + ", but this is not supported yet");

  scanner_sptr->set_num_detectors_per_ring(num_detectors_per_ring);
  scanner_sptr->set_num_rings(num_rings);
  if (!is_list_file())
    scanner_sptr->set_max_num_non_arccorrected_bins(max_num_non_arccorrected_bins);
  scanner_sptr->set_ring_spacing(ring_spacing);
  scanner_sptr->set_intrinsic_azimuthal_tilt(intrinsic_tilt * _PI / 180);
  scanner_sptr->set_num_axial_blocks_per_bucket(num_axial_blocks_per_bucket);
  scanner_sptr->set_num_transaxial_blocks_per_bucket(num_transaxial_blocks_per_bucket);
  scanner_sptr->set_num_axial_crystals_per_block(num_axial_crystals_per_block);
  scanner_sptr->set_num_transaxial_crystals_per_block(num_transaxial_crystals_per_block);
  scanner_sptr->set_num_detector_layers(num_detector_layers);
  scanner_sptr->set_reference_energy(511.F);

  if (fabs(scanner_sptr->get_effective_ring_radius() - effective_ring_diameter / 2) > .1F)
    {
      const float def_DOI = .0F;
      warning("GEHDF5Wrapper: default STIR effective ring radius is " + std::to_string(scanner_sptr->get_effective_ring_radius())
              + ", while RDF says " + std::to_string(effective_ring_diameter / 2)
              + "\nWill adjust scanner info to fit with the RDF file using default average DOI of " + std::to_string(def_DOI)
              + "mm");
      scanner_sptr->set_inner_ring_radius((effective_ring_diameter / 2) - def_DOI);
      scanner_sptr->set_average_depth_of_interaction(def_DOI);
    }
  if (timingResolutionInPico > 0 // Signa files seem to have zero in this field
      && (fabs(scanner_sptr->get_timing_resolution() - timingResolutionInPico) > .1F))
    {
      warning("GEHDF5Wrapper: default STIR timing resolution is " + std::to_string(scanner_sptr->get_timing_resolution())
              + ", while RDF says " + std::to_string(timingResolutionInPico)
              + "\nWill adjust scanner info to fit with the RDF file");
      scanner_sptr->set_timing_resolution(timingResolutionInPico);
    }
  if (fabs(scanner_sptr->get_size_of_timing_pos() - coincTimingPrecisionInPico) > .1F)
    {
      warning("GEHDF5Wrapper: default STIR size of (unmashed) TOF bins is "
              + std::to_string(scanner_sptr->get_size_of_timing_pos()) + ", while RDF says "
              + std::to_string(coincTimingPrecisionInPico) + "\nWill adjust scanner info to fit with the RDF file");
      scanner_sptr->set_size_of_timing_poss(coincTimingPrecisionInPico);
    }
  if (std::abs(scanner_sptr->get_max_num_timing_poss() - num_tof_bins) > 0)
    {
      warning("GEHDF5Wrapper: default STIR number of (unmashed) TOF bins is "
              + std::to_string(scanner_sptr->get_max_num_timing_poss()) + ", while RDF says " + std::to_string(num_tof_bins)
              + "\nWill adjust scanner info to fit with the RDF file");
      scanner_sptr->set_max_num_timing_poss(num_tof_bins);
    }
  if (scanner_sptr->get_default_bin_size() <= 0.F)
    {
      warning("GEHDF5Wrapper: default bin-size is not set. This will create trouble for FBP etc");
    }
  if (scanner_sptr->get_default_num_arccorrected_bins() <= 0)
    {
      warning("GEHDF5Wrapper: default num_arccorrected bins is not set. This will create trouble for FBP etc");
    }
  if (scanner_sptr->get_energy_resolution() <= 0)
    {
      warning("GEHDF5Wrapper: energy resolution is not set. This will create trouble for scatter estimation");
    }

  return scanner_sptr;
}

void
GEHDF5Wrapper::initialise_proj_data_info_from_HDF5()
{
  shared_ptr<Scanner> scanner_sptr = get_scanner_from_HDF5();

  // TODO get TOF mashing when reading sinos as TOF
  const auto num_tof_bins = read_dataset_uint32("/HeaderData/Sorter/numTOF_bins");
  if (num_tof_bins > 1)
    warning("GE RDF data currently still read as non-TOF");

  this->proj_data_info_sptr
      = ProjDataInfo::construct_proj_data_info(scanner_sptr,
                                               /*span*/ 2,
                                               /* max_delta*/ scanner_sptr->get_num_rings() - 1,
                                               /* num_views */ scanner_sptr->get_num_detectors_per_ring() / 2,
                                               /* num_tangential_poss */ scanner_sptr->get_max_num_non_arccorrected_bins(),
                                               /* arc_corrected */ false,
                                               this->is_list_file() ? 1 : 0 // TODO change when reading sinos as TOF
      );
  this->proj_data_info_sptr->set_bed_position_horizontal(
      this->read_dataset_int32("/HeaderData/AcqParameters/LandmarkParameters/absTableLongitude")
      / 10.F); /* units in RDF are 0.1 mm */
  // this->proj_data_info_sptr->set_gantry_tilt(this->read_dataset_uint32("/HeaderData/AcqParameters/LandmarkParameters/gantryTilt"));
  // /* units in RDF are 0.25 degrees, patient relative */
  this->proj_data_info_sptr->set_bed_position_vertical(
      this->read_dataset_int32("/HeaderData/AcqParameters/LandmarkParameters/tableElevation")
      / 10.F); /* units in RDF are 0.1 mm */
}

unsigned int
GEHDF5Wrapper::get_num_singles_samples()
{
  return m_num_singles_samples;
}

void
GEHDF5Wrapper::initialise_exam_info()
{
  this->exam_info_sptr.reset(new ExamInfo());
  this->exam_info_sptr->imaging_modality = ImagingModality(ImagingModality::PT);
  {
    const std::uint32_t patientEntry = read_dataset_uint32("/HeaderData/AcqParameters/LandmarkParameters/patientEntry");
    const std::uint32_t patientPosition = read_dataset_uint32("/HeaderData/AcqParameters/LandmarkParameters/patientPosition");
    PatientPosition::OrientationValue orientation;
    PatientPosition::RotationValue rotation;
    switch (patientEntry)
      {
      case AcqPatientEntries::ACQ_HEAD_FIRST:
        orientation = PatientPosition::OrientationValue::head_in;
        break;
      case AcqPatientEntries::ACQ_FEET_FIRST:
        orientation = PatientPosition::OrientationValue::feet_in;
        break;
      default:
        orientation = PatientPosition::OrientationValue::unknown_orientation;
      }
    switch (patientPosition)
      {
      case AcqPatientPositions::ACQ_SUPINE:
        rotation = PatientPosition::RotationValue::supine;
        break;
      case AcqPatientPositions::ACQ_PRONE:
        rotation = PatientPosition::RotationValue::prone;
        break;
      case AcqPatientPositions::ACQ_LEFT_DECUB:
        rotation = PatientPosition::RotationValue::left;
        break;
      case AcqPatientPositions::ACQ_RIGHT_DECUB:
        rotation = PatientPosition::RotationValue::right;
        break;
      default:
        rotation = PatientPosition::RotationValue::unknown_rotation;
        break;
      }
    exam_info_sptr->patient_position = PatientPosition(orientation, rotation);
  }
  // PW Get the high and low energy threshold values from HDF5 header.
  unsigned int low_energy_thres = 0;
  unsigned int high_energy_thres = 0;

  H5::DataSet str_low_energy_thres = file.openDataSet("/HeaderData/AcqParameters/EDCATParameters/lower_energy_limit");
  H5::DataSet str_high_energy_thres = file.openDataSet("/HeaderData/AcqParameters/EDCATParameters/upper_energy_limit");

  str_low_energy_thres.read(&low_energy_thres, H5::PredType::NATIVE_UINT32);
  str_high_energy_thres.read(&high_energy_thres, H5::PredType::NATIVE_UINT32);

  // PW Set these values in exam_info_sptr.
  exam_info_sptr->set_high_energy_thres(static_cast<float>(high_energy_thres));
  exam_info_sptr->set_low_energy_thres(static_cast<float>(low_energy_thres));

  // read time since 1970
  std::uint32_t scanStartTime = 0;
  H5::DataSet scanStartTime_dataset = file.openDataSet("/HeaderData/AcqStats/scanStartTime");
  scanStartTime_dataset.read(&scanStartTime, H5::PredType::NATIVE_UINT32);
  exam_info_sptr->start_time_in_secs_since_1970 = double(scanStartTime);

  // get time frame
  std::uint32_t frameStartTime = 0;
  std::uint32_t frameDuration = 0;
  H5::DataSet frameStartTime_dataset = file.openDataSet("/HeaderData/AcqStats/frameStartTime");
  H5::DataSet frameDuration_dataset = file.openDataSet("/HeaderData/AcqStats/frameDuration");

  frameStartTime_dataset.read(&frameStartTime, H5::PredType::NATIVE_UINT32);
  frameDuration_dataset.read(&frameDuration, H5::PredType::NATIVE_UINT32);
  const double frame_start_time = double(frameStartTime - scanStartTime);

  std::vector<std::pair<double, double>> tf{ { frame_start_time, frame_start_time + frameDuration / 1000 } };

  TimeFrameDefinitions tm(tf);
  exam_info_sptr->set_time_frame_definitions(tm);

  // radionuclide
  {
    auto rn_name_ds = file.openDataSet("/HeaderData/ExamData/radionuclideName");
    H5::StrType str_type(rn_name_ds);
    std::string rn_name;
    rn_name_ds.read(rn_name, str_type);
    RadionuclideDB radionuclide_db;
    Radionuclide radionuclide = radionuclide_db.get_radionuclide(exam_info_sptr->imaging_modality, rn_name);

    const float positron_fraction = read_float(file, "/HeaderData/ExamData/positronFraction");
    const float half_life = read_float(file, "/HeaderData/ExamData/halfLife");
    if (radionuclide.get_half_life(false) < 0)
      radionuclide = Radionuclide(rn_name, 511.F, positron_fraction, half_life, exam_info_sptr->imaging_modality);
    exam_info_sptr->set_radionuclide(radionuclide);
  }
}

Succeeded
GEHDF5Wrapper::initialise_listmode_data()
{
  if (!is_list_file())
    error("The file provided is not listmode. Aborting");

  if (rdf_ver == 9)
    {
      m_address = "/ListData/listData";
      // These values are not in the file are come from info shared by GE.
      m_size_of_record_signature = 6;
      m_max_size_of_record = 16;

      unsigned int num_time_slices = 0;
      H5::DataSet timeframe_dataspace = file.openDataSet("/HeaderData/SinglesHeader/numValidSamples");
      timeframe_dataspace.read(&num_time_slices, H5::PredType::NATIVE_UINT32);
      if (num_time_slices == 0)
        {
          error("Zero number of valid samples singles samples in data. Aborting");
        }

      m_num_singles_samples = num_time_slices;
    }
  else
    return Succeeded::no;

  m_dataset_sptr.reset(new H5::DataSet(file.openDataSet(m_address)));

  m_dataspace = m_dataset_sptr->getSpace();
  m_dataset_list_Ndims = m_dataspace.getSimpleExtentNdims();

  // We allocate dims_out in the stack for efficiecy and safety but we need an error check just in case then
  if (m_dataset_list_Ndims > m_max_dataset_dims)
    error("Dataset dimensions (" + std::to_string(m_dataset_list_Ndims) + ") bigger than maximum of"
          + std::to_string(m_max_dataset_dims) + ". This is unexpected, Aborting.");
  hsize_t dims_out[m_max_dataset_dims];

  m_dataspace.getSimpleExtentDims(dims_out, NULL);
  m_list_size = dims_out[0];

  return Succeeded::yes;
}

Succeeded
GEHDF5Wrapper::initialise_singles_data()
{

  if (!is_list_file() && !is_sino_file())
    error("The file provided is not listmode or sinogram data. Aborting");

  if (rdf_ver == 9)
    {
      m_address = "/Singles/CrystalSingles/sample";
      // Get the DataSpace (metadata) corresponding to the DataSet that we want to read
      m_dataset_sptr.reset(new H5::DataSet(file.openDataSet(m_address + "1")));
      m_dataspace = m_dataset_sptr->getSpace();
      // Create an array to host the size of the dimensions
      const int rank = m_dataspace.getSimpleExtentNdims();
      // We allocate dims in the stack for efficiecy and safety but we need an error check just in case then
      if (rank > m_max_dataset_dims)
        error("Dataset dimensions (" + std::to_string(rank) + ") bigger than maximum of" + std::to_string(m_max_dataset_dims)
              + ". This is unexpected, Aborting.");
      hsize_t dims[m_max_dataset_dims];
      // Read size of dimensions
      m_dataspace.getSimpleExtentDims(dims, NULL);

      m_NX_SUB = dims[0]; // hyperslab dimensions
      m_NY_SUB = dims[1];
      m_NZ_SUB = (rank == 2) ? 1 : dims[2]; // Signa has rank==2, but this stay shere just in case...

      unsigned int num_time_slices = 0;
      H5::DataSet timeframe_dataspace = file.openDataSet("/HeaderData/SinglesHeader/numValidSamples");
      timeframe_dataspace.read(&num_time_slices, H5::PredType::NATIVE_UINT32);
      if (num_time_slices == 0)
        {
          error("Zero number of valid samples singles samples in data. Aborting");
        }

      m_num_singles_samples = num_time_slices;

#if 0
        m_NX = dims[0];       // output buffer dimensions
        m_NY = dims[1];
        m_NZ = (rank==2)? 1 : dims[2];
#endif
    }
  else
    return Succeeded::no;

  return Succeeded::yes;
}

Succeeded
GEHDF5Wrapper::initialise_proj_data(const unsigned int view_num)
{
  if (!is_sino_file())
    error("The file provided is not sinogram data. Aborting");

  if (rdf_ver == 9)
    {
      // Is the file compressed?
      unsigned int is_compressed;
      H5::DataSet str_file_version = file.openDataSet("/HeaderData/Sorter/Segment2/compDataSegSize");
      str_file_version.read(&is_compressed, H5::PredType::NATIVE_UINT32);
      if (is_compressed)
        error("The RDF9 file sinogram is compressed, we won't be able to read it. Please uncompress it and retry. Aborting");
    }

  if (view_num == 0 || view_num > static_cast<unsigned>(this->get_scanner_sptr()->get_num_detectors_per_ring() / 2))
    error("internal error in GE HDF5 code: view number " + std::to_string(view_num) + " is incorrect");

  const auto num_tof_bins = read_dataset_uint32("/HeaderData/Sorter/numTOF_bins");

  if (rdf_ver == 9)
    {
      if (num_tof_bins > 1)
        {
          m_address = "/SegmentData/Segment2/3D_TOF_Sinogram/view" + std::to_string(view_num);
          m_dataset_sptr.reset(new H5::DataSet(file.openDataSet(m_address)));
        }
      else
        {
          m_address = "/SegmentData/Segment2/3D_Sinogram/view" + std::to_string(view_num);
          m_dataset_sptr.reset(new H5::DataSet(file.openDataSet(m_address)));
        }
      m_dataspace = m_dataset_sptr->getSpace();
      // Create an array to host the size of the dimensions
      const int rank = m_dataspace.getSimpleExtentNdims();
      // We allocate dims in the stack for efficiecy and safety but we need an error check just in case then
      if (rank > m_max_dataset_dims)
        error("Dataset dimensions (" + std::to_string(rank) + ") bigger than maximum of" + std::to_string(m_max_dataset_dims)
              + ". This is unexpected, Aborting.");
      hsize_t dims[m_max_dataset_dims];
      // Read size of dimensions
      m_dataspace.getSimpleExtentDims(dims, NULL);

      // AB for signa, these where [1981,27,357] and [45,448,357]
      m_NX_SUB = dims[0]; // hyperslab dimensions
      m_NY_SUB = dims[1];
      m_NZ_SUB = rank > 2 ? dims[2] : 1;
    }
  else
    return Succeeded::no;

  return Succeeded::yes;
}

// PW The geo factors are stored in geo3d file under the file path called /SegmentData/Segment4/3D_Norm_correction/slice%d where
// slice numbers go from 1 to 16. Here this path is initialised, along with the output buffer and hyperslab.
//
Succeeded
GEHDF5Wrapper::initialise_geo_factors_data(const unsigned int slice_num)
{
  if (!is_geo_file())
    error("The file provided is not geometry data. Aborting");

  if (slice_num == 0)
    error("internal error in GE HDF5 geo code: slice number " + std::to_string(slice_num) + " is incorrect");

  if (rdf_ver == 9)
    {
      m_address = "/SegmentData/Segment4/3D_Norm_Correction/slice";
      {
        // Open Dataset and get Dataspace(metadata)
        m_dataset_sptr.reset(new H5::DataSet(file.openDataSet(m_address + std::to_string(slice_num))));
        m_dataspace = m_dataset_sptr->getSpace();

        // Read dimensions
        const int rank = m_dataspace.getSimpleExtentNdims();
        // We allocate dims in the stack for efficiecy and safety but we need an error check just in case then
        if (rank > m_max_dataset_dims)
          error("Dataset dimensions (" + std::to_string(rank) + ") bigger than maximum of" + std::to_string(m_max_dataset_dims)
                + ". This is unexpected, Aborting.");
        hsize_t dims[m_max_dataset_dims];

        m_dataspace.getSimpleExtentDims(dims, NULL);

        m_NX_SUB = dims[0]; // hyperslab dimensions
        m_NY_SUB = dims[1];
        m_NZ_SUB = (rank == 2) ? 1 : dims[2]; // Signa has rank==2, but this stay shere just in case...
#if 0
            m_NX = dims[0];        // output buffer dimensions
            m_NY = dims[1];
            m_NZ = (rank==2)? 1 : dims[2]; // Signa has rank==2, but this stay shere just in case...
#endif
      }
    }
  else
    return Succeeded::no;

  return Succeeded::yes;
}
// This info is in norm3d file
Succeeded
GEHDF5Wrapper::initialise_efficiency_factors()
{
  if (!is_norm_file())
    error("The file provided is not norm data. Aborting");

  if (rdf_ver == 9)
    {

      m_address = "/3DCrystalEfficiency/crystalEfficiency";
      // Get the DataSpace (metadata) corresponding to the DataSet that we want to read
      m_dataset_sptr.reset(new H5::DataSet(file.openDataSet(m_address)));
      m_dataspace = m_dataset_sptr->getSpace();
      // Create an array to host the size of the dimensions
      const int rank = m_dataspace.getSimpleExtentNdims();
      // We allocate dims in the stack for efficiecy and safety but we need an error check just in case then
      if (rank > m_max_dataset_dims)
        error("Dataset dimensions (" + std::to_string(rank) + ") bigger than maximum of" + std::to_string(m_max_dataset_dims)
              + ". This is unexpected, Aborting.");
      hsize_t dims[m_max_dataset_dims];
      // Read size of dimensions
      m_dataspace.getSimpleExtentDims(dims, NULL);

      m_NX_SUB = dims[0]; // hyperslab dimensions
      // TODO: Why is this divided by 2??
      m_NY_SUB = dims[1] / 2; // should be equal to scanner_sptr->get_num_detectors_per_ring();
      m_NZ_SUB = (rank == 2) ? 1 : dims[2];
#if 0
        m_NX = dims[0];       // output buffer dimensions
        m_NY = dims[1]/2;  // should be equal to scanner_sptr->get_num_detectors_per_ring();
        m_NZ = (rank==2)? 1 : dims[2];
#endif
    }
  else
    return Succeeded::no;

  return Succeeded::yes;
}

// Developed for listmode access
Succeeded
GEHDF5Wrapper::read_list_data(char* output, const std::streampos offset, const hsize_t size) const
{
  if (!is_list_file())
    error("The file provided is not list data. Aborting");
  const hsize_t pos = static_cast<hsize_t>(offset);
  m_dataspace.selectHyperslab(H5S_SELECT_SET, &size, &pos);
  const H5::DataSpace memspace(m_dataset_list_Ndims, &size);
  m_dataset_sptr->read(output, H5::PredType::STD_U8LE, memspace, m_dataspace);

  return Succeeded::yes;
}

// Developed for ProjData
Succeeded
GEHDF5Wrapper::read_sinogram(Array<3, unsigned char>& output,
                             const std::array<hsize_t, 3>& offset,
                             const std::array<hsize_t, 3>& stride)
{
  // AB: this is only used for proj data, so for now lets ensure the file is sino. If its reused, we can change this.
  if (!is_sino_file())
    error("File is not sinogram. Aborting");

  if (offset[0] != 0 || offset[1] != 0
      || offset[2] != 0) // AB there are other C++ ways of doing this, but this is the most readable really.
    error("Only {0,0,0} offset supported. Aborting");
  if (stride[0] != 1 || stride[1] != 1 || stride[2] != 1)
    error("Only {1,1,1} stride supported. Aborting");

  // We know the size of the DataSpace
  hsize_t str_dimsf[3]{ m_NX_SUB, m_NY_SUB, m_NZ_SUB };

  // get a temporary buffer here,
  std::vector<unsigned char> aux_buffer(m_NX_SUB * m_NY_SUB * m_NZ_SUB);

  m_dataspace.selectHyperslab(H5S_SELECT_SET, str_dimsf, offset.data());
  H5::DataSpace memspace(3, str_dimsf);
  m_dataset_sptr->read(static_cast<void*>(aux_buffer.data()), H5::PredType::STD_U8LE, memspace, m_dataspace);

  // the data is not in the correct size if its RDF9, so we will need to transpose the output of the data read.
  if (rdf_ver == 9)
    {
      output.resize(IndexRange3D(m_NZ_SUB, m_NY_SUB, m_NX_SUB));
      // now transpose the output.
      // AB: todo
      // todo 1: test
      for (unsigned int i = 0; i < m_NZ_SUB; ++i)
        {
          unsigned int ioffset = i * m_NX_SUB * m_NY_SUB;
          for (unsigned int j = 0; j < m_NY_SUB; ++j)
            {
              unsigned int joffset = j * m_NX_SUB;
              for (unsigned int k = 0; k < m_NX_SUB; ++k)
                {
                  output[i][j][k] = aux_buffer[ioffset + joffset + k];
                }
            }
        }
    }

  return Succeeded::yes;
}

// PW Developed for Geometric Correction Factors
Succeeded
GEHDF5Wrapper::read_geometric_factors(Array<1, unsigned int>& output,
                                      const std::array<hsize_t, 2>& offset,
                                      const std::array<hsize_t, 2>& count,
                                      const std::array<hsize_t, 2>& stride)

{
  // AB: this is only used for geo data, so for now lets ensure the file is sino. If its reused, we can change this.
  if (!is_geo_file())
    error("File is Geometry. Aborting");

  if (count[0] == 0 || count[1] == 0) // AB there are other C++ ways of doing this, but this is the most readable really.
    error("Requested zero data to read. Aborting");
  if (stride[0] != 1 || stride[1] != 1)
    error("Only {1,1} stride supported. Aborting");

  Array<1, unsigned int> aux_reader;
  output.resize(count[0] * count[1]);
  aux_reader.resize(count[0] * count[1]);

  m_dataspace.selectHyperslab(H5S_SELECT_SET, count.data(), offset.data());
  H5::DataSpace memspace(2, count.data());
  m_dataset_sptr->read(aux_reader.get_data_ptr(), H5::PredType::NATIVE_UINT32, memspace, m_dataspace);
  aux_reader.release_data_ptr();

  // GE/RDF9 stores the tangetial axis reversed to STIR. Flip.
  for (unsigned int ax = 0; ax < count[0]; ++ax)
    for (unsigned int tan = 0; tan < count[1]; ++tan)
      output[ax * count[1] + ((count[1] - 1) - tan)] = aux_reader[ax * count[1] + tan];
  return Succeeded::yes;
}

// PW Developed for Efficiency Factors
Succeeded
GEHDF5Wrapper::read_efficiency_factors(Array<1, float>& output,
                                       const std::array<hsize_t, 2>& offset,
                                       const std::array<hsize_t, 2>& stride)

{
  if (!is_norm_file())
    error("The file provided is not norm data. Aborting");

  if (offset[0] != 0 || offset[1] != 0) // AB there are other C++ ways of doing this, but this is the most readable really.
    error("Only {0,0} offset supported. Aborting");
  if (stride[0] != 1 || stride[1] != 1)
    error("Only {1,1} stride supported. Aborting");

  // We know the size of the DataSpace
  hsize_t str_dimsf[2]{ m_NX_SUB, m_NY_SUB };
  Array<1, float> aux_reader;
  output.resize(m_NX_SUB * m_NY_SUB);
  aux_reader.resize(m_NX_SUB * m_NY_SUB);

  m_dataspace.selectHyperslab(H5S_SELECT_SET, str_dimsf, offset.data());
  H5::DataSpace memspace(2, str_dimsf);
  m_dataset_sptr->read(aux_reader.get_data_ptr(), H5::PredType::NATIVE_FLOAT, memspace, m_dataspace);
  aux_reader.release_data_ptr();

  // GE/RDF9 stores the tangetial axis reversed to STIR. Flip.
  for (unsigned int ax = 0; ax < m_NX_SUB; ++ax)
    for (unsigned int tan = 0; tan < m_NY_SUB; ++tan)
      output[ax * m_NY_SUB + ((m_NY_SUB - 1) - tan)] = aux_reader[ax * m_NY_SUB + tan];

  return Succeeded::yes;
}

// Developed for Singles
Succeeded
GEHDF5Wrapper::read_singles(Array<1, unsigned int>& output, const unsigned int current_id)
{
  BOOST_STATIC_ASSERT(sizeof(unsigned int) == 4); // Compilation time assert.
  if (!is_list_file() && !is_sino_file())
    error("The file provided is not listmode or sinogram data. Aborting");

  if (current_id == 0 || current_id > this->m_num_singles_samples)
    error("internal error in GE HDF5 code: singles slice_id " + std::to_string(current_id) + " is incorrect");

  Array<1, unsigned int> aux_reader;
  output.resize(m_NX_SUB * m_NY_SUB);
  aux_reader.resize(m_NX_SUB * m_NY_SUB);

  // AB: todo check if output allocated data size is correct.
  m_dataset_sptr.reset(new H5::DataSet(file.openDataSet(m_address + std::to_string(current_id))));
  m_dataset_sptr->read(aux_reader.get_data_ptr(), H5::PredType::NATIVE_UINT32);
  aux_reader.release_data_ptr();

  // GE/RDF9 stores the tangetial axis reversed to STIR. Flip.
  for (unsigned int ax = 0; ax < m_NX_SUB; ++ax)
    for (unsigned int tan = 0; tan < m_NY_SUB; ++tan)
      output[ax * m_NY_SUB + ((m_NY_SUB - 1) - tan)] = aux_reader[ax * m_NY_SUB + tan];

  return Succeeded::yes;
}

} // namespace RDF_HDF5
} // namespace GE
END_NAMESPACE_STIR
