//
//
/*!
  \file
  \ingroup normalisation

  \brief Declaration of class stir::ChainedBinNormalisation

  \author Kris Thielemans
*/
/*
    Copyright (C) 2003- 2005, Hammersmith Imanet Ltd
    This file is part of STIR.

    SPDX-License-Identifier: Apache-2.0

    See STIR/LICENSE.txt for details
*/

#ifndef __stir_recon_buildblock_ChainedBinNormalisation_H__
#define __stir_recon_buildblock_ChainedBinNormalisation_H__

#include "stir/recon_buildblock/BinNormalisation.h"
#include "stir/RegisteredParsingObject.h"

START_NAMESPACE_STIR

/*!
  \ingroup normalisation
  \brief A BinNormalisation class that simply multiplies the factors given by
  2 BinNormalisation objects.

  This is especially useful to combine the 'usual' normalisation factors and attenuation factors
  in PET. As both are multiplicative corrections, they both belong in the BinNormalisation
  hierarchy.

  \par Parsing details
  \verbatim
  Chained Bin Normalisation Parameters:=
  ; type of one of the bin normalisations, followed by its parameters
  Bin Normalisation to apply first := <ASCII>

  ; type of the other, and its parameters
  Bin Normalisation to apply second := <ASCII>
  END Chained Bin Normalisation Parameters :=
  \endverbatim
  \par Example
  This example shows how to construct the parameter file for the case that there
  are normalisation factors in a file \a norm.hs and an attenuation image in a file
  \a atten.hv.
  \see BinNormalisationFromProjData, BinNormalisationFromAttenuationImage.

  \verbatim
  Bin Normalisation type := Chained
  Chained Bin Normalisation Parameters:=
    Bin Normalisation to apply first := from projdata
      Bin Normalisation From ProjData :=
        normalisation projdata filename:= norm.hs
      End Bin Normalisation From ProjData:=
    Bin Normalisation to apply second := From Attenuation Image
      Bin Normalisation From Attenuation Image:=
        attenuation_image_filename := atten.hv
        forward projector type := ray tracing
          Forward Projector Using Ray Tracing Parameters :=
          End Forward Projector Using Ray Tracing Parameters :=
      End Bin Normalisation From Attenuation Image :=
  END Chained Bin Normalisation Parameters :=
  \endverbatim
*/
class ChainedBinNormalisation : public RegisteredParsingObject<ChainedBinNormalisation, BinNormalisation>
{
private:
  using base_type = BinNormalisation;

public:
  //! Name which will be used when parsing a BinNormalisation object
  static const char* const registered_name;

  //! Default constructor
  /*!
    \warning You should not call any member functions for any object just
    constructed with this constructor. Initialise the object properly first
    by parsing.
  */
  ChainedBinNormalisation();

  ChainedBinNormalisation(shared_ptr<BinNormalisation> const& apply_first, shared_ptr<BinNormalisation> const& apply_second);

  //! Checks if we can handle certain projection data.
  /*! Calls set_up for the BinNormalisation members. */
  Succeeded set_up(const shared_ptr<const ExamInfo>& exam_info_sptr, const shared_ptr<const ProjDataInfo>&) override;

  // import all apply/undo methods from base-class (we'll override some below)
  using base_type::apply;
  using base_type::undo;

  //! Normalise some data
  /*!
    This calls apply() of the 2 BinNormalisation members
  */
  void apply(RelatedViewgrams<float>& viewgrams) const override;
#if 0
  virtual void apply(ProjData&) const override;
#endif

  virtual void apply_only_first(RelatedViewgrams<float>& viewgrams) const;

  virtual void apply_only_first(ProjData&) const;

  virtual void apply_only_second(RelatedViewgrams<float>& viewgrams) const;

  virtual void apply_only_second(ProjData&) const;

  //! Undo the normalisation of some data
  /*!
    This calls undo() of the 2 BinNormalisation members.
  */
  void undo(RelatedViewgrams<float>& viewgrams) const override;
#if 0
  virtual void undo(ProjData&) const override;
#endif

  virtual void undo_only_first(RelatedViewgrams<float>& viewgrams) const;

  virtual void undo_only_first(ProjData&) const;

  virtual void undo_only_second(RelatedViewgrams<float>& viewgrams) const;

  virtual void undo_only_second(ProjData&) const;

  float get_bin_efficiency(const Bin& bin) const override;

  //! Returns the is_trivial() status of the first normalisation object.
  //! \warning Currently, if the object has not been set the function throws an error.
  virtual bool is_first_trivial() const;
  //! Returns the is_trivial() status of the second normalisation object.
  //! \warning Currently, if the object has not been set the function throws an error.
  virtual bool is_second_trivial() const;

  //! returns if the object can only handle TOF data
  /*!
    Checks if either of the normalisation objects can only handle TOF.
  */
  virtual bool is_TOF_only_norm() const override;

  virtual shared_ptr<BinNormalisation> get_first_norm() const;

  virtual shared_ptr<BinNormalisation> get_second_norm() const;

private:
  shared_ptr<BinNormalisation> apply_first;
  shared_ptr<BinNormalisation> apply_second;
  // parsing stuff
  void set_defaults() override;
  void initialise_keymap() override;
  bool post_processing() override;
};

END_NAMESPACE_STIR

#endif
