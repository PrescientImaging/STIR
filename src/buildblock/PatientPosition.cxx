/*
    Copyright (C) 2004 - 2007-10-08, Hammersmith Imanet Ltd
    Copyright (C) 2013, University College London
    This file is part of STIR.

    SPDX-License-Identifier: Apache-2.0

    See STIR/LICENSE.txt for details
*/
/*!
  \file
  \ingroup buildblock

  \brief Implementations of class stir::PatientPosition

  \author Kris Thielemans
*/

#include "stir/PatientPosition.h"

START_NAMESPACE_STIR

PatientPosition::PatientPosition(PatientPosition::PositionValue position)
{
  switch (position)
    {
    case FFP:
      orientation = feet_in;
      rotation = prone;
      break;
    case HFP:
      orientation = head_in;
      rotation = prone;
      break;
    case FFS:
      orientation = feet_in;
      rotation = supine;
      break;
    case HFS:
      orientation = head_in;
      rotation = supine;
      break;
    case FFDR:
      orientation = feet_in;
      rotation = right;
      break;
    case HFDR:
      orientation = head_in;
      rotation = right;
      break;
    case FFDL:
      orientation = feet_in;
      rotation = left;
      break;
    case HFDL:
      orientation = head_in;
      rotation = left;
      break;
    case unknown_position:
      orientation = unknown_orientation;
      rotation = unknown_rotation;
      break;
    }
}

PatientPosition::PositionValue
PatientPosition::get_position() const
{
  // make use of order of enum's
  if (orientation <= feet_in && rotation <= left)
    {
      return static_cast<PositionValue>(orientation * 4 + rotation);
    }
  else
    {
      return unknown_position;
    }
}

const char* const
PatientPosition::get_position_as_string() const
{
  switch (this->get_position())
    {
    case HFP:
      return "HFP";
    case HFS:
      return "HFS";
    case HFDR:
      return "HFDR";
    case HFDL:
      return "HFDL";
    case FFDR:
      return "FFDR";
    case FFDL:
      return "FFDL";
    case FFP:
      return "FFP";
    case FFS:
      return "FFS";
    case unknown_position:
    default:
      return "unknown";
    }
}

END_NAMESPACE_STIR
