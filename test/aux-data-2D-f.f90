

!****************************************************************************
!****************************************************************************
!****************************************************************************

      subroutine SET_DATA ( arr, rank, dims, hwidth )

        implicit none
        include 'ADCL.inc'

        integer rank, dims(2), hwidth
        DATATYPE arr(dims(1),dims(2))

        integer i, j

        do i=1,hwidth
           do j = 1, dims(2)
              arr(i,j) = -1
           end do
        end do

        do i = dims(1)-hwidth+1, dims(1)
           do j = 1, dims(2)
              arr(i, j) = -1
           end do
        end do

        do i=1,dims(1)
           do j = 1, hwidth
              arr(i,j) = -1
           end do
        end do

        do i = 1, dims(1)
           do j = dims(2)-hwidth+1, dims(2)
              arr(i, j) = -1
           end do
        end do


        do i = hwidth+1, dims(1)-hwidth
           do j = hwidth+1, dims(2)-hwidth
              arr(i,j) = rank
           end do
        end do

        return
      end subroutine SET_DATA 

!****************************************************************************
!****************************************************************************
!****************************************************************************
      subroutine DUMP_VECTOR ( arr, rank, dims )

        implicit none
        include 'ADCL.inc'

        integer rank, dims(2)
        DATATYPE arr(dims(1), dims(2))
        integer i, j

        do j = 1, dims(1)
           write (*,*) rank, (arr(j,i), i=1,dims(2))
        end do
        return
      end subroutine DUMP_VECTOR


!****************************************************************************
!****************************************************************************
!****************************************************************************
      subroutine DUMP_VECTOR_MPI ( arr, dims, comm )

        implicit none
        include 'ADCL.inc'

        integer dims(2), comm
        DATATYPE arr(dims(1), dims(2))
        integer i, j, iproc, rank, size, ierror

        call MPI_Comm_rank ( comm, rank, ierror )
        call MPI_Comm_size ( comm, size, ierror )

        do iproc = 0, size-1
           if ( iproc .eq. rank) then
              do j = 1, dims(1)
                 write (*,*) rank, (arr(j,i), i=1,dims(2))
              end do
           end if 
           call MPI_Barrier ( comm, ierror )
        end do
        return
      end subroutine DUMP_VECTOR_MPI


!****************************************************************************
!****************************************************************************
!****************************************************************************

      subroutine CHECK_DATA ( arr, rank, dims, hwidth, neighbors ) 

        implicit none
        include 'ADCL.inc'

        integer rank, dims(2), hwidth, neighbors(4)
        DATATYPE arr(dims(1),dims(2))
        integer lres, gres, i, j, ierr
        DATATYPE should_be
        
        lres = 1
      
        if ( neighbors(1) .eq. MPI_PROC_NULL ) then 
           should_be = -1
        else
           should_be = neighbors(1)
        endif
        do i=1,hwidth
           do j = hwidth+1, dims(2)-hwidth
              if ( arr(i,j) .ne. should_be ) then
                 lres = 0 
              endif
           end do
        end do

        if ( neighbors(2) .eq. MPI_PROC_NULL ) then 
           should_be = -1
        else
           should_be = neighbors(2)
        endif
        do i = dims(1)-hwidth+1, dims(1)
           do j = hwidth+1, dims(2)-hwidth
              if ( arr(i, j) .ne. should_be ) then 
                 lres = 0
              endif
           end do
        end do


        if ( neighbors(3) .eq. MPI_PROC_NULL ) then 
           should_be = -1
        else
           should_be = neighbors(3)
        endif
        do i=hwidth+1,dims(1)-hwidth
           do j = 1, hwidth
              if ( arr(i,j) .ne. should_be ) then
                 lres = 0
              endif
           end do
        end do

        if ( neighbors(4) .eq. MPI_PROC_NULL ) then 
           should_be = -1
        else
           should_be = neighbors(4)
        endif
        do i = hwidth+1, dims(1)-hwidth
           do j = dims(2)-hwidth+1, dims(2)
              if ( arr(i, j) .ne. should_be ) then 
                 lres = 0
              endif
           end do
        end do


        do i = hwidth+1, dims(1)-hwidth
           do j = hwidth+1, dims(2)-hwidth
              if ( arr(i,j) .ne. rank ) then 
                 lres = 0
              endif
           end do
        end do



        call MPI_Allreduce ( lres, gres, 1, MPI_INTEGER, MPI_MIN, MPI_COMM_WORLD, ierr)
        if ( gres .eq. 0 ) then
           call DUMP_VECTOR ( arr, rank, dims )
           if ( rank .eq. 0 ) then
              write (*,*) '2-D Fortran testsuite hwidth =', hwidth, &
                   'nc = ', 0, ' failed'  
           end if
        else
           if ( rank .eq. 0 ) then
              write (*,*) '2-D Fortran testsuite hwidth =', hwidth, &
                   'nc = ', 0, ' passed'  
           end if
        end if


        return
      end subroutine CHECK_DATA


