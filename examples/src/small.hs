!INTERFILE  :=
name of data file := small.s
!imaging modality := PET
originating system := RATPET
; optional: values: PET|nucmed *defaults to PET for backwards compatibility)
imaging modality:=PET
; optional patient position keywords (defaulting to "unknown")
; orientation: allowed values: head_in|feet_in|other|unknown
patient orientation := head_in
; rotation: allowed values: prone|supine|other|unknown
patient rotation :=  supine
!GENERAL DATA :=
!GENERAL IMAGE DATA :=
!type of data := PET
imagedata byte order := LITTLEENDIAN
!PET STUDY (General) :=
!PET data type := Emission
applied corrections := {arc correction}
!number format := float
!number of bytes per pixel := 4
number of dimensions := 4
matrix axis label [4] := segment
!matrix size [4] := 3
matrix axis label [3] := view
!matrix size [3] := 28
matrix axis label [2] := axial coordinate
!matrix size [2] := { 11,15,11}
matrix axis label [1] := tangential coordinate
!matrix size [1] := 56
minimum ring difference per segment := { -4,-1,2}
maximum ring difference per segment := { -2,1,4}
number of rings := 8
number of detectors per ring := 112
inner ring diameter (cm) := 11.5
distance between rings (cm) := 0.625
default bin size (cm) := 0.165
number of blocks_per_bucket in axial direction := 1
number of crystals_per_block in axial direction := 8
number of crystals_per_block in transaxial direction := 7
number of detector layers := 1
image scaling factor[1] := 1
data offset in bytes[1] := 0
number of time frames := 1
!END OF INTERFILE :=
