!INTERFILE  :=
; sample Interfile header, created with create_projdata_template for the GE Signa PET/MR
; with span=2 and full ring difference (which is how GE compresses their sinograms)
; (and some edits for clarification)
; Check PET_Interfile_header.hs for more information and some other keywords

!imaging modality := PT
name of data file := PET_TOF_Interfile_header_Signa_PETMR.s
originating system := GE Signa PET/MR
!version of keys := STIR6.0
!GENERAL DATA :=
!GENERAL IMAGE DATA :=
!type of data := PET
imagedata byte order := LITTLEENDIAN
!PET STUDY (General) :=
!PET data type := Emission
applied corrections := {None}
!number format := float
!number of bytes per pixel := 4
number of dimensions := 5
matrix axis label [5] := timing positions
!matrix size [5] := 27
matrix axis label [4] := segment
!matrix size [4] := 45
matrix axis label [3] := view
!matrix size [3] := 224
matrix axis label [2] := axial coordinate
!matrix size [2] := { 1,5,9,13,17,21,25,29,33,37,41,45,49,53,57,61,65,69,73,77,81,85,89,85,81,77,73,69,65,61,57,53,49,45,41,37,33,29,25,21,17,13,9,5,1}
matrix axis label [1] := tangential coordinate
!matrix size [1] := 357
TOF mashing factor := 13
minimum ring difference per segment := { -44,-43,-41,-39,-37,-35,-33,-31,-29,-27,-25,-23,-21,-19,-17,-15,-13,-11,-9,-7,-5,-3,-1,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44}
maximum ring difference per segment := { -44,-42,-40,-38,-36,-34,-32,-30,-28,-26,-24,-22,-20,-18,-16,-14,-12,-10,-8,-6,-4,-2,1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31,33,35,37,39,41,43,44}
Scanner parameters:=
  Scanner type := GE Signa PET/MR
  Number of rings                          := 45
  Number of detectors per ring             := 448
  Inner ring diameter (cm)                 := 62.36
  Average depth of interaction (cm)        := 0.85
  Distance between rings (cm)              := 0.556
  Default bin size (cm)                    := 0.201565
  View offset (degrees)                    := -5.23
  Maximum number of non-arc-corrected bins := 357
  Default number of arc-corrected bins     := 331
  Energy resolution := 0.105
  Reference energy (in keV) := 511
  Maximum number of (unmashed) TOF time bins := 351
  Size of unmashed TOF time bins (ps)        := 6.84615
  TOF timing resolution (ps)                 := 390
  Number of blocks per bucket in transaxial direction         := 4
  Number of blocks per bucket in axial direction              := 5
  Number of crystals per block in axial direction             := 9
  Number of crystals per block in transaxial direction        := 4
  Number of detector layers                                   := 1
  Number of crystals per singles unit in axial direction      := 1
  Number of crystals per singles unit in transaxial direction := 1
End scanner parameters:=

number of time frames := 1
start vertical bed position (mm) := 0
start horizontal bed position (mm) := 0
!END OF INTERFILE :=
