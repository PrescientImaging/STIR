//
//
/*
    Copyright (C) 2000 PARAPET partners
    Copyright (C) 2000- 2007-10-08, Hammersmith Imanet Ltd
    Copyright (C) 2011-07-01 - 2011, Kris Thielemans
    This file is part of STIR.

    SPDX-License-Identifier: Apache-2.0 AND License-ref-PARAPET-license
    See STIR/LICENSE.txt for details
*/
/*!
  \file
  \ingroup Coordinate
  \brief inline implementations for the stir::Coordinate3D class

  \author Sanida Mustafovic
  \author Kris Thielemans
  \author PARAPET project


*/

START_NAMESPACE_STIR

template <class coordT>
Coordinate3D<coordT>::Coordinate3D()
    : base_type()
{}

template <class coordT>
Coordinate3D<coordT>::Coordinate3D(const coordT& c1, const coordT& c2, const coordT& c3)
    : base_type()
{
  (*this)[1] = c1;
  (*this)[2] = c2;
  (*this)[3] = c3;
}

template <class coordT>
Coordinate3D<coordT>::Coordinate3D(const BasicCoordinate<3, coordT>& c)
    : base_type(c)
{}

template <class coordT>
Coordinate3D<coordT>&
Coordinate3D<coordT>::operator=(const BasicCoordinate<3, coordT>& c)
{
  base_type::operator=(c);
  return *this;
}

END_NAMESPACE_STIR
