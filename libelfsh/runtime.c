/**
 * @file runtime.c
 * @ingroup libelfsh
** runtime.c for libelfsh
** 
** 
** This file contains runtime routines used for memory mapping
** in the debugger
** 
** Started Sun 05 May 2005 22:29:54 mm
** 
**
** $Id: runtime.c,v 1.9 2008-02-16 12:32:27 thor Exp $
**
*/
#include "libelfsh.h"


/**
 * @brief Map a new area in memory 
 * @param segment
 * @return
 */
eresi_Addr	 elfsh_runtime_map(elfsh_Phdr *segment)
{
  eresi_Addr	addr;
  int		prot = 0;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  if (elfsh_segment_is_readable(segment))
    prot |= PROT_READ;
  if (elfsh_segment_is_writable(segment))
    prot |= PROT_WRITE;
  if (elfsh_segment_is_executable(segment))
    prot |= PROT_EXEC;

#if	__DEBUG_RUNTIME__
  printf("[DEBUG_RUNTIME] MMAP: " XFMT " of %d bytes \n", segment->p_vaddr, segment->p_memsz);
#endif

#if defined(KERNSH)

  if (elfsh_is_runtime_mode())
    {
      if (kernsh_alloc(segment->p_memsz, ((unsigned long *)&addr)) == -1)
	{
	  PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
		       "Cannot alloc memory", -1);
	}

      PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, (addr));
    }

#endif

#if defined(IRIX)
{
 int	zero;

 zero = fopen("/dev/zero", O_RDWR);
 if (zero < 0)
   PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
		      "Failed to open /dev/zero", 
		      ELFSH_INVALID_ADDR);

 addr = (eresi_Addr) mmap((void *) 0, segment->p_memsz, 
			   prot,
			   //elfsh_get_segment_flags(segment), 
			   MAP_PRIVATE, 
			   zero, 0);
 close(zero);
}
#else  
  addr = (eresi_Addr) mmap((void *) 0, segment->p_memsz, 
			   prot,
			   MAP_PRIVATE | MAP_ANONYMOUS, 
			   0, 0);
#endif


  if (addr == 0 && segment->p_vaddr)
    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
		      "Mmap refused the mapping", 
		      ELFSH_INVALID_ADDR);

#if	__DEBUG_RUNTIME__
  else
    printf("[DEBUG_RUNTIME] Section Mapped at addr %08X (%u) with prot %c%c%c\n", 
	   addr, segment->p_memsz,
	   (elfsh_segment_is_readable(segment)   ? 'R' : '-'),
	   (elfsh_segment_is_writable(segment)   ? 'W' : '-'),
	   (elfsh_segment_is_executable(segment) ? 'X' : '-'));
#endif

  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, (addr));
}


/**
 * @brief Unmap a previously requested area 
 * @param segment
 * @return
 */
int		elfsh_runtime_unmap(elfsh_Phdr *segment)
{
  int		ret;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);
  
  ret = munmap((void *) segment->p_vaddr, segment->p_memsz);

  //if (!ret)
  //memset(segment, 0x00, sizeof(elfsh_Phdr));

  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, ret);
}

/**
 * Remap an existing zone with a bigger size 
 * @param segment
 * @param moresize
 * @return
 */
int		elfsh_runtime_remap(elfsh_Phdr *segment, uint32_t moresize)
{
  eresi_Addr	addr;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);
  
  elfsh_runtime_unmap(segment);
  segment->p_memsz += moresize;
  addr = elfsh_runtime_map(segment);
  if (!addr)
    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__,
                      "Could not extend mmaped memory",
                      ELFSH_INVALID_ADDR);
  
  segment->p_vaddr = segment->p_paddr = addr;
  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, addr);
}

/**
 * @brief Synchronize ondisk modifications in memory
 * @return Always returns 0
 */
int		elfsh_runtime_sync()
{
  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);
  

  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);
}

/**
 * @brief Put write capability on the zone 
 * @param file 
 * @param addr
 * @param sz
 * @return
 */
int		elfsh_munprotect(elfshobj_t *file, 
				 eresi_Addr addr, 
				 uint32_t sz)
{
  elfshsect_t	*sect;
  elfsh_Phdr	*phdr;
  int		retval;
  int		prot;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);
#if defined(KERNSH)

  if (elfsh_is_runtime_mode())
    {
      PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);
    }

#endif

  if (!elfsh_is_runtime_mode())
    PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);

  sect = elfsh_get_parent_section(file, addr, NULL);
  if (!sect)
    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
			"Cannot find parent section", -1);

  phdr = elfsh_get_parent_segment(file, sect);
  prot = 0;

  if (elfsh_segment_is_readable(phdr))
    prot |= PROT_READ;
  if (elfsh_segment_is_writable(phdr))
    prot |= PROT_WRITE;
  if (elfsh_segment_is_executable(phdr))
    prot |= PROT_EXEC;

  retval = mprotect((void *) (addr - addr % getpagesize()), 
		    getpagesize(), PROT_READ | PROT_WRITE);

  if (retval != 0)
    {
      perror("mprotect");
      PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
			"Failed mprotect", -1);
    }
  /* Return the original rights */
  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 
		     prot);
}

/**
 * @brief Restore original rights 
 * @param addr
 * @param sz
 * @param prot
 * @return
 */
int		elfsh_mprotect(eresi_Addr addr, uint32_t sz, int prot)
{
  int		retval;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);
#if defined(KERNSH)
  
  if (elfsh_is_runtime_mode())
    {
      PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);
    }

#endif
  if (!elfsh_is_runtime_mode())
    PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);

  retval = mprotect((void *) (long) addr - (long) addr % getpagesize(), 
		    getpagesize(), prot);

  if (retval != 0)
      PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
			"Failed munprotect", -1);

  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, retval);
}
