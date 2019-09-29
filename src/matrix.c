/**
 * Copyright @ 深圳市谷米万物科技有限公司. 2009-2019. All rights reserved.
 * File name:        matrix.c
 * Author:           王志华       
 * Version:          1.0
 * Date:             2019-09-07
 * Description:      矩阵运算
 * Others:           
 * Function List:    
    
 * History: 
    1. Date:         2019-09-07
       Author:       王志华
       Modification: 创建初始版本
    2. Date:          
       Author:         
       Modification: 

 */

#include <stdarg.h>
#include <math.h>
#include <gm_memory.h>
#include <gm_stdlib.h>
#include "matrix.h"
#include "log_service.h"

#define ASSERT_RETURN(c) \
if(!c)\
{\
	return;\
}\

static double doubleabs(double data) 
{
	if (data > 0) 
	{
		return data;
	}
	else
	{
		return -1.0*data;
	}
}


Matrix* matrix_create(int rows, int cols) 
{
	Matrix* p_m = (Matrix*)GM_MemoryAlloc(sizeof(Matrix));
	int i = 0;
	p_m->rows = rows;
	p_m->cols = cols;
	p_m->data = (double**) GM_MemoryAlloc(sizeof(double*) * p_m->rows);
	for (i = 0; i < p_m->rows; ++i) 
	{	
		int j = 0;
    	p_m->data[i] = (double*) GM_MemoryAlloc(sizeof(double) * p_m->cols);
    	for (j = 0; j < p_m->cols; ++j) 
		{
      		p_m->data[i][j] = 0.0;
    	}
  	}
  	return p_m;
}

void matrix_destroy(Matrix* p_m) 
{
	int i = 0;
	ASSERT_RETURN(p_m->data);
	for (i = 0; i < p_m->rows; ++i) 
	{
    	GM_MemoryFree(p_m->data[i]);
  	}
  	GM_MemoryFree(p_m->data);
	GM_MemoryFree(p_m);
}

void matrix_set(Matrix* p_m, ...) 
{
	int i = 0;
	va_list ap;
	va_start(ap, p_m);
  
	for (i = 0; i < p_m->rows; ++i) 
	{
		int j = 0;
		for (j = 0; j < p_m->cols; ++j) 
		{
     		p_m->data[i][j] = va_arg(ap, double);
    	}
	}
	va_end(ap);
}

void matrix_set_identity(Matrix* p_m) 
{
	int i = 0;
	ASSERT_RETURN (p_m->rows == p_m->cols);
	
	for (i = 0; i < p_m->rows; ++i) 
	{
		int j = 0;
	    for (j = 0; j < p_m->cols; ++j) 
		{
	      	if (i == j) 
			{
				p_m->data[i][j] = 1.0;
	      	} 
			else 
			{
				p_m->data[i][j] = 0.0;
	      	}
	    }
  	}
}

void matrix_copy(Matrix source, Matrix* p_destination)
{
	int i = 0;
	ASSERT_RETURN (source.rows == p_destination->rows);

	ASSERT_RETURN (source.cols ==p_destination->cols);
	
  	for (i = 0; i < source.rows; ++i) 
	{
		int j = 0;
    	for (j = 0; j < source.cols; ++j)
		{
      		p_destination->data[i][j] = source.data[i][j];
    	}
  	}
}

void matrix_print(Matrix m) 
{
	int i = 0;
	char buf[1024] = {0};
  	for (i = 0; i < m.rows; ++i) 
	{
		int j = 0;
    	for (j = 0; j < m.cols; ++j) 
		{
      		if (j > 0) 
			{
				GM_snprintf(buf + GM_strlen(buf),sizeof(buf) - GM_strlen(buf),"%s"," ");
      		}
			GM_snprintf(buf + GM_strlen(buf),sizeof(buf) - GM_strlen(buf),"%6.2f", m.data[i][j]);
    	}
    	LOG(DEBUG,buf);
  	}
}

void matrix_add(Matrix a, Matrix b, Matrix* p_c)
{
	int i = 0;

	ASSERT_RETURN(a.rows == b.rows);
	ASSERT_RETURN(a.rows == p_c->rows);
	ASSERT_RETURN(a.cols == b.cols);
	ASSERT_RETURN(a.cols == p_c->cols);
	for (i = 0; i < a.rows; ++i) 
	{
		int j = 0;
		for (j = 0; j < a.cols; ++j) 
		{
	  		p_c->data[i][j] = a.data[i][j] + b.data[i][j];
		}
	}
}

void matrix_subtract(Matrix a, Matrix b, Matrix* p_c) 
{
	int i = 0;
	ASSERT_RETURN(a.rows == b.rows);
	ASSERT_RETURN(a.rows == p_c->rows);
	ASSERT_RETURN(a.cols == b.cols);
	ASSERT_RETURN(a.cols == p_c->cols);
	for (i = 0; i < a.rows; ++i) 
	{
		int j = 0;
		for (j = 0; j < a.cols; ++j) 
		{
	      	p_c->data[i][j] = a.data[i][j] - b.data[i][j];
	    }
	}
}

void matrix_subtract_from_identity(Matrix* p_a) 
{
	int i = 0;

	ASSERT_RETURN(p_a->rows == p_a->cols);
	for (i = 0; i < p_a->rows; ++i)
	{
		int j = 0;
		for (j = 0; j < p_a->cols; ++j) 
		{
		  	if (i == j) 
		  	{
				p_a->data[i][j] = 1.0 - p_a->data[i][j];
		  	} 
			else 
			{
				p_a->data[i][j] = 0.0 - p_a->data[i][j];
		  	}
		}
	}
}

void matrix_multiply(Matrix a, Matrix b, Matrix* p_c)
{
	int i = 0;
	ASSERT_RETURN(a.cols == b.rows);
	ASSERT_RETURN(a.rows == p_c->rows);
	ASSERT_RETURN(b.cols == p_c->cols);
	for (i = 0; i < p_c->rows; ++i) 
	{
		int j = 0;
		for (j = 0; j < p_c->cols; ++j)
		{
			int k = 0;
			p_c->data[i][j] = 0.0;
			for (k = 0; k < a.cols; ++k) 
			{
				p_c->data[i][j] += a.data[i][k] * b.data[k][j];
			}
		}
	}
}

void matrix_multiply_by_transpose(Matrix a, Matrix b, Matrix* p_c) 
{
	int i = 0;

	ASSERT_RETURN(a.cols == b.cols);
	ASSERT_RETURN(a.rows == p_c->rows);
	ASSERT_RETURN(b.rows == p_c->cols);
	for (i = 0; i < p_c->rows; ++i) 
	{
		int j = 0;
		for (j = 0; j < p_c->cols; ++j) 
		{
			int k = 0;
			p_c->data[i][j] = 0.0;
			for (k = 0; k < a.cols; ++k) 
			{
				p_c->data[i][j] += a.data[i][k] * b.data[j][k];
			}
		}
	}
}

void matrix_transpose(Matrix input, Matrix* p_output) 
{
	int i = 0;

	ASSERT_RETURN(input.rows == p_output->cols);
	ASSERT_RETURN(input.cols == p_output->rows);
	for (i = 0; i < input.rows; ++i) 
	{
		int j = 0;
		for ( j = 0; j < input.cols; ++j) 
		{
			p_output->data[j][i] = input.data[i][j];
		}
	}
}

bool matrix_equal(Matrix a, Matrix b, double tolerance) 
{
	int i = 0;
  
	if (a.rows != b.rows)
	{
		return false;
	}
	if (a.cols != b.cols)
	{
		return false;
	}
	
	for (i = 0; i < a.rows; ++i) 
	{
		int j = 0;
		for (j = 0; j < a.cols; ++j) 
		{
			if (doubleabs(a.data[i][j] - b.data[i][j]) > tolerance) 
			{
				return false;
			}
		}
	}
	return true;
}

void matrix_scale(Matrix* p_m, double scalar) 
{
	int i = 0;
	for (i = 0; i < p_m->rows; ++i) 
	{
		int j = 0;
		for (j = 0; j < p_m->cols; ++j) 
		{
		 	p_m->data[i][j] *= scalar;
		}
	}
}

void matrix_swap_rows(Matrix* p_m, int r1, int r2) 
{
	double* tmp = p_m->data[r1];

	ASSERT_RETURN(r1 != r2);
	p_m->data[r1] = p_m->data[r2];
	p_m->data[r2] = tmp;
}

void matrix_scale_row(Matrix* p_m, int r, double scalar) 
{
	int i = 0;

	for (i = 0; i < p_m->cols; ++i) 
	{
		p_m->data[r][i] *= scalar;
	}
}

void matrix_shear_row(Matrix* p_m, int r1, int r2, double scalar) 
{
	int i = 0;

	ASSERT_RETURN(r1 != r2);
	for (i = 0; i < p_m->cols; ++i)
	{
		p_m->data[r1][i] += scalar * p_m->data[r2][i];
	}
}

bool matrix_destructive_invert(Matrix input, Matrix* p_output) 
{
	int i = 0;

	LOG(DEBUG,"matrix_destructive_invert:input rows=%d,cols=%d",input.rows,input.cols);
	LOG(DEBUG,"matrix_destructive_invert:output rows=%d,cols=%d",p_output->rows,p_output->cols);

	if(input.rows != input.cols)
	{
		return false;
	}
	
	if(input.rows != p_output->rows)
	{
		return false;
	}
	
	if(input.rows != p_output->cols)
	{
		return false;
	}
	

	matrix_set_identity(p_output);

	for (i = 0; i < input.rows; ++i) 
	{
		double scalar = 0;
		int j = 0;
		
		if (input.data[i][i] == 0.0)
		{
			int r = 0;
			for (r = i + 1; r < input.rows; ++r)
			{
				if (input.data[r][i] != 0.0) 
				{
				    break;
				}
			}
			if (r == input.rows)
			{
				return false;
			}
			matrix_swap_rows(&input, i, r);
			matrix_swap_rows(p_output, i, r);
		}

		scalar = 1.0 / input.data[i][i];
		matrix_scale_row(&input, i, scalar);
		matrix_scale_row(p_output, i, scalar);

		for (j = 0; j < input.rows; ++j) 
		{
			double shear_needed = -input.data[j][i];
			if (i == j) 
			{
				continue;
			}
			matrix_shear_row(&input, j, i, shear_needed);
			matrix_shear_row(p_output, j, i, shear_needed);
		}
	}
	return true;
}

