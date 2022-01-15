module types
  use, intrinsic :: iso_fortran_env
  implicit none

  ! use the Fortran 2008 intrinsic constants to define real kinds
  integer, parameter :: sp = REAL32
  integer, parameter :: dp = REAL64
  integer, parameter :: qp = REAL128

end module types
