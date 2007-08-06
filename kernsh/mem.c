/*
** mem.c for kernsh
** 
** $Id: mem.c,v 1.5 2007-08-06 15:40:39 pouik Exp $
**
*/
#include "kernsh.h"
#include "libkernsh.h"

/* Display the sys_call_table */
int		cmd_sct()
{
  int		ret;
  int		index;
  listent_t     *actual;
  list_t	*h;
  libkernshsyscall_t *sct;
  char		buff[BUFSIZ];

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

#if defined(USE_READLN)
  rl_callback_handler_remove();
#endif

  XALLOC(__FILE__, 
	 __FUNCTION__, 
	 __LINE__,
	 h,
	 sizeof(list_t),
	 -1);

  list_init(h, "cmd_sct_list", ASPECT_TYPE_UNKNOW);

  ret = kernsh_sct(h);

  memset(buff, '\0', sizeof(buff));
  snprintf(buff, sizeof(buff), 
	   "%s\n\n",
	   revm_colorfieldstr("[+] SYS_CALL_TABLE"));
  revm_output(buff);

  for (index = 0, actual = h->head; 
       index < h->elmnbr; 
       index++, actual = actual->next)
    {
      sct = (libkernshsyscall_t *) actual->data;
      memset(buff, '\0', sizeof(buff));
      snprintf(buff, sizeof(buff),
	       "%s %-40s %s %s\n", 
	       revm_colornumber("id:%-10u", (unsigned int)index),
	       revm_colortypestr_fmt("%s", sct->name),
	       revm_colorstr("@"),
	       revm_coloraddress(XFMT, (elfsh_Addr) sct->addr));
      revm_output(buff);
      revm_endline();
      
    }
  
  revm_output("\n");

  list_destroy(h);

  
#if defined(USE_READLN)
  rl_callback_handler_install(vm_get_prompt(), vm_ln_handler);
  readln_column_update();
#endif

  if (ret)
    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
		      "Cannot get syscalltable", -1);

  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);
}

/* Display the idt */
int		cmd_idt()
{
  int		ret;
  int		index;
  listent_t     *actual;
  list_t	*h;
  libkernshint_t *dint;
  char		buff[BUFSIZ];

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

#if defined(USE_READLN)
  rl_callback_handler_remove();
#endif

  XALLOC(__FILE__, 
	 __FUNCTION__, 
	 __LINE__,
	 h,
	 sizeof(list_t),
	 -1);

  list_init(h, "cmd_idt_list", ASPECT_TYPE_UNKNOW);

  ret = kernsh_idt(h);
        
  memset(buff, '\0', sizeof(buff));
  snprintf(buff, sizeof(buff), 
	   "%s\n\n",
	   revm_colorfieldstr("[+] IDT"));
  revm_output(buff);
  
  for (index = 0, actual = h->head; 
       index < h->elmnbr; 
       index++, actual = actual->next)
    {
      dint = (libkernshint_t *) actual->data;

      snprintf(buff, sizeof(buff),
	       "%s %-40s %s %s\n",
	       revm_colornumber("id:%-10u", (unsigned int)index),
	       revm_colortypestr_fmt("%s", dint->name),
	       revm_colorstr("@"),
	       revm_coloraddress(XFMT, (elfsh_Addr) dint->addr));
      revm_output(buff);
      revm_endline();
    }

  revm_output("\n");
  list_destroy(h);

#if defined(USE_READLN)
  rl_callback_handler_install(vm_get_prompt(), vm_ln_handler);
  readln_column_update();
#endif

  if (ret)
    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
		      "Cannot get idt", -1);

  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);
}

/* Display the gdt */
int		cmd_gdt()
{
  int		ret;
  int		index;
  int		i;
  listent_t     *actual;
  list_t	*h;
  libkernshsgdt_t *sgdt;
  char		buff[BUFSIZ];

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

#if defined(USE_READLN)
  rl_callback_handler_remove();
#endif

  XALLOC(__FILE__, 
	 __FUNCTION__, 
	 __LINE__,
	 h,
	 sizeof(list_t),
	 -1);

  list_init(h, "cmd_gdt_list", ASPECT_TYPE_UNKNOW);

  ret = kernsh_gdt(h);
        
  memset(buff, '\0', sizeof(buff));
  snprintf(buff, sizeof(buff), 
	   "%s\n\n",
	   revm_colorfieldstr("[+] GDT"));
  revm_output(buff);
  
  for (i = 0, index = 0, actual = h->head; 
       index < h->elmnbr; 
       i+=8, index++, actual = actual->next)
    {
      sgdt = (libkernshsgdt_t *) actual->data;
      
      snprintf(buff, sizeof(buff),
	       "%s%s %s %s\n",
	       revm_coloraddress("%.8lX", (elfsh_Addr) sgdt->deb),
	       revm_coloraddress("%.8lX", (elfsh_Addr) sgdt->fin),
	       revm_colorstr("@"),
	       revm_coloraddress(XFMT, (elfsh_Addr) sgdt->addr));

      revm_output(buff);
      
      revm_endline();
    } 
	     
  revm_output("\n");

  list_destroy(h);

#if defined(USE_READLN)
  rl_callback_handler_install(vm_get_prompt(), vm_ln_handler);
  readln_column_update();
#endif

  if (ret)
    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
		      "Cannot get gdt", -1);

  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);
}

/* Manipulate memory device */
int		cmd_mem()
{
  char          *param, *param2;
  char		buff[256];
  unsigned long	addr;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

#if defined(USE_READLN)
  rl_callback_handler_remove();
#endif

  param = world.curjob->curcmd->param[0];
  param2 = world.curjob->curcmd->param[1];

  if (param)
    {
      if (param2 && !strcmp(param, "alloc"))
	{
	  if (kernsh_alloc_contiguous(atoi(param2), &addr))
	    {
	      revm_setvar_int("_", -1);
	      PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
			   "Cannot alloc contiguous memory", -1);
	    }
	  memset(buff, '\0', sizeof(buff));
	  snprintf(buff, sizeof(buff), 
		   "%s %s %s %s %s %s\n\n",
		   revm_colorfieldstr("ALLOCATE"),
		   revm_colornumber("%u", (unsigned int)atoi(param2)),
		   revm_colorfieldstr("octet(s)"),
		   revm_colorfieldstr("OF CONTIGUOUS MEMORY"),
		   revm_colorstr("@"),
		   revm_coloraddress(XFMT, (elfsh_Addr) addr));
	  revm_output(buff);
	  revm_setvar_long("_", addr);
	}
      else if (param2 && !strcmp(param, "free"))
	{
	  addr = strtoul( param2, NULL, 16 );
	  if(kernsh_free_contiguous(addr))
	    {
	      revm_setvar_int("_", -1);
	      PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
			   "Cannot free contiguous memory", -1);
	    }
	  memset(buff, '\0', sizeof(buff));
	  snprintf(buff, sizeof(buff), 
		   "%s %s %s %s\n\n",
		   revm_colorfieldstr("FREE"),
		   revm_colorfieldstr("CONTIGUOUS MEMORY"),
		   revm_colorstr("@"),
		   revm_coloraddress(XFMT, (elfsh_Addr) addr));
	  revm_output(buff);
	  revm_setvar_int("_", 0);
	}
      else if (param2 && !strcmp(param, "alloc_nc"))
	{
	  if(kernsh_alloc_noncontiguous(atoi(param2), &addr))
	    {
	      revm_setvar_int("_", -1);
	      PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
			   "Cannot get alloc non contiguous memory", -1);
	    }
	  memset(buff, '\0', sizeof(buff));
	  snprintf(buff, sizeof(buff), 
		   "%s %s %s %s %s %s\n\n",
		   revm_colorfieldstr("ALLOCATE"),
		   revm_colornumber("%u", (unsigned int)atoi(param2)),
		   revm_colorfieldstr("octet(s)"),
		   revm_colorfieldstr("OF NON CONTIGUOUS MEMORY"),
		   revm_colorstr("@"),
		   revm_coloraddress(XFMT, (elfsh_Addr) addr));
	  revm_output(buff);
	  revm_setvar_long("_", addr);
	}
      else if (param2 && !strcmp(param, "free_nc"))
	{
	  addr = strtoul( param2, NULL, 16 );
	  if(kernsh_free_noncontiguous(addr))
	    {
	      revm_setvar_int("_", -1);
	      PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
			   "Cannot get free non contiguous memory", -1);
	    }
	  memset(buff, '\0', sizeof(buff));
	  snprintf(buff, sizeof(buff), 
		   "%s %s %s %s\n\n",
		   revm_colorfieldstr("FREE"),
		   revm_colorfieldstr("NONCONTIGUOUS MEMORY"),
		   revm_colorstr("@"),
		   revm_coloraddress(XFMT, (elfsh_Addr) addr));
	  revm_output(buff);
	  revm_setvar_int("_", 0);
	}
      else if (param2 && !strcmp(param, "sym"))
	{
	  if(kernsh_get_addr_by_name(param2, &addr, strlen(param2)))
	    {
	      revm_setvar_int("_", -1);
	      PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
			   "Cannot get symbol", -1);
	    }
	  memset(buff, '\0', sizeof(buff));
	  snprintf(buff, sizeof(buff), 
		   "%s %s %s %s %s\n\n",
		   revm_colorfieldstr("SYMBOL"),
		   revm_colorstr(param2),
		   revm_colorfieldstr("is"),
		   revm_colorstr("@"),
		   revm_coloraddress(XFMT, (elfsh_Addr) addr));
	  revm_output(buff);
	  revm_setvar_long("_", addr);
	}
      else 
	{

	  revm_setvar_int("_", -1);
	}
    }

#if defined(USE_READLN)
  rl_callback_handler_install(vm_get_prompt(), vm_ln_handler);
  readln_column_update();
#endif

  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);
}
