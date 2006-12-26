/*
** vectors.c for libaspect in elfsh
**
** Implement the modularity for the framework
**
** Started Dec 22 2006 02:57:03 mayhem
**
*/
#include "libaspect.h"

/* The hash tables of vectors */
hash_t	vector_hash;


/* Initialize the vector hash */
void		aspect_vectors_init()
{
  hash_init(&vector_hash, 11);
}

/* Project each dimension and write the desired function pointer */
void		aspect_project_vectdim(vector_t *vect, unsigned int *dim, 
				       unsigned int dimsz, unsigned long fct)
{
  unsigned long	*tmp;
  unsigned int	idx;

  tmp = vect->hook;
  for (idx = 0; idx < dimsz; idx++)
    {
      tmp += dim[idx];
      if (idx + 1 < dimsz)
	tmp  = (unsigned long *) *tmp;
    }
  *tmp = (unsigned long) fct;
}


/* Project each dimension and get the requested function pointer */
void*			aspect_project_coords(vector_t *vect, 
					     unsigned int *dim, 
					     unsigned int dimsz)
{
  unsigned long		*tmp;
  unsigned int		idx;

  tmp = vect->hook;
  for (idx = 0; idx < dimsz; idx++)
    {
      tmp += dim[idx];
      tmp  = (unsigned long *) *tmp;
    }
  return (tmp);
}


/* Allocate recursively the hook array */
int		aspect_recursive_vectalloc(unsigned long *tab, unsigned int *dims, 
					  unsigned int depth, unsigned int dimsz)
{
  unsigned int		idx;

  if (depth == dimsz)
    return (0);
  for (idx = 0; idx < dims[depth - 1]; idx++)
    {
      tab[idx] = (unsigned long) elfsh_calloc(dims[depth] * sizeof(unsigned long), 
					      1);
      if (tab[idx] == (unsigned long) NULL) 
	{
	  write(1, "Out of memory\n", 14);
	  return (-1);
	}
      aspect_recursive_vectalloc((unsigned long *) tab[idx], dims, 
				depth + 1, dimsz);
    }
  return (0);
}


/* Register a new vector. A vector is an multidimentional array of hooks */
int		aspect_register_vector(char		*name, 
				      void		*registerfunc, 
				      void		*defaultfunc,
				      unsigned int	*dimensions, 
				      unsigned int	dimsz)
{
  vector_t	*vector;
  unsigned long	*ptr;

  if (!registerfunc || !defaultfunc || !dimsz || !dimensions)
    {
      write(1, "Invalid NULL parameters\n", 24);
      return (-1);
    }
  vector = calloc(sizeof(vector_t), 1);
  if (vector == NULL)
    return (-1);
  ptr = calloc(dimensions[0] * sizeof(unsigned long), 1);
  if (!ptr)
    return (-1);
  vector->hook = ptr;
  if (dimsz > 1)
    aspect_recursive_vectalloc((unsigned long *) vector->hook, 
			       dimensions, 1, dimsz);
  vector->arraysz       = dimsz;
  vector->arraydims     = dimensions;
  vector->register_func = registerfunc;
  vector->default_func  = defaultfunc;
  hash_add(&vector_hash, name, vector);
  return (0);
}
